/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
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

#define PLL_16FFT_HSDIV_CTRL_OFFSET             (0x80U)
#define PLL_16FFT_HSDIV_CTRL_RESET              BIT(31)
#define PLL_16FFT_HSDIV_CTRL_CLKOUT_EN          BIT(15)
#define PLL_16FFT_HSDIV_CTRL_SYNC_DIS           BIT(8)
#define PLL_16FFT_HSDIV_CTRL_HSDIV_SHIFT        (0U)
#define PLL_16FFT_HSDIV_CTRL_HSDIV_MASK         ((0x7fU << 0U))
/* TODO: Recheck the timeout value */
#define PLL_16FFT_RAW_LOCK_TIMEOUT                              10000

/* TODO: enable calibration support for PLL */
__wkupsramfunc int32_t pll_restore(struct pll_raw_data *pll)
{
	uint8_t i;
	uint32_t ctrl, cfg;
	int32_t ret = 0;
	uint32_t pll_stat;
	uint32_t timeout;

	/* Unlock write access */
	mmio_write_32(pll->base + PLL_16FFT_LOCKKEY0_OFFSET, PLL_16FFT_LOCKKEY0_VALUE);
	mmio_write_32(pll->base + PLL_16FFT_LOCKKEY1_OFFSET, PLL_16FFT_LOCKKEY1_VALUE);

	ctrl = mmio_read_32(pll->base + PLL_16FFT_CTRL_OFFSET);

	/* Always bypass if we lose lock */
	ctrl |= PLL_16FFT_CTRL_BYP_ON_LOCKLOSS;

	/* Prefer glitchless bypass */
	if ((ctrl & PLL_16FFT_CTRL_INTL_BYP_EN) == PLL_16FFT_CTRL_INTL_BYP_EN) {
		ctrl |= PLL_16FFT_CTRL_BYPASS_EN;
		ctrl &= ~PLL_16FFT_CTRL_INTL_BYP_EN;
	}

	/* Always enable output if PLL */
	ctrl |= PLL_16FFT_CTRL_CLK_POSTDIV_EN;

	/* Currently unused by all PLLs */
	ctrl &= ~PLL_16FFT_CTRL_CLK_4PH_EN;

	/* Make sure we have fractional support if required */
	if (pll->freq_ctrl1 != 0U) {
		ctrl |= PLL_16FFT_CTRL_DSM_EN;
	} else {
		ctrl &= ~PLL_16FFT_CTRL_DSM_EN;
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

	mmio_write_32(pll->base + PLL_16FFT_FREQ_CTRL0_OFFSET, pll->freq_ctrl0);
	mmio_write_32(pll->base + PLL_16FFT_FREQ_CTRL1_OFFSET, pll->freq_ctrl1);
	mmio_write_32(pll->base + PLL_16FFT_DIV_CTRL_OFFSET, pll->div_ctrl);

	/* Make sure PLL is enabled */
	ctrl = mmio_read_32(pll->base + PLL_16FFT_CTRL_OFFSET);
	if ((ctrl & PLL_16FFT_CTRL_PLL_EN) == 0U) {
		ctrl |= PLL_16FFT_CTRL_PLL_EN;
	}

	if ((ctrl & PLL_16FFT_CTRL_BYPASS_EN) != 0U) {
		/*
		 * The PLL_CTRL BYPASS_EN should be cleared to make sure the clocks are running
		 * at the locked PLL frequency.
		 */
		ctrl &= ~PLL_16FFT_CTRL_BYPASS_EN;
		mmio_write_32(pll->base + PLL_16FFT_CTRL_OFFSET, ctrl);

		timeout = PLL_16FFT_RAW_LOCK_TIMEOUT;
		do {
			timeout--;
			pll_stat = mmio_read_32(pll->base + PLL_16FFT_STAT_OFFSET) & ((uint32_t)PLL_16FFT_STAT_LOCK);
		} while ((timeout > 0U) && (pll_stat != 1U));
		if (timeout == 0U) {
			ret = -1;
			lpm_seq_trace_fail(0xF2);

		}
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
	uint32_t ctrl;

	/* Select reference clk for PLL and HSDIV clk outputs */
	pll_bypass(pll);
	pll_bypass_hsdivs(pll);

	/* Disable the PLL */
	ctrl = mmio_read_32(pll->base + PLL_16FFT_CTRL_OFFSET);
	ctrl &= ~PLL_16FFT_CTRL_PLL_EN;
	mmio_write_32(pll->base + PLL_16FFT_CTRL_OFFSET, ctrl);
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

__wkupsramfunc void pll_bypass(struct pll_raw_data *pll)
{
	uint32_t ctrl;

	ctrl = mmio_read_32(pll->base + PLL_16FFT_CTRL_OFFSET);
	ctrl |= PLL_16FFT_CTRL_BYPASS_EN;
	mmio_write_32(pll->base + PLL_16FFT_CTRL_OFFSET, ctrl);
}

__wkupsramfunc void pll_unbypass(struct pll_raw_data *pll)
{
	uint32_t ctrl;

	ctrl = mmio_read_32(pll->base + PLL_16FFT_CTRL_OFFSET);
	ctrl &= (~PLL_16FFT_CTRL_BYPASS_EN);
	mmio_write_32(pll->base + PLL_16FFT_CTRL_OFFSET, ctrl);
}
