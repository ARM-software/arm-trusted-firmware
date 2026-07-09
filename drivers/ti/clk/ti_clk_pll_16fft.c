/*
 * Copyright (c) 2025-2026 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI 16FFT PLL Driver
 *
 * This driver implements support for TI's 16FFT (16-bit Fractional Feedback)
 * PLL variant, which provides fractional frequency synthesis capabilities.
 * It supports VCO frequencies from 800 MHz to 3.2 GHz, fractional-N synthesis
 * with calibration, HSDIV (High-Speed Divider) output control, PLL lock
 * detection and wait functions, and parent frequency modification.
 */

#include <assert.h>
#include <cdefs.h>
#include <errno.h>

#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <ti_clk_div.h>
#include <ti_clk_mux.h>
#include <ti_clk_pll.h>
#include <ti_clk_pll_16fft.h>
#include <ti_container_of.h>

/* PLL instance offset - each PLL instance is 0x1000 bytes apart */
#define PLL_16FFT_INSTANCE_OFFSET		0x1000UL

/* Register offsets */
#define PLL_16FFT_CFG				0x08UL
#define PLL_16FFT_CFG_PLL_TYPE_SHIFT		0UL
#define PLL_16FFT_CFG_PLL_TYPE_MASK		(0x3UL << 0UL)
#define PLL_16FFT_CFG_PLL_TYPE_FRAC2		0UL
#define PLL_16FFT_CFG_PLL_TYPE_FRACF		1UL

#define PLL_16FFT_LOCKKEY0			0x10UL
#define PLL_16FFT_LOCKKEY0_VALUE		0x68EF3490UL

#define PLL_16FFT_LOCKKEY1			0x14UL
#define PLL_16FFT_LOCKKEY1_VALUE		0xD172BC5AUL

#define PLL_16FFT_CTRL				0x20UL
#define PLL_16FFT_CTRL_BYPASS_EN		BIT(31)
#define PLL_16FFT_CTRL_BYP_ON_LOCKLOSS		BIT(16)
#define PLL_16FFT_CTRL_PLL_EN			BIT(15)
#define PLL_16FFT_CTRL_INTL_BYP_EN		BIT(8)
#define PLL_16FFT_CTRL_CLK_4PH_EN		BIT(5)
#define PLL_16FFT_CTRL_CLK_POSTDIV_EN		BIT(4)
#define PLL_16FFT_CTRL_DSM_EN			BIT(1)
#define PLL_16FFT_CTRL_DAC_EN			BIT(0)

#define PLL_16FFT_STAT				0x24UL
#define PLL_16FFT_STAT_LOCK			BIT(0)

#define PLL_16FFT_FREQ_CTRL0			0x30UL
#define PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT	0UL
#define PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MSK	(0xfffUL << 0UL)

#define PLL_16FFT_FREQ_CTRL1			0x34UL
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT	0UL
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MSK	(0xffffffUL << 0UL)
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BIT_CNT	24UL

#define PLL_16FFT_DIV_CTRL			0x38UL
#define PLL_16FFT_DIV_CTRL_POST_DIV2_SHIFT	24UL
#define PLL_16FFT_DIV_CTRL_POST_DIV2_MASK	(0x7UL << 24UL)
#define PLL_16FFT_DIV_CTRL_POST_DIV1_SHIFT	16UL
#define PLL_16FFT_DIV_CTRL_POST_DIV1_MASK	(0x7UL << 16UL)
#define PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT	0UL
#define PLL_16FFT_DIV_CTRL_REF_DIV_MASK		(0x3fUL << 0UL)

#define PLL_16FFT_CAL_CTRL			0x60UL
#define PLL_16FFT_CAL_CTRL_CAL_EN		BIT(31)
#define PLL_16FFT_CAL_CTRL_FAST_CAL		BIT(20)
#define PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT	16UL
#define PLL_16FFT_CAL_CTRL_CAL_CNT_MASK		(0x7UL << 16UL)
#define PLL_16FFT_CAL_CTRL_CAL_BYP		BIT(15)
#define PLL_16FFT_CAL_CTRL_CAL_IN_SHIFT		0U
#define PLL_16FFT_CAL_CTRL_CAL_IN_MASK		(0xFFFU << 0U)

#define PLL_16FFT_CAL_STAT			0x64UL
#define PLL_16FFT_CAL_STAT_CAL_LOCK		BIT(31)

#define PLL_16FFT_HSDIV_CTRL			0x80UL
#define PLL_16FFT_HSDIV_CTRL_CLKOUT_EN		BIT(15)

/* Register accessor functions */
static inline uint32_t ti_pll_16fft_read(const struct ti_clk_data_pll_16fft *pll,
					 uint32_t reg)
{
	return mmio_read_32(pll->base + (PLL_16FFT_INSTANCE_OFFSET * pll->idx) + reg);
}

static inline void ti_pll_16fft_write(const struct ti_clk_data_pll_16fft *pll,
				      uint32_t reg, uint32_t val)
{
	mmio_write_32((uintptr_t)(pll->base + (PLL_16FFT_INSTANCE_OFFSET * pll->idx) + reg), val);
}

/**
 * ti_pll_16fft_pllm_valid() - Check if a pllm value is valid for this PLL.
 * @clock_ptr: unused.
 * @pllm: The integer multiplier value to validate.
 * @is_frac: True if fractional mode is in use.
 *
 * Return: true if pllm is within the valid range for the given mode, false otherwise.
 */
static bool ti_pll_16fft_pllm_valid(struct ti_clk *clock_ptr __unused, uint32_t pllm,
				    bool is_frac)
{
	if ((is_frac && (pllm >= 20UL) && (pllm <= 320UL)) ||
	    (!is_frac && (pllm >= 16UL) && (pllm <= 640UL))) {
		return true;
	}
	return false;
}

/**
 * ti_pll_16fft_bin() - Assign a bin preference for a given PLL configuration.
 * @clock_ptr: unused.
 * @plld: unused.
 * @pllm: unused.
 * @is_frac: True if fractional mode is in use.
 * @clkod: unused.
 *
 * Prefer non-fractional configuration if possible.
 *
 * Return: 1 for integer mode (preferred), 0 for fractional mode.
 */
static int32_t ti_pll_16fft_bin(struct ti_clk *clock_ptr __unused,
				uint32_t plld __unused,
				uint32_t pllm __unused, bool is_frac,
				uint32_t clkod __unused)
{
	return is_frac ? 0 : 1;
}

/**
 * ti_pll_16fft_vco_fitness() - Return a fitness score for a VCO frequency.
 * @clock_ptr: unused.
 * @vco: VCO frequency in Hz.
 * @is_frac: unused.
 *
 * Prefer higher VCO frequencies.
 *
 * Return: The VCO frequency itself as the fitness score.
 */
static uint32_t ti_pll_16fft_vco_fitness(struct ti_clk *clock_ptr __unused,
					 uint32_t vco, bool is_frac __unused)
{
	return vco;
}

/*
 * There are two post dividers. Total post divide is postdiv1 * postdiv2.
 * postdiv1 must always be greater than or equal to postdiv2. Rather than
 * factor the divider, just keep a small 50 byte lookup table.
 *
 * Note that this means that even though that there are 6 bits used for
 * divider values, there are only 25 divider values (rather than 64).
 */
#define PDV(pd1, pd2) ((uint8_t)((pd1) | ((pd2) << 4UL)))
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

/**
 * ti_pll_16fft_clkod_valid() - Check if a clkod value is valid for this PLL.
 * @clock_ptr: unused.
 * @clkod: The output divider value to validate.
 *
 * Return: true if clkod has a valid postdiv_mapping entry, false otherwise.
 */
static bool ti_pll_16fft_clkod_valid(struct ti_clk *clock_ptr __unused, uint32_t clkod)
{
	return (clkod < ARRAY_SIZE(postdiv_mapping)) && (postdiv_mapping[clkod] != 0U);
}

static const struct ti_pll_data ti_pll_16fft_raw_data = {
	.plld_max	= 1U,
	.pllm_max	= 640U,
	.pllfm_bits	= (uint32_t) PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BIT_CNT,
	.clkod_max	= 1U,
	.pllm_valid	= ti_pll_16fft_pllm_valid,
	.bin		= ti_pll_16fft_bin,
	.vco_fitness	= ti_pll_16fft_vco_fitness,
};

static const struct ti_pll_data ti_pll_16fft_postdiv_data = {
	.plld_max	= 1U,
	.pllm_max	= 640U,
	.pllfm_bits	= (uint32_t) PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BIT_CNT,
	.clkod_max	= 49U,
	.pllm_valid	= ti_pll_16fft_pllm_valid,
	.clkod_valid	= ti_pll_16fft_clkod_valid,
	.bin		= ti_pll_16fft_bin,
	.vco_fitness	= ti_pll_16fft_vco_fitness,
};

static const struct ti_pll_data ti_pll_16fft_hsdiv_data = {
	.plld_max	= 1U,
	.pllm_max	= 640U,
	.pllfm_bits	= (uint32_t) PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BIT_CNT,
	.clkod_max	= 128U,
	.pllm_valid	= ti_pll_16fft_pllm_valid,
	.bin		= ti_pll_16fft_bin,
	.vco_fitness	= ti_pll_16fft_vco_fitness,
};

/**
 * ti_clk_pll_16fft_cal_option3() - Perform option 3 PLL calibration.
 * @pll: PLL instance data.
 *
 * Implements the option 3 PLL calibration method using FASTCAL mode to obtain
 * a calibration value for the FRACF PLL type.
 */
static void ti_clk_pll_16fft_cal_option3(const struct ti_clk_data_pll_16fft *pll)
{
	uint32_t cal;

	cal = ti_pll_16fft_read(pll, PLL_16FFT_CAL_CTRL);

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
	ti_pll_16fft_write(pll, PLL_16FFT_CAL_CTRL, cal);

	/* Wait 1us before enabling the CAL_EN field */
	udelay(1U); /* Wait 1us */

	cal = ti_pll_16fft_read(pll, PLL_16FFT_CAL_CTRL);

	/* Enable calibration for FRACF */
	cal |= PLL_16FFT_CAL_CTRL_CAL_EN;

	/* Note this register does not readback the written value. */
	ti_pll_16fft_write(pll, PLL_16FFT_CAL_CTRL, cal);
}

static void ti_clk_pll_16fft_disable_cal(const struct ti_clk_data_pll_16fft *pll)
{
	uint32_t cal, stat;

	cal = ti_pll_16fft_read(pll, PLL_16FFT_CAL_CTRL);
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_EN;
	/* Note this register does not readback the written value. */
	ti_pll_16fft_write(pll, PLL_16FFT_CAL_CTRL, cal);
	do {
		stat = ti_pll_16fft_read(pll, PLL_16FFT_CAL_STAT);
	} while ((stat & PLL_16FFT_CAL_STAT_CAL_LOCK) != 0U);
}

/**
 * ti_clk_pll_16fft_check_lock() - Check if the PLL VCO/DCO is locked.
 * @pll: PLL instance data.
 *
 * Return: true if the PLL is locked, false otherwise.
 */
static bool ti_clk_pll_16fft_check_lock(const struct ti_clk_data_pll_16fft *pll)
{
	uint32_t stat;

	stat = ti_pll_16fft_read(pll, PLL_16FFT_STAT);
	return (stat & PLL_16FFT_STAT_LOCK) != 0U;
}

static int32_t ti_clk_pll_16fft_enable_pll(const struct ti_clk_data_pll_16fft *pll)
{
	uint32_t ctrl;
	int32_t err = 0;

	ctrl = ti_pll_16fft_read(pll, PLL_16FFT_CTRL);

	if ((ctrl & PLL_16FFT_CTRL_PLL_EN) == 0U) {
		ctrl |= PLL_16FFT_CTRL_PLL_EN;
		ti_pll_16fft_write(pll, PLL_16FFT_CTRL, ctrl);
		udelay(1U); /* Wait 1us */
	}

	return err;
}

static int32_t ti_clk_pll_16fft_disable_pll(const struct ti_clk_data_pll_16fft *pll)
{
	uint32_t ctrl;
	int32_t err = 0;

	ctrl = ti_pll_16fft_read(pll, PLL_16FFT_CTRL);

	if ((ctrl & PLL_16FFT_CTRL_PLL_EN) != 0U) {
		ctrl &= (uint32_t)~PLL_16FFT_CTRL_PLL_EN;
		ti_pll_16fft_write(pll, PLL_16FFT_CTRL, ctrl);
		udelay(1U); /* Wait 1us */
	}

	return err;
}

/**
 * ti_clk_pll_16fft_check_cal_lock() - Check if PLL deskew calibration is complete.
 * @pll: PLL instance data.
 *
 * Return: true if calibration lock is achieved, false otherwise.
 */
static bool ti_clk_pll_16fft_check_cal_lock(const struct ti_clk_data_pll_16fft *pll)
{
	uint32_t stat;

	stat = ti_pll_16fft_read(pll, PLL_16FFT_CAL_STAT);
	return (stat & PLL_16FFT_CAL_STAT_CAL_LOCK) != 0U;
}

static bool ti_clk_pll_16fft_wait_for_lock(struct ti_clk *clock_ptr)
{
	const struct ti_clk_data_pll_16fft *pll;
	const struct ti_clk_data_pll *data_pll;
	uint32_t i;
	bool success;

	uint32_t freq_ctrl1;
	uint32_t pllfm;
	uint32_t pll_type;
	uint32_t cfg;
	int32_t err;
	uint32_t cal, cal_en;

	assert(clock_ptr != NULL);

	data_pll = ti_container_of(clock_ptr->data, const struct ti_clk_data_pll,
				   data);
	pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft, data_pll);

	/*
	 * Minimum VCO input freq is 5MHz, and the longest a lock should
	 * be consider to be timed out after 750 cycles. Be conservative
	 * and assume each loop takes 10 cycles and we run at a
	 * max of 1GHz. That gives 15000 loop cycles. We may end up waiting
	 * longer than necessary for timeout, but that should be ok.
	 */
	success = false;
	for (i = 0U; i < (150U * 100U); i++) {
		if (ti_clk_pll_16fft_check_lock(pll)) {
			success = true;
			break;
		}
	}

	/* Disable calibration in the fractional mode of the FRACF PLL based on data
	 * from silicon and simulation data.
	 */
	freq_ctrl1 = ti_pll_16fft_read(pll, PLL_16FFT_FREQ_CTRL1);
	pllfm = freq_ctrl1 & PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MSK;
	pllfm >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT;

	cfg = ti_pll_16fft_read(pll, PLL_16FFT_CFG);
	pll_type = (cfg & PLL_16FFT_CFG_PLL_TYPE_MASK) >> PLL_16FFT_CFG_PLL_TYPE_SHIFT;

	cal = ti_pll_16fft_read(pll, PLL_16FFT_CAL_CTRL);
	cal_en = (cal & PLL_16FFT_CAL_CTRL_CAL_EN);

	if (!success ||
	    (pll_type != PLL_16FFT_CFG_PLL_TYPE_FRACF) || (pllfm != 0U) ||
	    (cal_en == 0U)) {
		return success;
	}

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
	for (i = 0U; i < (4350U * 100U); i++) {
		if (ti_clk_pll_16fft_check_cal_lock(pll)) {
			return true;
		}
	}

	/* In case of cal lock failure, operate without calibration */
	/* Disable PLL */
	err = ti_clk_pll_16fft_disable_pll(pll);
	if (err != 0) {
		return false;
	}

	/* Disable Calibration */
	ti_clk_pll_16fft_disable_cal(pll);

	/* Enable PLL */
	err = ti_clk_pll_16fft_enable_pll(pll);
	if (err != 0) {
		return false;
	}

	/* Wait for PLL Lock */
	success = false;
	for (i = 0U; i < (150U * 100U); i++) {
		if (ti_clk_pll_16fft_check_lock(pll)) {
			success = true;
			break;
		}
	}
	return success;
}

/**
 * ti_clk_pll_16fft_is_bypass() - Query the bypass state of the PLL.
 * @pll: PLL instance data.
 *
 * Return: true if the PLL is in bypass mode, false otherwise.
 */
static bool ti_clk_pll_16fft_is_bypass(const struct ti_clk_data_pll_16fft *pll)
{
	bool ret = false;
	uint32_t ctrl;

	/* IDLE Bypass */
	ctrl = ti_pll_16fft_read(pll, PLL_16FFT_CTRL);
	ret = (ctrl & PLL_16FFT_CTRL_BYPASS_EN) != 0U;

	return ret;
}

static int32_t ti_clk_pll_16fft_bypass(struct ti_clk *clock_ptr, bool bypass)
{
	const struct ti_clk_data_pll_16fft *pll;
	const struct ti_clk_data_pll *data_pll;
	uint32_t ctrl;

	data_pll = ti_container_of(clock_ptr->data, const struct ti_clk_data_pll,
				   data);
	pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft,
			      data_pll);

	ctrl = ti_pll_16fft_read(pll, PLL_16FFT_CTRL);
	/* Enable bypass only if its not bypassed already */
	if (bypass && ((ctrl & PLL_16FFT_CTRL_BYPASS_EN) == 0U)) {
		ctrl |= PLL_16FFT_CTRL_BYPASS_EN;
		ti_pll_16fft_write(pll, PLL_16FFT_CTRL, ctrl);
	} else if (!bypass && ((ctrl & PLL_16FFT_CTRL_BYPASS_EN) == PLL_16FFT_CTRL_BYPASS_EN)) {
		/* Disable bypass only if its bypassed */
		ctrl &= (uint32_t)~PLL_16FFT_CTRL_BYPASS_EN;
		ti_pll_16fft_write(pll, PLL_16FFT_CTRL, ctrl);
	}
	return 0;
}

static uint32_t ti_clk_pll_16fft_get_freq_internal(struct ti_clk *clock_ptr,
						   uint32_t clkod)
{
	const struct ti_clk_data_pll_16fft *pll;
	const struct ti_clk_data_pll *data_pll;
	uint32_t ret = 0U;
	uint32_t freq_ctrl0;
	uint32_t freq_ctrl1;
	uint32_t div_ctrl;
	uint32_t pllm;
	uint32_t m_frac_mult;
	uint32_t plld;
	uint32_t clkod_plld;

	data_pll = ti_container_of(clock_ptr->data, const struct ti_clk_data_pll,
				   data);
	pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft, data_pll);
	freq_ctrl0 = ti_pll_16fft_read(pll, PLL_16FFT_FREQ_CTRL0);
	freq_ctrl1 = ti_pll_16fft_read(pll, PLL_16FFT_FREQ_CTRL1);
	div_ctrl = ti_pll_16fft_read(pll, PLL_16FFT_DIV_CTRL);

	pllm = (uint32_t)(freq_ctrl0 & PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MSK);
	pllm >>= PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT;

	m_frac_mult = freq_ctrl1 & PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MSK;
	m_frac_mult >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT;

	plld = div_ctrl & PLL_16FFT_DIV_CTRL_REF_DIV_MASK;
	plld >>= PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT;

	clkod_plld = clkod * plld;

	if (clkod_plld != 0UL) {
		uint32_t parent_freq_hz;
		uint64_t ret64;
		uint64_t rem;

		/* Calculate non-fractional part */
		parent_freq_hz = ti_clk_get_parent_freq(clock_ptr);
		ret64 = ((uint64_t) parent_freq_hz / clkod_plld) * pllm;
		rem = ((uint64_t) parent_freq_hz % clkod_plld) * pllm;
		ret64 += rem / (uint64_t) clkod_plld;
		rem = rem % (uint64_t) clkod_plld;

		if (m_frac_mult != 0UL) {
			uint64_t fret;	     /* Fraction return value */
			uint64_t frem;	     /* Fraction remainder */
			const uint32_t mask = (1UL << PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BIT_CNT) -
				1UL;

			/* Calculate fractional component of frequency */
			fret = ((uint64_t) parent_freq_hz / clkod_plld) * m_frac_mult;
			frem = ((uint64_t) parent_freq_hz % clkod_plld) * m_frac_mult;
			fret += frem / (uint64_t) clkod_plld;
			frem = frem % (uint64_t) clkod_plld;

			/* Fold back into return/remainder */
			frem += (fret & (uint64_t) mask) * clkod_plld;

			/* Round the fractional component. Above mid point is rounded up*/
			fret += (uint32_t) 1U << (PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BIT_CNT - 1U);

			ret64 += fret >> PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BIT_CNT;
			rem += frem >> PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BIT_CNT;

			ret64 += ((uint64_t) rem) / clkod_plld;
		}

		ret = (uint32_t) ret64;
	}

	return ret;
}

static bool ti_clk_pll_16fft_program_freq(struct ti_clk *pll_clk,
					  struct ti_clk *div_clk,
					  const struct ti_clk_data_pll_16fft *pll,
					  uint32_t plld,
					  uint32_t pllm,
					  uint32_t pllfm,
					  uint32_t clkod)
{
	const struct ti_clk_drv_div *drv_div;
	uint32_t freq_ctrl0;
	uint32_t freq_ctrl1;
	uint32_t div_ctrl;
	uint32_t ctrl;
	bool ret = true;
	uint32_t cfg;
	uint32_t pll_type;

	pll_clk->flags &= (uint8_t) ~TI_CLK_FLAG_CACHED;

	if (!ti_clk_pll_16fft_is_bypass(pll)) {
		/* Put the PLL into bypass */
		(void)ti_clk_pll_16fft_bypass(pll_clk, true);
	}

	/* Disable the PLL */
	(void)ti_clk_pll_16fft_disable_pll(pll);

	cfg = ti_pll_16fft_read(pll, PLL_16FFT_CFG);
	pll_type = (cfg & PLL_16FFT_CFG_PLL_TYPE_MASK) >> PLL_16FFT_CFG_PLL_TYPE_SHIFT;

	/* Program the new rate */
	freq_ctrl0 = ti_pll_16fft_read(pll, PLL_16FFT_FREQ_CTRL0);
	freq_ctrl1 = ti_pll_16fft_read(pll, PLL_16FFT_FREQ_CTRL1);
	div_ctrl = ti_pll_16fft_read(pll, PLL_16FFT_DIV_CTRL);

	freq_ctrl0 &= (uint32_t)~PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MSK;
	freq_ctrl0 |= (uint32_t)(pllm << PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT);

	freq_ctrl1 &= (uint32_t)~PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MSK;
	freq_ctrl1 |= (uint32_t)(pllfm << PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT);

	div_ctrl &= (uint32_t)~PLL_16FFT_DIV_CTRL_REF_DIV_MASK;
	div_ctrl |= (uint32_t)(plld << PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT);

	/* Make sure we have fractional support if required */
	ctrl = ti_pll_16fft_read(pll, PLL_16FFT_CTRL);

	/* Don't use internal bypass,it is not glitch free. Always prefer glitchless bypass */
	ctrl &= (uint32_t)~PLL_16FFT_CTRL_INTL_BYP_EN;

	/* Always enable output if PLL */
	ctrl |= PLL_16FFT_CTRL_CLK_POSTDIV_EN;

	/* Currently unused by all PLLs */
	ctrl &= (uint32_t)~PLL_16FFT_CTRL_CLK_4PH_EN;

	/* Always bypass if we lose lock */
	ctrl |= PLL_16FFT_CTRL_BYP_ON_LOCKLOSS;

	/* Enable fractional support if required */
	if (pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF) {
		if (pllfm != 0UL) {
			ctrl |= PLL_16FFT_CTRL_DSM_EN;
			ctrl |= PLL_16FFT_CTRL_DAC_EN;
		} else {
			ctrl &= (uint32_t)~PLL_16FFT_CTRL_DSM_EN;
			ctrl &= (uint32_t)~PLL_16FFT_CTRL_DAC_EN;
		}
	}

	/* Enable Fractional by default for PLL_16FFT_CFG_PLL_TYPE_FRAC2 */
	if (pll_type == PLL_16FFT_CFG_PLL_TYPE_FRAC2) {
		ctrl |= PLL_16FFT_CTRL_DSM_EN;
		ctrl |= PLL_16FFT_CTRL_DAC_EN;
	}

	ti_pll_16fft_write(pll, PLL_16FFT_FREQ_CTRL0, freq_ctrl0);
	ti_pll_16fft_write(pll, PLL_16FFT_FREQ_CTRL1, freq_ctrl1);
	ti_pll_16fft_write(pll, PLL_16FFT_DIV_CTRL, div_ctrl);
	ti_pll_16fft_write(pll, PLL_16FFT_CTRL, ctrl);

	/* Program output divider */
	if (div_clk != NULL) {
		if (div_clk->drv != NULL) {
			drv_div = ti_container_of(div_clk->drv,
						  const struct ti_clk_drv_div, drv);
		} else {
			drv_div = NULL;
		}
		if (drv_div && drv_div->set_div) {
			ret = drv_div->set_div(div_clk, clkod);
		}
	}

	if (ret) {
		ti_pll_16fft_write(pll, PLL_16FFT_CTRL, ctrl);
	}

	/* Configure PLL calibration*/
	if ((pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF) && ret) {
		if (pllfm != 0UL) {
			/* Disable Calibration in Fractional mode */
			ti_clk_pll_16fft_disable_cal(pll);
		} else {
			/* Enable Calibration in Integer mode */
			ti_clk_pll_16fft_cal_option3(pll);
		}
	}

	/*
	 * Wait at least 1 ref cycle before enabling PLL.
	 * Minimum VCO input frequency is 5MHz, therefore maximum
	 * wait time for 1 ref clock is 0.2us.
	 */
	if (ret) {
		udelay(1U);
		(void)ti_clk_pll_16fft_enable_pll(pll);
	}

	if ((pll_clk->ref_count != 0U) && ret) {
		/* Take the PLL out of bypass */
		ret = ti_clk_pll_16fft_wait_for_lock(pll_clk);
		if (ret) {
			(void)ti_clk_pll_16fft_bypass(pll_clk, false);
		}
	}

	return ret;
}

static uint32_t ti_pll_16fft_set_freq(struct ti_clk *pll_clk,
				      struct ti_clk *div_clk,
				      const struct ti_pll_data *pll_data_t,
				      uint32_t target_hz,
				      uint32_t min_hz,
				      uint32_t max_hz,
				      bool *changed)
{
	const struct ti_clk_data_pll_16fft *pll;
	const struct ti_clk_data_pll *data_pll;
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

	data_pll = ti_container_of(pll_clk->data, const struct ti_clk_data_pll,
				   data);
	pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft,
			      data_pll);

	was_bypass = ti_clk_pll_16fft_is_bypass(pll);

	freq_ctrl0 = ti_pll_16fft_read(pll, PLL_16FFT_FREQ_CTRL0);
	freq_ctrl1 = ti_pll_16fft_read(pll, PLL_16FFT_FREQ_CTRL1);
	div_ctrl = ti_pll_16fft_read(pll, PLL_16FFT_DIV_CTRL);

	/* Check current values */
	prev_pllm = (uint32_t)(freq_ctrl0 & PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MSK);
	prev_pllm >>= PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_SHIFT;

	prev_pllfm = freq_ctrl1 & PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MSK;
	prev_pllfm >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_SHIFT;

	prev_plld = div_ctrl & PLL_16FFT_DIV_CTRL_REF_DIV_MASK;
	prev_plld >>= PLL_16FFT_DIV_CTRL_REF_DIV_SHIFT;

	if (div_clk != NULL) {
		prev_clkod = ti_clk_get_div(div_clk);
	} else {
		prev_clkod = 1U;
	}

	/* Compute new values */
	parent_freq_hz = ti_clk_get_parent_freq(pll_clk);
	freq = ti_pll_calc(pll_clk, pll_data_t, parent_freq_hz,
			   target_hz, min_hz, max_hz,
			   &plld, &pllm, &pllfm, &clkod);

	*changed = true;
	/* No need to do anything if they haven't changed */
	if ((plld == prev_plld) && (pllm == prev_pllm) && (clkod == prev_clkod) &&
	    (pllfm == prev_pllfm) && !was_bypass) {
		*changed = false;
	}

	if ((freq != 0UL) && *changed) {
		if (!ti_clk_pll_16fft_program_freq(pll_clk, div_clk, pll,
						   plld, pllm, pllfm, clkod)) {
			freq = 0U;
		}
	}

	return freq;
}

/**
 * ti_pll_16fft_set_freq_table() - Set frequency using a PLL table entry.
 * @pll_clk: Parent PLL clock of the div_clk.
 * @div_clk: HSDIV clock whose frequency is to be changed.
 * @pll_data_t: PLL driver data describing valid parameter ranges.
 * @target_hz: Required output frequency in Hz.
 * @min_hz: Minimum acceptable frequency in Hz.
 * @max_hz: Maximum acceptable frequency in Hz.
 * @changed: Set to true if the PLL parameters were changed.
 *
 * Check if the desired frequency can be derived from VCO frequencies listed
 * in the PLL table. If so, program those parameters.
 *
 * Return: The actual frequency set if derivable from the PLL table, 0 otherwise.
 */
static uint32_t ti_pll_16fft_set_freq_table(struct ti_clk *pll_clk,
					    struct ti_clk *div_clk,
					    const struct ti_pll_data *pll_data_t,
					    uint32_t target_hz,
					    uint32_t min_hz,
					    uint32_t max_hz,
					    bool *changed)
{
	const struct ti_clk_data_pll_16fft *pll;
	const struct ti_clk_data_pll *data_pll;
	const struct ti_clk_data_div *data_div;
	uint32_t div0 = 0U, div1;
	uint32_t div0_delta = UINT_MAX;
	uint32_t div1_delta = UINT_MAX;
	uint32_t div0_hz, div1_hz;
	uint32_t freq = 0U;
	uint32_t input;
	size_t i;
	uint32_t n;
	uint32_t rem;
	uint64_t actual64;
	uint32_t actual = 0U;
	uint32_t clkod_plld;

	data_pll = ti_container_of(pll_clk->data, const struct ti_clk_data_pll,
				   data);
	pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft, data_pll);
	data_div = ti_container_of(div_clk->data, const struct ti_clk_data_div,
				   data);

	input = ti_clk_get_parent_freq(pll_clk);

	n = data_div->max_div;

	for (i = 0; i < data_pll->pll_entries_count; i++) {
		/*
		 * Determine actual frequency given table entry would produce:
		 *
		 * actual = input * pllm / (plld * clkod)
		 */
		clkod_plld = (uint32_t)ti_soc_pll_table[data_pll->pll_entries[i]].plld *
			     (uint32_t)ti_soc_pll_table[data_pll->pll_entries[i]].clkod;

		actual64 = ((uint64_t)input * ti_soc_pll_table[data_pll->pll_entries[i]].pllm) /
			   clkod_plld;
		rem = (uint32_t)(((uint64_t)input *
				  ti_soc_pll_table[data_pll->pll_entries[i]].pllm) %
				 clkod_plld);

		if (ti_soc_pll_table[data_pll->pll_entries[i]].pllfm != 0UL) {
			uint64_t fret;
			uint64_t frem;
			uint32_t stride = 1U;
			uint32_t pllfm_bits;
			uint32_t pllfm_range;
			uint32_t pllfm_mask;
			uint32_t pllm;

			pllfm_bits = pll_data_t->pllfm_bits;
			pllfm_range = (uint32_t) (1UL << pllfm_bits);
			pllfm_mask = pllfm_range - 1U;

			if (pll_data_t->pllm_stride != NULL) {
				pllm = ti_soc_pll_table[data_pll->pll_entries[i]].pllm;
				stride = pll_data_t->pllm_stride(pll_clk,
								 pllm);
			}

			/* Calculate fractional component of frequency */
			fret = ((uint64_t) input / clkod_plld) *
				ti_soc_pll_table[data_pll->pll_entries[i]].pllfm;
			frem = ((uint64_t) input % clkod_plld) *
				ti_soc_pll_table[data_pll->pll_entries[i]].pllfm;
			if (frem >= clkod_plld) {
				fret += ((uint64_t) frem) / clkod_plld;
				frem = ((uint64_t) frem) % clkod_plld;
			}
			fret *= stride;
			frem *= stride;
			if (frem >= clkod_plld) {
				fret += ((uint64_t) frem) / clkod_plld;
				frem = ((uint64_t) frem) % clkod_plld;
			}
			frem += ((uint64_t) (fret & pllfm_mask)) * clkod_plld;

			/* Add fractional part */
			actual64 += fret >> pllfm_bits;
			rem += (uint32_t) (frem >> pllfm_bits);

			actual64 += ((uint64_t) rem) / clkod_plld;
		}

		actual = (uint32_t) actual64;

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

	if (*changed) {
		if (!ti_clk_pll_16fft_program_freq(pll_clk, div_clk, pll,
						   ti_soc_pll_table[data_pll->pll_entries[i]].plld,
						   ti_soc_pll_table[data_pll->pll_entries[i]].pllm,
						   ti_soc_pll_table[data_pll->pll_entries[i]].pllfm,
						   div0)) {
			freq = 0U;
		}
	}

	return freq;
}

/**
 * ti_clk_pll_16fft_set_freq() - Set the 16FFT PLL output frequency.
 * @clock_ptr: The PLL clock to program.
 * @target_hz: Target frequency in Hz.
 * @min_hz: Minimum acceptable frequency in Hz.
 * @max_hz: Maximum acceptable frequency in Hz.
 * @changed: Set to true if PLL parameters were changed.
 *
 * Glue function called only by pll_init. Sets the frequency of the
 * foutpostdiv_clk output rather than the DCO directly.
 *
 * Return: The actual frequency set, or 0 on failure.
 */
static uint32_t ti_clk_pll_16fft_set_freq(struct ti_clk *clock_ptr,
					  uint32_t target_hz,
					  uint32_t min_hz,
					  uint32_t max_hz,
					  bool *changed)
{
	const struct ti_pll_data *pll_data_t;
	struct ti_clk *div_clk = NULL;
	struct ti_clk *sub_clk = NULL;
	struct ti_clk *pll_clk = NULL;
	const struct ti_clk_parent *p = NULL;
	uint32_t ret = 0U;
	uint32_t i;

	assert(clock_ptr != NULL);
	assert(changed != NULL);

	/* Find our associated postdiv */
	for (i = 0U; (div_clk == NULL) && (i < soc_clock_count); i++) {
		sub_clk = &soc_clocks[i];

		if (sub_clk->drv == &ti_clk_drv_div_pll_16fft_postdiv.drv) {
			p = ti_clk_mux_get_parent(sub_clk);
			if (p != NULL) {
				pll_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
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
	 * is a bit ambiguous.
	 *
	 * The current rule to resolve ambiguity is that if a PLL has a
	 * connected FOUTPOSTDIV, the default frequency given is for this
	 * output. If there is no FOUTPOSTDIV, the default frequency given
	 * is for FOUTVCO.
	 */

	if (div_clk != NULL) {
		/*
		 * This is a clock with FOUTPOSTDIV. Utilize the postdiv
		 * output divider when determining a new frequency.
		 */
		pll_data_t = &ti_pll_16fft_postdiv_data;
	} else {
		/*
		 * This is a clock without FOUTPOSTDIV. The clock
		 * output divider must be 1.
		 */
		pll_data_t = &ti_pll_16fft_raw_data;
	}

	ret = ti_pll_16fft_set_freq(clock_ptr, div_clk, pll_data_t,
				    target_hz, min_hz, max_hz,
				    changed);

	return ret;
}

/**
 * ti_clk_pll_16fft_get_freq() - Return the 16FFT PLL DCO frequency.
 * @clock_ptr: The PLL clock instance.
 *
 * Return: The DCO output frequency in Hz, or 0 if the PLL is disabled or in bypass.
 */
static uint32_t ti_clk_pll_16fft_get_freq(struct ti_clk *clock_ptr)
{
	uint32_t ret;

	assert(clock_ptr != NULL);

	/*
	 * If disabled, return the frequency we would be running at once
	 * we bring it out of bypass. If enabled and in bypass, return 0.
	 */
	if (clock_ptr->ref_count == 0U) {
		ret = 0U;
	} else if ((clock_ptr->flags & TI_CLK_FLAG_CACHED) != 0U) {
		ret = ti_clk_get_value(clock_ptr->freq_idx);
	} else {
		const struct ti_clk_data_pll_16fft *pll;
		const struct ti_clk_data_pll *data_pll;

		data_pll = ti_container_of(clock_ptr->data,
					   const struct ti_clk_data_pll, data);
		pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft,
				      data_pll);

		if (!ti_clk_pll_16fft_is_bypass(pll)) {
			ret = ti_clk_pll_16fft_get_freq_internal(clock_ptr, 1U);
		} else {
			ret = 0U;
		}
		ti_clk_set_value(clock_ptr->freq_idx, ret);
		clock_ptr->flags |= TI_CLK_FLAG_CACHED;
	}

	return ret;
}

static bool ti_clk_pll_16fft_set_state(struct ti_clk *clock_ptr, bool enabled)
{
	bool ret = true;

	if (enabled) {
		ret = ti_clk_pll_16fft_wait_for_lock(clock_ptr);
	}

	clock_ptr->flags &= (uint8_t) ~TI_CLK_FLAG_CACHED;
	(void)ti_clk_pll_16fft_bypass(clock_ptr, !enabled);

	return ret;
}

/**
 * ti_clk_pll_16fft_get_state() - Get the hardware state of the 16FFT PLL.
 * @clock_ptr: The PLL clock instance.
 *
 * Return: TI_CLK_HW_STATE_ENABLED, TI_CLK_HW_STATE_DISABLED, or
 *         TI_CLK_HW_STATE_TRANSITION depending on lock and bypass status.
 */
static uint32_t ti_clk_pll_16fft_get_state(struct ti_clk *clock_ptr)
{
	struct ti_clk *clock_parent = NULL;
	uint32_t ret = TI_CLK_HW_STATE_ENABLED;

	if (clock_ptr->ref_count == 0U) {
		ret = TI_CLK_HW_STATE_DISABLED;
	}

	/* PLLs can't function without an enabled parent */
	if (ret == TI_CLK_HW_STATE_ENABLED) {
		const struct ti_clk_parent *p;

		p = ti_clk_mux_get_parent(clock_ptr);
		if (p != NULL) {
			clock_parent = ti_clk_lookup(p->clk);
		}
		if (clock_parent != NULL) {
			ret = ti_clk_get_state(clock_parent);
		} else {
			ret = TI_CLK_HW_STATE_DISABLED;
		}
	}

	if (ret == TI_CLK_HW_STATE_ENABLED) {
		const struct ti_clk_data_pll_16fft *pll;
		const struct ti_clk_data_pll *data_pll;

		data_pll = ti_container_of(clock_ptr->data,
					   const struct ti_clk_data_pll, data);
		pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft,
				      data_pll);

		if (!ti_clk_pll_16fft_is_bypass(pll) && !ti_clk_pll_16fft_check_lock(pll)) {
			ret = TI_CLK_HW_STATE_TRANSITION;
		}
	}

	return ret;
}

static struct ti_clk *ti_clk_pll_16fft_hsdiv_get_pll_root(struct ti_clk *clock_ptr)
{
	const struct ti_clk_parent *p = NULL;
	struct ti_clk *pll_clk = NULL;

	/* Get PLL root */
	p = ti_clk_mux_get_parent(clock_ptr);
	if (p != NULL) {
		pll_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
	}

	return pll_clk;
}

static struct ti_clk *ti_clk_pll_16fft_postdiv_get_pll_root(struct ti_clk *clock_ptr)
{
	const struct ti_clk_parent *p = NULL;
	struct ti_clk *pll_clk = NULL;

	/* Get PLL post divider */
	p = ti_clk_mux_get_parent(clock_ptr);
	if (p != NULL) {
		pll_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
	}

	/* Verify correct clock tree */
	if (pll_clk && (pll_clk->drv == &ti_clk_drv_div_pll_16fft_postdiv.drv)) {
		/* Get PLL root */
		p = ti_clk_mux_get_parent(pll_clk);
		if (p != NULL) {
			pll_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
		} else {
			pll_clk = NULL;
		}
	}

	return pll_clk;
}

static int32_t ti_clk_pll_16fft_init_internal(struct ti_clk *clock_ptr)
{
	const struct ti_clk_data_pll_16fft *pll;
	const struct ti_clk_data_pll *data_pll;
	uint32_t i;
	int32_t ret;
	bool skip_hw_init = false;

	clock_ptr->flags &= (uint8_t) ~TI_CLK_FLAG_CACHED;

	data_pll = ti_container_of(clock_ptr->data, const struct ti_clk_data_pll,
				   data);
	pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft, data_pll);

	/*
	 * In order to honor the TI_CLK_DATA_FLAG_NO_HW_REINIT flag when set,
	 * we must check if the clk is enabled, and if so, skip re-setting
	 * default frequency if it is available.
	 */
	if (((clock_ptr->data_flags & TI_CLK_DATA_FLAG_NO_HW_REINIT) != 0U) &&
	    (ti_clk_pll_16fft_check_lock(pll) == true)) {
		skip_hw_init = true;
	}

	if (skip_hw_init == false) {
		ret = ti_pll_init(clock_ptr);
		if (ret != 0) {
			return ret;
		}
	}

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
	for (i = 0; i < soc_clock_count; i++) {
		struct ti_clk *sub_data = &soc_clocks[i];
		struct ti_clk *sub_clk = &soc_clocks[i];

		if ((sub_data->drv == &ti_clk_drv_div_reg.drv) &&
		    sub_data->drv->init) {
			if (ti_clk_pll_16fft_postdiv_get_pll_root(sub_clk) == clock_ptr) {
				(void)sub_data->drv->init(sub_clk);
			}
		} else if ((sub_data->drv == &ti_clk_drv_div_pll_16fft_hsdiv.drv) &&
			   sub_data->drv->init) {
			if (ti_clk_pll_16fft_hsdiv_get_pll_root(sub_clk) == clock_ptr) {
				(void)sub_data->drv->init(sub_clk);
			}
		} else {
			/* Do Nothing */
		}
	}

	/*
	 * We must always assume we are enabled as we could be operating
	 * clocks in bypass.
	 */
	clock_ptr->flags |= TI_CLK_FLAG_PWR_UP_EN;

	return 0;
}

static int32_t ti_clk_pll_16fft_init(struct ti_clk *clock_ptr)
{
	assert(clock_ptr != NULL);

	return ti_clk_pll_16fft_init_internal(clock_ptr);
}

const struct ti_clk_drv ti_clk_drv_pll_16fft = {
	.init = ti_clk_pll_16fft_init,
	.get_freq = ti_clk_pll_16fft_get_freq,
	.set_freq = ti_clk_pll_16fft_set_freq,
	.get_state = ti_clk_pll_16fft_get_state,
	.set_state = ti_clk_pll_16fft_set_state,
};

static uint32_t ti_clk_pll_16fft_postdiv_set_freq(struct ti_clk *clock_ptr,
						  uint32_t target_hz,
						  uint32_t min_hz,
						  uint32_t max_hz,
						  bool *changed)
{
	if ((clock_ptr->data_flags & TI_CLK_DATA_FLAG_MODIFY_PARENT_FREQ) != 0U) {
		/* Program the whole PLL */
		const struct ti_clk_parent *p;
		struct ti_clk *pll_clk;

		p = ti_clk_mux_get_parent(clock_ptr);
		if (p == NULL) {
			return 0U;
		}

		pll_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
		if (pll_clk == NULL) {
			return 0U;
		}

		return ti_pll_16fft_set_freq(pll_clk, clock_ptr,
					     &ti_pll_16fft_postdiv_data,
					     target_hz, min_hz,
					     max_hz,
					     changed);
	}

	/* Just program the output divider. */
	return ti_clk_div_set_freq(clock_ptr, target_hz, min_hz, max_hz,
				   changed);
}

static bool ti_clk_pll_16fft_postdiv_valid_div(struct ti_clk *clock_ptr __unused,
					       uint32_t div)
{
	return (div < ARRAY_SIZE(postdiv_mapping)) && (postdiv_mapping[div] != 0U);
}

static bool ti_clk_pll_16fft_postdiv_set_div(struct ti_clk *clock_ptr, uint32_t div)
{
	const struct ti_clk_parent *p;
	struct ti_clk *pll_clk = NULL;

	p = ti_clk_mux_get_parent(clock_ptr);
	if (p != NULL) {
		pll_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
	}

	if (pll_clk != NULL) {
		const struct ti_clk_data_pll *data_pll;
		const struct ti_clk_data_pll_16fft *pll;
		uint32_t div_ctrl;
		uint32_t post_div1;
		uint32_t post_div2;
		uint8_t val;

		data_pll = ti_container_of(pll_clk->data,
					   const struct ti_clk_data_pll, data);
		pll = ti_container_of(data_pll, const struct ti_clk_data_pll_16fft,
				      data_pll);

		div_ctrl = ti_pll_16fft_read(pll, PLL_16FFT_DIV_CTRL);
		div_ctrl &= (uint32_t)~PLL_16FFT_DIV_CTRL_POST_DIV2_MASK;
		div_ctrl &= (uint32_t)~PLL_16FFT_DIV_CTRL_POST_DIV1_MASK;

		val = postdiv_mapping[div];
		post_div1 = (uint32_t) val & 0xFU;
		post_div2 = (uint32_t) val >> 4UL;

		div_ctrl |= post_div1 << PLL_16FFT_DIV_CTRL_POST_DIV1_SHIFT;
		div_ctrl |= post_div2 << PLL_16FFT_DIV_CTRL_POST_DIV2_SHIFT;

		ti_pll_16fft_write(pll, PLL_16FFT_DIV_CTRL, div_ctrl);
	}

	return true;
}

static uint32_t ti_clk_pll_16fft_postdiv_get_div(struct ti_clk *clock_ptr)
{
	const struct ti_clk_parent *p;
	struct ti_clk *pll_clk;
	const struct ti_clk_data_pll *data_pll;
	const struct ti_clk_data_pll_16fft *pll;
	uint32_t div_ctrl;
	uint32_t post_div1;
	uint32_t post_div2;

	p = ti_clk_mux_get_parent(clock_ptr);
	if (p == NULL) {
		return 0U;
	}

	pll_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
	if (pll_clk == NULL) {
		return 0U;
	}

	data_pll = ti_container_of(pll_clk->data,
				   const struct ti_clk_data_pll, data);
	pll = ti_container_of(data_pll,
			      const struct ti_clk_data_pll_16fft, data_pll);

	div_ctrl = ti_pll_16fft_read(pll, PLL_16FFT_DIV_CTRL);
	post_div1 = (uint32_t)(div_ctrl & PLL_16FFT_DIV_CTRL_POST_DIV1_MASK);
	post_div1 >>= PLL_16FFT_DIV_CTRL_POST_DIV1_SHIFT;
	post_div2 = div_ctrl & PLL_16FFT_DIV_CTRL_POST_DIV2_MASK;
	post_div2 >>= PLL_16FFT_DIV_CTRL_POST_DIV2_SHIFT;

	return post_div1 * post_div2;
}

const struct ti_clk_drv_div ti_clk_drv_div_pll_16fft_postdiv = {
	.drv = {
		.init = ti_clk_div_init,
		.get_freq = ti_clk_div_get_freq,
		.set_freq = ti_clk_pll_16fft_postdiv_set_freq,
	},
	.valid_div = ti_clk_pll_16fft_postdiv_valid_div,
	.set_div = ti_clk_pll_16fft_postdiv_set_div,
	.get_div = ti_clk_pll_16fft_postdiv_get_div,
};

static uint32_t ti_clk_pll_16fft_hsdiv_set_freq(struct ti_clk *clock_ptr,
						uint32_t target_hz,
						uint32_t min_hz,
						uint32_t max_hz,
						bool *changed)
{
	uint32_t ret = 0U;
	const struct ti_clk_parent *p;
	struct ti_clk *pll_clk = NULL;

	if ((clock_ptr->data_flags & TI_CLK_DATA_FLAG_MODIFY_PARENT_FREQ) == 0U) {
		/* Just program the output divider. */
		return ti_clk_div_set_freq(clock_ptr, target_hz, min_hz, max_hz,
					   changed);
	}

	/* Program the whole PLL */
	p = ti_clk_mux_get_parent(clock_ptr);
	if (p != NULL) {
		pll_clk = ti_clk_lookup((ti_clk_idx_t) p->clk);
	}

	if (pll_clk == NULL) {
		return ret;
	}

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
	ret = ti_clk_div_set_freq_static_parent(clock_ptr,
						target_hz,
						target_hz,
						target_hz,
						changed);

	if (ret != 0U) {
		return ret;
	}

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
	ret = ti_pll_16fft_set_freq_table(pll_clk,
					  clock_ptr,
					  &ti_pll_16fft_hsdiv_data,
					  target_hz,
					  target_hz,
					  target_hz,
					  changed);

	if (ret != 0U) {
		return ret;
	}

	/*
	 * First try setting the exact target_hz frequency
	 * without using the min and max range. We do this
	 * because some times even when the target frequency
	 * is achievable, the calculations will result
	 * in choosing a value other than the target from the
	 * range provided.
	 */
	ret = ti_pll_16fft_set_freq(pll_clk, clock_ptr,
				    &ti_pll_16fft_hsdiv_data,
				    target_hz,
				    target_hz, target_hz,
				    changed);
	if (ret != 0U) {
		return ret;
	}

	/*
	 * If the previous step failed in setting the exact
	 * target_hz, use the min and max range provided.
	 */
	ret = ti_pll_16fft_set_freq(pll_clk,
				    clock_ptr,
				    &ti_pll_16fft_hsdiv_data,
				    target_hz,
				    min_hz,
				    max_hz,
				    changed);

	return ret;
}

static int32_t ti_clk_pll_16fft_hsdiv_init(struct ti_clk *clkp)
{
	const struct ti_clk_data_div *data_div;
	const struct ti_clk_data_div_reg *data_reg;
	const struct ti_clk_drv_div *drv_div;
	bool skip_hw_init = false;
	uint32_t hsdiv_ctrl;

	data_div = ti_container_of(clkp->data, const struct ti_clk_data_div,
				   data);
	data_reg = ti_container_of(data_div, const struct ti_clk_data_div_reg,
				   data_div);
	drv_div = ti_container_of(clkp->drv, const struct ti_clk_drv_div, drv);

	hsdiv_ctrl = mmio_read_32(data_reg->reg);

	if (((clkp->data_flags & TI_CLK_DATA_FLAG_NO_HW_REINIT) != 0U) &&
	    ((hsdiv_ctrl & PLL_16FFT_HSDIV_CTRL_CLKOUT_EN) != 0U)) {
		if (drv_div->get_div != NULL) {
			/*
			 * In order to honor the TI_CLK_DATA_FLAG_NO_HW_REINIT flag when set,
			 * we must check if the HSDIV is configured. If the HSDIV
			 * is the default value of 1 then we consider it is not configured.
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
			mmio_clrbits_32((uintptr_t)data_reg->reg,
					PLL_16FFT_HSDIV_CTRL_CLKOUT_EN);

			if (!drv_div->set_div(clkp, data_div->default_div)) {
				return -EINVAL;
			}

			/* Enable HSDIV */
			mmio_setbits_32((uintptr_t)data_reg->reg,
					PLL_16FFT_HSDIV_CTRL_CLKOUT_EN);
		}
	}

	return 0;
}

const struct ti_clk_drv_div ti_clk_drv_div_pll_16fft_hsdiv = {
	.drv = {
		.init = ti_clk_pll_16fft_hsdiv_init,
		.get_freq = ti_clk_div_get_freq,
		.set_freq = ti_clk_pll_16fft_hsdiv_set_freq,
	},
	.set_div = ti_clk_div_reg_set_div,
	.get_div = ti_clk_div_reg_get_div,
};

const struct ti_clk_drv_div ti_clk_drv_div_pll_16fft_postdiv_hsdiv = {
	.drv = {
		.set_freq = ti_clk_div_set_freq,
		.get_freq = ti_clk_div_get_freq,
		.init = ti_clk_pll_16fft_hsdiv_init,
	},
	.set_div = ti_clk_div_reg_set_div,
	.get_div = ti_clk_div_reg_get_div,
};
