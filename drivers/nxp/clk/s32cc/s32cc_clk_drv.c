/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>

#include <s32cc-clk-regs.h>

#include <common/debug.h>
#include <drivers/clk.h>
#include <lib/mmio.h>
#include <s32cc-clk-modules.h>
#include <s32cc-clk-utils.h>

#define MAX_STACK_DEPTH		(15U)

struct s32cc_clk_drv {
	uintptr_t fxosc_base;
};

static int update_stack_depth(unsigned int *depth)
{
	if (*depth == 0U) {
		return -ENOMEM;
	}

	(*depth)--;
	return 0;
}

static struct s32cc_clk_drv *get_drv(void)
{
	static struct s32cc_clk_drv driver = {
		.fxosc_base = FXOSC_BASE_ADDR,
	};

	return &driver;
}

static int enable_module(const struct s32cc_clk_obj *module, unsigned int *depth);

static int enable_clk_module(const struct s32cc_clk_obj *module,
			     const struct s32cc_clk_drv *drv,
			     unsigned int *depth)
{
	const struct s32cc_clk *clk = s32cc_obj2clk(module);
	int ret;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if (clk == NULL) {
		return -EINVAL;
	}

	if (clk->module != NULL) {
		return enable_module(clk->module, depth);
	}

	if (clk->pclock != NULL) {
		return enable_clk_module(&clk->pclock->desc, drv, depth);
	}

	return -EINVAL;
}

static void enable_fxosc(const struct s32cc_clk_drv *drv)
{
	uintptr_t fxosc_base = drv->fxosc_base;
	uint32_t ctrl;

	ctrl = mmio_read_32(FXOSC_CTRL(fxosc_base));
	if ((ctrl & FXOSC_CTRL_OSCON) != U(0)) {
		return;
	}

	ctrl = FXOSC_CTRL_COMP_EN;
	ctrl &= ~FXOSC_CTRL_OSC_BYP;
	ctrl |= FXOSC_CTRL_EOCV(0x1);
	ctrl |= FXOSC_CTRL_GM_SEL(0x7);
	mmio_write_32(FXOSC_CTRL(fxosc_base), ctrl);

	/* Switch ON the crystal oscillator. */
	mmio_setbits_32(FXOSC_CTRL(fxosc_base), FXOSC_CTRL_OSCON);

	/* Wait until the clock is stable. */
	while ((mmio_read_32(FXOSC_STAT(fxosc_base)) & FXOSC_STAT_OSC_STAT) == U(0)) {
	}
}

static int enable_osc(const struct s32cc_clk_obj *module,
		      const struct s32cc_clk_drv *drv,
		      unsigned int *depth)
{
	const struct s32cc_osc *osc = s32cc_obj2osc(module);
	int ret = 0;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	switch (osc->source) {
	case S32CC_FXOSC:
		enable_fxosc(drv);
		break;
	/* FIRC and SIRC oscillators are enabled by default */
	case S32CC_FIRC:
		break;
	case S32CC_SIRC:
		break;
	default:
		ERROR("Invalid oscillator %d\n", osc->source);
		ret = -EINVAL;
		break;
	};

	return ret;
}

static int enable_module(const struct s32cc_clk_obj *module, unsigned int *depth)
{
	const struct s32cc_clk_drv *drv = get_drv();
	int ret = 0;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if (drv == NULL) {
		return -EINVAL;
	}

	switch (module->type) {
	case s32cc_osc_t:
		ret = enable_osc(module, drv, depth);
		break;
	case s32cc_clk_t:
		ret = enable_clk_module(module, drv, depth);
		break;
	case s32cc_clkmux_t:
		ret = -ENOTSUP;
		break;
	case s32cc_shared_clkmux_t:
		ret = -ENOTSUP;
		break;
	case s32cc_pll_t:
		ret = -ENOTSUP;
		break;
	case s32cc_pll_out_div_t:
	case s32cc_fixed_div_t:
		ret = -ENOTSUP;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int s32cc_clk_enable(unsigned long id)
{
	unsigned int depth = MAX_STACK_DEPTH;
	const struct s32cc_clk *clk;

	clk = s32cc_get_arch_clk(id);
	if (clk == NULL) {
		return -EINVAL;
	}

	return enable_module(&clk->desc, &depth);
}

static void s32cc_clk_disable(unsigned long id)
{
}

static bool s32cc_clk_is_enabled(unsigned long id)
{
	return false;
}

static unsigned long s32cc_clk_get_rate(unsigned long id)
{
	return 0;
}

static int set_module_rate(const struct s32cc_clk_obj *module,
			   unsigned long rate, unsigned long *orate,
			   unsigned int *depth);

static int set_osc_freq(const struct s32cc_clk_obj *module, unsigned long rate,
			unsigned long *orate, unsigned int *depth)
{
	struct s32cc_osc *osc = s32cc_obj2osc(module);
	int ret;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if ((osc->freq != 0UL) && (rate != osc->freq)) {
		ERROR("Already initialized oscillator. freq = %lu\n",
		      osc->freq);
		return -EINVAL;
	}

	osc->freq = rate;
	*orate = osc->freq;

	return 0;
}

static int set_clk_freq(const struct s32cc_clk_obj *module, unsigned long rate,
			unsigned long *orate, unsigned int *depth)
{
	const struct s32cc_clk *clk = s32cc_obj2clk(module);
	int ret;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if ((clk->min_freq != 0UL) && (clk->max_freq != 0UL) &&
	    ((rate < clk->min_freq) || (rate > clk->max_freq))) {
		ERROR("%lu frequency is out of the allowed range: [%lu:%lu]\n",
		      rate, clk->min_freq, clk->max_freq);
		return -EINVAL;
	}

	if (clk->module != NULL) {
		return set_module_rate(clk->module, rate, orate, depth);
	}

	if (clk->pclock != NULL) {
		return set_clk_freq(&clk->pclock->desc, rate, orate, depth);
	}

	return -EINVAL;
}

static int set_pll_freq(const struct s32cc_clk_obj *module, unsigned long rate,
			unsigned long *orate, unsigned int *depth)
{
	struct s32cc_pll *pll = s32cc_obj2pll(module);
	int ret;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if ((pll->vco_freq != 0UL) && (pll->vco_freq != rate)) {
		ERROR("PLL frequency was already set\n");
		return -EINVAL;
	}

	pll->vco_freq = rate;
	*orate = pll->vco_freq;

	return 0;
}

static int set_pll_div_freq(const struct s32cc_clk_obj *module, unsigned long rate,
			    unsigned long *orate, unsigned int *depth)
{
	struct s32cc_pll_out_div *pdiv = s32cc_obj2plldiv(module);
	const struct s32cc_pll *pll;
	unsigned long prate, dc;
	int ret;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if (pdiv->parent == NULL) {
		ERROR("Failed to identify PLL divider's parent\n");
		return -EINVAL;
	}

	pll = s32cc_obj2pll(pdiv->parent);
	if (pll == NULL) {
		ERROR("The parent of the PLL DIV is invalid\n");
		return -EINVAL;
	}

	prate = pll->vco_freq;

	/**
	 * The PLL is not initialized yet, so let's take a risk
	 * and accept the proposed rate.
	 */
	if (prate == 0UL) {
		pdiv->freq = rate;
		*orate = rate;
		return 0;
	}

	/* Decline in case the rate cannot fit PLL's requirements. */
	dc = prate / rate;
	if ((prate / dc) != rate) {
		return -EINVAL;
	}

	pdiv->freq = rate;
	*orate = pdiv->freq;

	return 0;
}

static int set_module_rate(const struct s32cc_clk_obj *module,
			   unsigned long rate, unsigned long *orate,
			   unsigned int *depth)
{
	int ret = 0;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	switch (module->type) {
	case s32cc_clk_t:
		ret = set_clk_freq(module, rate, orate, depth);
		break;
	case s32cc_osc_t:
		ret = set_osc_freq(module, rate, orate, depth);
		break;
	case s32cc_pll_t:
		ret = set_pll_freq(module, rate, orate, depth);
		break;
	case s32cc_pll_out_div_t:
		ret = set_pll_div_freq(module, rate, orate, depth);
		break;
	case s32cc_clkmux_t:
	case s32cc_shared_clkmux_t:
	case s32cc_fixed_div_t:
		ret = -ENOTSUP;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int s32cc_clk_set_rate(unsigned long id, unsigned long rate,
			      unsigned long *orate)
{
	unsigned int depth = MAX_STACK_DEPTH;
	const struct s32cc_clk *clk;
	int ret;

	clk = s32cc_get_arch_clk(id);
	if (clk == NULL) {
		return -EINVAL;
	}

	ret = set_module_rate(&clk->desc, rate, orate, &depth);
	if (ret != 0) {
		ERROR("Failed to set frequency (%lu MHz) for clock %lu\n",
		      rate, id);
	}

	return ret;
}

static int s32cc_clk_get_parent(unsigned long id)
{
	return -ENOTSUP;
}

static int s32cc_clk_set_parent(unsigned long id, unsigned long parent_id)
{
	const struct s32cc_clk *parent;
	const struct s32cc_clk *clk;
	bool valid_source = false;
	struct s32cc_clkmux *mux;
	uint8_t i;

	clk = s32cc_get_arch_clk(id);
	if (clk == NULL) {
		return -EINVAL;
	}

	parent = s32cc_get_arch_clk(parent_id);
	if (parent == NULL) {
		return -EINVAL;
	}

	if (!is_s32cc_clk_mux(clk)) {
		ERROR("Clock %lu is not a mux\n", id);
		return -EINVAL;
	}

	mux = s32cc_clk2mux(clk);
	if (mux == NULL) {
		ERROR("Failed to cast clock %lu to clock mux\n", id);
		return -EINVAL;
	}

	for (i = 0; i < mux->nclks; i++) {
		if (mux->clkids[i] == parent_id) {
			valid_source = true;
			break;
		}
	}

	if (!valid_source) {
		ERROR("Clock %lu is not a valid clock for mux %lu\n",
		      parent_id, id);
		return -EINVAL;
	}

	mux->source_id = parent_id;

	return 0;
}

void s32cc_clk_register_drv(void)
{
	static const struct clk_ops s32cc_clk_ops = {
		.enable		= s32cc_clk_enable,
		.disable	= s32cc_clk_disable,
		.is_enabled	= s32cc_clk_is_enabled,
		.get_rate	= s32cc_clk_get_rate,
		.set_rate	= s32cc_clk_set_rate,
		.get_parent	= s32cc_clk_get_parent,
		.set_parent	= s32cc_clk_set_parent,
	};

	clk_register(&s32cc_clk_ops);
}

