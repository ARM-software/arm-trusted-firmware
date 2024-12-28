/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_pll_16fft.h>
#include <device.h>
#include <lib/container_of.h>
#include <lib/io.h>
#include <types/array_size.h>
#include <clk_pll.h>
#include <clk_mux.h>
#include <clk_div.h>
#include <div64.h>
#include <compiler.h>
#include <lib/trace.h>
#include <drivers/delay_timer.h>
#include <osal/osal_clock_user.h>

#define PLL_16FFT_PID	(idx)			((0x1000UL * (idx)) + 0x00UL)
#define PLL_16FFT_CFG(idx)			((0x1000UL * (idx)) + 0x08UL)
#define PLL_16FFT_CFG_HSDIV_PRSNC(n)		BIT((n) + 16UL)
#define PLL_16FFT_CFG_SSM			BIT(11)
#define PLL_16FFT_CFG_SSM_WVTBL			BIT(8)
#define PLL_16FFT_CFG_PLL_TYPE_SHIFT		(0UL)
#define PLL_16FFT_CFG_PLL_TYPE_MASK		(0x3UL << 0UL)
#define PLL_16FFT_CFG_PLL_TYPE_FRAC2		0UL
#define PLL_16FFT_CFG_PLL_TYPE_FRACF		1UL
#define PLL_16FFT_CFG_PLL_TYPE_DESKEW		2UL

#define PLL_16FFT_LOCKKEY0(idx)			((0x1000UL * (idx)) + 0x10UL)
#define PLL_16FFT_LOCKKEY0_VALUE		0x68EF3490UL

#define PLL_16FFT_LOCKKEY1(idx)			((0x1000UL * (idx)) + 0x14UL)
#define PLL_16FFT_LOCKKEY1_VALUE		0xD172BC5AUL

#define PLL_16FFT_CTRL(idx)			((0x1000UL * (idx)) + 0x20UL)
#define PLL_16FFT_CTRL_BYPASS_EN		BIT(31)
#define PLL_16FFT_CTRL_BYP_ON_LOCKLOSS		BIT(16)
#define PLL_16FFT_CTRL_PLL_EN			BIT(15)
#define PLL_16FFT_CTRL_INTL_BYP_EN		BIT(8)
#define PLL_16FFT_CTRL_CLK_4PH_EN		BIT(5)
#define PLL_16FFT_CTRL_CLK_POSTDIV_EN		BIT(4)
#define PLL_16FFT_CTRL_DSM_EN			BIT(1)
#define PLL_16FFT_CTRL_DAC_EN			BIT(0)

#define PLL_16FFT_STAT(idx)			((0x1000UL * (idx)) + 0x24UL)
#define PLL_16FFT_STAT_LOCK			BIT(0)

#define PLL_16FFT_FREQ_CTRL0(idx)		((0x1000UL * (idx)) + 0x30UL)
#define PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT	0UL
#define PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK	(0xfffUL << 0UL)

#define PLL_16FFT_FREQ_CTRL1(idx)		((0x1000UL * (idx)) + 0x34UL)
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT	0UL
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK	(0xffffffUL << 0UL)
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS	24UL

#define PLL_16FFT_DIV_CTRL(idx)			((0x1000UL * (idx)) + 0x38UL)
#define PLL_16FFT_DIV_CTRL_POST_DIV2_SHIFT	24UL
#define PLL_16FFT_DIV_CTRL_POST_DIV2_MASK	(0x7UL << 24UL)
#define PLL_16FFT_DIV_CTRL_POST_DIV1_SHIFT	16UL
#define PLL_16FFT_DIV_CTRL_POST_DIV1_MASK	(0x7UL << 16UL)
#define PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT	0UL
#define PLL_16FFT_DIV_CTRL_REF_DIV_MASK		(0x3fUL << 0UL)

#define PLL_16FFT_SS_CTRL(idx)			((0x1000UL * (idx)) + 0x40UL)
#define PLL_16FFT_SS_SPREAD(idx)		((0x1000UL * (idx)) + 0x44UL)

#define PLL_16FFT_CAL_CTRL(idx)			((0x1000UL * (idx)) + 0x60UL)
#define PLL_16FFT_CAL_CTRL_CAL_EN		BIT(31)
#define PLL_16FFT_CAL_CTRL_FAST_CAL		BIT(20)
#define PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT	16UL
#define PLL_16FFT_CAL_CTRL_CAL_CNT_MASK		(0x7UL << 16UL)
#define PLL_16FFT_CAL_CTRL_CAL_BYP		BIT(15)
#define PLL_16FFT_CAL_CTRL_CAL_IN_SHIFT		0U
#define PLL_16FFT_CAL_CTRL_CAL_IN_MASK		(0xFFFU << 0U)

#define PLL_16FFT_CAL_STAT(idx)			((0x1000U * (idx)) + 0x64U)
#define PLL_16FFT_CAL_STAT_CAL_LOCK		BIT(31)
#define PLL_16FFT_CAL_STAT_LOCK_CNT_SHIFT	16U
#define PLL_16FFT_CAL_STAT_LOCK_CNT_MASK	(0xFU << 16U)
#define PLL_16FFT_CAL_STAT_CAL_OUT_SHIFT	0U
#define PLL_16FFT_CAL_STAT_CAL_OUT_MASK		(0xFFFU)

#define PLL_16FFT_HSDIV_CTRL(idx, n)		((0x1000UL * (idx)) + 0x80UL + ((n) * 4UL))
#define PLL_16FFT_HSDIV_CTRL_RESET		BIT(31)
#define PLL_16FFT_HSDIV_CTRL_CLKOUT_EN		BIT(15)
#define PLL_16FFT_HSDIV_CTRL_SYNC_DIS		BIT(8)
#define PLL_16FFT_HSDIV_CTRL_HSDIV_SHIFT	0UL
#define PLL_16FFT_HSDIV_CTRL_HSDIV_MASK		(0x7fUL << 0UL)

static bool pll_16fft_pllm_valid(struct clk *clock_ptr UNUSED, uint32_t pllm, bool is_frac);

static int32_t pll_16fft_bin(struct clk *clock_ptr UNUSED, uint32_t plld UNUSED,
			     uint32_t pllm UNUSED, bool is_frac, uint32_t clkod UNUSED);

static uint32_t pll_16fft_vco_fitness(struct clk *clock_ptr UNUSED, uint32_t vco,
				      bool is_frac UNUSED);

static bool pll_16fft_clkod_valid(struct clk *clock_ptr UNUSED, uint32_t clkod);

static const struct pll_data pll_16fft_raw_data = {
	.plld_max	= 1U,
	.pllm_max	= 640U,
	.pllfm_bits	= (uint32_t) PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS,
	.clkod_max	= 1U,
	.pllm_valid	= pll_16fft_pllm_valid,
	.bin		= pll_16fft_bin,
	.vco_fitness	= pll_16fft_vco_fitness,
};


static const struct pll_data pll_16fft_postdiv_data = {
	.plld_max	= 1U,
	.pllm_max	= 640U,
	.pllfm_bits	= (uint32_t) PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS,
	.clkod_max	= 49U,
	.pllm_valid	= pll_16fft_pllm_valid,
	.clkod_valid	= pll_16fft_clkod_valid,
	.bin		= pll_16fft_bin,
	.vco_fitness	= pll_16fft_vco_fitness,
};


static const struct pll_data pll_16fft_hsdiv_data = {
	.plld_max	= 1U,
	.pllm_max	= 640U,
	.pllfm_bits	= (uint32_t) PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS,
	.clkod_max	= 128U,
	.pllm_valid	= pll_16fft_pllm_valid,
	.bin		= pll_16fft_bin,
	.vco_fitness	= pll_16fft_vco_fitness,
};

#if defined(CONFIG_CLK_PLL_16FFT_FRACF_CALIBRATION)
/*
 * \brief Implement the option 3 PLL calibration method.
 *
 * This calibration method calibrates the PLL and allows a calibration value
 * to be obtained when calibration is complete. It utilizes FASTCAL mode
 * to obtain a calibration value.
 *
 * \param pll The PLL data associated with this FRACF PLL.
 */
static void clk_pll_16fft_cal_option3(const struct clk_data_pll_16fft *pll)
{
	uint32_t cal;

	cal = readl(pll->base + (uint32_t) PLL_16FFT_CAL_CTRL(pll->idx));

	/* Enable fast cal mode */
	cal |= PLL_16FFT_CAL_CTRL_FAST_CAL;

	/* Disable calibration bypass */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_BYP;

	/* Set CALCNT to 2 */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_CNT_MASK;
	cal |= (uint32_t) 2U << (uint32_t) PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT;

	/* Set CAL_IN to 0 */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_IN_MASK;

	/* Note this register does not readback the written value. */
	ti_clk_writel(cal, (uint32_t) pll->base + (uint32_t) PLL_16FFT_CAL_CTRL(pll->idx));

	/* Wait 1us before enabling the CAL_EN field */
	osal_delay(1UL); /* Wait 1us */

	cal = readl(pll->base + (uint32_t) PLL_16FFT_CAL_CTRL(pll->idx));

	/* Enable calibration for FRACF */
	cal |= PLL_16FFT_CAL_CTRL_CAL_EN;

	/* Note this register does not readback the written value. */
	ti_clk_writel(cal, (uint32_t) pll->base + (uint32_t) PLL_16FFT_CAL_CTRL(pll->idx));
}

static void clk_pll_16fft_disable_cal(const struct clk_data_pll_16fft *pll)
{
	uint32_t cal, stat;

	cal = readl(pll->base + (uint32_t) PLL_16FFT_CAL_CTRL(pll->idx));
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_EN;
	/* Note this register does not readback the written value. */
	ti_clk_writel(cal, (uint32_t) pll->base + (uint32_t) PLL_16FFT_CAL_CTRL(pll->idx));
	do {
		stat = readl(pll->base + (uint32_t) PLL_16FFT_CAL_STAT(pll->idx));
	} while ((stat & PLL_16FFT_CAL_STAT_CAL_LOCK) != 0U);
}
#else
static void clk_pll_16fft_cal_option3(const struct clk_data_pll_16fft *pll __maybe_unused)
{

}
static void clk_pll_16fft_disable_cal(const struct clk_data_pll_16fft *pll __maybe_unused)
{

}
#endif

/*
 * \brief Check if the pllm value is valid
 *
 * \param clk The PLL clock.
 * \param pllm The multiplier value
 *
 * \return true if pllm value is valid, false otherwise
 */
static bool pll_16fft_pllm_valid(struct clk *clock_ptr UNUSED, uint32_t pllm, bool is_frac)
{
	bool ret = true;

	if (is_frac) {
		if ((pllm < 20UL) || (pllm > 320UL)) {
			ret = false;
		}
	} else {
		if ((pllm < 16UL) || (pllm > 640UL)) {
			ret = false;
		}
	}
	return ret;
}

/* Prefer non-fractional configuration if possible */
static int32_t pll_16fft_bin(struct clk *clock_ptr UNUSED, uint32_t plld UNUSED,
			     uint32_t pllm UNUSED, bool is_frac, uint32_t clkod UNUSED)
{
	int32_t ret;

	if (is_frac) {
		ret = 0;
	} else {
		ret = 1;
	}

	return ret;
}

/* Prefer higher VCO frequencies */
static uint32_t pll_16fft_vco_fitness(struct clk *clock_ptr UNUSED, uint32_t vco,
				      bool is_frac UNUSED)
{
	return vco;
}

/*
 * \brief Check if the PLL VCO/DCO is locked.
 *
 * \param clk The PLL clock.
 *
 * \return true if VCO/DCO is locked, false otherwise
 */
static bool clk_pll_16fft_check_lock(const struct clk_data_pll_16fft *pll)
{
	uint32_t stat;

	stat = readl(pll->base + (uint32_t) PLL_16FFT_STAT(pll->idx));
	return (stat & PLL_16FFT_STAT_LOCK) != 0U;
}

static int32_t clk_pll_16fft_enable_pll(const struct clk_data_pll_16fft *pll)
{
	uint32_t ctrl;
	int32_t err = SUCCESS;

	ctrl = readl(pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));

	if ((ctrl & PLL_16FFT_CTRL_PLL_EN) == 0U) {
		ctrl |= PLL_16FFT_CTRL_PLL_EN;
		ti_clk_writel(ctrl, pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));
		if (err == SUCCESS) {
			osal_delay(1UL); /* Wait 1us */
		}
	}

	return err;
}

static int32_t clk_pll_16fft_disable_pll(const struct clk_data_pll_16fft *pll)
{
	uint32_t ctrl;
	int32_t err = SUCCESS;

	ctrl = readl(pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));

	if ((ctrl & PLL_16FFT_CTRL_PLL_EN) != 0U) {
		ctrl &= ~PLL_16FFT_CTRL_PLL_EN;
		ti_clk_writel(ctrl, pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));
		if (err == SUCCESS) {
			osal_delay(1UL); /* Wait 1us */
		}
	}

	return err;
}

#if defined(CONFIG_CLK_PLL_16FFT_FRACF_CALIBRATION)
/*
 * \brief Check if the PLL deskew calibration is complete.
 *
 * \param clk The PLL clock.
 *
 * \return true if deskew calibration is complete, false otherwise
 */
static bool clk_pll_16fft_check_cal_lock(const struct clk_data_pll_16fft *pll)
{
	uint32_t stat;

	stat = readl(pll->base + (uint32_t) PLL_16FFT_CAL_STAT(pll->idx));
	return (stat & PLL_16FFT_CAL_STAT_CAL_LOCK) != 0U;
}
#endif

static bool clk_pll_16fft_wait_for_lock(struct clk *clock_ptr)
{
	const struct clk_data *clock_data;
	const struct clk_data_pll_16fft *pll;
	const struct clk_data_pll *data_pll;
	uint32_t i;
	bool success;

#if defined(CONFIG_CLK_PLL_16FFT_FRACF_CALIBRATION)
	uint32_t freq_ctrl1;
	uint32_t pllfm;
#endif

	clock_data = clk_get_data(clock_ptr);
	data_pll = container_of(clock_data->data, const struct clk_data_pll,
				data);
	pll = container_of(data_pll, const struct clk_data_pll_16fft,
			   data_pll);


	/*
	 * Minimum VCO input freq is 5MHz, and the longest a lock should
	 * be consider to be timed out after 750 cycles. Be conservative
	 * and assume each loop takes 10 cycles and we run at a
	 * max of 1GHz. That gives 15000 loop cycles. We may end up waiting
	 * longer than necessary for timeout, but that should be ok.
	 */
	success = false;
	for (i = 0U; i < (150U * 100U); i++) {
		if (clk_pll_16fft_check_lock(pll)) {
			success = true;
			break;
		}
	}

#if defined(CONFIG_CLK_PLL_16FFT_FRACF_CALIBRATION)
	{
		/* Disable calibration in the fractional mode of the FRACF PLL based on data
		 * from silicon and simulation data.
		 */
		freq_ctrl1 = readl(pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL1(pll->idx));
		pllfm = freq_ctrl1 & PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK;
		pllfm >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT;
		uint32_t pll_type;
		uint32_t cfg;

		cfg = readl(pll->base + (uint32_t) PLL_16FFT_CFG(pll->idx));
		pll_type = (cfg & PLL_16FFT_CFG_PLL_TYPE_MASK) >> PLL_16FFT_CFG_PLL_TYPE_SHIFT;

		cal = readl(pll->base + (uint32_t) PLL_16FFT_CAL_CTRL(pll->idx));
		cal_en = (cal & PLL_16FFT_CAL_CTRL_CAL_EN);

		if (success &&
		    (pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF) && (pllfm == 0UL) &&
		    (cal_en == 1UL)) {
			/*
			 * Wait for calibration lock.
			 *
			 * Lock should occur within:
			 *
			 *	170 * 2^(5+CALCNT) / PFD
			 *	21760 / PFD
			 *
			 * CALCNT = 2, PFD = 5-50MHz. This gives a range of 0.435mS to
			 * 4.35mS depending on PFD frequency.
			 *
			 * Be conservative and assume each loop takes 10 cycles and we run at a
			 * max of 1GHz. That gives 435000 loop cycles. We may end up waiting
			 * longer than necessary for timeout, but that should be ok.
			 *
			 * The recommend timeout for CALLOCK to go high is 4.35 ms
			 */
			success = false;
			for (i = 0U; i < (4350U * 100U); i++) {
				if (clk_pll_16fft_check_cal_lock(pll)) {
					success = true;
					break;
				}
			}

			/* In case of cal lock failure, operate without calibration */
			if (success != true) {
				success = true;
				/* Disable PLL */
				err = clk_pll_16fft_disable_pll(pll);
				if (err != SUCCESS) {
					success = false;
				}

				if (success == true) {
					/* Disable Calibration */
					clk_pll_16fft_disable_cal(pll);

					/* Enable PLL */
					err = clk_pll_16fft_enable_pll(pll);
					if (err != SUCCESS) {
						success = false;
					}
				}

				if (success == true) {
					/* Wait for PLL Lock */
					for (i = 0U; i < (150U * 100U); i++) {
						if (clk_pll_16fft_check_lock(pll)) {
							success = true;
							break;
						}
					}
				}
			}
		}
	}
#endif
	return success;
}

/*
 * \brief Query bypass state of PLL
 *
 * \param clk The PLL clock.
 *
 * \return true if PLL is in bypass.
 */
static bool clk_pll_16fft_is_bypass(const struct clk_data_pll_16fft *pll)
{
	bool ret = false;
	uint32_t ctrl;

	/* IDLE Bypass */
	ctrl = readl(pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));
	ret = (ctrl & PLL_16FFT_CTRL_BYPASS_EN) != 0U;

	return ret;
}

static int32_t clk_pll_16fft_bypass(struct clk *clock_ptr, bool bypass)
{
	const struct clk_data *clock_data;
	const struct clk_data_pll_16fft *pll;
	const struct clk_data_pll *data_pll;
	uint32_t ctrl;

	clock_data = clk_get_data(clock_ptr);
	data_pll = container_of(clock_data->data, const struct clk_data_pll,
				data);
	pll = container_of(data_pll, const struct clk_data_pll_16fft,
			   data_pll);

	ctrl = readl(pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));
	if (bypass) {
		/* Enable bypass */
		ctrl |= PLL_16FFT_CTRL_BYPASS_EN;
	} else {
		/* Disable bypass */
		ctrl &= ~PLL_16FFT_CTRL_BYPASS_EN;
	}
	ti_clk_writel(ctrl, pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));
	return SUCCESS;
}

static uint32_t clk_pll_16fft_get_freq_internal(struct clk *clock_ptr, uint32_t clkod)
{
	const struct clk_data *clock_data = clk_get_data(clock_ptr);
	const struct clk_data_pll_16fft *pll;
	const struct clk_data_pll *data_pll;
	uint32_t ret = 0U;
	uint32_t freq_ctrl0;
	uint32_t freq_ctrl1;
	uint32_t div_ctrl;
	uint32_t pllm;
	uint32_t m_frac_mult;
	uint32_t plld;
	uint32_t clkod_plld;

	data_pll = container_of(clock_data->data, const struct clk_data_pll,
				data);
	pll = container_of(data_pll, const struct clk_data_pll_16fft,
			   data_pll);
	freq_ctrl0 = readl(pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL0(pll->idx));
	freq_ctrl1 = readl(pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL1(pll->idx));
	div_ctrl = readl(pll->base + (uint32_t) PLL_16FFT_DIV_CTRL(pll->idx));

	pllm = freq_ctrl0 & PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK;
	pllm >>= PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT;

	m_frac_mult = freq_ctrl1 & PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK;
	m_frac_mult >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT;

	plld = div_ctrl & PLL_16FFT_DIV_CTRL_REF_DIV_MASK;
	plld >>= PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT;

	clkod_plld = clkod * plld;

	if (clkod_plld != 0UL) {
		uint32_t parent_freq_hz;
		uint64_t ret64;
		uint64_t rem;

		/* Calculate non-fractional part */
		parent_freq_hz = clk_get_parent_freq(clock_ptr);
		ret64 = (uint64_t) ((parent_freq_hz / clkod_plld) * pllm);
		rem = (uint64_t) ((parent_freq_hz % clkod_plld) * pllm);
		if (rem > (uint64_t) ULONG_MAX) {
			/*
			 * Remainder should always fit within 32 bits.
			 * We add this in case of a programming error
			 * or unexpected input.
			 *
			 * clkod_plld - 12 bits
			 * pllm -	12 bits
			 * total -	24 bits (should not need div64)
			 */
			ret64 += pm_div64(&rem, clkod_plld);
		} else {
			ret64 += rem / (uint64_t) clkod_plld;
			rem = rem % (uint64_t) clkod_plld;
		}

		if (m_frac_mult != 0UL) {
			uint64_t fret;	     /* Fraction return value */
			uint64_t frem;	     /* Fraction remainder */
			const uint32_t mask = (1UL << PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS) -
				1UL;

			/* Calculate fractional component of frequency */
			fret = ((uint64_t) (parent_freq_hz / clkod_plld)) * m_frac_mult;
			frem = ((uint64_t) (parent_freq_hz % clkod_plld)) * m_frac_mult;
			if (frem > (uint64_t) ULONG_MAX) {
				/*
				 * m_frac_mult - 24 bits
				 * clkod_plld - 12 bits
				 * total - 36 bits
				 *
				 * NB: Overflow, 64 bit div may be required.
				 *
				 * plld max - 63
				 * clkod max - 7*7 = 49
				 * m_frac_mult max - 0xffffff
				 * max clkod = ULONG_MAX / 0xffffff = 255
				 *
				 * Any combined plld/clkod value over 255 may
				 * lead to pm_div64 being required.
				 */
				fret += pm_div64(&frem, clkod_plld);
			} else {
				fret += frem / (uint64_t) clkod_plld;
				frem = frem % (uint64_t) clkod_plld;
			}

			/* Fold back into return/remainder */
			frem += (fret & (uint64_t) mask) * clkod_plld;

			/* Round the fractional component. Above mid point is rounded up*/
			fret += (uint32_t) 1U << (PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS - 1U);

			ret64 += fret >> PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS;
			rem += frem >> PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS;

			ret64 += (uint64_t) (((uint32_t) rem) / clkod_plld);
			rem = ((uint64_t) rem) % clkod_plld;
		}

		if (ret64 > (uint64_t) ULONG_MAX) {
			/* FIXME: Handle PLL value overflow */
			ret = (uint32_t) ULONG_MAX;
		} else {
			ret = (uint32_t) ret64;
		}
	}

	return ret;
}


static bool clk_pll_16fft_program_freq(struct clk			*pll_clk,
				       struct clk			*div_clk,
				       const struct clk_data_pll_16fft *pll,
				       uint32_t				plld,
				       uint32_t				pllm,
				       uint32_t				pllfm,
				       uint32_t				clkod)
{
	const struct clk_data *div_clk_data;
	const struct clk_drv_div *drv_div;
	uint32_t freq_ctrl0;
	uint32_t freq_ctrl1;
	uint32_t div_ctrl;
	uint32_t ctrl;
	bool ret = true;
	int32_t err;
	uint32_t cfg;
	uint32_t pll_type;

	pll_clk->flags &= (uint8_t) ~CLK_FLAG_CACHED;

	if (!clk_pll_16fft_is_bypass(pll)) {
		/* Put the PLL into bypass */
		err = clk_pll_16fft_bypass(pll_clk, true);
		if (err != SUCCESS) {
			ret = false;
		}
	}

	if (ret) {
		/* Disable the PLL */
		err = clk_pll_16fft_disable_pll(pll);
		if (err != SUCCESS) {
			ret = false;
		}
	}

	cfg = readl(pll->base + (uint32_t) PLL_16FFT_CFG(pll->idx));
	pll_type = (cfg & PLL_16FFT_CFG_PLL_TYPE_MASK) >> PLL_16FFT_CFG_PLL_TYPE_SHIFT;

	/* Program the new rate */
	freq_ctrl0 = readl(pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL0(pll->idx));
	freq_ctrl1 = readl(pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL1(pll->idx));
	div_ctrl = readl(pll->base + (uint32_t) PLL_16FFT_DIV_CTRL(pll->idx));

	freq_ctrl0 &= ~PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK;
	freq_ctrl0 |= pllm << PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT;

	freq_ctrl1 &= ~PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK;
	freq_ctrl1 |= pllfm << PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT;

	div_ctrl &= ~PLL_16FFT_DIV_CTRL_REF_DIV_MASK;
	div_ctrl |= plld << PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT;

	/* Make sure we have fractional support if required */
	ctrl = readl(pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));

	/* Don't use internal bypass,it is not glitch free. Always prefer glitchless bypass */
	ctrl &= ~PLL_16FFT_CTRL_INTL_BYP_EN;

	/* Always enable output if PLL */
	ctrl |= PLL_16FFT_CTRL_CLK_POSTDIV_EN;

	/* Currently unused by all PLLs */
	ctrl &= ~PLL_16FFT_CTRL_CLK_4PH_EN;

	/* Always bypass if we lose lock */
	ctrl |= PLL_16FFT_CTRL_BYP_ON_LOCKLOSS;

	/* Enable fractional support if required */
	if (pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF) {
		if (pllfm != 0UL) {
			ctrl |= PLL_16FFT_CTRL_DSM_EN;
			ctrl |= PLL_16FFT_CTRL_DAC_EN;
		} else {
			ctrl &= ~PLL_16FFT_CTRL_DSM_EN;
			ctrl &= ~PLL_16FFT_CTRL_DAC_EN;
		}
	}

	/* Enable Fractional by default for PLL_16FFT_CFG_PLL_TYPE_FRAC2 */
	if (pll_type == PLL_16FFT_CFG_PLL_TYPE_FRAC2) {
		ctrl |= PLL_16FFT_CTRL_DSM_EN;
		ctrl |= PLL_16FFT_CTRL_DAC_EN;
	}

	if (ret) {
		ti_clk_writel(freq_ctrl0,
			      (uint32_t) pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL0(pll->idx));
	}
	if (ret) {
		ti_clk_writel(freq_ctrl1,
			      (uint32_t) pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL1(pll->idx));
	}
	if (ret) {
		ti_clk_writel(div_ctrl,
			      (uint32_t) pll->base + (uint32_t) PLL_16FFT_DIV_CTRL(pll->idx));
	}
	if (ret) {
		ti_clk_writel(ctrl,
			      (uint32_t) pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));
	}

	/* Program output divider */
	if (div_clk && ret) {
		div_clk_data = clk_get_data(div_clk);
		if (div_clk_data->drv != NULL) {
			drv_div = container_of(div_clk_data->drv,
					       const struct clk_drv_div, drv);
		} else {
			drv_div = NULL;
		}
		if (drv_div && drv_div->set_div) {
			ret = drv_div->set_div(div_clk, clkod);
		}
	}

	if (ret) {
		ti_clk_writel(ctrl, (uint32_t) pll->base + (uint32_t) PLL_16FFT_CTRL(pll->idx));
	}

	/* Configure PLL calibration*/
	if ((pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF) && ret) {
		if (pllfm != 0UL) {
			/* Disable Calibration in Fractional mode */
			clk_pll_16fft_disable_cal(pll);
		} else {
			/* Enable Calibration in Integer mode */
			clk_pll_16fft_cal_option3(pll);
		}
	}

	/*
	 * Wait at least 1 ref cycle before enabling PLL.
	 * Minimum VCO input frequency is 5MHz, therefore maximum
	 * wait time for 1 ref clock is 0.2us.
	 */
	if (ret) {
		osal_delay(1UL);
		err = clk_pll_16fft_enable_pll(pll);
		if (err != SUCCESS) {
			ret = false;
		}
	}

	if ((pll_clk->ref_count != 0U) && ret) {
		/* Take the PLL out of bypass */
		ret = clk_pll_16fft_wait_for_lock(pll_clk);
		if (ret) {
			err = clk_pll_16fft_bypass(pll_clk, false);
			if (err != SUCCESS) {
				ret = false;
			}
		}
	}

	return ret;
}

static uint32_t clk_pll_16fft_internal_set_freq(struct clk *pll_clk,
						struct clk *div_clk,
						const struct pll_data *pll_data_t,
						uint32_t target_hz,
						uint32_t min_hz,
						uint32_t max_hz,
						bool query, bool *changed)
{
	const struct clk_data *pll_clk_data;
	const struct clk_data_pll_16fft *pll;
	const struct clk_data_pll *data_pll;
	uint32_t freq_ctrl0;
	uint32_t freq_ctrl1;
	uint32_t div_ctrl;
	uint32_t prev_pllm;
	uint32_t prev_pllfm;
	uint32_t prev_plld;
	uint32_t prev_clkod;
	uint32_t pllm = 0U;
	uint32_t pllfm = 0U;
	uint32_t plld = 0U;
	uint32_t clkod = 0U;
	uint32_t freq;
	uint32_t parent_freq_hz;
	bool was_bypass;

	pll_clk_data = clk_get_data(pll_clk);
	data_pll = container_of(pll_clk_data->data, const struct clk_data_pll,
				data);
	pll = container_of(data_pll, const struct clk_data_pll_16fft,
			   data_pll);

	was_bypass = clk_pll_16fft_is_bypass(pll);

	freq_ctrl0 = readl(pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL0(pll->idx));
	freq_ctrl1 = readl(pll->base + (uint32_t) PLL_16FFT_FREQ_CTRL1(pll->idx));
	div_ctrl = readl(pll->base + (uint32_t) PLL_16FFT_DIV_CTRL(pll->idx));

	/* Check current values */
	prev_pllm = freq_ctrl0 & PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK;
	prev_pllm >>= PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT;

	prev_pllfm = freq_ctrl1 & PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK;
	prev_pllfm >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT;

	prev_plld = div_ctrl & PLL_16FFT_DIV_CTRL_REF_DIV_MASK;
	prev_plld >>= PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT;

	if (div_clk != NULL) {
		prev_clkod = clk_get_div(div_clk);
	} else {
		prev_clkod = 1U;
	}

	/* Compute new values */
	parent_freq_hz = clk_get_parent_freq(pll_clk);
	freq = pll_calc(pll_clk, pll_data_t, parent_freq_hz,
			target_hz, min_hz, max_hz,
			&plld, &pllm, &pllfm, &clkod);

	*changed = true;
	/* No need to do anything if they haven't changed */
	if ((plld == prev_plld) && (pllm == prev_pllm) && (clkod == prev_clkod) &&
	    (pllfm == prev_pllfm) && !was_bypass) {
		*changed = false;
	}

	if ((freq != 0UL) && !query && *changed) {
		if (!clk_pll_16fft_program_freq(pll_clk, div_clk, pll,
						plld, pllm, pllfm, clkod)) {
			freq = 0U;
		}
	}

	return freq;
}

/**
 * @brief Check desired frequency is possible from VCO frequencies in PLL table.
 *
 * @param pll_clk Parent clock PLL of the div_clk
 * @param div_clk Clock data that requires frequency change ( HSDIV clk)
 * @param target_hz Required frequency
 * @param min_hz Minimum Required frequency
 * @param max_hz Maximum Required frequency
 * @param query If TRUE don't change the register value
 *		If FALSE change the register value to set required frequency
 * @param changed Indicates change in plld,pllm and pllfm
 *
 * @return uint32_t If the frequency can be derived from the PLL table,
 *                  return the required frequency, else, return 0.
 */
static uint32_t clk_pll_16fft_internal_set_freq_from_pll_table(struct clk *pll_clk,
							       struct clk *div_clk,
							       const struct pll_data *pll_data_t,
							       uint32_t target_hz,
							       uint32_t min_hz,
							       uint32_t max_hz,
							       bool query, bool *changed)
{
	const struct clk_data *pll_clk_data;
	const struct clk_data_pll_16fft *pll;
	const struct clk_data_pll *data_pll;
	const struct clk_data_div *data_div;
	uint32_t div0 = 0U, div1;
	uint32_t div0_delta = UINT_MAX;
	uint32_t div1_delta = UINT_MAX;
	uint32_t div0_hz, div1_hz;
	uint32_t freq = 0U;
	uint32_t input;
	bool pll_consider_done = false;
	int32_t i;
	uint32_t n;
	uint64_t rem64;
	uint32_t rem;
	uint64_t actual64;
	uint32_t actual = 0U;
	uint32_t clkod_plld;

	pll_clk_data = clk_get_data(pll_clk);
	data_pll = container_of(pll_clk_data->data, const struct clk_data_pll, data);
	pll = container_of(data_pll, const struct clk_data_pll_16fft, data_pll);
	data_div = container_of(pll_clk_data->data, const struct clk_data_div, data);

	input = clk_get_parent_freq(pll_clk);

	n = data_div->n;

	for (i = 0; data_pll->pll_entries[i] != PLL_TABLE_LAST; i++) {
		if (!pll_consider_done) {
			/*
			 * Determine actual frequency given table entry would produce:
			 *
			 * actual = input * pllm / (plld * clkod)
			 *
			 * Note: We break up the calculation in order to avoid a div64.
			 */
			clkod_plld = (uint32_t) (soc_pll_table[data_pll->pll_entries[i]].plld *
						 soc_pll_table[data_pll->pll_entries[i]].clkod);

			actual64 = ((uint64_t) (input / clkod_plld)) *
				soc_pll_table[data_pll->pll_entries[i]].pllm;
			rem64 = ((uint64_t) (input % clkod_plld)) *
				soc_pll_table[data_pll->pll_entries[i]].pllm;

			if (rem64 > (uint64_t) ULONG_MAX) {
				/*
				 * Note: This is only true if clkod * plld * pllm >
				 * ULONG_MAX. This is not true for any currently
				 * supported PLLs, this 64 bit division is only
				 * included for completeness
				 */
				actual64 += pm_div64(&rem64, clkod_plld);
				rem = (uint32_t) rem64;
			} else {
				actual64 += (uint64_t) (((uint32_t) rem64) / clkod_plld);
				rem = ((uint32_t) rem64) % clkod_plld;
			}

			if (soc_pll_table[data_pll->pll_entries[i]].pllfm != 0UL) {
				uint64_t fret;
				uint64_t frem;
				uint32_t stride = 1U;
				uint32_t pllfm_bits;
				uint32_t pllfm_range;
				uint32_t pllfm_mask;

				pllfm_bits = pll_data_t->pllfm_bits;
				pllfm_range = (uint32_t) (1UL << pllfm_bits);
				pllfm_mask = pllfm_range - 1U;

				if (pll_data_t->pllm_stride != NULL) {
					stride = pll_data_t->pllm_stride(pll_clk, soc_pll_table[data_pll->pll_entries[i]].pllm);
				}

				/* Calculate fractional component of frequency */
				fret = ((uint64_t) (input / clkod_plld)) *
					soc_pll_table[data_pll->pll_entries[i]].pllfm;
				frem = ((uint64_t) (input % clkod_plld)) *
					soc_pll_table[data_pll->pll_entries[i]].pllfm;
				if (frem > (uint64_t) ULONG_MAX) {
					fret += pm_div64(&frem, clkod_plld);
				} else if (frem >= clkod_plld) {
					fret += (uint64_t) (((uint32_t) frem) / clkod_plld);
					frem =	(uint64_t) (((uint32_t) frem) % clkod_plld);
				} else {
					/* Do Nothing */
				}
				fret *= stride;
				frem *= stride;
				if (frem > (uint64_t) ULONG_MAX) {
					fret += pm_div64(&frem, clkod_plld);
				} else if (frem >= clkod_plld) {
					fret += (uint64_t) (((uint32_t) frem) / clkod_plld);
					frem =	(uint64_t) (((uint32_t) frem) % clkod_plld);
				} else {
					/* Do Nothing */
				}
				frem += (uint64_t) (((uint32_t) (fret & pllfm_mask)) * clkod_plld);

				/* Add fractional part */
				actual64 += fret >> pllfm_bits;
				rem += (uint32_t) (frem >> pllfm_bits);

				actual64 += (uint64_t) (((uint32_t) rem) / clkod_plld);
				rem += ((uint32_t) rem) % clkod_plld;
			}

			if (actual64 > (uint64_t) ULONG_MAX) {
				pll_consider_done = true;
			} else {
				actual = (uint32_t) actual64;
			}
		}

		/* Calculate 2 best potential dividers for HSDIV */
		div0 = actual / target_hz;

		/*
		 * Prevent out-of-bounds divider value. Rest of the code in the
		 * function will check if the resulting divider value is within
		 * the allowable min/max range.
		 */
		if (div0 > (n - 1U)) {
			div0 = n - 1U;
		}
		div1 = div0 + 1U;
		if (div0 != 0UL) {
			div0_hz = actual / div0;
			/* Check for in range */
			if (div0_hz <= max_hz) {
				div0_delta = div0_hz - target_hz;
			}
		}
		div1_hz = actual / div1;
		if (div1_hz >= min_hz) {
			div1_delta = target_hz - div1_hz;
		}

		/* Make sure at least one of them is acceptable */
		if ((div1_delta == UINT_MAX) && ((div0_delta == UINT_MAX))) {
			*changed = false;
			freq = 0U;
		} else {
			if (div0_delta > div1_delta) {
				div0 = div1;
			}
			*changed = true;
			freq = actual / div0;
			break;
		}
	}

	if (!query && *changed) {
		if (!clk_pll_16fft_program_freq(pll_clk, div_clk, pll,
						soc_pll_table[data_pll->pll_entries[i]].plld,
						soc_pll_table[data_pll->pll_entries[i]].pllm,
						soc_pll_table[data_pll->pll_entries[i]].pllfm,
						div0)) {
			freq = 0U;
		}
	}

	return freq;
}

/*
 * Bit of a glue function, there is no way for the clock tree to call this
 * directly, it is only called by pll_init. We actually want to set the
 * frequency of foutpostdiv_clk not the DCO.
 */
static uint32_t clk_pll_16fft_set_freq(struct clk *clock_ptr,
				       uint32_t target_hz,
				       uint32_t min_hz,
				       uint32_t max_hz,
				       bool query, bool *changed)
{
	const struct pll_data *pll_data_t;
	struct clk *div_clk = NULL;
	uint32_t ret = 0U;
	uint32_t i;

	/* Find our associated postdiv */
	for (i = 0U; (div_clk == NULL) && (i < soc_clock_count); i++) {
		const struct clk_data *sub_data = &soc_clock_data[i];

		if (sub_data->drv == &clk_drv_div_pll_16fft_postdiv.drv) {
			struct clk *sub_clk = &soc_clocks[i];
			const struct clk_parent *p;

			p = clk_get_parent(sub_clk);
			if (p != NULL) {
				struct clk *pll_clk;

				pll_clk = clk_lookup((clk_idx_t) p->clk);
				if (pll_clk == clock_ptr) {
					div_clk = sub_clk;
				}
			}
		}
	}

	/*
	 * Since this function is only called by pll_init, the frequency
	 * passed in will always be from autogen. Because this PLL type
	 * can have many output dividers, including FOUTPOSTDIV and
	 * the various HSDIV options, the frequency listed by autogen
	 * is a bit amiguous.
	 *
	 * The current rule to resolve abiguity is that if a PLL has a
	 * connected FOUTPOSTDIV, the default frequency given is for this
	 * output. If there is no FOUTPOSTDIV, the default frequency given
	 * is for FOUTVCO.
	 */

	if (div_clk != NULL) {
		/*
		 * This is a clock with FOUTPOSTDIV. Utilize the postdiv
		 * output divider when determining a new frequency.
		 */
		pll_data_t = &pll_16fft_postdiv_data;
	} else {
		/*
		 * This is a clock without FOUTPOSTDIV. The clock
		 * output divider must be 1.
		 */
		pll_data_t = &pll_16fft_raw_data;
	}

	ret = clk_pll_16fft_internal_set_freq(clock_ptr, div_clk, pll_data_t,
					      target_hz, min_hz, max_hz,
					      query, changed);

	return ret;
}

/*
 * \brief Return the 16FFT PLL DCO frequency.
 *
 * \param clk The 16FFT PLL clock.
 *
 * \return The DCO frequency of the 16FFT in Hz.
 */
static uint32_t clk_pll_16fft_get_freq(struct clk *clock_ptr)
{
	const struct clk_data *clock_data = clk_get_data(clock_ptr);
	uint32_t ret;


	/*
	 * If disabled, return the frequency we would be running at once
	 * we bring it out of bypass. If enabled and in bypass, return 0.
	 */
	if (clock_ptr->ref_count == 0U) {
		ret = 0U;
	} else if ((clock_ptr->flags & CLK_FLAG_CACHED) != 0U) {
		ret = soc_clock_values[clock_data->freq_idx];
	} else {
		const struct clk_data_pll_16fft *pll;
		const struct clk_data_pll *data_pll;

		data_pll = container_of(clock_data->data, const struct clk_data_pll,
					data);
		pll = container_of(data_pll, const struct clk_data_pll_16fft,
				   data_pll);

		if (!clk_pll_16fft_is_bypass(pll)) {
			ret = clk_pll_16fft_get_freq_internal(clock_ptr, 1U);
		} else {
			ret = 0U;
		}
		soc_clock_values[clock_data->freq_idx] = ret;
		clock_ptr->flags |= CLK_FLAG_CACHED;
	}

	return ret;
}

static bool clk_pll_16fft_set_state(struct clk *clock_ptr, bool enabled)
{
	bool ret = true;

	if (enabled) {
		ret = clk_pll_16fft_wait_for_lock(clock_ptr);
	}

	if (ret) {
		int32_t err;

		clock_ptr->flags &= (uint8_t) ~CLK_FLAG_CACHED;
		err = clk_pll_16fft_bypass(clock_ptr, !enabled);
		if (err != SUCCESS) {
			ret = false;
		}
	}

	return ret;
}

/*
 * \brief Get the state of the 16FFT PLL.
 *
 * \param clk The 16FFT PLL clock.
 *
 * \return Returns HW state using the CLK_HW_STATE macro.
 */
static uint32_t clk_pll_16fft_get_state(struct clk *clock_ptr)
{
	struct clk *clock_parent = NULL;
	uint32_t ret = CLK_HW_STATE_ENABLED;

	if (clock_ptr->ref_count == 0U) {
		ret = CLK_HW_STATE_DISABLED;
	}

	/* PLLs can't function without an enabled parent */
	if (ret == CLK_HW_STATE_ENABLED) {
		const struct clk_parent *p;

		p = clk_get_parent(clock_ptr);
		if (p != NULL) {
			clock_parent = clk_lookup(p->clk);
		}
		if (clock_parent != NULL) {
			ret = clk_get_state(clock_parent);
		} else {
			ret = CLK_HW_STATE_DISABLED;
		}
	}

	if (ret == CLK_HW_STATE_ENABLED) {
		const struct clk_data *clock_data;
		const struct clk_data_pll_16fft *pll;
		const struct clk_data_pll *data_pll;

		clock_data = clk_get_data(clock_ptr);
		data_pll = container_of(clock_data->data, const struct clk_data_pll,
					data);
		pll = container_of(data_pll, const struct clk_data_pll_16fft,
				   data_pll);

		if (!clk_pll_16fft_is_bypass(pll) && !clk_pll_16fft_check_lock(pll)) {
			ret = CLK_HW_STATE_TRANSITION;
		}
	}

	return ret;
}

static struct clk *clk_pll_16fft_hsdiv_get_pll_root(struct clk *clock_ptr)
{
	const struct clk_parent *p = NULL;
	struct clk *pll_clk = NULL;

	/* Get PLL root */
	p = clk_get_parent(clock_ptr);
	if (p != NULL) {
		pll_clk = clk_lookup((clk_idx_t) p->clk);
	}

	return pll_clk;
}

static struct clk *clk_pll_16fft_postdiv_get_pll_root(struct clk *clock_ptr)
{
	const struct clk_parent *p = NULL;
	struct clk *pll_clk = NULL;
	const struct clk_data *pll_clk_data;

	/* Get PLL post divider */
	p = clk_get_parent(clock_ptr);
	if (p != NULL) {
		pll_clk = clk_lookup((clk_idx_t) p->clk);
	}

	/* Verify correct clock tree */
	pll_clk_data = clk_get_data(clock_ptr);
	if (pll_clk && (pll_clk_data->drv == &clk_drv_div_pll_16fft_postdiv.drv)) {
		/* Get PLL root */
		p = clk_get_parent(pll_clk);
		if (p != NULL) {
			pll_clk = clk_lookup((clk_idx_t) p->clk);
		} else {
			pll_clk = NULL;
		}
	}

	return pll_clk;
}

static int32_t clk_pll_16fft_init_internal(struct clk *clock_ptr)
{
	const struct clk_data *clock_data = clk_get_data(clock_ptr);
	const struct clk_data_pll_16fft *pll;
	const struct clk_data_pll *data_pll;
	uint32_t i;
	int32_t ret = SUCCESS;
	bool skip_hw_init = false;

	clock_ptr->flags &= (uint8_t) ~CLK_FLAG_CACHED;

	data_pll = container_of(clock_data->data, const struct clk_data_pll,
				data);
	pll = container_of(data_pll, const struct clk_data_pll_16fft,
			   data_pll);

	/*
	 * Unlock write access. Note this register does not readback the
	 * written value.
	 */
	ti_clk_writel((uint32_t) PLL_16FFT_LOCKKEY0_VALUE,
		      (uint32_t) pll->base + (uint32_t) PLL_16FFT_LOCKKEY0(pll->idx));
	ti_clk_writel((uint32_t) PLL_16FFT_LOCKKEY1_VALUE,
		      (uint32_t) pll->base + (uint32_t) PLL_16FFT_LOCKKEY1(pll->idx));

	/*
	 * In order to honor the CLK_DATA_FLAG_NO_HW_REINIT flag when set,
	 * we must check if the clk is enabled, and if so, skip re-setting
	 * default frequency if it is available.
	 */
	if (((clock_data->flags & CLK_DATA_FLAG_NO_HW_REINIT) != 0U) &&
	    (clk_pll_16fft_check_lock(pll) == true)) {
		skip_hw_init = true;
	}

	if (skip_hw_init == false) {
		ret = pll_init(clock_ptr);
	}

	if (ret == SUCCESS) {
		/*
		 * Find and program hsdiv defaults.
		 *
		 * HSDIV defaults must be programmed before programming the
		 * PLL since their power on default is /1. Most DCO
		 * frequencies will exceed clock rate maximums of the HSDIV
		 * outputs.
		 *
		 * We walk through the clock tree to find all the clocks
		 * with the hsdiv driver who have this PLL for a parent.
		 */
		for (i = 0; (i < soc_clock_count) && (ret == SUCCESS); i++) {
			const struct clk_data *sub_data = soc_clock_data + i;
			struct clk *sub_clk = soc_clocks + i;

			if ((sub_data->drv == &clk_drv_div_reg.drv) &&
			    sub_data->drv->init) {
				if (clk_pll_16fft_postdiv_get_pll_root(sub_clk) == clock_ptr) {
					sub_data->drv->init(sub_clk);
				}
			} else if ((sub_data->drv == &clk_drv_div_pll_16fft_hsdiv.drv) &&
				   sub_data->drv->init) {
				if (clk_pll_16fft_hsdiv_get_pll_root(sub_clk) == clock_ptr) {
					sub_data->drv->init(sub_clk);
				}
			} else {
				/* Do Nothing */
			}
		}
	}

	/*
	 * We must always assume we are enabled as we could be operating
	 * clocks in bypass.
	 */
	if (ret == SUCCESS) {
		clock_ptr->flags |= CLK_FLAG_PWR_UP_EN;
	}

	return ret;
}

static int32_t clk_pll_16fft_init(struct clk *clock_ptr)
{
	const struct clk_data *clock_data = clk_get_data(clock_ptr);
	const struct clk_data_pll *data_pll;
	int32_t ret;

	data_pll = container_of(clock_data->data, const struct clk_data_pll,
				data);

	if (pm_devgroup_is_enabled(data_pll->devgrp)) {
		ret = clk_pll_16fft_init_internal(clock_ptr);
	} else {
		ret = -EDEFER;
	}

	return ret;
}

const struct clk_drv clk_drv_pll_16fft = {
	.init		= clk_pll_16fft_init,
	.get_freq	= clk_pll_16fft_get_freq,
	.set_freq	= clk_pll_16fft_set_freq,
	.get_state	= clk_pll_16fft_get_state,
	.set_state	= clk_pll_16fft_set_state,
};

/*
 * There are two post dividers. Total post divide is postdiv1 * postdiv2.
 * postdiv1 must always be greater than or equal to postdiv2. Rather than
 * factor the divider, just keep a small 50 byte lookup table.
 *
 * Note that this means that even though that there are 6 bits used for
 * divider values, there are only 25 divider values (rather than 64).
 */
#define PDV(pd1, pd2)		((pd1) | ((pd2) << 4UL))
#define PDV_ENTRY(div, pd1)[(div)] = PDV((pd1), (div) / (pd1))
static const uint8_t postdiv_mapping[(7U * 7U) + 1U] = {
	PDV_ENTRY(1U,  1U),
	PDV_ENTRY(2U,  2U),
	PDV_ENTRY(3U,  3U),
	PDV_ENTRY(4U,  4U),
	PDV_ENTRY(5U,  5U),
	PDV_ENTRY(6U,  6U),
	PDV_ENTRY(7U,  7U),
	PDV_ENTRY(8U,  4U),
	PDV_ENTRY(9U,  3U),
	PDV_ENTRY(10U, 5U),
	PDV_ENTRY(12U, 4U),
	PDV_ENTRY(14U, 7U),
	PDV_ENTRY(15U, 5U),
	PDV_ENTRY(16U, 4U),
	PDV_ENTRY(18U, 6U),
	PDV_ENTRY(20U, 5U),
	PDV_ENTRY(21U, 7U),
	PDV_ENTRY(24U, 6U),
	PDV_ENTRY(25U, 5U),
	PDV_ENTRY(28U, 7U),
	PDV_ENTRY(30U, 6U),
	PDV_ENTRY(35U, 7U),
	PDV_ENTRY(36U, 6U),
	PDV_ENTRY(42U, 7U),
	PDV_ENTRY(49U, 7U),
};

/*
 * \brief Check if the clkod value is valid
 *
 * \param clk The PLL clock.
 * \param clkod The output divider value
 *
 * \return true if clkod value is valid, false otherwise
 */
static bool pll_16fft_clkod_valid(struct clk *clock_ptr UNUSED, uint32_t clkod)
{
	return (clkod < ARRAY_SIZE(postdiv_mapping)) && (postdiv_mapping[clkod] != 0U);
}

static uint32_t clk_pll_16fft_postdiv_set_freq(struct clk *clock_ptr,
					       uint32_t target_hz,
					       uint32_t min_hz,
					       uint32_t max_hz,
					       bool query, bool *changed)
{
	const struct clk_data *clock_data = clk_get_data(clock_ptr);
	uint32_t ret = 0U;

	if ((clock_data->flags & CLK_DATA_FLAG_MODIFY_PARENT_FREQ) != 0U) {
		/* Program the whole PLL */
		const struct clk_parent *p;
		struct clk *pll_clk = NULL;

		p = clk_get_parent(clock_ptr);
		if (p != NULL) {
			pll_clk = clk_lookup((clk_idx_t) p->clk);
		}

		if (pll_clk != NULL) {
			ret = clk_pll_16fft_internal_set_freq(pll_clk, clock_ptr,
							      &pll_16fft_postdiv_data,
							      target_hz, min_hz, max_hz,
							      query, changed);
		}
	} else {
		/* Just program the output divider. */
		ret = clk_div_set_freq(clock_ptr, target_hz, min_hz, max_hz,
				       query, changed);
	}

	return ret;
}

static bool clk_pll_16fft_postdiv_valid_div(struct clk *clock_ptr UNUSED, uint32_t d)
{
	return (d < ARRAY_SIZE(postdiv_mapping)) && (postdiv_mapping[d] != 0U);
}

static bool clk_pll_16fft_postdiv_set_div(struct clk *clock_ptr, uint32_t d)
{
	const struct clk_parent *p;
	struct clk *pll_clk = NULL;

	p = clk_get_parent(clock_ptr);
	if (p != NULL) {
		pll_clk = clk_lookup((clk_idx_t) p->clk);
	}

	if (pll_clk != NULL) {
		const struct clk_data *pll_clk_data = clk_get_data(pll_clk);
		const struct clk_data_pll *data_pll;
		const struct clk_data_pll_16fft *pll;
		uint32_t div_ctrl;
		uint32_t post_div1;
		uint32_t post_div2;
		uint8_t v;

		data_pll = container_of(pll_clk_data->data,
					const struct clk_data_pll, data);
		pll = container_of(data_pll, const struct clk_data_pll_16fft,
				   data_pll);

		div_ctrl = readl(pll->base + (uint32_t) PLL_16FFT_DIV_CTRL(pll->idx));
		div_ctrl &= ~PLL_16FFT_DIV_CTRL_POST_DIV2_MASK;
		div_ctrl &= ~PLL_16FFT_DIV_CTRL_POST_DIV1_MASK;

		v = postdiv_mapping[d];
		post_div1 = (uint32_t) v & 0xFU;
		post_div2 = (uint32_t) v >> 4UL;

		div_ctrl |= post_div1 << PLL_16FFT_DIV_CTRL_POST_DIV1_SHIFT;
		div_ctrl |= post_div2 << PLL_16FFT_DIV_CTRL_POST_DIV2_SHIFT;

		ti_clk_writel(div_ctrl, pll->base + (uint32_t) PLL_16FFT_DIV_CTRL(pll->idx));
	}

	return SUCCESS;
}

static uint32_t clk_pll_16fft_postdiv_get_div(struct clk *clock_ptr)
{
	const struct clk_parent *p;
	struct clk *pll_clk = NULL;
	uint32_t ret = 0U;

	p = clk_get_parent(clock_ptr);
	if (p != NULL) {
		pll_clk = clk_lookup((clk_idx_t) p->clk);
	}

	if (pll_clk != NULL) {
		const struct clk_data *pll_clk_data = clk_get_data(pll_clk);
		const struct clk_data_pll *data_pll;
		const struct clk_data_pll_16fft *pll;
		uint32_t div_ctrl;
		uint32_t post_div1;
		uint32_t post_div2;

		data_pll = container_of(pll_clk_data->data,
					const struct clk_data_pll, data);
		pll = container_of(data_pll, const struct clk_data_pll_16fft,
				   data_pll);

		div_ctrl = readl(pll->base + (uint32_t) PLL_16FFT_DIV_CTRL(pll->idx));
		post_div1 = div_ctrl & PLL_16FFT_DIV_CTRL_POST_DIV1_MASK;
		post_div1 >>= PLL_16FFT_DIV_CTRL_POST_DIV1_SHIFT;
		post_div2 = div_ctrl & PLL_16FFT_DIV_CTRL_POST_DIV2_MASK;
		post_div2 >>= PLL_16FFT_DIV_CTRL_POST_DIV2_SHIFT;

		ret = post_div1 * post_div2;
	}
	return ret;
}

const struct clk_drv_div clk_drv_div_pll_16fft_postdiv = {
	.drv			= {
		.init		= clk_div_init,
		.notify_freq	= clk_div_notify_freq,
		.get_freq	= clk_div_get_freq,
		.set_freq	= clk_pll_16fft_postdiv_set_freq,
	},
	.valid_div		= clk_pll_16fft_postdiv_valid_div,
	.set_div		= clk_pll_16fft_postdiv_set_div,
	.get_div		= clk_pll_16fft_postdiv_get_div,
};

static uint32_t clk_pll_16fft_hsdiv_set_freq(struct clk *clock_ptr,
					     uint32_t target_hz,
					     uint32_t min_hz,
					     uint32_t max_hz,
					     bool query, bool *changed)
{
	const struct clk_data *clock_data = clk_get_data(clock_ptr);
	uint32_t ret = 0U;

	if ((clock_data->flags & CLK_DATA_FLAG_MODIFY_PARENT_FREQ) != 0U) {
		/* Program the whole PLL */
		const struct clk_parent *p;
		struct clk *pll_clk = NULL;

		p = clk_get_parent(clock_ptr);
		if (p != NULL) {
			pll_clk = clk_lookup((clk_idx_t) p->clk);
		}

		if (pll_clk != NULL) {
			/*
			 * Before changing the VCO frequency of PLL.
			 * Check that new target freq can be obtained with current parent
			 * frequency by changing the divider.
			 *
			 * Here both min and max freq is given same as target freq.
			 * Reason :
			 * If min and max frequency is given it might set to frequency other
			 * than target frequency even if target frequency can be obtained by
			 * changing the PLL vco frequency.
			 */
			ret = clk_div_set_freq_static_parent(clock_ptr, target_hz, target_hz,
							     target_hz, query, changed);

			if (ret == 0U) {
				/*
				 * If the requested frequency cannot be achieved by changing the
				 * hsdiv and clk_modify_parent_frequency is enabled for that hsdiv,
				 * the VCO frequency of the PLL will be altered to obtain the
				 * required frequency.However, in this case, the VCO frequency
				 * can vary between 800MHz to 3.2 GHz for 16fft PLLs, and the
				 * user has no control over it.
				 * To tackle this issue, if the required frequency is not
				 * achievable, one can refer to the PLL table,
				 * which contains a list of alternate suggested VCO frequency
				 * values for the corresponding PLL.
				 *
				 * If the target frequency cannot be achieved using values
				 * from the PLL table,then use pll_internal_calc function to
				 * determine the optimal VCO frequency.
				 */
				ret = clk_pll_16fft_internal_set_freq_from_pll_table(pll_clk,
										     clock_ptr,
										     &pll_16fft_hsdiv_data,
										     target_hz,
										     target_hz,
										     target_hz,
										     query,
										     changed);

				if (ret == 0U) {
					/*
					 * First try setting the exact target_hz frequency
					 * without using the min and max range. We do this
					 * because some times even when the target frequency
					 * is acheivable, the calculations will result
					 * in choosing a value other than the target from the
					 * range provided.
					 */
					ret = clk_pll_16fft_internal_set_freq(pll_clk, clock_ptr,
									      &pll_16fft_hsdiv_data,
									      target_hz,
									      target_hz, target_hz,
									      query, changed);
					/*
					 * If the previous step failed in setting the exact
					 * target_hz, use the min and max range provided.
					 */
					if (ret == 0U) {
						ret = clk_pll_16fft_internal_set_freq(pll_clk,
										      clock_ptr,
										      &pll_16fft_hsdiv_data,
										      target_hz,
										      min_hz,
										      max_hz,
										      query,
										      changed);
					}
				}
			}
		}
	} else {
		/* Just program the output divider. */
		ret = clk_div_set_freq(clock_ptr, target_hz, min_hz, max_hz,
				       query, changed);
	}

	return ret;
}

static int32_t clk_pll_16fft_hsdiv_init(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;
	const struct clk_data_div_reg *data_reg;
	const struct clk_drv_div *drv_div;
	int32_t ret = SUCCESS;
	bool skip_hw_init = false;
	uint32_t hsdiv_ctrl;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	data_reg = container_of(data_div, const struct clk_data_div_reg,
				data_div);
	drv_div = container_of(clk_datap->drv, const struct clk_drv_div, drv);

	hsdiv_ctrl = readl(data_reg->reg);

	if (((clk_datap->flags & CLK_DATA_FLAG_NO_HW_REINIT) != 0U) &&
	    ((hsdiv_ctrl & PLL_16FFT_HSDIV_CTRL_CLKOUT_EN) != 0U)) {
		if (drv_div->get_div != NULL) {
			/*
			 * In order to honor the CLK_DATA_FLAG_NO_HW_REINIT flag when set,
			 * we must check if the HSDIV is confgiured. If the HSDIV
			 * is the defaut value of 1 then we consider it is not configured.
			 */
			if (drv_div->get_div(clkp) != 1U) {
				skip_hw_init = true;
			}
		}
	}

	if ((drv_div->get_div != NULL) && ((hsdiv_ctrl & PLL_16FFT_HSDIV_CTRL_CLKOUT_EN) != 0U)) {
		if (data_div->default_div > 0U) {
			/*
			 * If the HSDIV value is already configured to the
			 * expected value, then don't reconfigure.
			 */
			if (drv_div->get_div(clkp) == data_div->default_div) {
				skip_hw_init = true;
			}
		}
	}

	if (skip_hw_init == false) {
		if (data_div->default_div && drv_div->set_div) {
			/* Disable HSDIV */
			hsdiv_ctrl &= ~PLL_16FFT_HSDIV_CTRL_CLKOUT_EN;
			ti_clk_writel(hsdiv_ctrl, data_reg->reg);

			if (ret == SUCCESS) {
				if (!drv_div->set_div(clkp, data_div->default_div)) {
					ret = -EINVAL;
				}
			}

			/* Enable HSDIV */
			if (ret == SUCCESS) {
				hsdiv_ctrl = readl(data_reg->reg);
				hsdiv_ctrl |= PLL_16FFT_HSDIV_CTRL_CLKOUT_EN;
				ti_clk_writel(hsdiv_ctrl, data_reg->reg);
			}
		}
	}

	return ret;
}

const struct clk_drv_div clk_drv_div_pll_16fft_hsdiv = {
	.drv			= {
		.init		= clk_pll_16fft_hsdiv_init,
		.notify_freq	= clk_div_notify_freq,
		.get_freq	= clk_div_get_freq,
		.set_freq	= clk_pll_16fft_hsdiv_set_freq,
	},
	.set_div		= clk_div_reg_set_div,
	.get_div		= clk_div_reg_get_div,
};

const struct clk_drv_div clk_drv_div_pll_16fft_postdiv_hsdiv = {
	.drv			= {
		.notify_freq	= clk_div_notify_freq,
		.set_freq	= clk_div_set_freq,
		.get_freq	= clk_div_get_freq,
		.init		= clk_pll_16fft_hsdiv_init,
#ifdef CONFIG_LPM_CLK
		.suspend_save	= clk_div_suspend_save,
		.resume_restore = clk_div_resume_restore,
#endif
	},
	.set_div		= clk_div_reg_set_div,
	.get_div		= clk_div_reg_get_div,
};
