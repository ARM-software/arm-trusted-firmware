// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#include <assert.h>
#include <errno.h>

#include <drivers/delay_timer.h>
#include <drivers/scmi.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#include <plat_private.h>
#include <platform_def.h>
#include <rockchip_sip_svc.h>
#include <rv1126b_clk.h>
#include <scmi_clock.h>
#include <soc.h>

enum pll_type_sel {
	PLL_SEL_AUTO, /* all plls (normal pll or pvtpll) */
	PLL_SEL_PVT,
	PLL_SEL_NOR,
	PLL_SEL_AUTO_NOR /* all normal plls (apll/gpll/npll) */
};

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define GPLL_RATE			1188000000
#define CPLL_RATE			1000000000
#define AUPLL_RATE			786431952

#define MAX_RATE_TABLE			16

#define CLKDIV_6BITS_SHF(div, shift)	BITS_WITH_WMASK(div, 0x3fU, shift)
#define CLKDIV_5BITS_SHF(div, shift)	BITS_WITH_WMASK(div, 0x1fU, shift)
#define CLKDIV_4BITS_SHF(div, shift)	BITS_WITH_WMASK(div, 0xfU, shift)
#define CLKDIV_3BITS_SHF(div, shift)	BITS_WITH_WMASK(div, 0x7U, shift)
#define CLKDIV_2BITS_SHF(div, shift)	BITS_WITH_WMASK(div, 0x3U, shift)
#define CLKDIV_1BITS_SHF(div, shift)	BITS_WITH_WMASK(div, 0x1U, shift)

#define CPU_PLL_PATH_SLOWMODE		BITS_WITH_WMASK(0U, 0x3U, 0)
#define CPU_PLL_PATH_NORMAL		BITS_WITH_WMASK(1U, 0x3U, 0)
#define CPU_PLL_PATH_DEEP_SLOW		BITS_WITH_WMASK(2U, 0x3U, 0)

#define CRU_PLL_POWER_DOWN		BIT_WITH_WMSK(13)
#define CRU_PLL_POWER_UP		WMSK_BIT(13)

#define PRATE(x) static const unsigned long const x[]
#define PINFO(x) static const uint32_t const x[]

PRATE(p_24m)		= { OSC_HZ };
PRATE(p_100m)		= { 100 * MHz};
PRATE(p_200m)		= { 198 * MHz};
PRATE(p_300m)		= { 297 * MHz};

PINFO(clk_user_otpc_s_info)	= { 0, 0, 0, 0x20200308, 12, 3, 0x20200800, 12 };
PINFO(clk_sbpi_otpc_s_info)	= { 0, 0, 0, 0, 0, 0, 0x20200800, 11 };
PINFO(pclk_otpc_s_info)		= { 0, 0, 0, 0, 0, 0, 0x20200800, 10 };
PINFO(pclk_key_r_s_info)	= { 0, 0, 0, 0, 0, 0, 0x20200800, 13 };
PINFO(hclk_kl_rkce_s_info)	= { 0, 0, 0, 0, 0, 0, 0x20200800, 9 };
PINFO(hclk_rkce_s_info)		= { 0, 0, 0, 0, 0, 0, 0x20200800, 8 };
PINFO(pclk_wdt_s_info)		= { 0, 0, 0, 0, 0, 0, 0x20200800, 6 };
PINFO(tclk_wdt_s_info)		= { 0, 0, 0, 0, 0, 0, 0x20200800, 7 };
PINFO(clk_stimer0_info)		= { 0, 0, 0, 0, 0, 0, 0x20200800, 4 };
PINFO(clk_stimer1_info)		= { 0, 0, 0, 0, 0, 0, 0x20200800, 5 };
PINFO(pclk_stimer_info)		= { 0, 0, 0, 0, 0, 0, 0x20200800, 3 };
PINFO(hclk_rkrng_s_info)	= { 0, 0, 0, 0, 0, 0, 0x20200808, 14 };
PINFO(clk_pka_rkce_s_info)	= { 0, 0, 0, 0, 0, 0, 0x20200808, 13 };
PINFO(aclk_rkce_s_info)		= { 0, 0, 0, 0, 0, 0, 0x20200808, 12 };

#define RV1126B_SCMI_CLOCK(_id, _name, _data, _table, _cnt, _is_s)	\
rk_scmi_clock_t _name = {						\
	.id	= _id,							\
	.name = #_name,							\
	.clk_ops = _data,						\
	.rate_table = _table,						\
	.rate_cnt = _cnt,						\
	.is_security = _is_s,						\
}

#define RV1126B_SCMI_CLOCK_COM(_id, _name,  _parent_table, _info, _data,	\
			     _table, is_d, _is_s)			\
rk_scmi_clock_t _name = {						\
	.id	= _id,							\
	.name = #_name,							\
	.parent_table = _parent_table,					\
	.info = _info,							\
	.clk_ops = _data,						\
	.rate_table = _table,						\
	.rate_cnt = ARRAY_SIZE(_table),					\
	.is_dynamic_prate = is_d,					\
	.is_security = _is_s,						\
}

static unsigned long rv1126b_common_rates[] = {
	400000, 24000000, 50000000, 100000000, 198000000, 297000000, 396000000,
};

static const struct rk_clk_ops clk_scmi_ops_com = {
	.get_rate = clk_scmi_common_get_rate,
	.set_rate = clk_scmi_common_set_rate,
	.set_status = clk_scmi_common_set_status,
};

static const struct rk_clk_ops clk_scmi_ops_gate = {
	.get_rate = clk_scmi_common_get_rate,
	.set_status = clk_scmi_common_set_status,
};

RV1126B_SCMI_CLOCK_COM(CLK_USER_OTPC_S, clk_user_otpc_s, p_24m, clk_user_otpc_s_info,
				      &clk_scmi_ops_com, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(CLK_SBPI_OTPC_S, clk_sbpi_otpc_s, p_24m, clk_sbpi_otpc_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(PCLK_OTPC_S, pclk_otpc_s, p_100m, pclk_otpc_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(PCLK_KEY_READER_S, pclk_key_r_s, p_100m, pclk_key_r_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(HCLK_KL_RKCE_S, hclk_kl_rkce_s, p_200m, hclk_kl_rkce_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(HCLK_RKCE_S, hclk_rkce_s, p_200m, hclk_rkce_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(PCLK_WDT_S, pclk_wdt_s, p_100m, pclk_wdt_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, true, true);
RV1126B_SCMI_CLOCK_COM(TCLK_WDT_S, tclk_wdt_s, p_24m, tclk_wdt_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(CLK_STIMER0, clk_stimer0, p_100m, clk_stimer0_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(CLK_STIMER1, clk_stimer1, p_100m, clk_stimer1_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(PLK_STIMER, pclk_stimer, p_100m, pclk_stimer_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(HCLK_RKRNG_S, hclk_rkrng_s, p_200m, hclk_rkrng_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(CLK_PKA_RKCE_S, clk_pka_rkce_s, p_200m, clk_pka_rkce_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);
RV1126B_SCMI_CLOCK_COM(ACLK_RKCE_S, aclk_rkce_s, p_300m, aclk_rkce_s_info,
				      &clk_scmi_ops_gate, rv1126b_common_rates, false, true);

static rk_scmi_clock_t *clock_table[] = {
	[CLK_USER_OTPC_S]	= &clk_user_otpc_s,
	[CLK_SBPI_OTPC_S]	= &clk_sbpi_otpc_s,
	[PCLK_OTPC_S]		= &pclk_otpc_s,
	[PCLK_KEY_READER_S]	= &pclk_key_r_s,
	[HCLK_KL_RKCE_S]	= &hclk_kl_rkce_s,
	[HCLK_RKCE_S]		= &hclk_rkce_s,
	[PCLK_WDT_S]		= &pclk_wdt_s,
	[TCLK_WDT_S]		= &tclk_wdt_s,
	[CLK_STIMER0]		= &clk_stimer0,
	[CLK_STIMER1]		= &clk_stimer1,
	[PLK_STIMER]		= &pclk_stimer,
	[HCLK_RKRNG_S]		= &hclk_rkrng_s,
	[CLK_PKA_RKCE_S]	= &clk_pka_rkce_s,
	[ACLK_RKCE_S]		= &aclk_rkce_s,
};

size_t rockchip_scmi_clock_count(unsigned int agent_id __unused)
{
	return CLK_NR_CLKS;
}

rk_scmi_clock_t *rockchip_scmi_get_clock(uint32_t agent_id __unused,
					 uint32_t clock_id)
{
	rk_scmi_clock_t *table = NULL;

	if (clock_id < ARRAY_SIZE(clock_table)) {
		table = clock_table[clock_id];
		if (table == NULL)
			return NULL;
	}

	if ((table != NULL) && (table->is_security == 0))
		return table;
	else
		return NULL;

	return NULL;
}

void rockchip_clock_init(void)
{
	/* disable aclk_rkce_s_en and clk_pka_rkce_s_en */
	mmio_write_32(0x20200808, 0x30003000);
}
