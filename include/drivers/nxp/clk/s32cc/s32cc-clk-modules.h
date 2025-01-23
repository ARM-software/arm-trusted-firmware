/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2025 NXP
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
	s32cc_dfs_t,
	s32cc_dfs_div_t,
	s32cc_cgm_div_t,
	s32cc_clkmux_t,
	s32cc_shared_clkmux_t,
	s32cc_fixed_div_t,
	s32cc_part_t,
	s32cc_part_block_t,
	s32cc_part_block_link_t,
};

enum s32cc_clk_source {
	S32CC_FIRC,
	S32CC_FXOSC,
	S32CC_SIRC,
	S32CC_ARM_PLL,
	S32CC_ARM_DFS,
	S32CC_PERIPH_PLL,
	S32CC_CGM0,
	S32CC_CGM1,
	S32CC_DDR_PLL,
	S32CC_PERIPH_DFS,
	S32CC_CGM5,
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

#define S32CC_OSC_INIT_FREQ(SOURCE, FREQ) \
{                                         \
	.desc = {                         \
		.type = s32cc_osc_t,      \
	},                                \
	.source = (SOURCE),               \
	.freq = (FREQ),                   \
}

#define S32CC_OSC_INIT(SOURCE) \
	S32CC_OSC_INIT_FREQ(SOURCE, 0)

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

struct s32cc_dfs {
	struct s32cc_clk_obj desc;
	struct s32cc_clk_obj *parent;
	enum s32cc_clk_source instance;
	uintptr_t base;
};

#define S32CC_DFS_INIT(PARENT, INSTANCE) \
{                                        \
	.desc = {                        \
		.type = s32cc_dfs_t,     \
	},                               \
	.parent = &(PARENT).desc,        \
	.instance = (INSTANCE),          \
}

struct s32cc_dfs_div {
	struct s32cc_clk_obj desc;
	struct s32cc_clk_obj *parent;
	uint32_t index;
	unsigned long freq;
};

#define S32CC_DFS_DIV_INIT(PARENT, INDEX) \
{                                         \
	.desc = {                         \
		.type = s32cc_dfs_div_t,  \
	},                                \
	.parent = &(PARENT).desc,         \
	.index = (INDEX),                 \
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

#define S32CC_FREQ_CLK(PARENT_MODULE, PARENT, MIN_F, MAX_F) \
{                                                           \
	.desc = {                                           \
		.type = s32cc_clk_t,                        \
	},                                                  \
	.pclock = (PARENT),                                 \
	.module = (PARENT_MODULE),                          \
	.min_freq = (MIN_F),                                \
	.max_freq = (MAX_F),                                \
}

#define S32CC_FREQ_MODULE_CLK(PARENT_MODULE, MIN_F, MAX_F) \
	S32CC_FREQ_CLK(&(PARENT_MODULE).desc, NULL, MIN_F, MAX_F)

#define S32CC_MODULE_CLK(PARENT_MODULE) \
	S32CC_FREQ_MODULE_CLK(PARENT_MODULE, 0, 0)

#define S32CC_CHILD_CLK(PARENT, MIN_F, MAX_F) \
	S32CC_FREQ_CLK(NULL, &(PARENT), MIN_F, MAX_F)

struct s32cc_part {
	struct s32cc_clk_obj desc;
	uint32_t partition_id;
};

#define S32CC_PART(PART_NUM)          \
{                                     \
	.desc = {                     \
		.type = s32cc_part_t, \
	},                            \
	.partition_id = (PART_NUM),   \
}

enum s32cc_part_block_type {
	s32cc_part_block0,
	s32cc_part_block1,
	s32cc_part_block2,
	s32cc_part_block3,
	s32cc_part_block4,
	s32cc_part_block5,
	s32cc_part_block6,
	s32cc_part_block7,
	s32cc_part_block8,
	s32cc_part_block9,
	s32cc_part_block10,
	s32cc_part_block11,
	s32cc_part_block12,
	s32cc_part_block13,
	s32cc_part_block14,
	s32cc_part_block15,
};

struct s32cc_part_block {
	struct s32cc_clk_obj desc;
	struct s32cc_part *part;
	enum s32cc_part_block_type block;
	bool status;
};

#define S32CC_PART_BLOCK_STATUS(PART_META, BLOCK_TYPE, STATUS) \
{                                                              \
	.desc = {                                              \
		.type = s32cc_part_block_t,                    \
	},                                                     \
	.part = (PART_META),                                   \
	.block = (BLOCK_TYPE),                                 \
	.status = (STATUS),                                    \
}

#define S32CC_PART_BLOCK(PARENT, BLOCK_TYPE) \
	S32CC_PART_BLOCK_STATUS(PARENT, BLOCK_TYPE, true)

#define S32CC_PART_BLOCK_NO_STATUS(PARENT, BLOCK_TYPE) \
	S32CC_PART_BLOCK_STATUS(PARENT, BLOCK_TYPE, false)

struct s32cc_part_block_link {
	struct s32cc_clk_obj desc;
	struct s32cc_clk_obj *parent;
	struct s32cc_part_block *block;
};

#define S32CC_PART_BLOCK_LINK(PARENT, BLOCK)     \
{                                                \
	.desc = {                                \
		.type = s32cc_part_block_link_t, \
	},                                       \
	.parent = &(PARENT).desc,                \
	.block = (BLOCK),                        \
}

struct s32cc_cgm_div {
	struct s32cc_clk_obj desc;
	struct s32cc_clk_obj *parent;
	unsigned long freq;
	uint32_t index;
};

#define S32CC_CGM_DIV_INIT(PARENT, INDEX) \
{                                         \
	.desc = {                         \
		.type = s32cc_cgm_div_t,  \
	},                                \
	.parent = &(PARENT).desc,         \
	.index = (INDEX),                 \
}

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

static inline struct s32cc_dfs *s32cc_obj2dfs(const struct s32cc_clk_obj *mod)
{
	uintptr_t dfs_addr;

	dfs_addr = ((uintptr_t)mod) - offsetof(struct s32cc_dfs, desc);
	return (struct s32cc_dfs *)dfs_addr;
}

static inline struct s32cc_dfs_div *s32cc_obj2dfsdiv(const struct s32cc_clk_obj *mod)
{
	uintptr_t dfs_div_addr;

	dfs_div_addr = ((uintptr_t)mod) - offsetof(struct s32cc_dfs_div, desc);
	return (struct s32cc_dfs_div *)dfs_div_addr;
}

static inline struct s32cc_part *s32cc_obj2part(const struct s32cc_clk_obj *mod)
{
	uintptr_t part_addr;

	part_addr = ((uintptr_t)mod) - offsetof(struct s32cc_part, desc);
	return (struct s32cc_part *)part_addr;
}

static inline struct s32cc_part_block *
s32cc_obj2partblock(const struct s32cc_clk_obj *mod)
{
	uintptr_t part_blk_addr;

	part_blk_addr = ((uintptr_t)mod) - offsetof(struct s32cc_part_block, desc);
	return (struct s32cc_part_block *)part_blk_addr;
}

static inline struct s32cc_part_block_link *
s32cc_obj2partblocklink(const struct s32cc_clk_obj *mod)
{
	uintptr_t blk_link;

	blk_link = ((uintptr_t)mod) - offsetof(struct s32cc_part_block_link, desc);
	return (struct s32cc_part_block_link *)blk_link;
}

static inline struct s32cc_cgm_div *s32cc_obj2cgmdiv(const struct s32cc_clk_obj *mod)
{
	uintptr_t cgm_div_addr;

	cgm_div_addr = ((uintptr_t)mod) - offsetof(struct s32cc_cgm_div, desc);
	return (struct s32cc_cgm_div *)cgm_div_addr;
}

#endif /* S32CC_CLK_MODULES_H */
