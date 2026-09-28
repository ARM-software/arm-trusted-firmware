/*
 * Copyright (c) 2019-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "agilex72_clock_manager.h"

/*
 * Agilex72 clock manager ATF BL2 is the FSBL (like Agilex5):
 * config_clkmgr_handoff() brings up the SDM-preset GPPLLs and the clock
 * manager. Rates are read back as golden GPPLL C-output Hz / live divider
 * (the VCO config is owned by the SDM, not ATF).
 */

/* *_CTR count divider: effective divisor = count[10:0] + 1 */
static uint32_t clkmgr_free_ctr_div(uint32_t ctr_off)
{
	uint32_t cnt = mmio_read_32(CLKMGR_BASE + ctr_off) &
		       CLKMGR_FREE_CTR_CNT_MASK;

	return cnt + 1U;
}

/* Power-of-two field divider: effective divisor = 1 << field */
static uint32_t clkmgr_pow2_div(uint32_t reg_off, uint32_t shift, uint32_t width)
{
	uint32_t field = (mmio_read_32(CLKMGR_BASE + reg_off) >> shift) &
			 (BIT(width) - 1U);

	return BIT(field);
}

/* comp0 clock (A520 complex 0 / legacy MPU): GPPLL1_C0 / comp0 counter */
static uint32_t get_mpu_clk(void)
{
	return (uint32_t)(AGILEX72_GPPLL1_C0_HZ /
			  clkmgr_free_ctr_div(CLKMGR_COMP0_CTR));
}

/* LSP main NoC clock: GPPLL0_C1 / lspnoc counter */
static uint32_t get_l4_main_clk(void)
{
	return (uint32_t)(AGILEX72_GPPLL0_C1_HZ /
			  clkmgr_free_ctr_div(CLKMGR_LSPNOC_FREE_CTR));
}

/* LSP mp clock: lsp_main / nocdiv[LSPMP] */
static uint32_t get_l4_mp_clk(void)
{
	return get_l4_main_clk() /
	       clkmgr_pow2_div(CLKMGR_NOCDIV, CLKMGR_NOCDIV_LSPMP_SHIFT,
			       CLKMGR_NOCDIV_WIDTH);
}

/* LSP sp clock: lsp_main / nocdiv[LSPSP] (UART/I2C/timers) */
static uint32_t get_l4_sp_clk(void)
{
	return get_l4_main_clk() /
	       clkmgr_pow2_div(CLKMGR_NOCDIV, CLKMGR_NOCDIV_LSPSP_SHIFT,
			       CLKMGR_NOCDIV_WIDTH);
}

/* LSP sys free clock (watchdog): lsp_main / nocdiv[LSPSYS] */
static uint32_t get_wdt_clk(void)
{
	return get_l4_main_clk() /
	       clkmgr_pow2_div(CLKMGR_NOCDIV, CLKMGR_NOCDIV_LSPSYS_SHIFT,
			       CLKMGR_NOCDIV_WIDTH);
}

/* Slow peripherals (UART/I2C/timers) run from lsp_sp */
static uint32_t get_uart_clk(void)
{
	return get_l4_sp_clk();
}

/* EMAC data clock (EMAC-A path): GPPLL0_C0 / emaca counter / otherdiv[EMACA] */
static uint32_t get_emaca_clk(void)
{
	uint32_t emaca_free = (uint32_t)(AGILEX72_GPPLL0_C0_HZ /
					 clkmgr_free_ctr_div(CLKMGR_EMACA_CTR));

	return emaca_free /
	       clkmgr_pow2_div(CLKMGR_OTHERDIV, CLKMGR_OTHERDIV_EMACA_SHIFT,
			       CLKMGR_NOCDIV_WIDTH);
}

/* SDMMC / mem-device PHY: GPPLL0_C3 / memdevphy counter / peripctr[SDMMC0] */
static uint32_t get_sdmmc_clk(void)
{
	uint32_t memdevphy_free =
		(uint32_t)(AGILEX72_GPPLL0_C3_HZ /
			   clkmgr_free_ctr_div(CLKMGR_MEMDEVPHY_FREE_CTR));

	return memdevphy_free /
	       clkmgr_pow2_div(CLKMGR_PERIPCTR, CLKMGR_PERIPCTR_SDMMC0_SHIFT,
			       CLKMGR_PERIPCTR_WIDTH);
}

/*
 * Clock-manager bring-up (ATF BL2 as FSBL): secure unlock, GPPLL analog
 * enable, wait for lock, program the free-counter dividers, exit boot-clock
 * bypass. The analog enable is gated on the live STAT lock bits (not a build
 * flag), so one binary covers both silicon (GPPLLs lock after the GLOB
 * sequence) and platforms that hand them over already locked (analog enable
 * skipped, lock wait immediate).
 */

/* Unlock clock-manager writes at the secure manager. */
static void clkmgr_secure_unlock(void)
{
	mmio_write_32(AGILEX72_SECMGR_CLKMGR_UNLOCK_ADDR,
		      AGILEX72_SECMGR_CLKMGR_UNLOCK_VAL);
}

/*
 * 5-step GLOB enable/bias/unfreeze for all three GPPLLs. The SDM has loaded
 * each GPPLL's config; this only starts the analog PLL so it can lock. Called
 * only when the GPPLLs are not already locked.
 */
static void gppll_analog_enable(void)
{
	static const uint32_t globs[] = {
		CLKMGR_PLL0_GLOB, CLKMGR_PLL1_GLOB, CLKMGR_PLL2_GLOB,
	};
	static const uint32_t stages[] = {
		CLKMGR_PLLGLOB_S1_CSR_EN,
		CLKMGR_PLLGLOB_S2_BIAS_ON,
		CLKMGR_PLLGLOB_S3_FRZREG_OFF,
		CLKMGR_PLLGLOB_S4_IOTRI_ON,
		CLKMGR_PLLGLOB_S5_FBLVDS_ON,
	};

	for (uint32_t s = 0U; s < ARRAY_SIZE(stages); s++) {
		for (uint32_t p = 0U; p < ARRAY_SIZE(globs); p++) {
			mmio_write_32(CLKMGR_BASE + globs[p], stages[s]);
		}

		/* Let bias/unfreeze settle between the middle stages. */
		if ((s == 1U) || (s == 2U)) {
			udelay(5);
		}
	}
}

/* True when STAT reports GPPLL0/1/2 all locked. */
static bool gppll_all_locked(void)
{
	return (mmio_read_32(CLKMGR(STAT)) & CLKMGR_STAT_ALL_LOCKED) ==
	       CLKMGR_STAT_ALL_LOCKED;
}

/* Poll until all three GPPLLs report lock; -ETIMEDOUT on failure. */
static int gppll_wait_lock(void)
{
	uint32_t timeout = CLKMGR_PLL_LOCK_TIMEOUT_US;

	while ((mmio_read_32(CLKMGR(STAT)) & CLKMGR_STAT_ALL_LOCKED) !=
	       CLKMGR_STAT_ALL_LOCKED) {
		if (timeout-- == 0U) {
			ERROR("CLKMGR: GPPLL lock timeout, STAT=0x%08x\n",
			      mmio_read_32(CLKMGR(STAT)));
			return -ETIMEDOUT;
		}
		udelay(1);
	}

	return 0;
}

/*
 * Leave boot-clock bypass: switch the tree onto the locked GPPLLs (CTRL=0),
 * wait for the FSM to go idle, then release the CPU/peripheral counter resets.
 * Returns -ETIMEDOUT (without releasing the resets) if BUSY never clears.
 */
static int clkmgr_exit_boot_bypass(void)
{
	uint32_t timeout = CLKMGR_BUSY_TIMEOUT_US;

	mmio_write_32(CLKMGR(CTRL), 0U);

	while ((mmio_read_32(CLKMGR(STAT)) & CLKMGR_STAT_BUSY_BIT) != 0U) {
		if (timeout-- == 0U) {
			ERROR("CLKMGR: BUSY did not clear after boot-mode exit\n");
			return -ETIMEDOUT;
		}
		udelay(1);
	}

	mmio_clrbits_32(CLKMGR_BASE + CLKMGR_EXTCNTRST,
			CLKMGR_EXTCNTRST_CPU_RELEASE);
	mmio_clrbits_32(CLKMGR_BASE + CLKMGR_PERICTL_EXTCNTRST,
			CLKMGR_PERICTL_EXTCNTRST_RELEASE);

	return 0;
}

/*
 * Golden count-divider presets for the free counters. The SDM does not always
 * program these to their target values (stale counts would mis-clock the CPU /
 * LSP-NoC tree), so as the FSBL ATF sets them here. Effective divisor =
 * count + 1; SRC is forced to 0 to select the GPPLL C-output parent.
 */
struct clkmgr_free_ctr_preset {
	uint32_t off;
	uint32_t cnt;
};

static const struct clkmgr_free_ctr_preset clkmgr_free_ctr_golden[] = {
	{ CLKMGR_COMP0_CTR,		0U },	/* comp0/MPU  GPPLL1_C0 /1  = 1850 MHz */
	{ CLKMGR_CORE2_CTR,		0U },	/* A720 core2 GPPLL2_C0 /1  = 2500 MHz */
	{ CLKMGR_CORE3_CTR,		0U },	/* A720 core3 GPPLL2_C1 /1  = 2500 MHz */
	{ CLKMGR_DSU_CTR,		0U },	/* DSU        GPPLL1_C1 /1  = 1850 MHz */
	{ CLKMGR_CCU_FREE_CTR,		0U },	/* CCU free   GPPLL0_C0 /1  = 1000 MHz */
	{ CLKMGR_LSPNOC_FREE_CTR,	0U },	/* LSP main   GPPLL0_C1 /1  =  500 MHz */
	{ CLKMGR_MEMDEVPHY_FREE_CTR,	9U },	/* mem PHY    GPPLL0_C3 /10 =  200 MHz */
	{ CLKMGR_XSPIPHY_FREE_CTR,	9U },	/* xSPI PHY   GPPLL0_C3 /10 =  200 MHz */
};

/* Program the clock-manager free counters to the golden count dividers. */
static void clkmgr_program_free_counters(void)
{
	for (unsigned int i = 0U; i < ARRAY_SIZE(clkmgr_free_ctr_golden); i++) {
		uint32_t off = clkmgr_free_ctr_golden[i].off;
		uint32_t val = mmio_read_32(CLKMGR_BASE + off);

		val &= ~(CLKMGR_FREE_CTR_CNT_MASK | CLKMGR_FREE_CTR_SRC_MASK);
		val |= clkmgr_free_ctr_golden[i].cnt & CLKMGR_FREE_CTR_CNT_MASK;
		mmio_write_32(CLKMGR_BASE + off, val);
	}
}

int config_clkmgr_handoff(handoff *hoff_ptr)
{
	int ret;

	(void)hoff_ptr;

	/* 1. Allow clock-manager register writes. */
	clkmgr_secure_unlock();

	/* 2. Start the GPPLLs so they can lock, unless already locked. */
	if (!gppll_all_locked()) {
		gppll_analog_enable();
	} else {
		VERBOSE("CLKMGR: GPPLLs already report locked; skipping analog enable\n");
	}

	/* 3. Wait for GPPLL lock; abort the bring-up on failure. */
	ret = gppll_wait_lock();
	if (ret != 0) {
		return ret;
	}

	/* 4. Program the count dividers to golden before releasing counters. */
	clkmgr_program_free_counters();

	/* 5. Exit boot-clock bypass and release counter resets. */
	return clkmgr_exit_boot_bypass();
}

/* Return the clock value of a given system component */
uint32_t clkmgr_get_rate(uint32_t clk_id)
{
	uint32_t clk_rate;

	switch (clk_id) {
	case CLKMGR_MPU_CLK_ID:
		clk_rate = get_mpu_clk();
		break;

	case CLKMGR_L4_MAIN_CLK_ID:
		clk_rate = get_l4_main_clk();
		break;

	case CLKMGR_L4_MP_CLK_ID:
		clk_rate = get_l4_mp_clk();
		break;

	case CLKMGR_L4_SP_CLK_ID:
		clk_rate = get_l4_sp_clk();
		break;

	case CLKMGR_UART_CLK_ID:
		clk_rate = get_uart_clk();
		break;

	case CLKMGR_WDT_CLK_ID:
		clk_rate = get_wdt_clk();
		break;

	case CLKMGR_EMAC0_CLK_ID:
		clk_rate = get_emaca_clk();
		break;

	case CLKMGR_SDMMC_CLK_ID:
		clk_rate = get_sdmmc_clk();
		break;

	default:
		ERROR("CLKMGR: %s: Invalid clock ID\n", __func__);
		clk_rate = 0U;
		break;
	}

	return clk_rate;
}

/* Return mpu_periph_clk tick */
unsigned int plat_get_syscnt_freq2(void)
{
	return PLAT_SYS_COUNTER_FREQ_IN_TICKS;
}
