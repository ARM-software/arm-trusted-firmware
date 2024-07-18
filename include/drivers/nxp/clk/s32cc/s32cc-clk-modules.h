/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2024 NXP
 */
#ifndef S32CC_CLK_MODULES_H
#define S32CC_CLK_MODULES_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#define MHZ	UL(1000000)
#define GHZ	(UL(1000) * MHZ)

enum s32cc_clkm_type {
	s32cc_osc_t,
	s32cc_clk_t,
	s32cc_pll_t,
	s32cc_pll_out_div_t,
	s32cc_clkmux_t,
	s32cc_shared_clkmux_t,
	s32cc_fixed_div_t,
};

enum s32cc_clk_source {
	S32CC_FIRC,
	S32CC_FXOSC,
	S32CC_SIRC,
	S32CC_ARM_PLL,
	S32CC_CGM1,
};

struct s32cc_clk_obj {
	enum s32cc_clkm_type type;
	uint32_t refcount;
};

struct s32cc_osc {
	struct s32cc_clk_obj desc;
	enum s32cc_clk_source source;
	unsigned long freq;
	void *base;
};

#define S32CC_OSC_INIT(SOURCE)       \
{                                    \
	.desc = {                    \
		.type = s32cc_osc_t, \
	},                           \
	.source = (SOURCE),          \
}

struct s32cc_clkmux {
	struct s32cc_clk_obj desc;
	enum s32cc_clk_source module;
	uint8_t index; /* Mux index in parent module */
	unsigned long source_id; /* Selected source */
	uint8_t nclks; /* Number of input clocks */
	unsigned long clkids[5]; /* IDs of the input clocks */
};

#define S32CC_CLKMUX_TYPE_INIT(TYPE, MODULE, INDEX, NCLKS, ...) \
{                                                               \
	.desc = {                                               \
		.type = (TYPE),                                 \
	},                                                      \
	.module = (MODULE),                                     \
	.index = (INDEX),                                       \
	.nclks = (NCLKS),                                       \
	.clkids = {__VA_ARGS__},                                \
}

#define S32CC_CLKMUX_INIT(MODULE, INDEX, NCLKS, ...)     \
	S32CC_CLKMUX_TYPE_INIT(s32cc_clkmux_t, MODULE,   \
			       INDEX, NCLKS, __VA_ARGS__)

#define S32CC_SHARED_CLKMUX_INIT(MODULE, INDEX, NCLKS, ...)   \
	S32CC_CLKMUX_TYPE_INIT(s32cc_shared_clkmux_t, MODULE, \
			       INDEX, NCLKS, __VA_ARGS__)

struct s32cc_pll {
	struct s32cc_clk_obj desc;
	struct s32cc_clk_obj *source;
	enum s32cc_clk_source instance;
	unsigned long vco_freq;
	uint32_t ndividers;
	uintptr_t base;
};

#define S32CC_PLL_INIT(PLL_MUX_CLK, INSTANCE, NDIVIDERS) \
{                                                        \
	.desc = {                                        \
		.type = s32cc_pll_t,                     \
	},                                               \
	.source = &(PLL_MUX_CLK).desc,                   \
	.instance = (INSTANCE),                          \
	.ndividers = (NDIVIDERS),                        \
}

struct s32cc_pll_out_div {
	struct s32cc_clk_obj desc;
	struct s32cc_clk_obj *parent;
	uint32_t index;
	unsigned long freq;
};

#define S32CC_PLL_OUT_DIV_INIT(PARENT, INDEX)  \
{                                              \
	.desc = {                              \
		.type = s32cc_pll_out_div_t,   \
	},                                     \
	.parent = &(PARENT).desc,              \
	.index = (INDEX),                      \
}

#define S32CC_PLL_OUT_DIV_INIT(PARENT, INDEX)  \
{                                              \
	.desc = {                              \
		.type = s32cc_pll_out_div_t,   \
	},                                     \
	.parent = &(PARENT).desc,              \
	.index = (INDEX),                      \
}

struct s32cc_fixed_div {
	struct s32cc_clk_obj desc;
	struct s32cc_clk_obj *parent;
	uint32_t rate_div;
};

#define S32CC_FIXED_DIV_INIT(PARENT, RATE_DIV) \
{                                              \
	.desc = {                              \
		.type = s32cc_fixed_div_t,     \
	},                                     \
	.parent = &(PARENT).desc,              \
	.rate_div = (RATE_DIV),                \
}

struct s32cc_clk {
	struct s32cc_clk_obj desc;
	struct s32cc_clk_obj *module;
	struct s32cc_clk *pclock;
	unsigned long min_freq;
	unsigned long max_freq;
};

struct s32cc_clk_array {
	unsigned long type_mask;
	struct s32cc_clk **clks;
	size_t n_clks;
};

#define S32CC_FREQ_MODULE(PARENT_MODULE, MIN_F, MAX_F) \
{                                                      \
	.desc = {                                      \
		.type = s32cc_clk_t,                   \
	},                                             \
	.module = &(PARENT_MODULE).desc,               \
	.min_freq = (MIN_F),                           \
	.max_freq = (MAX_F),                           \
}

#define S32CC_FREQ_MODULE_CLK(PARENT_MODULE, MIN_F, MAX_F) \
	S32CC_FREQ_MODULE(PARENT_MODULE, MIN_F, MAX_F)

#define S32CC_MODULE_CLK(PARENT_MODULE) \
	S32CC_FREQ_MODULE_CLK(PARENT_MODULE, 0, 0)

static inline struct s32cc_osc *s32cc_obj2osc(const struct s32cc_clk_obj *mod)
{
	uintptr_t osc_addr;

	osc_addr = ((uintptr_t)mod) - offsetof(struct s32cc_osc, desc);
	return (struct s32cc_osc *)osc_addr;
}

static inline struct s32cc_clk *s32cc_obj2clk(const struct s32cc_clk_obj *mod)
{
	uintptr_t clk_addr;

	clk_addr = ((uintptr_t)mod) - offsetof(struct s32cc_clk, desc);
	return (struct s32cc_clk *)clk_addr;
}

static inline bool is_s32cc_clk_mux(const struct s32cc_clk *clk)
{
	const struct s32cc_clk_obj *module;

	module = clk->module;
	if (module == NULL) {
		return false;
	}

	return (module->type == s32cc_clkmux_t) ||
	    (module->type == s32cc_shared_clkmux_t);
}

static inline struct s32cc_clkmux *s32cc_obj2clkmux(const struct s32cc_clk_obj *mod)
{
	uintptr_t cmux_addr;

	cmux_addr = ((uintptr_t)mod) - offsetof(struct s32cc_clkmux, desc);
	return (struct s32cc_clkmux *)cmux_addr;
}

static inline struct s32cc_clkmux *s32cc_clk2mux(const struct s32cc_clk *clk)
{
	if (!is_s32cc_clk_mux(clk)) {
		return NULL;
	}

	return s32cc_obj2clkmux(clk->module);
}

static inline struct s32cc_pll *s32cc_obj2pll(const struct s32cc_clk_obj *mod)
{
	uintptr_t pll_addr;

	pll_addr = ((uintptr_t)mod) - offsetof(struct s32cc_pll, desc);
	return (struct s32cc_pll *)pll_addr;
}

static inline struct s32cc_pll_out_div *s32cc_obj2plldiv(const struct s32cc_clk_obj *mod)
{
	uintptr_t plldiv_addr;

	plldiv_addr = ((uintptr_t)mod) - offsetof(struct s32cc_pll_out_div, desc);
	return (struct s32cc_pll_out_div *)plldiv_addr;
}

static inline struct s32cc_fixed_div *s32cc_obj2fixeddiv(const struct s32cc_clk_obj *mod)
{
	uintptr_t fdiv_addr;

	fdiv_addr = ((uintptr_t)mod) - offsetof(struct s32cc_fixed_div, desc);
	return (struct s32cc_fixed_div *)fdiv_addr;
}

#endif /* S32CC_CLK_MODULES_H */
