/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/clk.h>
#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <dt-bindings/clock/s32cc-scmi-clock.h>
#include <lib/utils_def.h>

#include <s32cc-clk-ids.h>

/*
 * Number of SCMI clock IDs exposed to the agent. IDs outside the mapping
 * table below are reported to the agent but resolve to "not supported".
 */
#define S32G_SCMI_CLK_COUNT	128U

struct s32g_scmi_clk {
	unsigned long arch_id;
	const char *name;
};

/*
 * Map an SCMI clock ID onto an S32CC architectural clock. An entry is only
 * valid when it carries a name; anything else is an unimplemented clock.
 */
#define SCMI_CLK(_scmi_id, _arch_id, _name)	\
	[_scmi_id] = { .arch_id = (_arch_id), .name = (_name), }

/* Runtime enable state as requested by the agent, indexed by SCMI clock ID. */
static bool s32g_scmi_clk_enabled[S32G_SCMI_CLK_COUNT];

static const struct s32g_scmi_clk *s32g_scmi_clk_get(unsigned int scmi_id)
{
	static const struct s32g_scmi_clk s32g_scmi_clocks[S32G_SCMI_CLK_COUNT] = {
		SCMI_CLK(S32CC_SCMI_CLK_A53, S32CC_CLK_A53_CORE, "a53"),
		SCMI_CLK(S32CC_SCMI_CLK_LINFLEX_XBAR, S32CC_CLK_LINFLEX,
			 "linflex_xbar"),
		SCMI_CLK(S32CC_SCMI_CLK_LINFLEX_LIN, S32CC_CLK_LINFLEX_BAUD,
			 "linflex_lin"),
		SCMI_CLK(S32CC_SCMI_CLK_USDHC_MODULE, S32CC_CLK_USDHC,
			 "usdhc_module"),
		SCMI_CLK(S32CC_SCMI_CLK_USDHC_CORE, S32CC_CLK_USDHC, "usdhc_core"),
		SCMI_CLK(S32CC_SCMI_CLK_DDR_AXI, S32CC_CLK_DDR, "ddr_axi"),
		SCMI_CLK(S32CC_SCMI_CLK_STM_MODULE, S32CC_CLK_XBAR_DIV3,
			 "stm_module"),
		SCMI_CLK(S32CC_SCMI_CLK_PIT_MODULE, S32CC_CLK_XBAR_DIV3,
			 "pit_module"),
		SCMI_CLK(S32CC_SCMI_CLK_PIT_REG, S32CC_CLK_XBAR_DIV3, "pit_reg"),
	};

	if (scmi_id >= S32G_SCMI_CLK_COUNT) {
		return NULL;
	}

	if (s32g_scmi_clocks[scmi_id].name == NULL) {
		return NULL;
	}

	return &s32g_scmi_clocks[scmi_id];
}

size_t plat_scmi_clock_count(unsigned int agent_id)
{
	return S32G_SCMI_CLK_COUNT;
}

const char *plat_scmi_clock_get_name(unsigned int agent_id,
				     unsigned int scmi_id)
{
	const struct s32g_scmi_clk *clk = s32g_scmi_clk_get(scmi_id);

	if (clk == NULL) {
		return NULL;
	}

	return clk->name;
}

unsigned long plat_scmi_clock_get_rate(unsigned int agent_id,
				       unsigned int scmi_id)
{
	const struct s32g_scmi_clk *clk = s32g_scmi_clk_get(scmi_id);

	if (clk == NULL) {
		return 0UL;
	}

	return clk_get_rate(clk->arch_id);
}

int32_t plat_scmi_clock_get_state(unsigned int agent_id,
				  unsigned int scmi_id)
{
	if (s32g_scmi_clk_get(scmi_id) == NULL) {
		return 0;
	}

	return s32g_scmi_clk_enabled[scmi_id] ? 1 : 0;
}

int32_t plat_scmi_clock_set_state(unsigned int agent_id,
				  unsigned int scmi_id, bool enable_not_disable)
{
	const struct s32g_scmi_clk *clk = s32g_scmi_clk_get(scmi_id);

	if (clk == NULL) {
		return SCMI_NOT_FOUND;
	}

	if (enable_not_disable) {
		if (clk_enable(clk->arch_id) != 0) {
			return SCMI_HARDWARE_ERROR;
		}
	} else {
		clk_disable(clk->arch_id);
	}

	s32g_scmi_clk_enabled[scmi_id] = enable_not_disable;

	return SCMI_SUCCESS;
}
