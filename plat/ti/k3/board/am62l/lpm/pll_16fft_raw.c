/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <k3_lpm_timeout.h>
#include <lib/mmio.h>
#include <lpm_trace.h>
#include <plat/common/platform.h>
#include <pll_16fft_raw.h>

/* PLL CTRL Registers */
#define PLLCTRL_RSCTRL_OFFSET			0x0E8U
#define PLLCTRL_RSCTRL_KEY			0x15a69U
#define PLLCTRL_RSISO_OFFSET			0x0F0U
#define PLLCTRL_PLLCTL_OFFSET			0x100U

/* 16FFT PLL Registers */
#define PLL_16FFT_PID_OFFSET                    (0x00U)
#define PLL_16FFT_CFG_OFFSET                    (0x08U)

#define PLL_16FFT_LOCKKEY0_OFFSET               (0x10U)
#define PLL_16FFT_LOCKKEY0_VALUE                (0x68EF3490U)

#define PLL_16FFT_LOCKKEY1_OFFSET               (0x14U)
#define PLL_16FFT_LOCKKEY1_VALUE                (0xD172BC5AU)

#define PLL_16FFT_CTRL_OFFSET                   (0x20U)
#define PLL_16FFT_CTRL_BYPASS_EN                BIT(31)
#define PLL_16FFT_CTRL_BYP_ON_LOCKLOSS          BIT(16)
#define PLL_16FFT_CTRL_PLL_EN                   BIT(15)
#define PLL_16FFT_CTRL_INTL_BYP_EN              BIT(8)
#define PLL_16FFT_CTRL_CLK_4PH_EN               BIT(5)
#define PLL_16FFT_CTRL_CLK_POSTDIV_EN           BIT(4)
#define PLL_16FFT_CTRL_DSM_EN                   BIT(1)
#define PLL_16FFT_CTRL_DAC_EN                   BIT(0)

#define PLL_16FFT_STAT_OFFSET                   (0x24U)
#define PLL_16FFT_STAT_LOCK                     BIT(0)

#define PLL_16FFT_FREQ_CTRL0_OFFSET             (0x30U)
#define PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT   (0U)
#define PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK    ((0xfffU << 0U))

#define PLL_16FFT_FREQ_CTRL1_OFFSET             (0x34U)
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT  (0U)
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK   ((0xffffffU << 0U))
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS   (24U)

#define PLL_16FFT_DIV_CTRL_OFFSET               (0x38U)
#define PLL_16FFT_DIV_CTRL_POST_DIV2_SHIFT      (24U)
#define PLL_16FFT_DIV_CTRL_POST_DIV2_MASK       ((0x7U << 24U))
#define PLL_16FFT_DIV_CTRL_POST_DIV1_SHIFT      (16U)
#define PLL_16FFT_DIV_CTRL_POST_DIV1_MASK       ((0x7U << 16U))
#define PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT        (0U)
#define PLL_16FFT_DIV_CTRL_REF_DIV_MASK         ((0x3fU << 0U))

#define PLL_16FFT_SS_CTRL_OFFSET                0x40U
#define PLL_16FFT_SS_SPREAD_OFFSET              0x44U

#define PLL_16FFT_CAL_CTRL_OFFSET               (0x60U)
#define PLL_16FFT_CAL_CTRL_CAL_EN               BIT(31)
#define PLL_16FFT_CAL_CTRL_FAST_CAL             BIT(20)
#define PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT        (16U)
#define PLL_16FFT_CAL_CTRL_CAL_CNT_MASK         ((0x7U << 16U))
#define PLL_16FFT_CAL_CTRL_CAL_BYP              BIT(15)
#define PLL_16FFT_CAL_CTRL_CAL_IN_SHIFT         (0U)
#define PLL_16FFT_CAL_CTRL_CAL_IN_MASK          ((0xFFFU << 0U))

#define PLL_16FFT_CAL_STAT_OFFSET               (0x64U)
#define PLL_16FFT_CAL_STAT_CAL_LOCK             BIT(31)
#define PLL_16FFT_CAL_STAT_LOCK_CNT_SHIFT       (16U)
#define PLL_16FFT_CAL_STAT_LOCK_CNT_MASK        ((0xFU << 16U))
#define PLL_16FFT_CAL_STAT_CAL_OUT_SHIFT        (0U)
#define PLL_16FFT_CAL_STAT_CAL_OUT_MASK         ((0xFFFU))

#define PLL_16FFT_HSDIV_CTRL_OFFSET             (0x80U)
#define PLL_16FFT_HSDIV_CTRL_RESET              BIT(31)
#define PLL_16FFT_HSDIV_CTRL_CLKOUT_EN          BIT(15)
#define PLL_16FFT_HSDIV_CTRL_SYNC_DIS           BIT(8)
#define PLL_16FFT_HSDIV_CTRL_HSDIV_SHIFT        (0U)
#define PLL_16FFT_HSDIV_CTRL_HSDIV_MASK         ((0x7fU << 0U))
/* TODO: Recheck the timeout value */
#define PLL_16FFT_RAW_LOCK_TIMEOUT		(10000U)
#define PLL_16FFT_CAL_LOCK_TIMEOUT		(435000U)

__wkupsramfunc void pll_cal_option3(uint32_t pll_base)
{
	uint32_t cal;

	cal = mmio_read_32(pll_base + PLL_16FFT_CAL_CTRL_OFFSET);

	/* Enable fast cal mode */
	cal |= PLL_16FFT_CAL_CTRL_FAST_CAL;

	/* Disable calibration bypass */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_BYP;

	/* Set CALCNT to 2 */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_CNT_MASK;
	cal |= 2U << PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT;

	/* Set CAL_IN to 0 */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_IN_MASK;

	/* Note this register does not readback the written value. */
	mmio_write_32(pll_base + PLL_16FFT_CAL_CTRL_OFFSET, cal);

	/* Wait 1us before enabling the CAL_EN field */
	k3_lpm_delay_1us();

	cal = mmio_read_32(pll_base + PLL_16FFT_CAL_CTRL_OFFSET);

	/* Enable calibration for FRACF */
	cal |= PLL_16FFT_CAL_CTRL_CAL_EN;

	/* Note this register does not readback the written value. */
	mmio_write_32(pll_base + PLL_16FFT_CAL_CTRL_OFFSET, cal);
}

/*
 * Enable or disable the PLL.
 *
 * @param pll_base Base address of the PLL.
 * @param enable Enable (TRUE) or disable (FALSE) the PLL.
 */
__wkupsramfunc void pll_enable(uint32_t pll_base, bool enable)
{
	uint32_t ctrl;

	ctrl = mmio_read_32(pll_base + PLL_16FFT_CTRL_OFFSET);
	if (enable) {
		ctrl |= PLL_16FFT_CTRL_PLL_EN;
	} else {
		ctrl &= ~PLL_16FFT_CTRL_PLL_EN;
	}
	mmio_write_32(pll_base + PLL_16FFT_CTRL_OFFSET, ctrl);
}

__wkupsramfunc void pll_bypass(struct pll_raw_data *pll, bool enable)
{
	uint32_t ctrl;

	ctrl = mmio_read_32(pll->base + PLL_16FFT_CTRL_OFFSET);

	if (enable) {
		ctrl |= PLL_16FFT_CTRL_BYPASS_EN;
	} else {
		ctrl &= ~PLL_16FFT_CTRL_BYPASS_EN;
	}

	mmio_write_32(pll->base + PLL_16FFT_CTRL_OFFSET, ctrl);
}


__wkupsramfunc int32_t pll_restore(struct pll_raw_data *pll)
{
	uint8_t i;
	uint32_t ctrl, cfg, cal;
	int32_t ret = 0;
	uint32_t timeout;

	/* Unlock write access */
	mmio_write_32(pll->base + PLL_16FFT_LOCKKEY0_OFFSET, PLL_16FFT_LOCKKEY0_VALUE);
	mmio_write_32(pll->base + PLL_16FFT_LOCKKEY1_OFFSET, PLL_16FFT_LOCKKEY1_VALUE);

	/* Make sure that PLL is in bypass mode */
	pll_bypass(pll, true);

	/* Make sure that PLL is disabled */
	pll_enable(pll->base, false);

	/* Restore the divider values */
	mmio_write_32(pll->base + PLL_16FFT_FREQ_CTRL0_OFFSET, pll->freq_ctrl0);
	mmio_write_32(pll->base + PLL_16FFT_FREQ_CTRL1_OFFSET, pll->freq_ctrl1);
	mmio_write_32(pll->base + PLL_16FFT_DIV_CTRL_OFFSET, pll->div_ctrl);

	ctrl = mmio_read_32(pll->base + PLL_16FFT_CTRL_OFFSET);

	/* Always bypass if we lose lock */
	ctrl |= PLL_16FFT_CTRL_BYP_ON_LOCKLOSS;

	/* Prefer glitchless bypass */
	ctrl &= ~PLL_16FFT_CTRL_INTL_BYP_EN;

	/* Always enable output if PLL */
	ctrl |= PLL_16FFT_CTRL_CLK_POSTDIV_EN;

	/* Currently unused by all PLLs */
	ctrl &= ~PLL_16FFT_CTRL_CLK_4PH_EN;

	/* Make sure we have fractional support if required */
	if (pll->freq_ctrl1 != 0U) {
		ctrl |= PLL_16FFT_CTRL_DSM_EN;
		ctrl |= PLL_16FFT_CTRL_DAC_EN;
	} else {
		ctrl &= ~PLL_16FFT_CTRL_DSM_EN;
		ctrl &= ~PLL_16FFT_CTRL_DAC_EN;
	}

	mmio_write_32(pll->base + PLL_16FFT_CTRL_OFFSET, ctrl);

	/* Program all HSDIV outputs */
	cfg = mmio_read_32(pll->base + PLL_16FFT_CFG_OFFSET);
	for (i = 0U; i < 16U; i++) {
		/* Program HSDIV output if present */
		if (((1U << (i + 16U)) & cfg) != 0U) {
			mmio_write_32(pll->base + PLL_16FFT_HSDIV_CTRL_OFFSET + (i * 0x4U), pll->hsdiv[i]);

		}
	}

	if (pll->freq_ctrl1 == 0UL) {
		/* Enable Calibration in Integer mode */
		pll_cal_option3(pll->base);
	} else {
		/* Disable Calibration in Fractional mode */
		cal = mmio_read_32(pll->base + PLL_16FFT_CAL_CTRL_OFFSET);
		cal &=   ~PLL_16FFT_CAL_CTRL_CAL_EN;
		mmio_write_32(pll->base + PLL_16FFT_CAL_CTRL_OFFSET, cal);
		timeout = PLL_16FFT_CAL_LOCK_TIMEOUT;
		while ((timeout > 0U) && (((mmio_read_32(pll->base + PLL_16FFT_CAL_STAT_OFFSET) & (PLL_16FFT_CAL_STAT_CAL_LOCK)) == 0U) == false)) {
			--timeout;
			k3_lpm_delay_1us();
		}
		if (timeout == 0U) {
			ret = -ETIMEDOUT;
		}
	}

	if (ret == 0) {
		/*
		 * Wait at least 1 ref cycle before enabling PLL.
		 * Minimum VCO input frequency is 5MHz, therefore maximum
		 * wait time for 1 ref clock is 0.2us.
		 */
		k3_lpm_delay_1us();

		/* Make sure PLL is enabled */
		pll_enable(pll->base, true);

		/* Wait for the PLL lock */
		timeout = PLL_16FFT_RAW_LOCK_TIMEOUT;
		while ((timeout > 0U) && (((mmio_read_32(pll->base + PLL_16FFT_STAT_OFFSET) & (PLL_16FFT_STAT_LOCK)) == 1U) == false)) {
			--timeout;
			k3_lpm_delay_1us();
		}
		if (timeout == 0U) {
			ret = -ETIMEDOUT;
		}
	}

	/* In case of cal lock failure, operate without calibration */
	if (ret != 0) {
		ret = 0;
		/* Disable PLL */
		pll_enable(pll->base, false);

		/* Disable Calibration */
		cal = mmio_read_32(pll->base + PLL_16FFT_CAL_CTRL_OFFSET);
		cal &= ~PLL_16FFT_CAL_CTRL_CAL_EN;
		mmio_write_32(pll->base + PLL_16FFT_CAL_CTRL_OFFSET, cal);
		timeout = PLL_16FFT_CAL_LOCK_TIMEOUT;
		while ((timeout > 0U) && (((mmio_read_32(pll->base + PLL_16FFT_CAL_STAT_OFFSET) & (PLL_16FFT_CAL_STAT_CAL_LOCK)) == 0U) == false)) {
			--timeout;
		}
		if (timeout == 0U) {
			ret = -ETIMEDOUT;
		}

		if (ret == 0) {
			/* Enable PLL */
			pll_enable(pll->base, true);

			/* Wait for PLL Lock */
			timeout = PLL_16FFT_RAW_LOCK_TIMEOUT;
			while ((timeout > 0U) && (((mmio_read_32(pll->base + PLL_16FFT_STAT_OFFSET) & (PLL_16FFT_STAT_LOCK)) == 1U) == false)) {
				--timeout;
			}
			if (timeout == 0U) {
				ret = -ETIMEDOUT;
			}
		}
	}

	if (ret == 0) {
		/*
		 * The PLL_CTRL BYPASS_EN should be cleared to make sure the clocks are running
		 * at the locked PLL frequency.
		 */
		pll_bypass(pll, false);
	}
	return ret;
}

__wkupsramfunc void pll_save(struct pll_raw_data *pll)
{
	uint8_t i;
	uint32_t cfg;

	pll->freq_ctrl0 = mmio_read_32(pll->base + PLL_16FFT_FREQ_CTRL0_OFFSET);
	pll->freq_ctrl1 = mmio_read_32(pll->base + PLL_16FFT_FREQ_CTRL1_OFFSET);
	pll->div_ctrl = mmio_read_32(pll->base + PLL_16FFT_DIV_CTRL_OFFSET);

	/* Enable all HSDIV outputs */
	cfg = mmio_read_32(pll->base + PLL_16FFT_CFG_OFFSET);
	for (i = 0U; i < 16U; i++) {
		/* Read HSDIV output if present */
		if (((1U << (i + 16U)) & cfg) != 0U) {
			pll->hsdiv[i] = mmio_read_32(pll->base + PLL_16FFT_HSDIV_CTRL_OFFSET + (i * 0x4U));
		}
	}
}

__wkupsramfunc void pll_disable(struct pll_raw_data *pll)
{
	/* Select reference clk for PLL and HSDIV clk outputs */
	pll_bypass(pll, true);
	pll_bypass_hsdivs(pll);

	/* Disable the PLL */
	pll_enable(pll->base, false);
}

__wkupsramfunc void pll_bypass_hsdivs(struct pll_raw_data *pll)
{
	uint8_t i;
	uint32_t cfg;

	cfg = mmio_read_32(pll->base + PLL_16FFT_CFG_OFFSET);
	for (i = 0U; i < 16U; i++) {
		/* Read HSDIV output if present */
		if (((1U << (i + 16U)) & cfg) != 0U) {
			mmio_write_32(pll->base + PLL_16FFT_HSDIV_CTRL_OFFSET + (i * 0x4U), 0x8001);
		}
	}
}
