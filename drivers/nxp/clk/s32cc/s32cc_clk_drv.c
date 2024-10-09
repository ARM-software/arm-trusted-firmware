/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <common/debug.h>
#include <drivers/clk.h>
#include <lib/mmio.h>
#include <s32cc-clk-ids.h>
#include <s32cc-clk-modules.h>
#include <s32cc-clk-regs.h>
#include <s32cc-clk-utils.h>
#include <s32cc-mc-me.h>

#define MAX_STACK_DEPTH		(40U)

/* This is used for floating-point precision calculations. */
#define FP_PRECISION		(100000000UL)

struct s32cc_clk_drv {
	uintptr_t fxosc_base;
	uintptr_t armpll_base;
	uintptr_t periphpll_base;
	uintptr_t armdfs_base;
	uintptr_t cgm0_base;
	uintptr_t cgm1_base;
	uintptr_t cgm5_base;
	uintptr_t ddrpll_base;
	uintptr_t mc_me;
	uintptr_t mc_rgm;
	uintptr_t rdc;
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
		.armpll_base = ARMPLL_BASE_ADDR,
		.periphpll_base = PERIPHPLL_BASE_ADDR,
		.armdfs_base = ARM_DFS_BASE_ADDR,
		.cgm0_base = CGM0_BASE_ADDR,
		.cgm1_base = CGM1_BASE_ADDR,
		.cgm5_base = MC_CGM5_BASE_ADDR,
		.ddrpll_base = DDRPLL_BASE_ADDR,
		.mc_me = MC_ME_BASE_ADDR,
		.mc_rgm = MC_RGM_BASE_ADDR,
		.rdc = RDC_BASE_ADDR,
	};

	return &driver;
}

static int enable_module(struct s32cc_clk_obj *module,
			 const struct s32cc_clk_drv *drv,
			 unsigned int depth);

static struct s32cc_clk_obj *get_clk_parent(const struct s32cc_clk_obj *module)
{
	const struct s32cc_clk *clk = s32cc_obj2clk(module);

	if (clk->module != NULL) {
		return clk->module;
	}

	if (clk->pclock != NULL) {
		return &clk->pclock->desc;
	}

	return NULL;
}

static int get_base_addr(enum s32cc_clk_source id, const struct s32cc_clk_drv *drv,
			 uintptr_t *base)
{
	int ret = 0;

	switch (id) {
	case S32CC_FXOSC:
		*base = drv->fxosc_base;
		break;
	case S32CC_ARM_PLL:
		*base = drv->armpll_base;
		break;
	case S32CC_PERIPH_PLL:
		*base = drv->periphpll_base;
		break;
	case S32CC_DDR_PLL:
		*base = drv->ddrpll_base;
		break;
	case S32CC_ARM_DFS:
		*base = drv->armdfs_base;
		break;
	case S32CC_CGM0:
		*base = drv->cgm0_base;
		break;
	case S32CC_CGM1:
		*base = drv->cgm1_base;
		break;
	case S32CC_CGM5:
		*base = drv->cgm5_base;
		break;
	case S32CC_FIRC:
		break;
	case S32CC_SIRC:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret != 0) {
		ERROR("Unknown clock source id: %u\n", id);
	}

	return ret;
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

static int enable_osc(struct s32cc_clk_obj *module,
		      const struct s32cc_clk_drv *drv,
		      unsigned int depth)
{
	const struct s32cc_osc *osc = s32cc_obj2osc(module);
	unsigned int ldepth = depth;
	int ret = 0;

	ret = update_stack_depth(&ldepth);
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

static struct s32cc_clk_obj *get_pll_parent(const struct s32cc_clk_obj *module)
{
	const struct s32cc_pll *pll = s32cc_obj2pll(module);

	if (pll->source == NULL) {
		ERROR("Failed to identify PLL's parent\n");
	}

	return pll->source;
}

static int get_pll_mfi_mfn(unsigned long pll_vco, unsigned long ref_freq,
			   uint32_t *mfi, uint32_t *mfn)

{
	unsigned long vco;
	unsigned long mfn64;

	/* FRAC-N mode */
	*mfi = (uint32_t)(pll_vco / ref_freq);

	/* MFN formula : (double)(pll_vco % ref_freq) / ref_freq * 18432.0 */
	mfn64 = pll_vco % ref_freq;
	mfn64 *= FP_PRECISION;
	mfn64 /= ref_freq;
	mfn64 *= 18432UL;
	mfn64 /= FP_PRECISION;

	if (mfn64 > UINT32_MAX) {
		return -EINVAL;
	}

	*mfn = (uint32_t)mfn64;

	vco = ((unsigned long)*mfn * FP_PRECISION) / 18432UL;
	vco += (unsigned long)*mfi * FP_PRECISION;
	vco *= ref_freq;
	vco /= FP_PRECISION;

	if (vco != pll_vco) {
		ERROR("Failed to find MFI and MFN settings for PLL freq %lu. Nearest freq = %lu\n",
		      pll_vco, vco);
		return -EINVAL;
	}

	return 0;
}

static struct s32cc_clkmux *get_pll_mux(const struct s32cc_pll *pll)
{
	const struct s32cc_clk_obj *source = pll->source;
	const struct s32cc_clk *clk;

	if (source == NULL) {
		ERROR("Failed to identify PLL's parent\n");
		return NULL;
	}

	if (source->type != s32cc_clk_t) {
		ERROR("The parent of the PLL isn't a clock\n");
		return NULL;
	}

	clk = s32cc_obj2clk(source);

	if (clk->module == NULL) {
		ERROR("The clock isn't connected to a module\n");
		return NULL;
	}

	source = clk->module;

	if ((source->type != s32cc_clkmux_t) &&
	    (source->type != s32cc_shared_clkmux_t)) {
		ERROR("The parent of the PLL isn't a MUX\n");
		return NULL;
	}

	return s32cc_obj2clkmux(source);
}

static void disable_odiv(uintptr_t pll_addr, uint32_t div_index)
{
	mmio_clrbits_32(PLLDIG_PLLODIV(pll_addr, div_index), PLLDIG_PLLODIV_DE);
}

static void enable_odiv(uintptr_t pll_addr, uint32_t div_index)
{
	mmio_setbits_32(PLLDIG_PLLODIV(pll_addr, div_index), PLLDIG_PLLODIV_DE);
}

static void disable_odivs(uintptr_t pll_addr, uint32_t ndivs)
{
	uint32_t i;

	for (i = 0; i < ndivs; i++) {
		disable_odiv(pll_addr, i);
	}
}

static void enable_pll_hw(uintptr_t pll_addr)
{
	/* Enable the PLL. */
	mmio_write_32(PLLDIG_PLLCR(pll_addr), 0x0);

	/* Poll until PLL acquires lock. */
	while ((mmio_read_32(PLLDIG_PLLSR(pll_addr)) & PLLDIG_PLLSR_LOCK) == 0U) {
	}
}

static void disable_pll_hw(uintptr_t pll_addr)
{
	mmio_write_32(PLLDIG_PLLCR(pll_addr), PLLDIG_PLLCR_PLLPD);
}

static int program_pll(const struct s32cc_pll *pll, uintptr_t pll_addr,
		       const struct s32cc_clk_drv *drv, uint32_t sclk_id,
		       unsigned long sclk_freq)
{
	uint32_t rdiv = 1, mfi, mfn;
	int ret;

	ret = get_pll_mfi_mfn(pll->vco_freq, sclk_freq, &mfi, &mfn);
	if (ret != 0) {
		return -EINVAL;
	}

	/* Disable ODIVs*/
	disable_odivs(pll_addr, pll->ndividers);

	/* Disable PLL */
	disable_pll_hw(pll_addr);

	/* Program PLLCLKMUX */
	mmio_write_32(PLLDIG_PLLCLKMUX(pll_addr), sclk_id);

	/* Program VCO */
	mmio_clrsetbits_32(PLLDIG_PLLDV(pll_addr),
			   PLLDIG_PLLDV_RDIV_MASK | PLLDIG_PLLDV_MFI_MASK,
			   PLLDIG_PLLDV_RDIV_SET(rdiv) | PLLDIG_PLLDV_MFI(mfi));

	mmio_write_32(PLLDIG_PLLFD(pll_addr),
		      PLLDIG_PLLFD_MFN_SET(mfn) | PLLDIG_PLLFD_SMDEN);

	enable_pll_hw(pll_addr);

	return ret;
}

static int enable_pll(struct s32cc_clk_obj *module,
		      const struct s32cc_clk_drv *drv,
		      unsigned int depth)
{
	const struct s32cc_pll *pll = s32cc_obj2pll(module);
	const struct s32cc_clkmux *mux;
	uintptr_t pll_addr = UL(0x0);
	unsigned int ldepth = depth;
	unsigned long sclk_freq;
	uint32_t sclk_id;
	int ret;

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	mux = get_pll_mux(pll);
	if (mux == NULL) {
		return -EINVAL;
	}

	if (pll->instance != mux->module) {
		ERROR("MUX type is not in sync with PLL ID\n");
		return -EINVAL;
	}

	ret = get_base_addr(pll->instance, drv, &pll_addr);
	if (ret != 0) {
		ERROR("Failed to detect PLL instance\n");
		return ret;
	}

	switch (mux->source_id) {
	case S32CC_CLK_FIRC:
		sclk_freq = 48U * MHZ;
		sclk_id = 0;
		break;
	case S32CC_CLK_FXOSC:
		sclk_freq = 40U * MHZ;
		sclk_id = 1;
		break;
	default:
		ERROR("Invalid source selection for PLL 0x%lx\n",
		      pll_addr);
		return -EINVAL;
	};

	return program_pll(pll, pll_addr, drv, sclk_id, sclk_freq);
}

static inline struct s32cc_pll *get_div_pll(const struct s32cc_pll_out_div *pdiv)
{
	const struct s32cc_clk_obj *parent;

	parent = pdiv->parent;
	if (parent == NULL) {
		ERROR("Failed to identify PLL divider's parent\n");
		return NULL;
	}

	if (parent->type != s32cc_pll_t) {
		ERROR("The parent of the divider is not a PLL instance\n");
		return NULL;
	}

	return s32cc_obj2pll(parent);
}

static void config_pll_out_div(uintptr_t pll_addr, uint32_t div_index, uint32_t dc)
{
	uint32_t pllodiv;
	uint32_t pdiv;

	pllodiv = mmio_read_32(PLLDIG_PLLODIV(pll_addr, div_index));
	pdiv = PLLDIG_PLLODIV_DIV(pllodiv);

	if (((pdiv + 1U) == dc) && ((pllodiv & PLLDIG_PLLODIV_DE) != 0U)) {
		return;
	}

	if ((pllodiv & PLLDIG_PLLODIV_DE) != 0U) {
		disable_odiv(pll_addr, div_index);
	}

	pllodiv = PLLDIG_PLLODIV_DIV_SET(dc - 1U);
	mmio_write_32(PLLDIG_PLLODIV(pll_addr, div_index), pllodiv);

	enable_odiv(pll_addr, div_index);
}

static struct s32cc_clk_obj *get_pll_div_parent(const struct s32cc_clk_obj *module)
{
	const struct s32cc_pll_out_div *pdiv = s32cc_obj2plldiv(module);

	if (pdiv->parent == NULL) {
		ERROR("Failed to identify PLL DIV's parent\n");
	}

	return pdiv->parent;
}

static int enable_pll_div(struct s32cc_clk_obj *module,
			  const struct s32cc_clk_drv *drv,
			  unsigned int depth)
{
	const struct s32cc_pll_out_div *pdiv = s32cc_obj2plldiv(module);
	uintptr_t pll_addr = 0x0ULL;
	unsigned int ldepth = depth;
	const struct s32cc_pll *pll;
	uint32_t dc;
	int ret;

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	pll = get_div_pll(pdiv);
	if (pll == NULL) {
		ERROR("The parent of the PLL DIV is invalid\n");
		return 0;
	}

	ret = get_base_addr(pll->instance, drv, &pll_addr);
	if (ret != 0) {
		ERROR("Failed to detect PLL instance\n");
		return -EINVAL;
	}

	dc = (uint32_t)(pll->vco_freq / pdiv->freq);

	config_pll_out_div(pll_addr, pdiv->index, dc);

	return 0;
}

static int cgm_mux_clk_config(uintptr_t cgm_addr, uint32_t mux, uint32_t source,
			      bool safe_clk)
{
	uint32_t css, csc;

	css = mmio_read_32(CGM_MUXn_CSS(cgm_addr, mux));

	/* Already configured */
	if ((MC_CGM_MUXn_CSS_SELSTAT(css) == source) &&
	    (MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SUCCESS) &&
	    ((css & MC_CGM_MUXn_CSS_SWIP) == 0U) && !safe_clk) {
		return 0;
	}

	/* Ongoing clock switch? */
	while ((mmio_read_32(CGM_MUXn_CSS(cgm_addr, mux)) &
		MC_CGM_MUXn_CSS_SWIP) != 0U) {
	}

	csc = mmio_read_32(CGM_MUXn_CSC(cgm_addr, mux));

	/* Clear previous source. */
	csc &= ~(MC_CGM_MUXn_CSC_SELCTL_MASK);

	if (!safe_clk) {
		/* Select the clock source and trigger the clock switch. */
		csc |= MC_CGM_MUXn_CSC_SELCTL(source) | MC_CGM_MUXn_CSC_CLK_SW;
	} else {
		/* Switch to safe clock */
		csc |= MC_CGM_MUXn_CSC_SAFE_SW;
	}

	mmio_write_32(CGM_MUXn_CSC(cgm_addr, mux), csc);

	/* Wait for configuration bit to auto-clear. */
	while ((mmio_read_32(CGM_MUXn_CSC(cgm_addr, mux)) &
		MC_CGM_MUXn_CSC_CLK_SW) != 0U) {
	}

	/* Is the clock switch completed? */
	while ((mmio_read_32(CGM_MUXn_CSS(cgm_addr, mux)) &
		MC_CGM_MUXn_CSS_SWIP) != 0U) {
	}

	/*
	 * Check if the switch succeeded.
	 * Check switch trigger cause and the source.
	 */
	css = mmio_read_32(CGM_MUXn_CSS(cgm_addr, mux));
	if (!safe_clk) {
		if ((MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SUCCESS) &&
		    (MC_CGM_MUXn_CSS_SELSTAT(css) == source)) {
			return 0;
		}

		ERROR("Failed to change the source of mux %" PRIu32 " to %" PRIu32 " (CGM=%lu)\n",
		      mux, source, cgm_addr);
	} else {
		if (((MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SAFE_CLK) ||
		     (MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SAFE_CLK_INACTIVE)) &&
		     ((MC_CGM_MUXn_CSS_SAFE_SW & css) != 0U)) {
			return 0;
		}

		ERROR("The switch of mux %" PRIu32 " (CGM=%lu) to safe clock failed\n",
		      mux, cgm_addr);
	}

	return -EINVAL;
}

static int enable_cgm_mux(const struct s32cc_clkmux *mux,
			  const struct s32cc_clk_drv *drv)
{
	uintptr_t cgm_addr = UL(0x0);
	uint32_t mux_hw_clk;
	int ret;

	ret = get_base_addr(mux->module, drv, &cgm_addr);
	if (ret != 0) {
		return ret;
	}

	mux_hw_clk = (uint32_t)S32CC_CLK_ID(mux->source_id);

	return cgm_mux_clk_config(cgm_addr, mux->index,
				  mux_hw_clk, false);
}

static struct s32cc_clk_obj *get_mux_parent(const struct s32cc_clk_obj *module)
{
	const struct s32cc_clkmux *mux = s32cc_obj2clkmux(module);
	struct s32cc_clk *clk;

	if (mux == NULL) {
		return NULL;
	}

	clk = s32cc_get_arch_clk(mux->source_id);
	if (clk == NULL) {
		ERROR("Invalid parent (%lu) for mux %" PRIu8 "\n",
		      mux->source_id, mux->index);
		return NULL;
	}

	return &clk->desc;
}

static int enable_mux(struct s32cc_clk_obj *module,
		      const struct s32cc_clk_drv *drv,
		      unsigned int depth)
{
	const struct s32cc_clkmux *mux = s32cc_obj2clkmux(module);
	unsigned int ldepth = depth;
	const struct s32cc_clk *clk;
	int ret = 0;

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	if (mux == NULL) {
		return -EINVAL;
	}

	clk = s32cc_get_arch_clk(mux->source_id);
	if (clk == NULL) {
		ERROR("Invalid parent (%lu) for mux %" PRIu8 "\n",
		      mux->source_id, mux->index);
		return -EINVAL;
	}

	switch (mux->module) {
	/* PLL mux will be enabled by PLL setup */
	case S32CC_ARM_PLL:
	case S32CC_PERIPH_PLL:
	case S32CC_DDR_PLL:
		break;
	case S32CC_CGM1:
		ret = enable_cgm_mux(mux, drv);
		break;
	case S32CC_CGM0:
		ret = enable_cgm_mux(mux, drv);
		break;
	case S32CC_CGM5:
		ret = enable_cgm_mux(mux, drv);
		break;
	default:
		ERROR("Unknown mux parent type: %d\n", mux->module);
		ret = -EINVAL;
		break;
	};

	return ret;
}

static struct s32cc_clk_obj *get_dfs_parent(const struct s32cc_clk_obj *module)
{
	const struct s32cc_dfs *dfs = s32cc_obj2dfs(module);

	if (dfs->parent == NULL) {
		ERROR("Failed to identify DFS's parent\n");
	}

	return dfs->parent;
}

static int enable_dfs(struct s32cc_clk_obj *module,
		      const struct s32cc_clk_drv *drv,
		      unsigned int depth)
{
	unsigned int ldepth = depth;
	int ret = 0;

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

static struct s32cc_dfs *get_div_dfs(const struct s32cc_dfs_div *dfs_div)
{
	const struct s32cc_clk_obj *parent = dfs_div->parent;

	if (parent->type != s32cc_dfs_t) {
		ERROR("DFS DIV doesn't have a DFS as parent\n");
		return NULL;
	}

	return s32cc_obj2dfs(parent);
}

static struct s32cc_pll *dfsdiv2pll(const struct s32cc_dfs_div *dfs_div)
{
	const struct s32cc_clk_obj *parent;
	const struct s32cc_dfs *dfs;

	dfs = get_div_dfs(dfs_div);
	if (dfs == NULL) {
		return NULL;
	}

	parent = dfs->parent;
	if (parent->type != s32cc_pll_t) {
		return NULL;
	}

	return s32cc_obj2pll(parent);
}

static int get_dfs_mfi_mfn(unsigned long dfs_freq, const struct s32cc_dfs_div *dfs_div,
			   uint32_t *mfi, uint32_t *mfn)
{
	uint64_t factor64, tmp64, ofreq;
	uint32_t factor32;

	unsigned long in = dfs_freq;
	unsigned long out = dfs_div->freq;

	/**
	 * factor = (IN / OUT) / 2
	 * MFI = integer(factor)
	 * MFN = (factor - MFI) * 36
	 */
	factor64 = ((((uint64_t)in) * FP_PRECISION) / ((uint64_t)out)) / 2ULL;
	tmp64 = factor64 / FP_PRECISION;
	if (tmp64 > UINT32_MAX) {
		return -EINVAL;
	}

	factor32 = (uint32_t)tmp64;
	*mfi = factor32;

	tmp64 = ((factor64 - ((uint64_t)*mfi * FP_PRECISION)) * 36UL) / FP_PRECISION;
	if (tmp64 > UINT32_MAX) {
		return -EINVAL;
	}

	*mfn = (uint32_t)tmp64;

	/* div_freq = in / (2 * (*mfi + *mfn / 36.0)) */
	factor64 = (((uint64_t)*mfn) * FP_PRECISION) / 36ULL;
	factor64 += ((uint64_t)*mfi) * FP_PRECISION;
	factor64 *= 2ULL;
	ofreq = (((uint64_t)in) * FP_PRECISION) / factor64;

	if (ofreq != dfs_div->freq) {
		ERROR("Failed to find MFI and MFN settings for DFS DIV freq %lu\n",
		      dfs_div->freq);
		ERROR("Nearest freq = %" PRIx64 "\n", ofreq);
		return -EINVAL;
	}

	return 0;
}

static int init_dfs_port(uintptr_t dfs_addr, uint32_t port,
			 uint32_t mfi, uint32_t mfn)
{
	uint32_t portsr, portolsr;
	uint32_t mask, old_mfi, old_mfn;
	uint32_t dvport;
	bool init_dfs;

	dvport = mmio_read_32(DFS_DVPORTn(dfs_addr, port));

	old_mfi = DFS_DVPORTn_MFI(dvport);
	old_mfn = DFS_DVPORTn_MFN(dvport);

	portsr = mmio_read_32(DFS_PORTSR(dfs_addr));
	portolsr = mmio_read_32(DFS_PORTOLSR(dfs_addr));

	/* Skip configuration if it's not needed */
	if (((portsr & BIT_32(port)) != 0U) &&
	    ((portolsr & BIT_32(port)) == 0U) &&
	    (mfi == old_mfi) && (mfn == old_mfn)) {
		return 0;
	}

	init_dfs = (portsr == 0U);

	if (init_dfs) {
		mask = DFS_PORTRESET_MASK;
	} else {
		mask = DFS_PORTRESET_SET(BIT_32(port));
	}

	mmio_write_32(DFS_PORTOLSR(dfs_addr), mask);
	mmio_write_32(DFS_PORTRESET(dfs_addr), mask);

	while ((mmio_read_32(DFS_PORTSR(dfs_addr)) & mask) != 0U) {
	}

	if (init_dfs) {
		mmio_write_32(DFS_CTL(dfs_addr), DFS_CTL_RESET);
	}

	mmio_write_32(DFS_DVPORTn(dfs_addr, port),
		      DFS_DVPORTn_MFI_SET(mfi) | DFS_DVPORTn_MFN_SET(mfn));

	if (init_dfs) {
		/* DFS clk enable programming */
		mmio_clrbits_32(DFS_CTL(dfs_addr), DFS_CTL_RESET);
	}

	mmio_clrbits_32(DFS_PORTRESET(dfs_addr), BIT_32(port));

	while ((mmio_read_32(DFS_PORTSR(dfs_addr)) & BIT_32(port)) != BIT_32(port)) {
	}

	portolsr = mmio_read_32(DFS_PORTOLSR(dfs_addr));
	if ((portolsr & DFS_PORTOLSR_LOL(port)) != 0U) {
		ERROR("Failed to lock DFS divider\n");
		return -EINVAL;
	}

	return 0;
}

static struct s32cc_clk_obj *
get_dfs_div_parent(const struct s32cc_clk_obj *module)
{
	const struct s32cc_dfs_div *dfs_div = s32cc_obj2dfsdiv(module);

	if (dfs_div->parent == NULL) {
		ERROR("Failed to identify DFS divider's parent\n");
	}

	return dfs_div->parent;
}

static int enable_dfs_div(struct s32cc_clk_obj *module,
			  const struct s32cc_clk_drv *drv,
			  unsigned int depth)
{
	const struct s32cc_dfs_div *dfs_div = s32cc_obj2dfsdiv(module);
	unsigned int ldepth = depth;
	const struct s32cc_pll *pll;
	const struct s32cc_dfs *dfs;
	uintptr_t dfs_addr = 0UL;
	uint32_t mfi, mfn;
	int ret = 0;

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	dfs = get_div_dfs(dfs_div);
	if (dfs == NULL) {
		return -EINVAL;
	}

	pll = dfsdiv2pll(dfs_div);
	if (pll == NULL) {
		ERROR("Failed to identify DFS divider's parent\n");
		return -EINVAL;
	}

	ret = get_base_addr(dfs->instance, drv, &dfs_addr);
	if ((ret != 0) || (dfs_addr == 0UL)) {
		return -EINVAL;
	}

	ret = get_dfs_mfi_mfn(pll->vco_freq, dfs_div, &mfi, &mfn);
	if (ret != 0) {
		return -EINVAL;
	}

	return init_dfs_port(dfs_addr, dfs_div->index, mfi, mfn);
}

typedef int (*enable_clk_t)(struct s32cc_clk_obj *module,
			    const struct s32cc_clk_drv *drv,
			    unsigned int depth);

static int enable_part(struct s32cc_clk_obj *module,
		       const struct s32cc_clk_drv *drv,
		       unsigned int depth)
{
	const struct s32cc_part *part = s32cc_obj2part(module);
	uint32_t part_no = part->partition_id;

	if ((drv->mc_me == 0UL) || (drv->mc_rgm == 0UL) || (drv->rdc == 0UL)) {
		return -EINVAL;
	}

	return mc_me_enable_partition(drv->mc_me, drv->mc_rgm, drv->rdc, part_no);
}

static int enable_part_block(struct s32cc_clk_obj *module,
			     const struct s32cc_clk_drv *drv,
			     unsigned int depth)
{
	const struct s32cc_part_block *block = s32cc_obj2partblock(module);
	const struct s32cc_part *part = block->part;
	uint32_t part_no = part->partition_id;
	unsigned int ldepth = depth;
	uint32_t cofb;
	int ret;

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	if ((block->block >= s32cc_part_block0) &&
	    (block->block <= s32cc_part_block15)) {
		cofb = (uint32_t)block->block - (uint32_t)s32cc_part_block0;
		mc_me_enable_part_cofb(drv->mc_me, part_no, cofb, block->status);
	} else {
		ERROR("Unknown partition block type: %d\n", block->block);
		return -EINVAL;
	}

	return 0;
}

static struct s32cc_clk_obj *
get_part_block_parent(const struct s32cc_clk_obj *module)
{
	const struct s32cc_part_block *block = s32cc_obj2partblock(module);

	return &block->part->desc;
}

static int enable_module_with_refcount(struct s32cc_clk_obj *module,
				       const struct s32cc_clk_drv *drv,
				       unsigned int depth);

static int enable_part_block_link(struct s32cc_clk_obj *module,
				  const struct s32cc_clk_drv *drv,
				  unsigned int depth)
{
	const struct s32cc_part_block_link *link = s32cc_obj2partblocklink(module);
	struct s32cc_part_block *block = link->block;
	unsigned int ldepth = depth;
	int ret;

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	/* Move the enablement algorithm to partition tree */
	return enable_module_with_refcount(&block->desc, drv, ldepth);
}

static struct s32cc_clk_obj *
get_part_block_link_parent(const struct s32cc_clk_obj *module)
{
	const struct s32cc_part_block_link *link = s32cc_obj2partblocklink(module);

	return link->parent;
}

static int no_enable(struct s32cc_clk_obj *module,
		     const struct s32cc_clk_drv *drv,
		     unsigned int depth)
{
	return 0;
}

static int exec_cb_with_refcount(enable_clk_t en_cb, struct s32cc_clk_obj *mod,
				 const struct s32cc_clk_drv *drv, bool leaf_node,
				 unsigned int depth)
{
	unsigned int ldepth = depth;
	int ret = 0;

	if (mod == NULL) {
		return 0;
	}

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	/* Refcount will be updated as part of the recursivity */
	if (leaf_node) {
		return en_cb(mod, drv, ldepth);
	}

	if (mod->refcount == 0U) {
		ret = en_cb(mod, drv, ldepth);
	}

	if (ret == 0) {
		mod->refcount++;
	}

	return ret;
}

static struct s32cc_clk_obj *get_module_parent(const struct s32cc_clk_obj *module);

static int enable_module(struct s32cc_clk_obj *module,
			 const struct s32cc_clk_drv *drv,
			 unsigned int depth)
{
	struct s32cc_clk_obj *parent = get_module_parent(module);
	static const enable_clk_t enable_clbs[12] = {
		[s32cc_clk_t] = no_enable,
		[s32cc_osc_t] = enable_osc,
		[s32cc_pll_t] = enable_pll,
		[s32cc_pll_out_div_t] = enable_pll_div,
		[s32cc_clkmux_t] = enable_mux,
		[s32cc_shared_clkmux_t] = enable_mux,
		[s32cc_dfs_t] = enable_dfs,
		[s32cc_dfs_div_t] = enable_dfs_div,
		[s32cc_part_t] = enable_part,
		[s32cc_part_block_t] = enable_part_block,
		[s32cc_part_block_link_t] = enable_part_block_link,
	};
	unsigned int ldepth = depth;
	uint32_t index;
	int ret = 0;

	ret = update_stack_depth(&ldepth);
	if (ret != 0) {
		return ret;
	}

	if (drv == NULL) {
		return -EINVAL;
	}

	index = (uint32_t)module->type;

	if (index >= ARRAY_SIZE(enable_clbs)) {
		ERROR("Undefined module type: %d\n", module->type);
		return -EINVAL;
	}

	if (enable_clbs[index] == NULL) {
		ERROR("Undefined callback for the clock type: %d\n",
		      module->type);
		return -EINVAL;
	}

	parent = get_module_parent(module);

	ret = exec_cb_with_refcount(enable_module, parent, drv,
				    false, ldepth);
	if (ret != 0) {
		return ret;
	}

	ret = exec_cb_with_refcount(enable_clbs[index], module, drv,
				    true, ldepth);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int enable_module_with_refcount(struct s32cc_clk_obj *module,
				       const struct s32cc_clk_drv *drv,
				       unsigned int depth)
{
	return exec_cb_with_refcount(enable_module, module, drv, false, depth);
}

static int s32cc_clk_enable(unsigned long id)
{
	const struct s32cc_clk_drv *drv = get_drv();
	unsigned int depth = MAX_STACK_DEPTH;
	struct s32cc_clk *clk;

	clk = s32cc_get_arch_clk(id);
	if (clk == NULL) {
		return -EINVAL;
	}

	return enable_module_with_refcount(&clk->desc, drv, depth);
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

static int set_fixed_div_freq(const struct s32cc_clk_obj *module, unsigned long rate,
			      unsigned long *orate, unsigned int *depth)
{
	const struct s32cc_fixed_div *fdiv = s32cc_obj2fixeddiv(module);
	int ret;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if (fdiv->parent == NULL) {
		ERROR("The divider doesn't have a valid parent\b");
		return -EINVAL;
	}

	ret = set_module_rate(fdiv->parent, rate * fdiv->rate_div, orate, depth);

	/* Update the output rate based on the parent's rate */
	*orate /= fdiv->rate_div;

	return ret;
}

static int set_mux_freq(const struct s32cc_clk_obj *module, unsigned long rate,
			unsigned long *orate, unsigned int *depth)
{
	const struct s32cc_clkmux *mux = s32cc_obj2clkmux(module);
	const struct s32cc_clk *clk = s32cc_get_arch_clk(mux->source_id);
	int ret;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if (clk == NULL) {
		ERROR("Mux (id:%" PRIu8 ") without a valid source (%lu)\n",
		      mux->index, mux->source_id);
		return -EINVAL;
	}

	return set_module_rate(&clk->desc, rate, orate, depth);
}

static int set_dfs_div_freq(const struct s32cc_clk_obj *module, unsigned long rate,
			    unsigned long *orate, unsigned int *depth)
{
	struct s32cc_dfs_div *dfs_div = s32cc_obj2dfsdiv(module);
	const struct s32cc_dfs *dfs;
	int ret;

	ret = update_stack_depth(depth);
	if (ret != 0) {
		return ret;
	}

	if (dfs_div->parent == NULL) {
		ERROR("Failed to identify DFS divider's parent\n");
		return -EINVAL;
	}

	/* Sanity check */
	dfs = s32cc_obj2dfs(dfs_div->parent);
	if (dfs->parent == NULL) {
		ERROR("Failed to identify DFS's parent\n");
		return -EINVAL;
	}

	if ((dfs_div->freq != 0U) && (dfs_div->freq != rate)) {
		ERROR("DFS DIV frequency was already set to %lu\n",
		      dfs_div->freq);
		return -EINVAL;
	}

	dfs_div->freq = rate;
	*orate = rate;

	return ret;
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

	ret = -EINVAL;

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
	case s32cc_fixed_div_t:
		ret = set_fixed_div_freq(module, rate, orate, depth);
		break;
	case s32cc_clkmux_t:
		ret = set_mux_freq(module, rate, orate, depth);
		break;
	case s32cc_shared_clkmux_t:
		ret = set_mux_freq(module, rate, orate, depth);
		break;
	case s32cc_dfs_t:
		ERROR("Setting the frequency of a DFS is not allowed!");
		break;
	case s32cc_dfs_div_t:
		ret = set_dfs_div_freq(module, rate, orate, depth);
		break;
	default:
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

static struct s32cc_clk_obj *get_no_parent(const struct s32cc_clk_obj *module)
{
	return NULL;
}

typedef struct s32cc_clk_obj *(*get_parent_clb_t)(const struct s32cc_clk_obj *clk_obj);

static struct s32cc_clk_obj *get_module_parent(const struct s32cc_clk_obj *module)
{
	static const get_parent_clb_t parents_clbs[12] = {
		[s32cc_clk_t] = get_clk_parent,
		[s32cc_osc_t] = get_no_parent,
		[s32cc_pll_t] = get_pll_parent,
		[s32cc_pll_out_div_t] = get_pll_div_parent,
		[s32cc_clkmux_t] = get_mux_parent,
		[s32cc_shared_clkmux_t] = get_mux_parent,
		[s32cc_dfs_t] = get_dfs_parent,
		[s32cc_dfs_div_t] = get_dfs_div_parent,
		[s32cc_part_t] = get_no_parent,
		[s32cc_part_block_t] = get_part_block_parent,
		[s32cc_part_block_link_t] = get_part_block_link_parent,
	};
	uint32_t index;

	if (module == NULL) {
		return NULL;
	}

	index = (uint32_t)module->type;

	if (index >= ARRAY_SIZE(parents_clbs)) {
		ERROR("Undefined module type: %d\n", module->type);
		return NULL;
	}

	if (parents_clbs[index] == NULL) {
		ERROR("Undefined parent getter for type: %d\n", module->type);
		return NULL;
	}

	return parents_clbs[index](module);
}

static int s32cc_clk_get_parent(unsigned long id)
{
	struct s32cc_clk *parent_clk;
	const struct s32cc_clk_obj *parent;
	const struct s32cc_clk *clk;
	unsigned long parent_id;
	int ret;

	clk = s32cc_get_arch_clk(id);
	if (clk == NULL) {
		return -EINVAL;
	}

	parent = get_module_parent(clk->module);
	if (parent == NULL) {
		return -EINVAL;
	}

	parent_clk = s32cc_obj2clk(parent);
	if (parent_clk == NULL) {
		return -EINVAL;
	}

	ret = s32cc_get_clk_id(parent_clk, &parent_id);
	if (ret != 0) {
		return ret;
	}

	if (parent_id > (unsigned long)INT_MAX) {
		return -E2BIG;
	}

	return (int)parent_id;
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

