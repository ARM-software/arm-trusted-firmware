/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for clock control.
 */

#include <arch_helpers.h>
#include <mmio.h>
#include <platform.h>
#include <stdbool.h>
#include <string.h>
#include "pm_api_clock.h"
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_common.h"
#include "pm_ipi.h"

#define CLK_NODE_MAX			U(6)

#define CLK_PARENTS_ID_LEN		U(16)
#define CLK_TOPOLOGY_NODE_OFFSET	U(16)
#define CLK_TOPOLOGY_PAYLOAD_LEN	U(12)
#define CLK_PARENTS_PAYLOAD_LEN		U(12)
#define CLK_TYPE_SHIFT			U(2)
#define CLK_CLKFLAGS_SHIFT		U(8)
#define CLK_TYPEFLAGS_SHIFT		U(24)

#define CLK_EXTERNAL_PARENT	(PARENT_CLK_EXTERNAL << CLK_PARENTS_ID_LEN)

#define NA_MULT					U(0)
#define NA_DIV					U(0)
#define NA_SHIFT				U(0)
#define NA_WIDTH				U(0)
#define NA_CLK_FLAGS				U(0)
#define NA_TYPE_FLAGS				U(0)

/* PLL nodes related definitions */
#define PLL_PRESRC_MUX_SHIFT			U(20)
#define PLL_PRESRC_MUX_WIDTH			U(3)
#define PLL_POSTSRC_MUX_SHIFT			U(24)
#define PLL_POSTSRC_MUX_WIDTH			U(3)
#define PLL_DIV2_MUX_SHIFT			U(16)
#define PLL_DIV2_MUX_WIDTH			U(1)
#define PLL_BYPASS_MUX_SHIFT			U(3)
#define PLL_BYPASS_MUX_WIDTH			U(1)

/* Peripheral nodes related definitions */
/* Peripheral Clocks */
#define PERIPH_MUX_SHIFT			U(0)
#define PERIPH_MUX_WIDTH			U(3)
#define PERIPH_DIV1_SHIFT			U(8)
#define PERIPH_DIV1_WIDTH			U(6)
#define PERIPH_DIV2_SHIFT			U(16)
#define PERIPH_DIV2_WIDTH			U(6)
#define PERIPH_GATE_SHIFT			U(24)
#define PERIPH_GATE_WIDTH			U(1)

#define USB_GATE_SHIFT				U(25)

/* External clock related definitions */

#define EXT_CLK_MIO_DATA(mio)				\
	[EXT_CLK_INDEX(EXT_CLK_MIO##mio)] = {		\
		.name = "mio_clk_"#mio,			\
	}

#define EXT_CLK_INDEX(n)	(n - CLK_MAX_OUTPUT_CLK)

/* Clock control related definitions */
#define BIT_MASK(x, y) (((1U << (y)) - 1) << (x))

#define ISPLL(id)	(id == CLK_APLL_INT ||	\
			 id == CLK_DPLL_INT ||  \
			 id == CLK_VPLL_INT ||  \
			 id == CLK_IOPLL_INT || \
			 id == CLK_RPLL_INT)


#define PLLCTRL_BP_MASK				BIT(3)
#define PLLCTRL_RESET_MASK			U(1)
#define PLL_FRAC_OFFSET				U(8)
#define PLL_FRAC_MODE				U(1)
#define PLL_INT_MODE				U(0)
#define PLL_FRAC_MODE_MASK			U(0x80000000)
#define PLL_FRAC_MODE_SHIFT			U(31)
#define PLL_FRAC_DATA_MASK			U(0xFFFF)
#define PLL_FRAC_DATA_SHIFT			U(0)
#define PLL_FBDIV_MASK				U(0x7F00)
#define PLL_FBDIV_WIDTH				U(7)
#define PLL_FBDIV_SHIFT				U(8)

#define CLK_PLL_RESET_ASSERT			U(1)
#define CLK_PLL_RESET_RELEASE			U(2)
#define CLK_PLL_RESET_PULSE	(CLK_PLL_RESET_ASSERT | CLK_PLL_RESET_RELEASE)

/* Common topology definitions */
#define GENERIC_MUX					\
	{						\
		.type = TYPE_MUX,			\
		.offset = PERIPH_MUX_SHIFT,		\
		.width = PERIPH_MUX_WIDTH,		\
		.clkflags = CLK_SET_RATE_NO_REPARENT |	\
			    CLK_IS_BASIC,		\
		.typeflags = NA_TYPE_FLAGS,		\
		.mult = NA_MULT,			\
		.div = NA_DIV,				\
	}

#define IGNORE_UNUSED_MUX				\
	{						\
		.type = TYPE_MUX,			\
		.offset = PERIPH_MUX_SHIFT,		\
		.width = PERIPH_MUX_WIDTH,		\
		.clkflags = CLK_IGNORE_UNUSED |		\
			    CLK_SET_RATE_NO_REPARENT |	\
			    CLK_IS_BASIC,		\
		.typeflags = NA_TYPE_FLAGS,		\
		.mult = NA_MULT,			\
		.div = NA_DIV,				\
	}

#define GENERIC_DIV(id)						\
	{							\
		.type = TYPE_DIV##id,				\
		.offset = PERIPH_DIV##id##_SHIFT,		\
		.width = PERIPH_DIV##id##_WIDTH,		\
		.clkflags = CLK_SET_RATE_NO_REPARENT |		\
			    CLK_IS_BASIC,			\
		.typeflags = CLK_DIVIDER_ONE_BASED |		\
			     CLK_DIVIDER_ALLOW_ZERO,		\
		.mult = NA_MULT,				\
		.div = NA_DIV,					\
	}

#define IGNORE_UNUSED_DIV(id)					\
	{							\
		.type = TYPE_DIV##id,				\
		.offset = PERIPH_DIV##id##_SHIFT,		\
		.width = PERIPH_DIV##id##_WIDTH,		\
		.clkflags = CLK_IGNORE_UNUSED |			\
			    CLK_SET_RATE_NO_REPARENT |		\
			    CLK_IS_BASIC,			\
		.typeflags = CLK_DIVIDER_ONE_BASED |		\
			     CLK_DIVIDER_ALLOW_ZERO,		\
		.mult = NA_MULT,				\
		.div = NA_DIV,					\
	}

#define GENERIC_GATE						\
	{							\
		.type = TYPE_GATE,				\
		.offset = PERIPH_GATE_SHIFT,			\
		.width = PERIPH_GATE_WIDTH,			\
		.clkflags = CLK_SET_RATE_PARENT |		\
			    CLK_SET_RATE_GATE |			\
			    CLK_IS_BASIC,			\
		.typeflags = NA_TYPE_FLAGS,			\
		.mult = NA_MULT,				\
		.div = NA_DIV,					\
	}

#define IGNORE_UNUSED_GATE					\
	{							\
		.type = TYPE_GATE,				\
		.offset = PERIPH_GATE_SHIFT,			\
		.width = PERIPH_GATE_WIDTH,			\
		.clkflags = CLK_SET_RATE_PARENT |		\
			    CLK_IGNORE_UNUSED |			\
			    CLK_IS_BASIC,			\
		.typeflags = NA_TYPE_FLAGS,			\
		.mult = NA_MULT,				\
		.div = NA_DIV,					\
	}

/**
 * struct pm_clock_node - Clock topology node information
 * @type:	Topology type (mux/div1/div2/gate/pll/fixed factor)
 * @offset:	Offset in control register
 * @width:	Width of the specific type in control register
 * @clkflags:	Clk specific flags
 * @typeflags:	Type specific flags
 * @mult:	Multiplier for fixed factor
 * @div:	Divisor for fixed factor
 */
struct pm_clock_node {
	uint16_t clkflags;
	uint16_t typeflags;
	uint8_t type;
	uint8_t offset;
	uint8_t width;
	uint8_t mult:4;
	uint8_t div:4;
};

/**
 * struct pm_clock - Clock structure
 * @name:	Clock name
 * @control_reg:	Control register address
 * @status_reg:	Status register address
 * @parents:	Parents for first clock node. Lower byte indicates parent
 *		clock id and upper byte indicate flags for that id.
 * pm_clock_node:	Clock nodes
 */
struct pm_clock {
	char name[CLK_NAME_LEN];
	uint8_t num_nodes;
	unsigned int control_reg;
	unsigned int status_reg;
	int32_t (*parents)[];
	struct pm_clock_node(*nodes)[];
};

/**
 * struct pm_clock - Clock structure
 * @name:		Clock name
 */
struct pm_ext_clock {
	char name[CLK_NAME_LEN];
};

/* PLL Clocks */
static struct pm_clock_node generic_pll_nodes[] = {
	{
		.type = TYPE_PLL,
		.offset = NA_SHIFT,
		.width = NA_WIDTH,
		.clkflags = CLK_SET_RATE_NO_REPARENT,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node ignore_unused_pll_nodes[] = {
	{
		.type = TYPE_PLL,
		.offset = NA_SHIFT,
		.width = NA_WIDTH,
		.clkflags = CLK_IGNORE_UNUSED | CLK_SET_RATE_NO_REPARENT,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node generic_pll_pre_src_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = PLL_PRESRC_MUX_SHIFT,
		.width = PLL_PRESRC_MUX_WIDTH,
		.clkflags = CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node generic_pll_half_nodes[] = {
	{
		.type = TYPE_FIXEDFACTOR,
		.offset = NA_SHIFT,
		.width = NA_WIDTH,
		.clkflags = CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT,
		.typeflags = NA_TYPE_FLAGS,
		.mult = 1,
		.div = 2,
	},
};

static struct pm_clock_node generic_pll_int_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = PLL_DIV2_MUX_SHIFT,
		.width =  PLL_DIV2_MUX_WIDTH,
		.clkflags = CLK_SET_RATE_NO_REPARENT |
			    CLK_SET_RATE_PARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node generic_pll_post_src_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = PLL_POSTSRC_MUX_SHIFT,
		.width = PLL_POSTSRC_MUX_WIDTH,
		.clkflags = CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node generic_pll_system_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = PLL_BYPASS_MUX_SHIFT,
		.width = PLL_BYPASS_MUX_WIDTH,
		.clkflags = CLK_SET_RATE_NO_REPARENT |
			    CLK_SET_RATE_PARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node acpu_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = PERIPH_MUX_SHIFT,
		.width = PERIPH_MUX_WIDTH,
		.clkflags = CLK_SET_RATE_NO_REPARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_DIV1,
		.offset = PERIPH_DIV1_SHIFT,
		.width = PERIPH_DIV1_WIDTH,
		.clkflags = CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_GATE,
		.offset = PERIPH_GATE_SHIFT,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_IGNORE_UNUSED |
			    CLK_IS_BASIC |
			    CLK_IS_CRITICAL,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node generic_mux_div_nodes[] = {
	GENERIC_MUX,
	GENERIC_DIV(1),
};

static struct pm_clock_node generic_mux_div_gate_nodes[] = {
	GENERIC_MUX,
	GENERIC_DIV(1),
	GENERIC_GATE,
};

static struct pm_clock_node generic_mux_div_unused_gate_nodes[] = {
	GENERIC_MUX,
	GENERIC_DIV(1),
	IGNORE_UNUSED_GATE,
};

static struct pm_clock_node generic_mux_div_div_gate_nodes[] = {
	GENERIC_MUX,
	GENERIC_DIV(1),
	GENERIC_DIV(2),
	GENERIC_GATE,
};

static struct pm_clock_node dp_audio_video_ref_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = PERIPH_MUX_SHIFT,
		.width = PERIPH_MUX_WIDTH,
		.clkflags = CLK_SET_RATE_NO_REPARENT |
			    CLK_SET_RATE_PARENT |
			    CLK_FRAC | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_DIV1,
		.offset = PERIPH_DIV1_SHIFT,
		.width = PERIPH_DIV1_WIDTH,
		.clkflags = CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT |
			    CLK_FRAC | CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_DIV2,
		.offset = PERIPH_DIV2_SHIFT,
		.width = PERIPH_DIV2_WIDTH,
		.clkflags = CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT |
			    CLK_FRAC | CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_GATE,
		.offset = PERIPH_GATE_SHIFT,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_SET_RATE_GATE |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node usb_nodes[] = {
	GENERIC_MUX,
	GENERIC_DIV(1),
	GENERIC_DIV(2),
	{
		.type = TYPE_GATE,
		.offset = USB_GATE_SHIFT,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC |
			    CLK_SET_RATE_GATE,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node generic_domain_crossing_nodes[] = {
	{
		.type = TYPE_DIV1,
		.offset = 8,
		.width = 6,
		.clkflags = CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node rpll_to_fpd_nodes[] = {
	{
		.type = TYPE_DIV1,
		.offset = 8,
		.width = 6,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node acpu_half_nodes[] = {
	{
		.type = TYPE_FIXEDFACTOR,
		.offset = 0,
		.width = 1,
		.clkflags = 0,
		.typeflags = 0,
		.mult = 1,
		.div = 2,
	},
	{
		.type = TYPE_GATE,
		.offset = 25,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_IGNORE_UNUSED |
			    CLK_SET_RATE_PARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node wdt_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 0,
		.width = 1,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_SET_RATE_NO_REPARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node ddr_nodes[] = {
	GENERIC_MUX,
	{
		.type = TYPE_DIV1,
		.offset = 8,
		.width = 6,
		.clkflags = CLK_IS_BASIC | CLK_IS_CRITICAL,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node pl_nodes[] = {
	GENERIC_MUX,
	{
		.type = TYPE_DIV1,
		.offset = PERIPH_DIV1_SHIFT,
		.width = PERIPH_DIV1_WIDTH,
		.clkflags = CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_DIV2,
		.offset = PERIPH_DIV2_SHIFT,
		.width = PERIPH_DIV2_WIDTH,
		.clkflags = CLK_IS_BASIC | CLK_SET_RATE_PARENT,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_GATE,
		.offset = PERIPH_GATE_SHIFT,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node gpu_pp0_nodes[] = {
	{
		.type = TYPE_GATE,
		.offset = 25,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node gpu_pp1_nodes[] = {
	{
		.type = TYPE_GATE,
		.offset = 26,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node gem_nodes[] = {
	GENERIC_MUX,
	{
		.type = TYPE_DIV1,
		.offset = 8,
		.width = 6,
		.clkflags = CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_DIV2,
		.offset = 16,
		.width = 6,
		.clkflags = CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_GATE,
		.offset = 25,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node gem0_tx_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 1,
		.width = 1,
		.clkflags = CLK_SET_RATE_NO_REPARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_GATE,
		.offset = 26,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node gem1_tx_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 6,
		.width = 1,
		.clkflags = CLK_SET_RATE_NO_REPARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_GATE,
		.offset = 26,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node gem2_tx_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 11,
		.width = 1,
		.clkflags = CLK_SET_RATE_NO_REPARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_GATE,
		.offset = 26,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node gem3_tx_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 16,
		.width = 1,
		.clkflags = CLK_SET_RATE_NO_REPARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	{
		.type = TYPE_GATE,
		.offset = 26,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_SET_RATE_PARENT | CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node gem_tsu_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 20,
		.width = 2,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_SET_RATE_NO_REPARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node can0_mio_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 0,
		.width = 7,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_SET_RATE_NO_REPARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node can1_mio_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 15,
		.width = 1,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_SET_RATE_NO_REPARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node can0_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 7,
		.width = 1,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_SET_RATE_NO_REPARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node can1_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 22,
		.width = 1,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_SET_RATE_NO_REPARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node cpu_r5_core_nodes[] = {
	{
		.type = TYPE_GATE,
		.offset = 25,
		.width = PERIPH_GATE_WIDTH,
		.clkflags = CLK_IGNORE_UNUSED |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node dll_ref_nodes[] = {
	{
		.type = TYPE_MUX,
		.offset = 0,
		.width = 3,
		.clkflags = CLK_SET_RATE_PARENT |
			    CLK_SET_RATE_NO_REPARENT |
			    CLK_IS_BASIC,
		.typeflags = NA_TYPE_FLAGS,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
};

static struct pm_clock_node timestamp_ref_nodes[] = {
	GENERIC_MUX,
	{
		.type = TYPE_DIV1,
		.offset = 8,
		.width = 6,
		.clkflags = CLK_IS_BASIC,
		.typeflags = CLK_DIVIDER_ONE_BASED | CLK_DIVIDER_ALLOW_ZERO,
		.mult = NA_MULT,
		.div = NA_DIV,
	},
	IGNORE_UNUSED_GATE,
};

static int32_t can_mio_parents[] = {
	EXT_CLK_MIO0, EXT_CLK_MIO1, EXT_CLK_MIO2, EXT_CLK_MIO3,
	EXT_CLK_MIO4, EXT_CLK_MIO5, EXT_CLK_MIO6, EXT_CLK_MIO7,
	EXT_CLK_MIO8, EXT_CLK_MIO9, EXT_CLK_MIO10, EXT_CLK_MIO11,
	EXT_CLK_MIO12, EXT_CLK_MIO13, EXT_CLK_MIO14, EXT_CLK_MIO15,
	EXT_CLK_MIO16, EXT_CLK_MIO17, EXT_CLK_MIO18, EXT_CLK_MIO19,
	EXT_CLK_MIO20, EXT_CLK_MIO21, EXT_CLK_MIO22, EXT_CLK_MIO23,
	EXT_CLK_MIO24, EXT_CLK_MIO25, EXT_CLK_MIO26, EXT_CLK_MIO27,
	EXT_CLK_MIO28, EXT_CLK_MIO29, EXT_CLK_MIO30, EXT_CLK_MIO31,
	EXT_CLK_MIO32, EXT_CLK_MIO33, EXT_CLK_MIO34, EXT_CLK_MIO35,
	EXT_CLK_MIO36, EXT_CLK_MIO37, EXT_CLK_MIO38, EXT_CLK_MIO39,
	EXT_CLK_MIO40, EXT_CLK_MIO41, EXT_CLK_MIO42, EXT_CLK_MIO43,
	EXT_CLK_MIO44, EXT_CLK_MIO45, EXT_CLK_MIO46, EXT_CLK_MIO47,
	EXT_CLK_MIO48, EXT_CLK_MIO49, EXT_CLK_MIO50, EXT_CLK_MIO51,
	EXT_CLK_MIO52, EXT_CLK_MIO53, EXT_CLK_MIO54, EXT_CLK_MIO55,
	EXT_CLK_MIO56, EXT_CLK_MIO57, EXT_CLK_MIO58, EXT_CLK_MIO59,
	EXT_CLK_MIO60, EXT_CLK_MIO61, EXT_CLK_MIO62, EXT_CLK_MIO63,
	EXT_CLK_MIO64, EXT_CLK_MIO65, EXT_CLK_MIO66, EXT_CLK_MIO67,
	EXT_CLK_MIO68, EXT_CLK_MIO69, EXT_CLK_MIO70, EXT_CLK_MIO71,
	EXT_CLK_MIO72, EXT_CLK_MIO73, EXT_CLK_MIO74, EXT_CLK_MIO75,
	EXT_CLK_MIO76, EXT_CLK_MIO77, CLK_NA_PARENT
};

/* Clock array containing clock informaton */
static struct pm_clock clocks[] = {
	[CLK_APLL_INT] = {
		.name = "apll_int",
		.control_reg = CRF_APB_APLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_APLL_PRE_SRC, CLK_NA_PARENT}),
		.nodes = &ignore_unused_pll_nodes,
		.num_nodes = ARRAY_SIZE(ignore_unused_pll_nodes),
	},
	[CLK_APLL_PRE_SRC] = {
		.name = "apll_pre_src",
		.control_reg = CRF_APB_APLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_pre_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_pre_src_nodes),
	},
	[CLK_APLL_HALF] = {
		.name = "apll_half",
		.control_reg = CRF_APB_APLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_APLL_INT, CLK_NA_PARENT}),
		.nodes = &generic_pll_half_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_half_nodes),
	},
	[CLK_APLL_INT_MUX] = {
		.name = "apll_int_mux",
		.control_reg = CRF_APB_APLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_APLL_INT,
			CLK_APLL_HALF,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_int_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_int_nodes),
	},
	[CLK_APLL_POST_SRC] = {
		.name = "apll_post_src",
		.control_reg = CRF_APB_APLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_post_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_post_src_nodes),
	},
	[CLK_APLL] = {
		.name = "apll",
		.control_reg = CRF_APB_APLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_APLL_INT_MUX,
			CLK_APLL_POST_SRC,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_system_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_system_nodes),
	},
	[CLK_DPLL_INT] = {
		.name = "dpll_int",
		.control_reg = CRF_APB_DPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_DPLL_PRE_SRC, CLK_NA_PARENT}),
		.nodes = &generic_pll_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_nodes),
	},
	[CLK_DPLL_PRE_SRC] = {
		.name = "dpll_pre_src",
		.control_reg = CRF_APB_DPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_pre_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_pre_src_nodes),
	},
	[CLK_DPLL_HALF] = {
		.name = "dpll_half",
		.control_reg = CRF_APB_DPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_DPLL_INT, CLK_NA_PARENT}),
		.nodes = &generic_pll_half_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_half_nodes),
	},
	[CLK_DPLL_INT_MUX] = {
		.name = "dpll_int_mux",
		.control_reg = CRF_APB_DPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_DPLL_INT,
			CLK_DPLL_HALF,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_int_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_int_nodes),
	},
	[CLK_DPLL_POST_SRC] = {
		.name = "dpll_post_src",
		.control_reg = CRF_APB_DPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_post_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_post_src_nodes),
	},
	[CLK_DPLL] = {
		.name = "dpll",
		.control_reg = CRF_APB_DPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_DPLL_INT_MUX,
			CLK_DPLL_POST_SRC,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_system_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_system_nodes),
	},
	[CLK_VPLL_INT] = {
		.name = "vpll_int",
		.control_reg = CRF_APB_VPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_VPLL_PRE_SRC, CLK_NA_PARENT}),
		.nodes = &ignore_unused_pll_nodes,
		.num_nodes = ARRAY_SIZE(ignore_unused_pll_nodes),
	},
	[CLK_VPLL_PRE_SRC] = {
		.name = "vpll_pre_src",
		.control_reg = CRF_APB_VPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_pre_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_pre_src_nodes),
	},
	[CLK_VPLL_HALF] = {
		.name = "vpll_half",
		.control_reg = CRF_APB_VPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_VPLL_INT, CLK_NA_PARENT}),
		.nodes = &generic_pll_half_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_half_nodes),
	},
	[CLK_VPLL_INT_MUX] = {
		.name = "vpll_int_mux",
		.control_reg = CRF_APB_VPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_VPLL_INT,
			CLK_VPLL_HALF,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_int_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_int_nodes),
	},
	[CLK_VPLL_POST_SRC] = {
		.name = "vpll_post_src",
		.control_reg = CRF_APB_VPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_post_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_post_src_nodes),
	},
	[CLK_VPLL] = {
		.name = "vpll",
		.control_reg = CRF_APB_VPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_VPLL_INT_MUX,
			CLK_VPLL_POST_SRC,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_system_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_system_nodes),
	},
	[CLK_IOPLL_INT] = {
		.name = "iopll_int",
		.control_reg = CRL_APB_IOPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_IOPLL_PRE_SRC, CLK_NA_PARENT}),
		.nodes = &generic_pll_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_nodes),
	},
	[CLK_IOPLL_PRE_SRC] = {
		.name = "iopll_pre_src",
		.control_reg = CRL_APB_IOPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_pre_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_pre_src_nodes),
	},
	[CLK_IOPLL_HALF] = {
		.name = "iopll_half",
		.control_reg = CRL_APB_IOPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_IOPLL_INT, CLK_NA_PARENT}),
		.nodes = &generic_pll_half_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_half_nodes),
	},
	[CLK_IOPLL_INT_MUX] = {
		.name = "iopll_int_mux",
		.control_reg = CRL_APB_IOPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_IOPLL_INT,
			CLK_IOPLL_HALF,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_int_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_int_nodes),
	},
	[CLK_IOPLL_POST_SRC] = {
		.name = "iopll_post_src",
		.control_reg = CRL_APB_IOPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_post_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_post_src_nodes),
	},
	[CLK_IOPLL] = {
		.name = "iopll",
		.control_reg = CRL_APB_IOPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_IOPLL_INT_MUX,
			CLK_IOPLL_POST_SRC,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_system_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_system_nodes),
	},
	[CLK_RPLL_INT] = {
		.name = "rpll_int",
		.control_reg = CRL_APB_RPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_RPLL_PRE_SRC, CLK_NA_PARENT}),
		.nodes = &generic_pll_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_nodes),
	},
	[CLK_RPLL_PRE_SRC] = {
		.name = "rpll_pre_src",
		.control_reg = CRL_APB_RPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),

		.nodes = &generic_pll_pre_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_pre_src_nodes),
	},
	[CLK_RPLL_HALF] = {
		.name = "rpll_half",
		.control_reg = CRL_APB_RPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {CLK_RPLL_INT, CLK_NA_PARENT}),
		.nodes = &generic_pll_half_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_half_nodes),
	},
	[CLK_RPLL_INT_MUX] = {
		.name = "rpll_int_mux",
		.control_reg = CRL_APB_RPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_RPLL_INT,
			CLK_RPLL_HALF,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_int_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_int_nodes),
	},
	[CLK_RPLL_POST_SRC] = {
		.name = "rpll_post_src",
		.control_reg = CRL_APB_RPLL_CTRL,
		.status_reg = CRF_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_VIDEO | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_ALT_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_AUX_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_GT_CRX_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_post_src_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_post_src_nodes),
	},
	[CLK_RPLL] = {
		.name = "rpll",
		.control_reg = CRL_APB_RPLL_CTRL,
		.status_reg = CRL_APB_PLL_STATUS,
		.parents = &((int32_t []) {
			CLK_RPLL_INT_MUX,
			CLK_RPLL_POST_SRC,
			CLK_NA_PARENT
		}),
		.nodes = &generic_pll_system_nodes,
		.num_nodes = ARRAY_SIZE(generic_pll_system_nodes),
	},
	/* Peripheral Clocks */
	[CLK_ACPU] = {
		.name = "acpu",
		.control_reg = CRF_APB_ACPU_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_APLL,
			CLK_DUMMY_PARENT,
			CLK_DPLL,
			CLK_VPLL,
			CLK_NA_PARENT
		}),
		.nodes = &acpu_nodes,
		.num_nodes = ARRAY_SIZE(acpu_nodes),
	},
	[CLK_DBG_TRACE] = {
		.name = "dbg_trace",
		.control_reg = CRF_APB_DBG_TRACE_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL_TO_FPD,
			CLK_DUMMY_PARENT,
			CLK_DPLL,
			CLK_APLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_DBG_FPD] = {
		.name = "dbg_fpd",
		.control_reg = CRF_APB_DBG_FPD_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL_TO_FPD,
			CLK_DUMMY_PARENT,
			CLK_DPLL,
			CLK_APLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_DBG_TSTMP] = {
		.name = "dbg_tstmp",
		.control_reg = CRF_APB_DBG_TSTMP_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL_TO_FPD,
			CLK_DUMMY_PARENT,
			CLK_DPLL,
			CLK_APLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_nodes),
	},
	[CLK_DP_VIDEO_REF] = {
		.name = "dp_video_ref",
		.control_reg = CRF_APB_DP_VIDEO_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_VPLL,
			CLK_DUMMY_PARENT,
			CLK_DPLL,
			CLK_RPLL_TO_FPD,
			CLK_NA_PARENT
		}),
		.nodes = &dp_audio_video_ref_nodes,
		.num_nodes = ARRAY_SIZE(dp_audio_video_ref_nodes),
	},
	[CLK_DP_AUDIO_REF] = {
		.name = "dp_audio_ref",
		.control_reg = CRF_APB_DP_AUDIO_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_VPLL,
			CLK_DUMMY_PARENT,
			CLK_DPLL,
			CLK_RPLL_TO_FPD,
			CLK_NA_PARENT
		}),
		.nodes = &dp_audio_video_ref_nodes,
		.num_nodes = ARRAY_SIZE(dp_audio_video_ref_nodes),
	},
	[CLK_DP_STC_REF] = {
		.name = "dp_stc_ref",
		.control_reg = CRF_APB_DP_STC_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_VPLL,
			CLK_DUMMY_PARENT,
			CLK_DPLL,
			CLK_RPLL_TO_FPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_DPDMA_REF] = {
		.name = "dpdma_ref",
		.control_reg = CRF_APB_DPDMA_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_APLL,
			CLK_DUMMY_PARENT,
			CLK_VPLL,
			CLK_DPLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_DDR_REF] = {
		.name = "ddr_ref",
		.control_reg = CRF_APB_DDR_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_DPLL,
			CLK_VPLL,
			CLK_NA_PARENT
		}),
		.nodes = &ddr_nodes,
		.num_nodes = ARRAY_SIZE(ddr_nodes),
	},
	[CLK_GPU_REF] = {
		.name = "gpu_ref",
		.control_reg = CRF_APB_GPU_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL_TO_FPD,
			CLK_DUMMY_PARENT,
			CLK_VPLL,
			CLK_DPLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_SATA_REF] = {
		.name = "sata_ref",
		.control_reg = CRF_APB_SATA_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL_TO_FPD,
			CLK_DUMMY_PARENT,
			CLK_APLL,
			CLK_DPLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_PCIE_REF] = {
		.name = "pcie_ref",
		.control_reg = CRF_APB_PCIE_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL_TO_FPD,
			CLK_DUMMY_PARENT,
			CLK_RPLL_TO_FPD,
			CLK_DPLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_GDMA_REF] = {
		.name = "gdma_ref",
		.control_reg = CRF_APB_GDMA_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_APLL,
			CLK_DUMMY_PARENT,
			CLK_VPLL,
			CLK_DPLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_GTGREF0_REF] = {
		.name = "gtgref0_ref",
		.control_reg = CRF_APB_GTGREF0_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL_TO_FPD,
			CLK_DUMMY_PARENT,
			CLK_APLL,
			CLK_DPLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_TOPSW_MAIN] = {
		.name = "topsw_main",
		.control_reg = CRF_APB_TOPSW_MAIN_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_APLL,
			CLK_DUMMY_PARENT,
			CLK_VPLL,
			CLK_DPLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_unused_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_unused_gate_nodes),
	},
	[CLK_TOPSW_LSBUS] = {
		.name = "topsw_lsbus",
		.control_reg = CRF_APB_TOPSW_LSBUS_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_APLL,
			CLK_DUMMY_PARENT,
			CLK_IOPLL_TO_FPD,
			CLK_DPLL,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_unused_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_unused_gate_nodes),
	},
	[CLK_IOU_SWITCH] = {
		.name = "iou_switch",
		.control_reg = CRL_APB_IOU_SWITCH_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_RPLL,
			CLK_DUMMY_PARENT,
			CLK_IOPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_unused_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_unused_gate_nodes),
	},
	[CLK_GEM0_REF] = {
		.name = "gem0_ref",
		.control_reg = CRL_APB_GEM0_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &gem_nodes,
		.num_nodes = ARRAY_SIZE(gem_nodes),
	},
	[CLK_GEM1_REF] = {
		.name = "gem1_ref",
		.control_reg = CRL_APB_GEM1_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &gem_nodes,
		.num_nodes = ARRAY_SIZE(gem_nodes),
	},
	[CLK_GEM2_REF] = {
		.name = "gem2_ref",
		.control_reg = CRL_APB_GEM2_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &gem_nodes,
		.num_nodes = ARRAY_SIZE(gem_nodes),
	},
	[CLK_GEM3_REF] = {
		.name = "gem3_ref",
		.control_reg = CRL_APB_GEM3_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &gem_nodes,
		.num_nodes = ARRAY_SIZE(gem_nodes),
	},
	[CLK_USB0_BUS_REF] = {
		.name = "usb0_bus_ref",
		.control_reg = CRL_APB_USB0_BUS_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &usb_nodes,
		.num_nodes = ARRAY_SIZE(usb_nodes),
	},
	[CLK_USB1_BUS_REF] = {
		.name = "usb1_bus_ref",
		.control_reg = CRL_APB_USB1_BUS_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &usb_nodes,
		.num_nodes = ARRAY_SIZE(usb_nodes),
	},
	[CLK_USB3_DUAL_REF] = {
		.name = "usb3_dual_ref",
		.control_reg = CRL_APB_USB3_DUAL_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &usb_nodes,
		.num_nodes = ARRAY_SIZE(usb_nodes),
	},
	[CLK_QSPI_REF] = {
		.name = "qspi_ref",
		.control_reg = CRL_APB_QSPI_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_SDIO0_REF] = {
		.name = "sdio0_ref",
		.control_reg = CRL_APB_SDIO0_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_VPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_SDIO1_REF] = {
		.name = "sdio1_ref",
		.control_reg = CRL_APB_SDIO1_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_VPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_UART0_REF] = {
		.name = "uart0_ref",
		.control_reg = CRL_APB_UART0_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_UART1_REF] = {
		.name = "uart1_ref",
		.control_reg = CRL_APB_UART1_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_SPI0_REF] = {
		.name = "spi0_ref",
		.control_reg = CRL_APB_SPI0_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_SPI1_REF] = {
		.name = "spi1_ref",
		.control_reg = CRL_APB_SPI1_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_CAN0_REF] = {
		.name = "can0_ref",
		.control_reg = CRL_APB_CAN0_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_CAN1_REF] = {
		.name = "can1_ref",
		.control_reg = CRL_APB_CAN1_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_NAND_REF] = {
		.name = "nand_ref",
		.control_reg = CRL_APB_NAND_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_GEM_TSU_REF] = {
		.name = "gem_tsu_ref",
		.control_reg = CRL_APB_GEM_TSU_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_DLL_REF] = {
		.name = "dll_ref",
		.control_reg = CRL_APB_DLL_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_RPLL,
			CLK_NA_PARENT
		}),
		.nodes = &dll_ref_nodes,
		.num_nodes = ARRAY_SIZE(dll_ref_nodes),
	},
	[CLK_ADMA_REF] = {
		.name = "adma_ref",
		.control_reg = CRL_APB_ADMA_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_RPLL,
			CLK_DUMMY_PARENT,
			CLK_IOPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_DBG_LPD] = {
		.name = "dbg_lpd",
		.control_reg = CRL_APB_DBG_LPD_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_RPLL,
			CLK_DUMMY_PARENT,
			CLK_IOPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_CPU_R5] = {
		.name = "cpu_r5",
		.control_reg = CRL_APB_CPU_R5_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_RPLL,
			CLK_DUMMY_PARENT,
			CLK_IOPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_unused_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_unused_gate_nodes),
	},
	[CLK_CSU_PLL] = {
		.name = "csu_pll",
		.control_reg = CRL_APB_CSU_PLL_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_PCAP] = {
		.name = "pcap",
		.control_reg = CRL_APB_PCAP_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_gate_nodes),
	},
	[CLK_LPD_LSBUS] = {
		.name = "lpd_lsbus",
		.control_reg = CRL_APB_LPD_LSBUS_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_RPLL,
			CLK_DUMMY_PARENT,
			CLK_IOPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_unused_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_unused_gate_nodes),
	},
	[CLK_LPD_SWITCH] = {
		.name = "lpd_switch",
		.control_reg = CRL_APB_LPD_SWITCH_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_RPLL,
			CLK_DUMMY_PARENT,
			CLK_IOPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_unused_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_unused_gate_nodes),
	},
	[CLK_I2C0_REF] = {
		.name = "i2c0_ref",
		.control_reg = CRL_APB_I2C0_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_I2C1_REF] = {
		.name = "i2c1_ref",
		.control_reg = CRL_APB_I2C1_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_TIMESTAMP_REF] = {
		.name = "timestamp_ref",
		.control_reg = CRL_APB_TIMESTAMP_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			EXT_CLK_PSS_REF | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &timestamp_ref_nodes,
		.num_nodes = ARRAY_SIZE(timestamp_ref_nodes),
	},
	[CLK_PL0_REF] = {
		.name = "pl0_ref",
		.control_reg = CRL_APB_PL0_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &pl_nodes,
		.num_nodes = ARRAY_SIZE(pl_nodes),
	},
	[CLK_PL1_REF] = {
		.name = "pl1_ref",
		.control_reg = CRL_APB_PL1_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &pl_nodes,
		.num_nodes = ARRAY_SIZE(pl_nodes),
	},
	[CLK_PL2_REF] = {
		.name = "pl2_ref",
		.control_reg = CRL_APB_PL2_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &pl_nodes,
		.num_nodes = ARRAY_SIZE(pl_nodes),
	},
	[CLK_PL3_REF] = {
		.name = "pl3_ref",
		.control_reg = CRL_APB_PL3_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_IOPLL,
			CLK_DUMMY_PARENT,
			CLK_RPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &pl_nodes,
		.num_nodes = ARRAY_SIZE(pl_nodes),
	},
	[CLK_AMS_REF] = {
		.name = "ams_ref",
		.control_reg = CRL_APB_AMS_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_RPLL,
			CLK_DUMMY_PARENT,
			CLK_IOPLL,
			CLK_DPLL_TO_LPD,
			CLK_NA_PARENT
		}),
		.nodes = &generic_mux_div_div_gate_nodes,
		.num_nodes = ARRAY_SIZE(generic_mux_div_div_gate_nodes),
	},
	[CLK_IOPLL_TO_FPD] = {
		.name = "iopll_to_fpd",
		.control_reg = CRL_APB_IOPLL_TO_FPD_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {CLK_IOPLL, CLK_NA_PARENT}),
		.nodes = &generic_domain_crossing_nodes,
		.num_nodes = ARRAY_SIZE(generic_domain_crossing_nodes),
	},
	[CLK_RPLL_TO_FPD] = {
		.name = "rpll_to_fpd",
		.control_reg = CRL_APB_RPLL_TO_FPD_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {CLK_RPLL, CLK_NA_PARENT}),
		.nodes = &rpll_to_fpd_nodes,
		.num_nodes = ARRAY_SIZE(rpll_to_fpd_nodes),
	},
	[CLK_APLL_TO_LPD] = {
		.name = "apll_to_lpd",
		.control_reg = CRF_APB_APLL_TO_LPD_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {CLK_APLL, CLK_NA_PARENT}),
		.nodes = &generic_domain_crossing_nodes,
		.num_nodes = ARRAY_SIZE(generic_domain_crossing_nodes),
	},
	[CLK_DPLL_TO_LPD] = {
		.name = "dpll_to_lpd",
		.control_reg = CRF_APB_DPLL_TO_LPD_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {CLK_DPLL, CLK_NA_PARENT}),
		.nodes = &generic_domain_crossing_nodes,
		.num_nodes = ARRAY_SIZE(generic_domain_crossing_nodes),
	},
	[CLK_VPLL_TO_LPD] = {
		.name = "vpll_to_lpd",
		.control_reg = CRF_APB_VPLL_TO_LPD_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {CLK_VPLL, CLK_NA_PARENT}),
		.nodes = &generic_domain_crossing_nodes,
		.num_nodes = ARRAY_SIZE(generic_domain_crossing_nodes),
	},
	/*
	 * This clock control requires different registers for mux and gate.
	 * Use control and status registers for the same.
	 */
	[CLK_GEM0_TX] = {
		.name = "gem0_tx",
		.control_reg = IOU_SLCR_GEM_CLK_CTRL,
		.status_reg = CRL_APB_GEM0_REF_CTRL,
		.parents = &((int32_t []) {
			CLK_GEM0_REF | (PARENT_CLK_NODE3 << CLK_PARENTS_ID_LEN),
			EXT_CLK_GEM0_EMIO | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &gem0_tx_nodes,
		.num_nodes = ARRAY_SIZE(gem0_tx_nodes),
	},
	/*
	 * This clock control requires different registers for mux and gate.
	 * Use control and status registers for the same.
	 */
	[CLK_GEM1_TX] = {
		.name = "gem1_tx",
		.control_reg = IOU_SLCR_GEM_CLK_CTRL,
		.status_reg = CRL_APB_GEM1_REF_CTRL,
		.parents = &((int32_t []) {
			CLK_GEM1_REF | (PARENT_CLK_NODE3 << CLK_PARENTS_ID_LEN),
			EXT_CLK_GEM1_EMIO | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &gem1_tx_nodes,
		.num_nodes = ARRAY_SIZE(gem1_tx_nodes),
	},
	/*
	 * This clock control requires different registers for mux and gate.
	 * Use control and status registers for the same.
	 */
	[CLK_GEM2_TX] = {
		.name = "gem2_tx",
		.control_reg = IOU_SLCR_GEM_CLK_CTRL,
		.status_reg = CRL_APB_GEM2_REF_CTRL,
		.parents = &((int32_t []) {
			CLK_GEM2_REF | (PARENT_CLK_NODE3 << CLK_PARENTS_ID_LEN),
			EXT_CLK_GEM2_EMIO | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &gem2_tx_nodes,
		.num_nodes = ARRAY_SIZE(gem2_tx_nodes),
	},
	/*
	 * This clock control requires different registers for mux and gate.
	 * Use control and status registers for the same.
	 */
	[CLK_GEM3_TX] = {
		.name = "gem3_tx",
		.control_reg = IOU_SLCR_GEM_CLK_CTRL,
		.status_reg = CRL_APB_GEM3_REF_CTRL,
		.parents = &((int32_t []) {
			CLK_GEM3_REF | (PARENT_CLK_NODE3 << CLK_PARENTS_ID_LEN),
			EXT_CLK_GEM3_EMIO | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &gem3_tx_nodes,
		.num_nodes = ARRAY_SIZE(gem3_tx_nodes),
	},
	[CLK_ACPU_HALF] = {
		.name = "acpu_half",
		.control_reg = CRF_APB_ACPU_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_ACPU | PARENT_CLK_NODE2 << CLK_PARENTS_ID_LEN,
			CLK_NA_PARENT
		}),
		.nodes = &acpu_half_nodes,
		.num_nodes = ARRAY_SIZE(acpu_half_nodes),
	},
	[CLK_WDT] = {
		.name = "wdt",
		.control_reg = FPD_SLCR_WDT_CLK_SEL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_TOPSW_LSBUS,
			EXT_CLK_SWDT0 | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &wdt_nodes,
		.num_nodes = ARRAY_SIZE(wdt_nodes),
	},
	[CLK_GPU_PP0_REF] = {
		.name = "gpu_pp0_ref",
		.control_reg = CRF_APB_GPU_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_GPU_REF | PARENT_CLK_NODE2 << CLK_PARENTS_ID_LEN,
			CLK_NA_PARENT
		}),
		.nodes = &gpu_pp0_nodes,
		.num_nodes = ARRAY_SIZE(gpu_pp0_nodes),
	},
	[CLK_GPU_PP1_REF] = {
		.name = "gpu_pp1_ref",
		.control_reg = CRF_APB_GPU_REF_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_GPU_REF | PARENT_CLK_NODE2 << CLK_PARENTS_ID_LEN,
			CLK_NA_PARENT
		}),
		.nodes = &gpu_pp1_nodes,
		.num_nodes = ARRAY_SIZE(gpu_pp1_nodes),
	},
	[CLK_GEM_TSU] = {
		.name = "gem_tsu",
		.control_reg = IOU_SLCR_GEM_CLK_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_GEM_TSU_REF,
			CLK_GEM_TSU_REF,
			EXT_CLK_MIO26 | CLK_EXTERNAL_PARENT,
			EXT_CLK_MIO50_OR_MIO51 | CLK_EXTERNAL_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &gem_tsu_nodes,
		.num_nodes = ARRAY_SIZE(gem_tsu_nodes),
	},
	[CLK_CPU_R5_CORE] = {
		.name = "cpu_r5_core",
		.control_reg = CRL_APB_CPU_R5_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_CPU_R5 | PARENT_CLK_NODE2 << CLK_PARENTS_ID_LEN,
			CLK_DUMMY_PARENT,
			CLK_NA_PARENT
		}),
		.nodes = &cpu_r5_core_nodes,
		.num_nodes = ARRAY_SIZE(cpu_r5_core_nodes),
	},
	[CLK_CAN0_MIO] = {
		.name = "can0_mio",
		.control_reg = IOU_SLCR_CAN_MIO_CTRL,
		.status_reg = 0,
		.parents = &can_mio_parents,
		.nodes = &can0_mio_nodes,
		.num_nodes = ARRAY_SIZE(can0_mio_nodes),
	},
	[CLK_CAN1_MIO] = {
		.name = "can1_mio",
		.control_reg = IOU_SLCR_CAN_MIO_CTRL,
		.status_reg = 0,
		.parents = &can_mio_parents,
		.nodes = &can1_mio_nodes,
		.num_nodes = ARRAY_SIZE(can1_mio_nodes),
	},
	[CLK_CAN0] = {
		.name = "can0",
		.control_reg = IOU_SLCR_CAN_MIO_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_CAN0_REF,
			CLK_CAN0_MIO,
			CLK_NA_PARENT
		}),
		.nodes = &can0_nodes,
		.num_nodes = ARRAY_SIZE(can0_nodes),
	},
	[CLK_CAN1] = {
		.name = "can1",
		.control_reg = IOU_SLCR_CAN_MIO_CTRL,
		.status_reg = 0,
		.parents = &((int32_t []) {
			CLK_CAN1_REF,
			CLK_CAN1_MIO,
			CLK_NA_PARENT
		}),
		.nodes = &can1_nodes,
		.num_nodes = ARRAY_SIZE(can1_nodes),
	},
};

static struct pm_ext_clock ext_clocks[] = {
	[EXT_CLK_INDEX(EXT_CLK_PSS_REF)] = {
		.name = "pss_ref_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_VIDEO)] = {
		.name = "video_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_PSS_ALT_REF)] = {
		.name = "pss_alt_ref_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_AUX_REF)] = {
		.name = "aux_ref_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_GT_CRX_REF)] = {
		.name = "video_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_SWDT0)] = {
		.name = "swdt0_ext_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_SWDT1)] = {
		.name = "swdt1_ext_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_GEM0_EMIO)] = {
		.name = "gem0_emio_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_GEM1_EMIO)] = {
		.name = "gem1_emio_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_GEM2_EMIO)] = {
		.name = "gem2_emio_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_GEM3_EMIO)] = {
		.name = "gem3_emio_clk",
	},
	[EXT_CLK_INDEX(EXT_CLK_MIO50_OR_MIO51)] = {
		.name = "mio_clk_50_51",
	},
	EXT_CLK_MIO_DATA(0),
	EXT_CLK_MIO_DATA(1),
	EXT_CLK_MIO_DATA(2),
	EXT_CLK_MIO_DATA(3),
	EXT_CLK_MIO_DATA(4),
	EXT_CLK_MIO_DATA(5),
	EXT_CLK_MIO_DATA(6),
	EXT_CLK_MIO_DATA(7),
	EXT_CLK_MIO_DATA(8),
	EXT_CLK_MIO_DATA(9),
	EXT_CLK_MIO_DATA(10),
	EXT_CLK_MIO_DATA(11),
	EXT_CLK_MIO_DATA(12),
	EXT_CLK_MIO_DATA(13),
	EXT_CLK_MIO_DATA(14),
	EXT_CLK_MIO_DATA(15),
	EXT_CLK_MIO_DATA(16),
	EXT_CLK_MIO_DATA(17),
	EXT_CLK_MIO_DATA(18),
	EXT_CLK_MIO_DATA(19),
	EXT_CLK_MIO_DATA(20),
	EXT_CLK_MIO_DATA(21),
	EXT_CLK_MIO_DATA(22),
	EXT_CLK_MIO_DATA(23),
	EXT_CLK_MIO_DATA(24),
	EXT_CLK_MIO_DATA(25),
	EXT_CLK_MIO_DATA(26),
	EXT_CLK_MIO_DATA(27),
	EXT_CLK_MIO_DATA(28),
	EXT_CLK_MIO_DATA(29),
	EXT_CLK_MIO_DATA(30),
	EXT_CLK_MIO_DATA(31),
	EXT_CLK_MIO_DATA(32),
	EXT_CLK_MIO_DATA(33),
	EXT_CLK_MIO_DATA(34),
	EXT_CLK_MIO_DATA(35),
	EXT_CLK_MIO_DATA(36),
	EXT_CLK_MIO_DATA(37),
	EXT_CLK_MIO_DATA(38),
	EXT_CLK_MIO_DATA(39),
	EXT_CLK_MIO_DATA(40),
	EXT_CLK_MIO_DATA(41),
	EXT_CLK_MIO_DATA(42),
	EXT_CLK_MIO_DATA(43),
	EXT_CLK_MIO_DATA(44),
	EXT_CLK_MIO_DATA(45),
	EXT_CLK_MIO_DATA(46),
	EXT_CLK_MIO_DATA(47),
	EXT_CLK_MIO_DATA(48),
	EXT_CLK_MIO_DATA(49),
	EXT_CLK_MIO_DATA(50),
	EXT_CLK_MIO_DATA(51),
	EXT_CLK_MIO_DATA(52),
	EXT_CLK_MIO_DATA(53),
	EXT_CLK_MIO_DATA(54),
	EXT_CLK_MIO_DATA(55),
	EXT_CLK_MIO_DATA(56),
	EXT_CLK_MIO_DATA(57),
	EXT_CLK_MIO_DATA(58),
	EXT_CLK_MIO_DATA(59),
	EXT_CLK_MIO_DATA(60),
	EXT_CLK_MIO_DATA(61),
	EXT_CLK_MIO_DATA(62),
	EXT_CLK_MIO_DATA(63),
	EXT_CLK_MIO_DATA(64),
	EXT_CLK_MIO_DATA(65),
	EXT_CLK_MIO_DATA(66),
	EXT_CLK_MIO_DATA(67),
	EXT_CLK_MIO_DATA(68),
	EXT_CLK_MIO_DATA(69),
	EXT_CLK_MIO_DATA(70),
	EXT_CLK_MIO_DATA(71),
	EXT_CLK_MIO_DATA(72),
	EXT_CLK_MIO_DATA(73),
	EXT_CLK_MIO_DATA(74),
	EXT_CLK_MIO_DATA(75),
	EXT_CLK_MIO_DATA(76),
	EXT_CLK_MIO_DATA(77),
};

/* Array of clock which are invalid for this variant */
static uint32_t pm_clk_invalid_list[] = {CLK_USB0, CLK_USB1, CLK_CSU_SPB};

/**
 * pm_clock_valid - Check if clock is valid or not
 * @clock_id	Id of the clock to be queried
 *
 * This function is used to check if given clock is valid
 * or not for the chip variant.
 *
 * List of invalid clocks are maintained in array list for
 * different variants.
 *
 * Return: Returns 1 if clock is valid else 0.
 */
static bool pm_clock_valid(unsigned int clock_id)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pm_clk_invalid_list); i++)
		if (pm_clk_invalid_list[i] == clock_id)
			return 0;

	return 1;
}

/**
 * pm_clock_type - Get clock's type
 * @clock_id	Id of the clock to be queried
 *
 * This function is used to check type of clock (OUTPUT/EXTERNAL).
 *
 * Return: Returns type of clock (OUTPUT/EXTERNAL).
 */
static unsigned int pm_clock_type(unsigned int clock_id)
{
	return (clock_id < CLK_MAX_OUTPUT_CLK) ?
		CLK_TYPE_OUTPUT : CLK_TYPE_EXTERNAL;
}

/**
 * pm_api_clock_get_num_clocks() - PM call to request number of clocks
 * @nclocks	Number of clocks
 *
 * This function is used by master to get number of clocks.
 *
 * @return	Returns success.
 */
enum pm_ret_status pm_api_clock_get_num_clocks(unsigned int *nclocks)
{
	*nclocks = CLK_MAX;

	return PM_RET_SUCCESS;
}

/**
 * pm_api_clock_get_name() - PM call to request a clock's name
 * @clock_id	Clock ID
 * @name	Name of clock (max 16 bytes)
 *
 * This function is used by master to get nmae of clock specified
 * by given clock ID.
 *
 * @return	Returns success. In case of error, name data is 0.
 */
enum pm_ret_status pm_api_clock_get_name(unsigned int clock_id, char *name)
{
	if (clock_id == CLK_MAX)
		memcpy(name, END_OF_CLK, CLK_NAME_LEN);
	else if (!pm_clock_valid(clock_id))
		memset(name, 0, CLK_NAME_LEN);
	else if (clock_id < CLK_MAX_OUTPUT_CLK)
		memcpy(name, clocks[clock_id].name, CLK_NAME_LEN);
	else
		memcpy(name, ext_clocks[clock_id - CLK_MAX_OUTPUT_CLK].name,
		       CLK_NAME_LEN);

	return PM_RET_SUCCESS;
}

/**
 * pm_api_clock_get_topology() - PM call to request a clock's topology
 * @clock_id	Clock ID
 * @index	Topology index for next toplogy node
 * @topology	Buffer to store nodes in topology and flags
 *
 * This function is used by master to get topology information for the
 * clock specified by given clock ID. Each response would return 3
 * topology nodes. To get next nodes, caller needs to call this API with
 * index of next node. Index starts from 0.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clock_get_topology(unsigned int clock_id,
					     unsigned int index,
					     uint32_t *topology)
{
	struct pm_clock_node *clock_nodes;
	uint8_t num_nodes;
	unsigned int i;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;


	memset(topology, 0, CLK_TOPOLOGY_PAYLOAD_LEN);
	clock_nodes = *clocks[clock_id].nodes;
	num_nodes = clocks[clock_id].num_nodes;

	/* Skip parent till index */
	if (index >= num_nodes)
		return PM_RET_SUCCESS;

	for (i = 0; i < 3U; i++) {
		if ((index + i) == num_nodes)
			break;
		topology[i] =  clock_nodes[index + i].type;
		topology[i] |= clock_nodes[index + i].clkflags <<
					CLK_CLKFLAGS_SHIFT;
		topology[i] |= clock_nodes[index + i].typeflags <<
					CLK_TYPEFLAGS_SHIFT;
	}

	return PM_RET_SUCCESS;
}

/**
 * pm_api_clock_get_fixedfactor_params() - PM call to request a clock's fixed
 *					   factor parameters for fixed clock
 * @clock_id	Clock ID
 * @mul		Multiplication value
 * @div		Divisor value
 *
 * This function is used by master to get fixed factor parameers for the
 * fixed clock. This API is application only for the fixed clock.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clock_get_fixedfactor_params(unsigned int clock_id,
						       uint32_t *mul,
						       uint32_t *div)
{
	struct pm_clock_node *clock_nodes;
	uint8_t num_nodes;
	unsigned int type, i;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	clock_nodes = *clocks[clock_id].nodes;
	num_nodes = clocks[clock_id].num_nodes;

	for (i = 0; i < num_nodes; i++) {
		type =  clock_nodes[i].type;
		if (type == TYPE_FIXEDFACTOR) {
			*mul = clock_nodes[i].mult;
			*div = clock_nodes[i].div;
			break;
		}
	}

	/* Clock is not fixed clock */
	if (i == num_nodes)
		return PM_RET_ERROR_ARGS;

	return PM_RET_SUCCESS;
}

/**
 * pm_api_clock_get_parents() - PM call to request a clock's first 3 parents
 * @clock_id	Clock ID
 * @index	Index of next parent
 * @parents	Parents of the given clock
 *
 * This function is used by master to get clock's parents information.
 * This API will return 3 parents with a single response. To get other
 * parents, master should call same API in loop with new parent index
 * till error is returned.
 *
 * E.g First call should have index 0 which will return parents 0, 1 and
 * 2. Next call, index should be 3 which will return parent 3,4 and 5 and
 * so on.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clock_get_parents(unsigned int clock_id,
					    unsigned int index,
					    uint32_t *parents)
{
	unsigned int i;
	int32_t *clk_parents;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	clk_parents = *clocks[clock_id].parents;
	if (clk_parents == NULL)
		return PM_RET_ERROR_ARGS;

	memset(parents, 0, CLK_PARENTS_PAYLOAD_LEN);

	/* Skip parent till index */
	for (i = 0; i < index; i++)
		if (clk_parents[i] == CLK_NA_PARENT)
			return PM_RET_SUCCESS;

	for (i = 0; i < 3; i++) {
		parents[i] = clk_parents[index + i];
		if (clk_parents[index + i] == CLK_NA_PARENT)
			break;
	}

	return PM_RET_SUCCESS;
}

/**
 * pm_api_clock_get_attributes() - PM call to request a clock's attributes
 * @clock_id	Clock ID
 * @attr	Clock attributes
 *
 * This function is used by master to get clock's attributes
 * (e.g. valid, clock type, etc).
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clock_get_attributes(unsigned int clock_id,
					       uint32_t *attr)
{
	if (clock_id >= CLK_MAX)
		return PM_RET_ERROR_ARGS;

	/* Clock valid bit */
	*attr = pm_clock_valid(clock_id);

	/* Clock type (Output/External) */
	*attr |= (pm_clock_type(clock_id) << CLK_TYPE_SHIFT);

	return PM_RET_SUCCESS;
}

/**
 * pll_get_lockbit() -  Returns lockbit index for pll id
 * @pll_id: Id of the pll
 *
 * This function return the PLL_LOCKED bit index in
 * pll status register accosiated with given pll id.
 *
 * Return: Returns bit index
 */
static int pll_get_lockbit(unsigned int pll_id)
{
	switch (pll_id) {
	case CLK_APLL_INT:
	case CLK_IOPLL_INT:
		return 0;
	case CLK_DPLL_INT:
	case CLK_RPLL_INT:
		return 1;
	case CLK_VPLL_INT:
		return 2;
	default:
		return -1;
	}
}

/**
 * pm_api_pll_bypass_and_reset() - Bypass and reset PLL
 * @clock_id: Id of the PLL
 *
 * This function is to bypass and reset PLL.
 */
static inline enum pm_ret_status
pm_api_pll_bypass_and_reset(unsigned int clock_id, unsigned int flag)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	unsigned int reg, val;
	int lockbit;

	reg = clocks[clock_id].control_reg;

	if (flag & CLK_PLL_RESET_ASSERT) {
		ret = pm_mmio_write(reg, PLLCTRL_BP_MASK, PLLCTRL_BP_MASK);
		if (ret != PM_RET_SUCCESS)
			return ret;
		ret = pm_mmio_write(reg, PLLCTRL_RESET_MASK,
				    PLLCTRL_RESET_MASK);
		if (ret != PM_RET_SUCCESS)
			return ret;
	}
	if (flag & CLK_PLL_RESET_RELEASE) {
		ret = pm_mmio_write(reg, PLLCTRL_RESET_MASK,
				    ~PLLCTRL_RESET_MASK);
		if (ret != PM_RET_SUCCESS)
			return ret;

		lockbit = pll_get_lockbit(clock_id);
		do {
			ret = pm_mmio_read(clocks[clock_id].status_reg, &val);
			if (ret != PM_RET_SUCCESS)
				return ret;
		} while ((lockbit >= 0) && !(val & (1 << lockbit)));

		ret = pm_mmio_write(reg, PLLCTRL_BP_MASK,
			      ~(unsigned int)PLLCTRL_BP_MASK);
	}
	return ret;
}

/**
 * pm_api_clk_enable_disable() - Enable/Disable the clock for given id
 * @clock_id: Id of the clock to be enabled
 * @enable: Enable(1)/Disable(0)
 *
 * This function is to enable/disable the clock which is not PLL.
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_api_clk_enable_disable(unsigned int clock_id,
						    unsigned int enable)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	struct pm_clock_node *nodes = *clocks[clock_id].nodes;
	uint8_t num_nodes = clocks[clock_id].num_nodes;
	unsigned int reg, val;
	uint8_t i = 0;
	uint8_t offset = NA_SHIFT, width = NA_WIDTH;

	if (clock_id == CLK_GEM0_TX || clock_id == CLK_GEM1_TX ||
	    clock_id == CLK_GEM2_TX || clock_id == CLK_GEM3_TX)
		reg = clocks[clock_id].status_reg;
	else
		reg = clocks[clock_id].control_reg;

	for (i = 0; i < num_nodes; i++) {
		if (nodes->type == TYPE_GATE) {
			offset = nodes->offset;
			width = nodes->width;
			break;
		}
		nodes++;
	}
	if (width == NA_WIDTH)
		return PM_RET_ERROR_NOTSUPPORTED;

	ret = pm_mmio_read(reg, &val);
	if (ret != PM_RET_SUCCESS)
		return ret;
	if ((val & BIT_MASK(offset, width)) == enable)
		return PM_RET_SUCCESS;

	if (enable == 0)
		val &= ~(BIT_MASK(offset, width));
	else
		val |= BIT_MASK(offset, width);

	ret = pm_mmio_write(reg, BIT_MASK(offset, width), val);

	return ret;
}

/**
 * pm_api_clock_enable() - Enable the clock for given id
 * @clock_id: Id of the clock to be enabled
 *
 * This function is used by master to enable the clock
 * including peripherals and PLL clocks.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_enable(unsigned int clock_id)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	/*
	 * PLL type clock should not enable explicitly.
	 * It is done by FSBL on boot-up and by PMUFW whenever required.
	 */
	if (!ISPLL(clock_id))
		ret = pm_api_clk_enable_disable(clock_id, 1);

	return ret;
}

/**
 * pm_api_clock_disable - Disable the clock for given id
 * @clock_id	Id of the clock to be disable
 *
 * This function is used by master to disable the clock
 * including peripherals and PLL clocks.
 *
 * Return: Returns status, either success or error+reason.
 */

enum pm_ret_status pm_api_clock_disable(unsigned int clock_id)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	/*
	 * PLL type clock should not be disabled explicitly.
	 * It is done by PMUFW if required.
	 */
	if (!ISPLL(clock_id))
		ret = pm_api_clk_enable_disable(clock_id, 0);

	return ret;
}

/**
 * pm_api_get_pll_state() - Get state of PLL
 * @clock_id	Id of the PLL
 * @state	State of PLL(1: Enable, 0: Reset)
 *
 * This function is to check state of PLL.
 */
static inline enum pm_ret_status pm_api_get_pll_state(unsigned int clock_id,
					unsigned int *state)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	unsigned int reg, val;

	reg = clocks[clock_id].control_reg;

	ret = pm_mmio_read(reg, &val);

	/* state:
	 * 1 - PLL is enabled
	 * 0 - PLL is in reset state
	 */
	*state = !(val & PLLCTRL_RESET_MASK);
	return ret;
}

/**
 * pm_api_get_clk_state() - Get the state of clock for given id
 * @clock_id: Id of the clock to be enabled
 * @state: Enable(1)/Disable(0)
 *
 * This function is to get state of the clock which is not PLL.
 *
 * Return: Returns status, either success or error+reason.
 */
static enum pm_ret_status pm_api_get_clk_state(unsigned int clock_id,
					       unsigned int *state)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	struct pm_clock_node *nodes = *clocks[clock_id].nodes;
	uint8_t num_nodes = clocks[clock_id].num_nodes;
	unsigned int reg, val;
	uint8_t i = 0;
	uint8_t offset = NA_SHIFT, width = NA_WIDTH;

	reg = clocks[clock_id].control_reg;

	for (i = 0; i < num_nodes; i++) {
		if (nodes->type == TYPE_GATE) {
			offset = nodes->offset;
			width = nodes->width;
		}
		nodes++;
	}
	if (width == NA_WIDTH)
		return PM_RET_ERROR_NOTSUPPORTED;

	ret = pm_mmio_read(reg, &val);
	*state = (val & BIT_MASK(offset, width)) >> offset;

	return ret;
}

/**
 * pm_api_clock_getstate - Get the clock state for given id
 * @clock_id	Id of the clock to be queried
 * @state	1/0 (Enabled/Disabled)
 *
 * This function is used by master to get the state of clock
 * including peripherals and PLL clocks.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_getstate(unsigned int clock_id,
					 unsigned int *state)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	if (ISPLL(clock_id))
		ret = pm_api_get_pll_state(clock_id, state);
	else
		ret = pm_api_get_clk_state(clock_id, state);

	return ret;
}

static enum pm_ret_status pm_api_clk_set_divider(unsigned int clock_id,
						 uint32_t divider)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	struct pm_clock_node *nodes;
	uint8_t num_nodes;
	uint16_t div1, div2;
	unsigned int reg, mask = 0, val = 0, i;
	uint8_t div1_width = NA_WIDTH, div1_offset = NA_SHIFT;
	uint8_t div2_width = NA_WIDTH, div2_offset = NA_SHIFT;

	div1 = (uint16_t)(divider & 0xFFFFU);
	div2 = (uint16_t)((divider >> 16) & 0xFFFFU);

	reg = clocks[clock_id].control_reg;

	nodes = *clocks[clock_id].nodes;
	num_nodes = clocks[clock_id].num_nodes;
	for (i = 0; i < num_nodes; i++) {
		if (nodes->type == TYPE_DIV1) {
			div1_offset = nodes->offset;
			div1_width = nodes->width;
		}
		if (nodes->type == TYPE_DIV2) {
			div2_offset = nodes->offset;
			div2_width = nodes->width;
		}
		nodes++;
	}

	if (div1 != (uint16_t)-1) {
		if (div1_width == NA_WIDTH)
			return PM_RET_ERROR_NOTSUPPORTED;
		val |= div1 << div1_offset;
		mask |= BIT_MASK(div1_offset, div1_width);
	}
	if (div2 != (uint16_t)-1) {
		if (div2_width == NA_WIDTH)
			return PM_RET_ERROR_NOTSUPPORTED;
		val |= div2 << div2_offset;
		mask |= BIT_MASK(div2_offset, div2_width);
	}
	ret = pm_mmio_write(reg, mask, val);

	return ret;
}

static enum pm_ret_status pm_api_pll_set_divider(unsigned int clock_id,
					  unsigned int divider)
{
	unsigned int reg = clocks[clock_id].control_reg;
	enum pm_ret_status ret;

	pm_api_pll_bypass_and_reset(clock_id, CLK_PLL_RESET_ASSERT);
	ret = pm_mmio_write(reg, PLL_FBDIV_MASK, divider << PLL_FBDIV_SHIFT);
	pm_api_pll_bypass_and_reset(clock_id, CLK_PLL_RESET_RELEASE);

	return ret;
}

/**
 * pm_api_clock_setdivider - Set the clock divider for given id
 * @clock_id	Id of the clock
 * @divider	Divider value
 *
 * This function is used by master to set divider for any clock
 * to achieve desired rate.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_setdivider(unsigned int clock_id,
					   unsigned int divider)
{
	enum pm_ret_status ret;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	if (ISPLL(clock_id))
		ret = pm_api_pll_set_divider(clock_id, divider);
	else
		ret = pm_api_clk_set_divider(clock_id, divider);

	return ret;
}

static enum pm_ret_status pm_api_clk_get_divider(unsigned int clock_id,
						 uint32_t *divider)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	struct pm_clock_node *nodes;
	uint8_t num_nodes;
	unsigned int reg, val, i, div1 = 0, div2 = 0;
	uint8_t div1_width = NA_WIDTH, div1_offset = NA_SHIFT;
	uint8_t div2_width = NA_WIDTH, div2_offset = NA_SHIFT;

	reg = clocks[clock_id].control_reg;

	nodes = *clocks[clock_id].nodes;
	num_nodes = clocks[clock_id].num_nodes;
	for (i = 0; i < num_nodes; i++) {
		if (nodes->type == TYPE_DIV1) {
			div1_offset = nodes->offset;
			div1_width = nodes->width;
		}
		if (nodes->type == TYPE_DIV2) {
			div2_offset = nodes->offset;
			div2_width = nodes->width;
		}
		nodes++;
	}

	ret = pm_mmio_read(reg, &val);

	if (div1_width == NA_WIDTH)
		return PM_RET_ERROR_ARGS;

	div1 = (val & BIT_MASK(div1_offset, div1_width)) >> div1_offset;

	if (div2_width != NA_WIDTH)
		div2 = (val & BIT_MASK(div2_offset, div2_width)) >> div2_offset;

	*divider = div1 | (div2 << 16);

	return ret;
}

static enum pm_ret_status pm_api_pll_get_divider(unsigned int clock_id,
					  unsigned int *divider)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	unsigned int reg, val;

	reg = clocks[clock_id].control_reg;

	ret = pm_mmio_read(reg, &val);
	*divider = (val & PLL_FBDIV_MASK) >> PLL_FBDIV_SHIFT;

	return ret;
}

/**
 * pm_api_clock_getdivider - Get the clock divider for given id
 * @clock_id	Id of the clock
 * @divider	Divider value
 *
 * This function is used by master to get divider values
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_getdivider(unsigned int clock_id,
					   unsigned int *divider)
{
	enum pm_ret_status ret;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	if (ISPLL(clock_id))
		ret = pm_api_pll_get_divider(clock_id, divider);
	else
		ret = pm_api_clk_get_divider(clock_id, divider);

	return ret;
}

/**
 * pm_api_clock_setrate - Set the clock rate for given id
 * @clock_id	Id of the clock
 * @rate	Rate value in hz
 *
 * This function is used by master to set rate for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_setrate(unsigned int clock_id,
					uint64_t rate)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_getrate - Get the clock rate for given id
 * @clock_id	Id of the clock
 * @rate	rate value in hz
 *
 * This function is used by master to get rate
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_getrate(unsigned int clock_id,
					uint64_t *rate)
{
	return PM_RET_ERROR_NOTSUPPORTED;
}

/**
 * pm_api_clock_setparent - Set the clock parent for given id
 * @clock_id	Id of the clock
 * @parent_idx	parent index
 *
 * This function is used by master to set parent for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_setparent(unsigned int clock_id,
					  unsigned int parent_idx)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	struct pm_clock_node *nodes;
	uint8_t num_nodes;
	unsigned int reg, val;
	int32_t *clk_parents;
	unsigned int i = 0;
	uint8_t  offset = NA_SHIFT, width = NA_WIDTH;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	clk_parents = *clocks[clock_id].parents;

	for (i = 0; i <= parent_idx; i++)
		if (clk_parents[i] == CLK_NA_PARENT)
			return PM_RET_ERROR_ARGS;

	nodes = *clocks[clock_id].nodes;
	num_nodes = clocks[clock_id].num_nodes;
	for (i = 0; i < num_nodes; i++) {
		if (nodes->type == TYPE_MUX) {
			offset = nodes->offset;
			width = nodes->width;
		}
		nodes++;
	}
	if (width == NA_WIDTH)
		return PM_RET_ERROR_NOTSUPPORTED;

	reg = clocks[clock_id].control_reg;
	val = parent_idx << offset;
	ret = pm_mmio_write(reg, BIT_MASK(offset, width), val);

	return ret;
}

/**
 * pm_api_clock_getparent - Get the clock parent for given id
 * @clock_id	Id of the clock
 * @parent_idx	parent index
 *
 * This function is used by master to get parent index
 * for any clock.
 *
 * Return: Returns status, either success or error+reason.
 */
enum pm_ret_status pm_api_clock_getparent(unsigned int clock_id,
					  unsigned int *parent_idx)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	struct pm_clock_node *nodes;
	uint8_t num_nodes;
	unsigned int reg, val;
	uint8_t i = 0, offset = NA_SHIFT, width = NA_WIDTH;

	if (!pm_clock_valid(clock_id))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(clock_id) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	nodes = *clocks[clock_id].nodes;
	num_nodes = clocks[clock_id].num_nodes;

	for (i = 0; i < num_nodes; i++) {
		if (nodes->type == TYPE_MUX) {
			offset = nodes->offset;
			width = nodes->width;
		}
		nodes++;
	}
	if (width == NA_WIDTH)
		return PM_RET_ERROR_NOTSUPPORTED;

	reg = clocks[clock_id].control_reg;
	ret = pm_mmio_read(reg, &val);
	val >>= offset;
	val &= ((1U << width) - 1);

	*parent_idx = val;

	return ret;
}

/**
 * pm_api_clk_set_pll_mode() -  Set PLL mode
 * @pll     PLL id
 * @mode    Mode fraction/integar
 *
 * This function sets PLL mode.
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clk_set_pll_mode(unsigned int pll,
					   unsigned int mode)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	unsigned int reg;

	if (!pm_clock_valid(pll))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(pll) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	if (!ISPLL(pll))
		return PM_RET_ERROR_NOTSUPPORTED;

	if (mode != PLL_FRAC_MODE && mode != PLL_INT_MODE)
		return PM_RET_ERROR_ARGS;

	reg = clocks[pll].control_reg + PLL_FRAC_OFFSET;

	ret = pm_mmio_write(reg, PLL_FRAC_MODE_MASK,
			    mode << PLL_FRAC_MODE_SHIFT);

	return ret;
}

/**
 * pm_ioctl_get_pll_mode() -  Get PLL mode
 * @pll     PLL id
 * @mode    Mode fraction/integar
 *
 * This function returns current PLL mode.
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clk_get_pll_mode(unsigned int pll,
					   unsigned int *mode)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	unsigned int val, reg;

	if (!pm_clock_valid(pll))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(pll) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	if (!ISPLL(pll))
		return PM_RET_ERROR_NOTSUPPORTED;

	reg = clocks[pll].control_reg + PLL_FRAC_OFFSET;

	ret = pm_mmio_read(reg, &val);
	val = val & PLL_FRAC_MODE_MASK;
	if (val == 0)
		*mode = PLL_INT_MODE;
	else
		*mode = PLL_FRAC_MODE;

	return ret;
}

/**
 * pm_api_clk_set_pll_frac_data() -  Set PLL fraction data
 * @pll     PLL id
 * @data    fraction data
 *
 * This function sets fraction data. It is valid for fraction
 * mode only.
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clk_set_pll_frac_data(unsigned int pll,
						unsigned int data)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	unsigned int val, reg, mode = 0;

	if (!pm_clock_valid(pll))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(pll) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	if (!ISPLL(pll))
		return PM_RET_ERROR_NOTSUPPORTED;

	ret = pm_api_clk_get_pll_mode(pll, &mode);
	if (ret != PM_RET_SUCCESS)
		return ret;
	if (mode == PLL_FRAC_MODE) {
		reg = clocks[pll].control_reg + PLL_FRAC_OFFSET;
		val = data << PLL_FRAC_DATA_SHIFT;
		ret = pm_mmio_write(reg, PLL_FRAC_DATA_MASK, val);
	} else {
		return PM_RET_ERROR_ARGS;
	}

	return ret;
}

/**
 * pm_api_clk_get_pll_frac_data() - Get PLL fraction data
 * @pll     PLL id
 * @data    fraction data
 *
 * This function returns fraction data value.
 *
 * @return      Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_clk_get_pll_frac_data(unsigned int pll,
						unsigned int *data)
{
	enum pm_ret_status ret = PM_RET_SUCCESS;
	unsigned int val, reg;

	if (!pm_clock_valid(pll))
		return PM_RET_ERROR_ARGS;

	if (pm_clock_type(pll) != CLK_TYPE_OUTPUT)
		return PM_RET_ERROR_NOTSUPPORTED;

	if (!ISPLL(pll))
		return PM_RET_ERROR_NOTSUPPORTED;

	reg = clocks[pll].control_reg + PLL_FRAC_OFFSET;

	ret = pm_mmio_read(reg, &val);
	*data = (val & PLL_FRAC_DATA_MASK);

	return ret;
}
