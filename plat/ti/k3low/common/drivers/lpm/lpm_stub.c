/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl31/bl31.h>
#include <board_def.h>
#include <ddr.h>
#include <errno.h>
#include <k3_lpm_timeout.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <lpm_trace.h>
#include <lpm_stub.h>
#include <plat/common/platform.h>
#include <pll_16fft_raw.h>
#include <psc_raw.h>
#include <rtc.h>
#include <ti_sci.h>

#define WFI_STATUS					(0x400)
#define MPU_TIFS_WFI_MASK				BIT(2)
#define WKUP0_EN					(0x4030U)
#define WKUP0_EN_ALL_SOURCES				(0x7FFFF)
#define WKUP0_SRC					(0x4040U)
#define RST_CTRL					(0x4000U)
#define PMCTRL_SYS					(0x80)
#define WKUP_CTRL_PMCTRL_IO_0				(0x84)
#define WKUP_CTRL_PMCTRL_IO_1				(0x88)
#define WKUP_CTRL_DEEPSLEEP_CTRL			(0x160)
#define CANUART_WAKE_RESUME_KEY0_STAT			(0x3100U)
#define CANUART_WAKE_OFF_MODE				(0x1310U)
#define CANUART_WAKE_OFF_MODE_STAT1			(0x130CU)
#define CANUART_WAKE_OFF_MODE_STAT1_ENABLED		(0x1U)
#define PSC_CORE_1_MDSTAT				(0x8A4U)
#define MDSTAT_STATE_MASK				(0x1FU)
#define GP_CORE_CTL					0
#define PD_DDR						2U
#define LPSC_MAIN_DDR_LOCAL				21U
#define LPSC_MAIN_DDR_CFG_ISO_N				22U
#define LPSC_MAIN_DDR_DATA_ISO_N			23U
#define LPSC_MAIN_GP_USB0				7
#define LPSC_MAIN_GP_USB0_ISO_N				8
#define LPSC_MAIN_GP_USB1				9
#define LPSC_MAIN_GP_USB1_ISO_N				10

#define TI_MAILBOX_MSG					UL(0x40)
#define MAILBOX_MSG_BUFFER_OFFSET			(0x100U)
#define TISCI_MSG_CORE_RESUME				(0x000A0304U)

#define PLLOFFSET(idx)					(0x1000 * (idx))
#define SCTLR_EL3_M_BIT					((uint32_t)1U << 0)

#define WKUP_CTRL_PMCTRL_IO_GLB_ENABLE_IO		1
#define WKUP_CTRL_DEEPSLEEP_CTRL_ENABLE_IO		(0x101U)
#define WKUP_CTRL_DEEPSLEEP_CTRL_DISABLE_IO		0
#define WKUP_CTRL_PMCTRL_IO_GLB_DISABLE_IO		0
#define WKUP_CTRL_PMCTRL_IO_0_ISOCLK_OVRD		BIT(0)
#define WKUP_CTRL_PMCTRL_IO_0_ISOOVR_EXTEND		BIT(4)
#define WKUP_CTRL_PMCTRL_IO_0_ISO_BYPASS		BIT(6)
#define WKUP_CTRL_PMCTRL_IO_0_WUCLK_CTRL		BIT(8)
#define WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS		BIT(25)
#define WKUP_CTRL_PMCTRL_IO_0_WUCLK_STATUS_ENABLED	1U
#define WKUP_CTRL_PMCTRL_IO_0_WUCLK_STATUS_DISABLED	0U
#define WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN		BIT(16)
#define WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL		BIT(24)
#define WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK (WKUP_CTRL_PMCTRL_IO_0_ISOCLK_OVRD	\
					  | WKUP_CTRL_PMCTRL_IO_0_ISOOVR_EXTEND	 \
					  | WKUP_CTRL_PMCTRL_IO_0_ISO_BYPASS	 \
					  | WKUP_CTRL_PMCTRL_IO_0_WUCLK_CTRL	 \
					  | WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN	 \
					  | WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL)

#define RTC_ONLY_PLUS_DDR_MAGIC_WORD		(0x6D555555U)
#define DEEP_SLEEP_MAGIC_WORD			(0xD5555555U)

/* counts of 1us delay for 100ms */
#define TIMEOUT_100MS					100000U

/* Main PLL to be saved and restored */
__wkupsramdata struct pll_raw_data main_pll0 = {
.base = K3_MAIN_PLL_MMR_BASE + PLLOFFSET(0U), };

__wkupsramdata struct pll_raw_data main_pll8 = {
.base = K3_MAIN_PLL_MMR_BASE + PLLOFFSET(8U), };

__wkupsramdata struct pll_raw_data main_pll17 = {
.base = K3_MAIN_PLL_MMR_BASE + PLLOFFSET(17U), };

/* Base addresses of main PLL structures to be saved and restored */
__wkupsramdata struct pll_raw_data *main_plls_save_rstr[3] = {
&main_pll0, &main_pll8, &main_pll17};

__wkupsramdata int num_main_plls_save_rstr = 3;
__wkupsramdata uint8_t usb0_state;
__wkupsramdata uint8_t usb1_state;

extern uint32_t k3low_lpm_switch_stack(uintptr_t jump, uintptr_t stack, uint32_t arg);
static void k3_lpm_jump_to_stub(uint32_t mode);

void k3low_config_wake_sources(bool enable)
{
	uint32_t wake_up_src;

	if (enable) {
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP0_EN,
			      WKUP0_EN_ALL_SOURCES);
	} else {
		wake_up_src = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP0_SRC);
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP0_EN, 0x00);
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP0_SRC, wake_up_src);
	}
}

void k3low_lpm_config_magic_words(uint32_t mode)
{
	if (mode == 0) {
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE +
			      CANUART_WAKE_OFF_MODE, DEEP_SLEEP_MAGIC_WORD);
	} else {
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE +
			      CANUART_WAKE_OFF_MODE,
			      RTC_ONLY_PLUS_DDR_MAGIC_WORD);
	}
}

bool k3low_lpm_check_can_io_latch(void)
{
	return (mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE +
			     CANUART_WAKE_OFF_MODE_STAT1) &
		CANUART_WAKE_OFF_MODE_STAT1_ENABLED);
}

/**
 * @brief Save main domain pll configuration
 *
 */
__wkupsramfunc static void save_main_pll(void)
{
	int i = 0;

	for (i = 0; i < num_main_plls_save_rstr; i++) {
		k3low_pll_save(main_plls_save_rstr[i]);
	}
}

/**
 * @brief Disable main domain plls
 *
 */
__wkupsramfunc static void disable_main_pll(void)
{
	int i;

	for (i = 0; i < num_main_plls_save_rstr; i++) {
		k3low_pll_disable(main_plls_save_rstr[i]);
	}
}

/**
 * @brief Save and disable USB LPSCs during suspend
 *
 * Saves the current state of both USB0 and USB1 LPSCs and their isolation
 * modules (ISO_N), then disables them to allow USB to act as a wake-up
 * source during deep sleep. The saved states are used during resume to
 * restore the USB modules to their pre-suspend configuration.
 *
 * @return 0 on success, error code otherwise
 */
__wkupsramfunc static int32_t save_and_disable_usb_lpsc(void)
{
	int32_t ret = 0;

	usb0_state = psc_raw_lpsc_get_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB0);
	k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB0, MDCTL_STATE_DISABLE, 0);
	k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, GP_CORE_CTL);
	ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, GP_CORE_CTL);

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB0_ISO_N,
					     MDCTL_STATE_DISABLE, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, GP_CORE_CTL);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, GP_CORE_CTL);
	}

	if (ret == 0) {
		usb1_state = psc_raw_lpsc_get_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB1);
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB1,
					     MDCTL_STATE_DISABLE, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, GP_CORE_CTL);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, GP_CORE_CTL);
	}

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB1_ISO_N,
					     MDCTL_STATE_DISABLE, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, GP_CORE_CTL);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, GP_CORE_CTL);
	}

	return ret;
}

/**
 * @brief Restore USB LPSCs during resume
 *
 * Restores both USB0 and USB1 LPSCs and their isolation modules (ISO_N)
 * to the states saved during suspend. This ensures USB modules are only
 * re-enabled if they were enabled before entering low power mode.
 *
 * @return 0 on success, error code otherwise
 */
__wkupsramfunc static int32_t restore_usb_lpsc(void)
{
	int32_t ret;

	k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB0, usb0_state, 0);
	k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, GP_CORE_CTL);
	ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, GP_CORE_CTL);

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB0_ISO_N,
					     usb0_state, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, GP_CORE_CTL);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, GP_CORE_CTL);
	}

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB1, usb1_state, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, GP_CORE_CTL);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, GP_CORE_CTL);
	}

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_GP_USB1_ISO_N,
					     usb1_state, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, GP_CORE_CTL);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, GP_CORE_CTL);
	}

	return ret;
}

/**
 * @brief Disable DDR LPSC
 *
 */
__wkupsramfunc static int32_t disable_ddr_lpsc(void)
{
	int32_t ret;

	k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_DDR_DATA_ISO_N,
				     MDCTL_STATE_SWRSTDISABLE, 0);
	k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, PD_DDR);
	ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, PD_DDR);

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE,
					     LPSC_MAIN_DDR_CFG_ISO_N,
					     MDCTL_STATE_SWRSTDISABLE, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, PD_DDR);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, PD_DDR);
	}

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE,
					     LPSC_MAIN_DDR_LOCAL,
					     MDCTL_STATE_SWRSTDISABLE, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, PD_DDR);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, PD_DDR);
	}

	return ret;
}

/**
 * @brief Enable DDR LPSC
 *
 */
__wkupsramfunc static int32_t enable_ddr_lpsc(void)
{
	int32_t ret;

	k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE, LPSC_MAIN_DDR_LOCAL,
				     MDCTL_STATE_ENABLE, 0);
	k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, PD_DDR);
	ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, PD_DDR);

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE,
					     LPSC_MAIN_DDR_CFG_ISO_N,
					     MDCTL_STATE_ENABLE, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, PD_DDR);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, PD_DDR);
	}

	if (ret == 0) {
		k3low_psc_raw_lpsc_set_state(K3_MAIN_PSC_BASE,
					     LPSC_MAIN_DDR_DATA_ISO_N,
					     MDCTL_STATE_ENABLE, 0);
		k3low_psc_raw_pd_initiate(K3_MAIN_PSC_BASE, PD_DDR);
		ret = k3low_psc_raw_pd_wait(K3_MAIN_PSC_BASE, PD_DDR);
	}

	return ret;
}

/**
 * @brief Restore main domain plls
 *
 */
__wkupsramfunc static int32_t restore_main_pll(void)
{
	int i;
	int32_t ret = 0;

	for (i = 0; i < num_main_plls_save_rstr; i++) {
		ret = k3low_pll_restore(main_plls_save_rstr[i]);
		if (ret != 0) {
			return ret;
		}
	}

	return ret;
}

__wkupsramfunc void k3low_lpm_abort(void)
{
	volatile int a = 0x1234;

	lpm_seq_trace_fail(LPM_SEQ_ABORT);
	while (a) {
		wfi();
	}
}

/**
 * @brief Wait for TIFS to be in WFI
 *
 */
__wkupsramfunc static bool lpm_sleep_wait_for_tifs_wfi(void)
{
	uint32_t reg;
	int32_t i = TIMEOUT_100MS;

	do {
		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WFI_STATUS);
		if ((reg & MPU_TIFS_WFI_MASK) == MPU_TIFS_WFI_MASK) {
			return true;
		}
		i--;
		k3low_lpm_delay_1us();
	} while (i != 0);
	return false;
}

/**
 * @brief Wait for secondary core to power off.
 *
 */
__wkupsramfunc static bool lpm_wait_for_secondary_core_down(void)
{
	uint32_t reg;
	int32_t i = TIMEOUT_100MS;

	do {
		reg = mmio_read_32(K3_MAIN_PSC_BASE + PSC_CORE_1_MDSTAT);
		if ((reg & MDSTAT_STATE_MASK) == MDCTL_STATE_SWRSTDISABLE) {
			return true;
		}
		i--;
		k3low_lpm_delay_1us();
	} while (i != 0);
	return false;
}

__wkupsramsuspendentry void k3low_lpm_stub_entry(uint32_t mode)
{
	if (mode == TI_K3_SLEEP_MODE_RTC_PLUS_DDR) {
		/* Wait for a53_1 to turn off */
		if (lpm_wait_for_secondary_core_down() == false) {
			lpm_seq_trace_fail(LPM_SEQ_SECONDARY_CORE_DOWN);
			k3low_lpm_abort();
		} else {
			lpm_seq_trace(LPM_SEQ_SECONDARY_CORE_DOWN);
		}

		if (lpm_sleep_wait_for_tifs_wfi() == false) {
			lpm_seq_trace_fail(LPM_SEQ_TIFS_WFI_WAIT);
			k3low_lpm_abort();
		} else {
			lpm_seq_trace(LPM_SEQ_TIFS_WFI_WAIT);
		}

		/* Place DDR into self-refresh */
		if (k3low_put_ddr_in_rtc_lpm() != 0) {
			lpm_seq_trace_fail(LPM_SEQ_DDR_SELF_REFRESH);
			k3low_lpm_abort();
		} else {
			lpm_seq_trace(LPM_SEQ_DDR_SELF_REFRESH);
		}

		/*	Disable the LPSCs for DDR */
		if (disable_ddr_lpsc() != 0) {
			lpm_seq_trace_fail(LPM_SEQ_DDR_LPSC_DISABLE);
			k3low_lpm_abort();
		} else {
			lpm_seq_trace(LPM_SEQ_DDR_LPSC_DISABLE);
		}

		save_main_pll();
		lpm_seq_trace(LPM_SEQ_SAVE_MAIN_PLL);

		disable_main_pll();
		lpm_seq_trace(LPM_SEQ_DISABLE_MAIN_PLL);

		/* configure the pmic input */
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + PMCTRL_SYS, 0x0U);
		lpm_seq_trace(LPM_SEQ_PMIC_CONFIG);
		dsb();
		isb();

		for (;;)
			wfi();

	} else if (mode == TI_K3_SLEEP_MODE_DEEP_SLEEP) {

		/* Wait for a53_1 to turn off */
		if (lpm_wait_for_secondary_core_down() == false) {
			lpm_seq_trace_fail(LPM_SEQ_SECONDARY_CORE_DOWN);
			k3low_lpm_abort();
		} else {
			lpm_seq_trace(LPM_SEQ_SECONDARY_CORE_DOWN);
		}
		if (save_and_disable_usb_lpsc() != 0) {
			lpm_seq_trace_fail(LPM_SEQ_USB_LPSC_DISABLE);
			k3low_lpm_abort();
		} else {
			lpm_seq_trace(LPM_SEQ_USB_LPSC_DISABLE);
		}

		save_main_pll();
		lpm_seq_trace(LPM_SEQ_SAVE_MAIN_PLL);

		if (k3low_ddr_deep_sleep_suspend_sequence() != 0) {
			lpm_seq_trace_fail(LPM_SEQ_SAVE_DDR_REGS);
			k3low_lpm_abort();
		} else {
			lpm_seq_trace(LPM_SEQ_SAVE_DDR_REGS);
		}

		/*	Disable the LPSCs for DDR */
		if (disable_ddr_lpsc() != 0) {
			lpm_seq_trace_fail(LPM_SEQ_DDR_LPSC_DISABLE);
			k3low_lpm_abort();
		} else {
			lpm_seq_trace(LPM_SEQ_DDR_LPSC_DISABLE);
		}

		disable_main_pll();
		lpm_seq_trace(LPM_SEQ_DISABLE_MAIN_PLL);

		dsb();
		isb();
		lpm_seq_trace(LPM_SEQ_BEFORE_WFI);

		for (;;) {
			wfi();
			lpm_seq_trace_fail(LPM_SEQ_UNEXPECTED_WFI_RETURN);
		}
	} else  {
		for (;;) {
			lpm_seq_trace_fail(LPM_SEQ_INVALID_MODE);
		}
	}
}

int32_t k3low_lpm_set_io_isolation(bool enable)
{
	int32_t ret = -ETIMEDOUT;
	uint32_t reg;

	if (enable) {
		mmio_write_32((WKUP_CTRL_MMR_SEC_5_BASE +
			       WKUP_CTRL_DEEPSLEEP_CTRL),
			      WKUP_CTRL_DEEPSLEEP_CTRL_ENABLE_IO);

		/* Set global wuen */
		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN;
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_0, reg);

		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_1);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN;
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_1, reg);

		/* Set global isoin */
		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL;
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_0, reg);

		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_1);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg | WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL;
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_1, reg);

		/* Wait for wu clock state to be 1 */
		do {
			ret = -ETIMEDOUT;
			reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE +
					   WKUP_CTRL_PMCTRL_IO_0);
			if ((reg & WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) ==
			    WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) {
				ret = 0;
				break;
			}
		} while (1);
		do {
			ret = -ETIMEDOUT;
			reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE +
					   WKUP_CTRL_PMCTRL_IO_1);
			if ((reg & WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) ==
			    WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS) {
				ret = 0;
				break;
			}
		} while (1);
	} else {
		/* Clear global wuen */
		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg & (~WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN);
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_0, reg);

		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_1);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg & (~WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN);
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_1, reg);

		/* Clear global isoin */
		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_0);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg & (~WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL);
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_0, reg);

		reg = mmio_read_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_1);
		reg = reg & WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK;
		reg = reg & (~WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL);
		mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + WKUP_CTRL_PMCTRL_IO_1, reg);

		mmio_write_32((WKUP_CTRL_MMR_SEC_5_BASE +
			       WKUP_CTRL_DEEPSLEEP_CTRL),
			      WKUP_CTRL_DEEPSLEEP_CTRL_DISABLE_IO);

		ret = 0;
	}
	return ret;
}

/**
 * @brief send core resume message to TIFS
 *
 */
__wkupsramfunc static void mailbox_send_message(void)
{
	uint32_t *dst_ptr = (void *)(MAILBOX_TX_START_REGION +
				      MAILBOX_MSG_BUFFER_OFFSET);

	dst_ptr[0] = 0x0;
	dst_ptr[1] = TISCI_MSG_CORE_RESUME;
	dst_ptr[2] = 0x0;
	dst_ptr[3] = 0x0;
	dst_ptr[4] = 0x0;

	mmio_write_32(TI_MAILBOX_TX_BASE + TI_MAILBOX_MSG,
		      (unsigned long)(void *)dst_ptr);
}

__wkupsramfunc void k3low_lpm_resume_c(void)
{
	if (restore_main_pll() != 0) {
		lpm_seq_trace_fail(LPM_SEQ_RESTORE_MAIN_PLL);
		k3low_lpm_abort();
	} else {
		lpm_seq_trace(LPM_SEQ_RESTORE_MAIN_PLL);
	}

	if (enable_ddr_lpsc() != 0) {
		lpm_seq_trace_fail(LPM_SEQ_ENABLE_DDR_LPSC);
		k3low_lpm_abort();
	} else {
		lpm_seq_trace(LPM_SEQ_ENABLE_DDR_LPSC);
	}

	if (k3low_ddr_deep_sleep_resume_sequence() != 0) {
		lpm_seq_trace_fail(LPM_SEQ_RESTORE_DDR_REGS);
		k3low_lpm_abort();
	} else {
		lpm_seq_trace(LPM_SEQ_RESTORE_DDR_REGS);
	}

	if (restore_usb_lpsc() != 0) {
		lpm_seq_trace_fail(LPM_SEQ_ENABLE_USB_LPSC);
		k3low_lpm_abort();
	} else {
		lpm_seq_trace(LPM_SEQ_ENABLE_USB_LPSC);
	}

	mailbox_send_message();
	lpm_seq_trace(LPM_SEQ_MAILBOX_SEND);

	for (;;) {
		wfi();
		lpm_seq_trace(LPM_SEQ_AFTER_WFI_RESUME);
	}
}

void k3low_suspend_to_ram(uint32_t mode)
{
	k3_lpm_jump_to_stub(mode);
}

#ifndef __ASSEMBLER__
IMPORT_SYM(unsigned long, __wkup_sram_start__, WKUP_SRAM_START);
IMPORT_SYM(unsigned long, __wkup_sram_end__, WKUP_SRAM_END);
IMPORT_SYM(unsigned long, __WKUP_SRAM_COPY_START__, WKUP_SRAM_COPY_START);
IMPORT_SYM(unsigned long, __wkup_sram_suspend_entry__, K3_SUSPEND_ENTRY);
#endif

/**
 * @brief function to jump to stub in wkup SRAM
 *
 */
static void k3_lpm_jump_to_stub(uint32_t mode)
{
	uintptr_t jump = (uintptr_t)K3_SUSPEND_ENTRY;
	uintptr_t stack = (uintptr_t)DEVICE_WKUP_SRAM_STACK_BASE;
	uint32_t sctlr;
	/* disable MMU */
	sctlr = (uint32_t)read_sctlr_el3();
	sctlr &= (uint32_t)~SCTLR_EL3_M_BIT;
	write_sctlr_el3((uint64_t)sctlr);

	k3low_lpm_switch_stack(jump, stack, mode);
}

int32_t k3low_lpm_stub_copy_to_sram(void)
{
	uintptr_t sram_base_addr = (uintptr_t)DEVICE_WKUP_SRAM_BASE;
	size_t sram_len = DEVICE_WKUP_SRAM_SIZE;
	void *a53_stub_start = (void *)WKUP_SRAM_COPY_START;
	size_t a53_stub_len = WKUP_SRAM_END - WKUP_SRAM_START;

	if (a53_stub_len > sram_len) {
		ERROR("A53 stub size (%zu) exceeds SRAM size (%zu)\n",
		      a53_stub_len, sram_len);
		return -1;
	}

	/* Copy stub code to SRAM */
	memcpy((void *)sram_base_addr, a53_stub_start, a53_stub_len);
	flush_dcache_range((uint64_t)sram_base_addr, a53_stub_len);

	return 0;
}
