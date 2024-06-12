/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2024 NXP
 */
#ifndef S32CC_CLK_MODULES_H
#define S32CC_CLK_MODULES_H

#include <inttypes.h>
#include <stddef.h>

#define MHZ	UL(1000000)
#define GHZ	(UL(1000) * MHZ)

enum s32cc_clkm_type {
	s32cc_osc_t,
	s32cc_clk_t,
};

enum s32cc_clk_source {
	S32CC_FIRC,
	S32CC_FXOSC,
	S32CC_SIRC,
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

#endif /* S32CC_CLK_MODULES_H */
