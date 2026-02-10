/*
 * Copyright (c) 2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <board_def.h>
#include <lib/mmio.h>
#include <standby.h>

#define MAIN_PSC_BASE		0x00400000
#define MAIN_PSC_MDCTL_BASE	0x00400A00
#define MAIN_PSC_MDSTAT_BASE	0x00400800
#define MAIN_PSC_PDCTL_BASE	0x00400300
#define MAIN_PSC_PDSTAT_BASE	0x00400200
#define MAIN_PSC_PTSTAT	(MAIN_PSC_BASE + PSC_PTSTAT)
#define MAIN_PSC_PTCMD		(MAIN_PSC_BASE + PSC_PTCMD)

#define PSC_PTCMD	0x120
#define PSC_PTSTAT	0x128

#define PSC_TIMEOUT_US  100000  /* 100ms timeout */

#define PSC_STATE_MASK 0x1U
#define LPSC_STATE_MASK 0x1fU
#define PLL_HSDIV_MASK 0xffU
#define PLL_CLK_EN_MASK 0x8000U
#define PLL_BYP_EN_MASK 0x80000000U

#define MAIN_PLL_MMR_CFG_BASE	(0x04060000UL)
#define WKUP_PLL_MMR_CFG_BASE   (0x04040000UL)

#define MAIN_PLL0_HSDIVx(x)	MAIN_PLL_MMR_CFG_BASE + 0x80 + (0x4 * x)
#define MAIN_PLL8_BASE MAIN_PLL_MMR_CFG_BASE + 0x1000 * 8
#define MAIN_PLL8_CTRL MAIN_PLL8_BASE + 0x20
#define MAIN_PLL17_BASE MAIN_PLL_MMR_CFG_BASE + 0x1000 * 17
#define MAIN_PLL17_CTRL MAIN_PLL17_BASE + 0x20
#define WKUP_MAIN_PLL0_HSDIVx(x)	WKUP_PLL_MMR_CFG_BASE + 0x80 + (0x4 * x)

#define PLL_COUNT_LOW_LAT_STBY 11

#define LPSC_ADDR(lpsc_id) MAIN_PSC_MDSTAT_BASE + (4 * lpsc_id)
#define PSC_ADDR(psc_id) MAIN_PSC_PDSTAT_BASE + (4 * psc_id)

#define EMIF_CTLCFG_DENALI_CTL_168 0x0F3082A0
#define EMIF_CTLCFG_DENALI_CTL_169 0x0F3082A4
#define EMIF_CTLCFG_DENALI_CTL_167 0x0F30829C

/* Values for DDR CTL CFG Registers */
#define DDR_AUTO_SR_CTRL_EN 0x07070007 /* Enables auto entry and exit from self-refresh */
#define DDR_AUTO_CLKGATE_EN 0x7 /* Enables memory clock gating during low power states */
#define DDR_PD_TIMEOUT 0x0000FF00 /* Timeout value for entry into power-down low power states */
#define DDR_SRSTATE_TIMOUT 0x0F0F00FF /* Timeout value for entry into self-refresh low power states */

#define DDR_CTL_COUNT_LOW_LAT_STBY 3

#define EN_AUTO_CLKGATE 0U
#define WKUP_CTRL_MMR_CFG5_CLKGATE_CTRL0 0x43054050

#define LPSC_COUNT_LOW_LAT_STBY 5

/*
 * @brief Structure to store power domain and corresponding lpsc
 * \param pd_id Power domain index of LPSC
 * \param lpsc_id LPSC index value
 */
struct pd_lpsc_id {
    uint8_t pd_id;
    uint8_t lpsc_id;
};

static const struct pd_lpsc_id stby_pd_lpsc_table[] = {
	{ .pd_id = 0, .lpsc_id = 1 },  /* LPSC_main_gp_test */
	{ .pd_id = 0, .lpsc_id = 2 },  /* LPSC_main_gp_pbist0 */
	{ .pd_id = 3, .lpsc_id = 33 }, /* LPSC_mainip_pbist */
	{ .pd_id = 4, .lpsc_id = 39 }, /* LPSC_main_mpu_clst0_pbist */
	{ .pd_id = 9, .lpsc_id = 55 }, /* LPSC_debugss */
};

struct ddr_ctrl_reg {
	uint32_t addr;
	uint32_t val;
};

static const struct ddr_ctrl_reg stby_ddr_ctrl_regs[] = {
	{ .addr = EMIF_CTLCFG_DENALI_CTL_168, .val = DDR_AUTO_CLKGATE_EN | DDR_PD_TIMEOUT },
	{ .addr = EMIF_CTLCFG_DENALI_CTL_169, .val = DDR_SRSTATE_TIMOUT },
	{ .addr = EMIF_CTLCFG_DENALI_CTL_167, .val = DDR_AUTO_SR_CTRL_EN },
};

struct am62l_pm_state {
    uint32_t pll_hsdiv_val[PLL_COUNT_LOW_LAT_STBY];
    uint32_t lpsc_value[LPSC_COUNT_LOW_LAT_STBY];
    uint32_t ddr_reg[DDR_CTL_COUNT_LOW_LAT_STBY];
    uint32_t auto_clk_gate;
};

static volatile struct am62l_pm_state saved_state;

/*
 *	state_entered - to track the state of individual cores
 *	Value : 0 - PSCI_LOCAL_STATE_RUN
 *			1 - CORE IDLE_STATE
 *			2 - LOW_LATENCY IDLE_STATE
 *			3 - HIGH_LATENCY IDLE_STATE
 */
static uint32_t state_entered[PLATFORM_CORE_COUNT] = {PSCI_LOCAL_STATE_RUN};

u_register_t am62l_standby_scr_reg;

void set_main_psc_state(uint32_t pd_id, uint32_t md_id, uint32_t pd_state, uint32_t md_state)
{
	uintptr_t mdctrl_ptr, mdstat_ptr, pdctrl_ptr, pdstat_ptr;
	volatile uint32_t mdctrl, mdstat, pdctrl, pdstat, psc_ptstat, psc_ptcmd;
	uint64_t tick_start, timeout_ticks;
	uint32_t ticks_per_us;

	// Calculate addresses with simplified approach
	mdctrl_ptr = MAIN_PSC_MDCTL_BASE + (4 * md_id);
	mdstat_ptr = MAIN_PSC_MDSTAT_BASE + (4 * md_id);
	pdctrl_ptr = MAIN_PSC_PDCTL_BASE + (4 * pd_id);
	pdstat_ptr = MAIN_PSC_PDSTAT_BASE + (4 * pd_id);

	// Use mmio_read_32 with simplified addresses
	mdctrl = mmio_read_32(mdctrl_ptr);
	mdstat = mmio_read_32(mdstat_ptr);
	pdctrl = mmio_read_32(pdctrl_ptr);
	pdstat = mmio_read_32(pdstat_ptr);

	VERBOSE("%s: before: md_id=%d, mdstat=0x%x, pdstat=0x%x\n", __func__, md_id, mdstat, pdstat);
	if (((pdstat & PSC_STATE_MASK) == pd_state) && ((mdstat & LPSC_STATE_MASK) == md_state))
		return;

	// Calculate timeout parameters
	ticks_per_us = plat_get_syscnt_freq2() / 1000000;
	tick_start = (uint32_t)read_cntpct_el0();
	timeout_ticks = PSC_TIMEOUT_US * ticks_per_us;

	// wait for GOSTAT to clear
	psc_ptstat = mmio_read_32(MAIN_PSC_PTSTAT);

	while ((psc_ptstat & BIT_32(pd_id)) != 0) {
		if (((uint32_t)read_cntpct_el0() - tick_start) > timeout_ticks) {
			ERROR("PSC timeout waiting for initial GOSTAT to clear for md_id %d and pd_id %d\n",
			      md_id ,pd_id);
			break;
		}
		psc_ptstat = mmio_read_32(MAIN_PSC_PTSTAT);
	}

	// Set PDCTL NEXT to new state
	mmio_write_32(pdctrl_ptr, (pdctrl & ~(PSC_STATE_MASK)) | pd_state);
	// Set MDCTL NEXT to new state
	mmio_write_32(mdctrl_ptr, (mdctrl & ~(LPSC_STATE_MASK)) | md_state);
	// Start power transition by setting PTCMD Go to 1
	psc_ptcmd = mmio_read_32(MAIN_PSC_PTCMD);
	psc_ptcmd |= BIT_32(pd_id);
	mmio_write_32(MAIN_PSC_PTCMD, psc_ptcmd);
	// return early in case powering off
	// This prevents the core from timing out waiting for GOSTAT to clear
	if (md_state == PSC_SYNCRESETDISABLE)
		return;

	// Reset timeout for second wait
	tick_start = (uint32_t)read_cntpct_el0();

	// Initial read
	psc_ptstat = mmio_read_32(MAIN_PSC_PTSTAT);

	// Wait loop with timeout
	while ((psc_ptstat & BIT_32(pd_id)) != 0) {
		if (((uint32_t)read_cntpct_el0() - tick_start) > timeout_ticks) {
			ERROR("PSC timeout waiting for GOSTAT to clear for md_id %d and pd_id %d\n",md_id ,pd_id);
			break;
		}
		psc_ptstat = mmio_read_32(MAIN_PSC_PTSTAT);
	}

	//check states
	mdstat = mmio_read_32(mdstat_ptr);
	pdstat = mmio_read_32(pdstat_ptr);
	VERBOSE("%s: after: md_id=%d, mdstat=0x%x, pdstat=0x%x\n", __func__, md_id, mdstat, pdstat);
}

/*
 * Save the current hardware state before entering standby mode.
 * Saves PLL configurations, LPSC states, DDR controller registers,
 * and clock gating settings.
 */
void am62l_standby_save_state()
{
    /* Save PLL values */
    for (int i = 0; i < PLL_COUNT_LOW_LAT_STBY - 1; i++) {
        saved_state.pll_hsdiv_val[i] = mmio_read_32(MAIN_PLL0_HSDIVx(i));
    }
    saved_state.pll_hsdiv_val[PLL_COUNT_LOW_LAT_STBY - 1] = mmio_read_32(MAIN_PLL8_CTRL);

    /* Save LPSC values */
    for (int i = 0; i < LPSC_COUNT_LOW_LAT_STBY; i++) {
        saved_state.lpsc_value[i] = mmio_read_32(LPSC_ADDR(stby_pd_lpsc_table[i].lpsc_id)) & LPSC_STATE_MASK;
    }

	/* Save EMIF LP control registers */
	for (int i = 0; i < DDR_CTL_COUNT_LOW_LAT_STBY; i++) {
		saved_state.ddr_reg[i] = mmio_read_32(stby_ddr_ctrl_regs[i].addr);
	}

	/* Save AUTO CLOCK GATING state */
    saved_state.auto_clk_gate = mmio_read_32(WKUP_CTRL_MMR_CFG5_CLKGATE_CTRL0);
}

/*
 * Configure the hardware for low-latency standby mode.
 * Scales down PLLs, disables unused peripherals, enables DDR self-refresh,
 * and configures automatic clock gating to minimize power consumption
 * while maintaining fast resume capability.
 */
void am62l_low_latency_standby_sequence()
{
	/* Change the LPSC values only if they are not already turned off */
	for (int i = 0; i < LPSC_COUNT_LOW_LAT_STBY; i++) {
		if (saved_state.lpsc_value[i] != PSC_SYNCRESETDISABLE) {
			set_main_psc_state(stby_pd_lpsc_table[i].pd_id, stby_pd_lpsc_table[i].lpsc_id, PSC_PD_ON, PSC_DISABLE);
		}
	}
	/*
	 * Low latency standby PLL scaling (ref: AM62L_OS_Idle_Setting_v1.1_AVV.xlsx) -
	 * PLL0_HSDIVOUT0 : 125Mhz, hsdiv = 15,
	 * PLL0_HSDIVOUT5 : 200Mhz, hsdiv = 4,
	 * PLL0_HSDIVOUT6 : 250Mhz, hsdiv = 3,
	 * PLL0_HSDIVOUT7 : 166Mhz, hsdiv = 5,
	 * PLL0_HSDIVOUT8 : 25Mhz,  hsdiv = 39,
	 * PLL0_HSDIVOUT9 : DISABLED,
	 * PLL8_HSDIVOUT0 : BYPASS,
	 */
	mmio_write_32(MAIN_PLL0_HSDIVx(0), (saved_state.pll_hsdiv_val[0] & ~(PLL_HSDIV_MASK)) | 0xf);
	mmio_write_32(MAIN_PLL0_HSDIVx(5), (saved_state.pll_hsdiv_val[5] & ~(PLL_HSDIV_MASK)) | 0x4);
	mmio_write_32(MAIN_PLL0_HSDIVx(6), (saved_state.pll_hsdiv_val[6] & ~(PLL_HSDIV_MASK)) | 0x3);
	mmio_write_32(MAIN_PLL0_HSDIVx(7), (saved_state.pll_hsdiv_val[7] & ~(PLL_HSDIV_MASK)) | 0x5);
	mmio_write_32(MAIN_PLL0_HSDIVx(8), (saved_state.pll_hsdiv_val[8] & ~(PLL_HSDIV_MASK)) | 0x27);
	mmio_write_32(MAIN_PLL0_HSDIVx(9), (saved_state.pll_hsdiv_val[9] & ~(PLL_CLK_EN_MASK)));

	/* Bypass MAIN PLL8 */
	mmio_write_32(MAIN_PLL8_CTRL, saved_state.pll_hsdiv_val[10] | PLL_BYP_EN_MASK);

	/* Enable DDR Auto self refresh */
	for (int i = 0; i < DDR_CTL_COUNT_LOW_LAT_STBY; i++) {
		mmio_write_32(stby_ddr_ctrl_regs[i].addr, stby_ddr_ctrl_regs[i].val);
	}

	/* Enable AUTO CLOCK GATING */
	mmio_write_32(WKUP_CTRL_MMR_CFG5_CLKGATE_CTRL0, EN_AUTO_CLKGATE);
	return;
}

/*
 * Restore the hardware state after exiting standby mode.
 * Restores PLL configurations, LPSC states, DDR controller registers,
 * and clock gating settings to their pre-standby values.
 */
void am62l_standby_restore_state()
{
    /* Restore AUTO CLOCK GATING state */
    mmio_write_32(WKUP_CTRL_MMR_CFG5_CLKGATE_CTRL0, saved_state.auto_clk_gate);

	/* Restore EMIF LP control registers */
	for (int i = 0; i < DDR_CTL_COUNT_LOW_LAT_STBY; i++) {
		mmio_write_32(stby_ddr_ctrl_regs[i].addr, saved_state.ddr_reg[i]);
	}

    /* Restore PLL */
    for (int i = 0;i < PLL_COUNT_LOW_LAT_STBY - 1;i++) {
        mmio_write_32(MAIN_PLL0_HSDIVx(i), saved_state.pll_hsdiv_val[i]);
    }
    mmio_write_32(MAIN_PLL8_CTRL, saved_state.pll_hsdiv_val[PLL_COUNT_LOW_LAT_STBY - 1]);

    /* Restore LPSC states */
    for (int i = 0;i < LPSC_COUNT_LOW_LAT_STBY;i++) {
        if (saved_state.lpsc_value[i] != PSC_SYNCRESETDISABLE) {
			set_main_psc_state(stby_pd_lpsc_table[i].pd_id, stby_pd_lpsc_table[i].lpsc_id, PSC_PD_ON, saved_state.lpsc_value[i]);
        }
    }
}

void am62l_enter_standby(uint32_t core, uint32_t cluster_pwr_state)
{
	uint32_t in_standby = state_entered[1-core];
	/* saving the original state of the system before entering standby mode */
	if (!in_standby) {
		am62l_standby_save_state();
	}
	state_entered[core] = cluster_pwr_state;

	if (!in_standby || in_standby < state_entered[core]) {
		if (state_entered[core] == LOW_LATENCY_IDLE_STATE) {
			am62l_low_latency_standby_sequence();
		}
	}
	/* Updating the SCR register to enter WFI */
	am62l_standby_scr_reg = read_scr_el3();
	write_scr_el3(am62l_standby_scr_reg | SCR_IRQ_BIT | SCR_FIQ_BIT);
	isb();
	dsb();
	return;
}

void am62l_exit_standby(uint32_t core, uint32_t cluster_pwr_state)
{
	/* Restoring the SCR register */
	write_scr_el3(am62l_standby_scr_reg);
	/* skipping the restore if other core still in an idle state */
	if (state_entered[core] <= state_entered[1-core]) {
		state_entered[core] = 0;
		return;
	}

	if (state_entered[core] == LOW_LATENCY_IDLE_STATE) {
		am62l_standby_restore_state();
	}
	/* now that both cores have exited cluster standby, we reset the state */
	state_entered[core] = 0;
	state_entered[1-core] = 0;
	return;
}

