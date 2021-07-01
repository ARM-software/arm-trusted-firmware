/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>

#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp2_ddr.h>
#include <drivers/st/stm32mp2_ddr_helpers.h>
#include <drivers/st/stm32mp2_ddr_regs.h>
#include <drivers/st/stm32mp_ddr.h>

#include <lib/mmio.h>

#include <platform_def.h>

/* HW idle period (unit: Multiples of 32 DFI clock cycles) */
#define HW_IDLE_PERIOD			0x3U

static enum stm32mp2_ddr_sr_mode saved_ddr_sr_mode;

#pragma weak stm32_ddrdbg_get_base
uintptr_t stm32_ddrdbg_get_base(void)
{
	return 0U;
}

static void set_qd1_qd3_update_conditions(struct stm32mp_ddrctl *ctl)
{
	mmio_setbits_32((uintptr_t)&ctl->dbg1, DDRCTRL_DBG1_DIS_DQ);

	stm32mp_ddr_set_qd3_update_conditions(ctl);
}

static void unset_qd1_qd3_update_conditions(struct stm32mp_ddrctl *ctl)
{
	stm32mp_ddr_unset_qd3_update_conditions(ctl);

	mmio_clrbits_32((uintptr_t)&ctl->dbg1, DDRCTRL_DBG1_DIS_DQ);
}

static void wait_dfi_init_complete(struct stm32mp_ddrctl *ctl)
{
	uint64_t timeout;
	uint32_t dfistat;

	timeout = timeout_init_us(DDR_TIMEOUT_US_1S);
	do {
		dfistat = mmio_read_32((uintptr_t)&ctl->dfistat);
		VERBOSE("[0x%lx] dfistat = 0x%x ", (uintptr_t)&ctl->dfistat, dfistat);

		if (timeout_elapsed(timeout)) {
			panic();
		}
	} while ((dfistat & DDRCTRL_DFISTAT_DFI_INIT_COMPLETE) == 0U);

	VERBOSE("[0x%lx] dfistat = 0x%x\n", (uintptr_t)&ctl->dfistat, dfistat);
}

static void disable_dfi_low_power_interface(struct stm32mp_ddrctl *ctl)
{
	uint64_t timeout;
	uint32_t dfistat;
	uint32_t stat;

	mmio_clrbits_32((uintptr_t)&ctl->dfilpcfg0, DDRCTRL_DFILPCFG0_DFI_LP_EN_SR);

	timeout = timeout_init_us(DDR_TIMEOUT_US_1S);
	do {
		dfistat = mmio_read_32((uintptr_t)&ctl->dfistat);
		stat = mmio_read_32((uintptr_t)&ctl->stat);
		VERBOSE("[0x%lx] dfistat = 0x%x ", (uintptr_t)&ctl->dfistat, dfistat);
		VERBOSE("[0x%lx] stat = 0x%x ", (uintptr_t)&ctl->stat, stat);

		if (timeout_elapsed(timeout)) {
			panic();
		}
	} while (((dfistat & DDRCTRL_DFISTAT_DFI_LP_ACK) != 0U) ||
		 ((stat & DDRCTRL_STAT_OPERATING_MODE_MASK) == DDRCTRL_STAT_OPERATING_MODE_SR));

	VERBOSE("[0x%lx] dfistat = 0x%x\n", (uintptr_t)&ctl->dfistat, dfistat);
	VERBOSE("[0x%lx] stat = 0x%x\n", (uintptr_t)&ctl->stat, stat);
}

void ddr_activate_controller(struct stm32mp_ddrctl *ctl, bool sr_entry)
{
	/*
	 * Manage quasi-dynamic registers modification
	 * dfimisc.dfi_frequency : Group 1
	 * dfimisc.dfi_init_complete_en and dfimisc.dfi_init_start : Group 3
	 */
	set_qd1_qd3_update_conditions(ctl);

	if (sr_entry) {
		mmio_setbits_32((uintptr_t)&ctl->dfimisc, DDRCTRL_DFIMISC_DFI_FREQUENCY);
	} else {
		mmio_clrbits_32((uintptr_t)&ctl->dfimisc, DDRCTRL_DFIMISC_DFI_FREQUENCY);
	}

	mmio_setbits_32((uintptr_t)&ctl->dfimisc, DDRCTRL_DFIMISC_DFI_INIT_START);
	mmio_clrbits_32((uintptr_t)&ctl->dfimisc, DDRCTRL_DFIMISC_DFI_INIT_START);

	wait_dfi_init_complete(ctl);

	udelay(DDR_DELAY_1US);

	if (sr_entry) {
		mmio_clrbits_32((uintptr_t)&ctl->dfimisc, DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	} else {
		mmio_setbits_32((uintptr_t)&ctl->dfimisc, DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);
	}

	udelay(DDR_DELAY_1US);

	unset_qd1_qd3_update_conditions(ctl);
}

#if STM32MP_LPDDR4_TYPE
static void disable_phy_ddc(void)
{
	/* Enable APB access to internal CSR registers */
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_APBONLY0_MICROCONTMUXSEL, 0U);
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_DRTUB0_UCCLKHCLKENABLES,
		      DDRPHY_DRTUB0_UCCLKHCLKENABLES_UCCLKEN |
		      DDRPHY_DRTUB0_UCCLKHCLKENABLES_HCLKEN);

	/* Disable DRAM drift compensation */
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_INITENG0_P0_SEQ0BDISABLEFLAG6, 0xFFFFU);

	/* Disable APB access to internal CSR registers */
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_DRTUB0_UCCLKHCLKENABLES,
		      DDRPHY_DRTUB0_UCCLKHCLKENABLES_HCLKEN);
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_APBONLY0_MICROCONTMUXSEL,
		      DDRPHY_APBONLY0_MICROCONTMUXSEL_MICROCONTMUXSEL);
}
#endif /* STM32MP_LPDDR4_TYPE */

void ddr_wait_lp3_mode(bool sr_entry)
{
	uint64_t timeout;
	bool repeat_loop = false;

	/* Enable APB access to internal CSR registers */
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_APBONLY0_MICROCONTMUXSEL, 0U);
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_DRTUB0_UCCLKHCLKENABLES,
		      DDRPHY_DRTUB0_UCCLKHCLKENABLES_UCCLKEN |
		      DDRPHY_DRTUB0_UCCLKHCLKENABLES_HCLKEN);

	timeout = timeout_init_us(DDR_TIMEOUT_US_1S);
	do {
		uint16_t phyinlpx = mmio_read_32(stm32mp_ddrphyc_base() +
						 DDRPHY_INITENG0_P0_PHYINLPX);

		if (timeout_elapsed(timeout)) {
			panic();
		}

		if (sr_entry) {
			repeat_loop = (phyinlpx & DDRPHY_INITENG0_P0_PHYINLPX_PHYINLP3) == 0U;
		} else {
			repeat_loop = (phyinlpx & DDRPHY_INITENG0_P0_PHYINLPX_PHYINLP3) != 0U;
		}
	} while (repeat_loop);

	/* Disable APB access to internal CSR registers */
#if STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_DRTUB0_UCCLKHCLKENABLES, 0U);
#else /* STM32MP_LPDDR4_TYPE */
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_DRTUB0_UCCLKHCLKENABLES,
		      DDRPHY_DRTUB0_UCCLKHCLKENABLES_HCLKEN);
#endif /* STM32MP_DDR3_TYPE || STM32MP_DDR4_TYPE */
	mmio_write_32(stm32mp_ddrphyc_base() + DDRPHY_APBONLY0_MICROCONTMUXSEL,
		      DDRPHY_APBONLY0_MICROCONTMUXSEL_MICROCONTMUXSEL);
}

static int sr_loop(bool is_entry)
{
	uint32_t type;
	uint32_t state __maybe_unused;
	uint64_t timeout = timeout_init_us(DDR_TIMEOUT_US_1S);
	bool repeat_loop = false;

	/*
	 * Wait for DDRCTRL to be out of or back to "normal/mission mode".
	 * Consider also SRPD mode for LPDDR4 only.
	 */
	do {
		type = mmio_read_32(stm32mp_ddrctrl_base() + DDRCTRL_STAT) &
		       DDRCTRL_STAT_SELFREF_TYPE_MASK;
#if STM32MP_LPDDR4_TYPE
		state = mmio_read_32(stm32mp_ddrctrl_base() + DDRCTRL_STAT) &
		       DDRCTRL_STAT_SELFREF_STATE_MASK;
#endif /* STM32MP_LPDDR4_TYPE */

		if (timeout_elapsed(timeout)) {
			return -ETIMEDOUT;
		}

		if (is_entry) {
#if STM32MP_LPDDR4_TYPE
			repeat_loop = (type == 0x0U) || (state != DDRCTRL_STAT_SELFREF_STATE_SRPD);
#else /* !STM32MP_LPDDR4_TYPE */
			repeat_loop = (type == 0x0U);
#endif /* STM32MP_LPDDR4_TYPE */
		} else {
#if STM32MP_LPDDR4_TYPE
			repeat_loop = (type != 0x0U) || (state != 0x0U);
#else /* !STM32MP_LPDDR4_TYPE */
			repeat_loop = (type != 0x0U);
#endif /* STM32MP_LPDDR4_TYPE */
		}
	} while (repeat_loop);

	return 0;
}

static int sr_entry_loop(void)
{
	return sr_loop(true);
}

int ddr_sr_exit_loop(void)
{
	return sr_loop(false);
}

static int sr_ssr_set(void)
{
	uintptr_t ddrctrl_base = stm32mp_ddrctrl_base();

	/*
	 * Disable Clock disable with LP modes
	 * (used in RUN mode for LPDDR2 with specific timing).
	 */
	mmio_clrbits_32(ddrctrl_base + DDRCTRL_PWRCTL, DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE);

	/* Disable automatic Self-Refresh mode */
	mmio_clrbits_32(ddrctrl_base + DDRCTRL_PWRCTL, DDRCTRL_PWRCTL_SELFREF_EN);

	mmio_write_32(stm32_ddrdbg_get_base() + DDRDBG_LP_DISABLE,
		      DDRDBG_LP_DISABLE_LPI_XPI_DISABLE | DDRDBG_LP_DISABLE_LPI_DDRC_DISABLE);

	return 0;
}

static int sr_ssr_entry(bool standby)
{
	uintptr_t ddrctrl_base = stm32mp_ddrctrl_base();
	uintptr_t rcc_base = stm32mp_rcc_base();

	if (stm32mp_ddr_disable_axi_port((struct stm32mp_ddrctl *)ddrctrl_base) != 0) {
		panic();
	}

#if STM32MP_LPDDR4_TYPE
	if (standby) {
		/* Disable DRAM drift compensation */
		disable_phy_ddc();
	}
#endif /* STM32MP_LPDDR4_TYPE */

	disable_dfi_low_power_interface((struct stm32mp_ddrctl *)ddrctrl_base);

	/* SW self refresh entry prequested */
	mmio_setbits_32(ddrctrl_base + DDRCTRL_PWRCTL, DDRCTRL_PWRCTL_SELFREF_SW);
#if STM32MP_LPDDR4_TYPE
	mmio_clrbits_32(ddrctrl_base + DDRCTRL_PWRCTL, DDRCTRL_PWRCTL_STAY_IN_SELFREF);
#endif /* STM32MP_LPDDR4_TYPE */

	if (sr_entry_loop() != 0) {
		return -1;
	}

	ddr_activate_controller((struct stm32mp_ddrctl *)ddrctrl_base, true);

	/* Poll on ddrphy_initeng0_phyinlpx.phyinlp3 = 1 */
	ddr_wait_lp3_mode(true);

	if (standby) {
		mmio_clrbits_32(stm32mp_pwr_base() + PWR_CR11, PWR_CR11_DDRRETDIS);
	}

	mmio_clrsetbits_32(rcc_base + RCC_DDRCPCFGR, RCC_DDRCPCFGR_DDRCPLPEN,
			   RCC_DDRCPCFGR_DDRCPEN);
	mmio_setbits_32(rcc_base + RCC_DDRPHYCCFGR, RCC_DDRPHYCCFGR_DDRPHYCEN);
	mmio_setbits_32(rcc_base + RCC_DDRITFCFGR, RCC_DDRITFCFGR_DDRPHYDLP);

	return 0;
}

static int sr_ssr_exit(void)
{
	uintptr_t ddrctrl_base = stm32mp_ddrctrl_base();
	uintptr_t rcc_base = stm32mp_rcc_base();

	mmio_setbits_32(rcc_base + RCC_DDRCPCFGR,
			RCC_DDRCPCFGR_DDRCPLPEN | RCC_DDRCPCFGR_DDRCPEN);
	mmio_clrbits_32(rcc_base + RCC_DDRITFCFGR, RCC_DDRITFCFGR_DDRPHYDLP);
	mmio_setbits_32(rcc_base + RCC_DDRPHYCCFGR, RCC_DDRPHYCCFGR_DDRPHYCEN);

	udelay(DDR_DELAY_1US);

	ddr_activate_controller((struct stm32mp_ddrctl *)ddrctrl_base, false);

	/* Poll on ddrphy_initeng0_phyinlpx.phyinlp3 = 0 */
	ddr_wait_lp3_mode(false);

	/* SW self refresh exit prequested */
	mmio_clrbits_32(ddrctrl_base + DDRCTRL_PWRCTL, DDRCTRL_PWRCTL_SELFREF_SW);

	if (ddr_sr_exit_loop() != 0) {
		return -1;
	}

	/* Re-enable DFI low-power interface */
	mmio_setbits_32(ddrctrl_base + DDRCTRL_DFILPCFG0, DDRCTRL_DFILPCFG0_DFI_LP_EN_SR);

	stm32mp_ddr_enable_axi_port((struct stm32mp_ddrctl *)ddrctrl_base);

	return 0;
}

static int sr_hsr_set(void)
{
	uintptr_t ddrctrl_base = stm32mp_ddrctrl_base();

	mmio_clrsetbits_32(stm32mp_rcc_base() + RCC_DDRITFCFGR,
			   RCC_DDRITFCFGR_DDRCKMOD_MASK, RCC_DDRITFCFGR_DDRCKMOD_HSR);

	/*
	 * manage quasi-dynamic registers modification
	 * hwlpctl.hw_lp_en : Group 2
	 */
	if (stm32mp_ddr_sw_selfref_entry((struct stm32mp_ddrctl *)ddrctrl_base) != 0) {
		panic();
	}
	stm32mp_ddr_start_sw_done((struct stm32mp_ddrctl *)ddrctrl_base);

	mmio_write_32(ddrctrl_base + DDRCTRL_HWLPCTL,
		      DDRCTRL_HWLPCTL_HW_LP_EN | DDRCTRL_HWLPCTL_HW_LP_EXIT_IDLE_EN |
		      (HW_IDLE_PERIOD << DDRCTRL_HWLPCTL_HW_LP_IDLE_X32_SHIFT));

	stm32mp_ddr_wait_sw_done_ack((struct stm32mp_ddrctl *)ddrctrl_base);
	stm32mp_ddr_sw_selfref_exit((struct stm32mp_ddrctl *)ddrctrl_base);

	return 0;
}

static int sr_hsr_entry(void)
{
	mmio_write_32(stm32mp_rcc_base() + RCC_DDRCPCFGR, RCC_DDRCPCFGR_DDRCPLPEN);

	return sr_entry_loop(); /* read_data should be equal to 0x223 */
}

static int sr_hsr_exit(void)
{
	mmio_write_32(stm32mp_rcc_base() + RCC_DDRCPCFGR,
		      RCC_DDRCPCFGR_DDRCPLPEN | RCC_DDRCPCFGR_DDRCPEN);

	/* TODO: check if ddr_sr_exit_loop() is needed here */

	return 0;
}

static int sr_asr_set(void)
{
	mmio_write_32(stm32_ddrdbg_get_base() + DDRDBG_LP_DISABLE, 0U);

	return 0;
}

static int sr_asr_entry(void)
{
	/*
	 * Automatically enter into self refresh when there is no ddr traffic
	 * for the delay programmed into SYSCONF_DDRC_AUTO_SR_DELAY register.
	 * Default value is 0x20 (unit: Multiples of 32 DFI clock cycles).
	 */
	return sr_entry_loop();
}

static int sr_asr_exit(void)
{
	return ddr_sr_exit_loop();
}

uint32_t ddr_get_io_calibration_val(void)
{
	/* TODO create related service */

	return 0U;
}

int ddr_sr_entry(bool standby)
{
	int ret = -EINVAL;

	switch (saved_ddr_sr_mode) {
	case DDR_SSR_MODE:
		ret = sr_ssr_entry(standby);
		break;
	case DDR_HSR_MODE:
		ret = sr_hsr_entry();
		break;
	case DDR_ASR_MODE:
		ret = sr_asr_entry();
		break;
	default:
		break;
	}

	return ret;
}

int ddr_sr_exit(void)
{
	int ret = -EINVAL;

	switch (saved_ddr_sr_mode) {
	case DDR_SSR_MODE:
		ret = sr_ssr_exit();
		break;
	case DDR_HSR_MODE:
		ret = sr_hsr_exit();
		break;
	case DDR_ASR_MODE:
		ret = sr_asr_exit();
		break;
	default:
		break;
	}

	return ret;
}

enum stm32mp2_ddr_sr_mode ddr_read_sr_mode(void)
{
	uint32_t pwrctl = mmio_read_32(stm32mp_ddrctrl_base() + DDRCTRL_PWRCTL);
	enum stm32mp2_ddr_sr_mode mode = DDR_SR_MODE_INVALID;

	switch (pwrctl & (DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE |
			  DDRCTRL_PWRCTL_SELFREF_EN)) {
	case 0U:
		mode = DDR_SSR_MODE;
		break;
	case DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE:
		mode = DDR_HSR_MODE;
		break;
	case DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE | DDRCTRL_PWRCTL_SELFREF_EN:
		mode = DDR_ASR_MODE;
		break;
	default:
		break;
	}

	return mode;
}

void ddr_set_sr_mode(enum stm32mp2_ddr_sr_mode mode)
{
	int ret = -EINVAL;

	if (mode == saved_ddr_sr_mode) {
		return;
	}

	switch (mode) {
	case DDR_SSR_MODE:
		ret = sr_ssr_set();
		break;
	case DDR_HSR_MODE:
		ret = sr_hsr_set();
		break;
	case DDR_ASR_MODE:
		ret = sr_asr_set();
		break;
	default:
		break;
	}

	if (ret != 0) {
		ERROR("Unknown Self Refresh mode\n");
		panic();
	}

	saved_ddr_sr_mode = mode;
}

void ddr_save_sr_mode(void)
{
	saved_ddr_sr_mode = ddr_read_sr_mode();
}

void ddr_restore_sr_mode(void)
{
	ddr_set_sr_mode(saved_ddr_sr_mode);
}

void ddr_sub_system_clk_init(void)
{
	mmio_write_32(stm32mp_rcc_base() + RCC_DDRCPCFGR,
		      RCC_DDRCPCFGR_DDRCPEN | RCC_DDRCPCFGR_DDRCPLPEN);
}

void ddr_sub_system_clk_off(void)
{
	uintptr_t rcc_base = stm32mp_rcc_base();

	/* Clear DDR IO retention */
	mmio_clrbits_32(stm32mp_pwr_base() + PWR_CR11, PWR_CR11_DDRRETDIS);

	/* Reset DDR sub system */
	mmio_write_32(rcc_base + RCC_DDRCPCFGR, RCC_DDRCPCFGR_DDRCPRST);
	mmio_write_32(rcc_base + RCC_DDRITFCFGR, RCC_DDRITFCFGR_DDRRST);
	mmio_write_32(rcc_base + RCC_DDRPHYCAPBCFGR, RCC_DDRPHYCAPBCFGR_DDRPHYCAPBRST);
	mmio_write_32(rcc_base + RCC_DDRCAPBCFGR, RCC_DDRCAPBCFGR_DDRCAPBRST);

	/* Deactivate clocks and PLL2 */
	mmio_clrbits_32(rcc_base + RCC_DDRPHYCCFGR, RCC_DDRPHYCCFGR_DDRPHYCEN);
	mmio_clrbits_32(rcc_base + RCC_PLL2CFGR1, RCC_PLL2CFGR1_PLLEN);
}
