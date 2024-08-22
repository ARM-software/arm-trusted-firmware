/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <drivers/delay_timer.h>
#include <drivers/scmi.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include <plat_private.h>
#include "rk3588_clk.h"
#include <rockchip_sip_svc.h>
#include <scmi_clock.h>
#include <soc.h>

enum pll_type_sel {
	PLL_SEL_AUTO, /* all plls (normal pll or pvtpll) */
	PLL_SEL_PVT,
	PLL_SEL_NOR,
	PLL_SEL_AUTO_NOR /* all normal plls (apll/gpll/npll) */
};

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define RK3588_CPUL_PVTPLL_CON0_L	0x40
#define RK3588_CPUL_PVTPLL_CON0_H	0x44
#define RK3588_CPUL_PVTPLL_CON1		0x48
#define RK3588_CPUL_PVTPLL_CON2		0x4c
#define RK3588_CPUB_PVTPLL_CON0_L	0x00
#define RK3588_CPUB_PVTPLL_CON0_H	0x04
#define RK3588_CPUB_PVTPLL_CON1		0x08
#define RK3588_CPUB_PVTPLL_CON2		0x0c
#define RK3588_DSU_PVTPLL_CON0_L	0x60
#define RK3588_DSU_PVTPLL_CON0_H	0x64
#define RK3588_DSU_PVTPLL_CON1		0x70
#define RK3588_DSU_PVTPLL_CON2		0x74
#define RK3588_GPU_PVTPLL_CON0_L	0x00
#define RK3588_GPU_PVTPLL_CON0_H	0x04
#define RK3588_GPU_PVTPLL_CON1		0x08
#define RK3588_GPU_PVTPLL_CON2		0x0c
#define RK3588_NPU_PVTPLL_CON0_L	0x0c
#define RK3588_NPU_PVTPLL_CON0_H	0x10
#define RK3588_NPU_PVTPLL_CON1		0x14
#define RK3588_NPU_PVTPLL_CON2		0x18
#define RK3588_PVTPLL_MAX_LENGTH	0x3f

#define GPLL_RATE			1188000000
#define CPLL_RATE			1500000000
#define SPLL_RATE			702000000
#define AUPLL_RATE			786431952
#define NPLL_RATE			850000000

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

/* core_i: from gpll or apll */
#define CLK_CORE_I_SEL_APLL		WMSK_BIT(6)
#define CLK_CORE_I_SEL_GPLL		BIT_WITH_WMSK(6)

/* clk_core:
 * from normal pll(core_i: gpll or apll) path or direct pass from apll
 */

/* cpul clk path */
#define CPUL_CLK_PATH_NOR_XIN		BITS_WITH_WMASK(0U, 0x3U, 14)
#define CPUL_CLK_PATH_NOR_GPLL		BITS_WITH_WMASK(1U, 0x3U, 14)
#define CPUL_CLK_PATH_NOR_LPLL		BITS_WITH_WMASK(2U, 0x3U, 14)

#define CPUL_CLK_PATH_LPLL		(BITS_WITH_WMASK(0U, 0x3U, 5) | \
					BITS_WITH_WMASK(0U, 0x3U, 12))
#define CPUL_CLK_PATH_DIR_LPLL		(BITS_WITH_WMASK(0x1, 0x3U, 5) | \
					BITS_WITH_WMASK(1U, 0x3U, 12))
#define CPUL_CLK_PATH_PVTPLL		(BITS_WITH_WMASK(0x2, 0x3U, 5) | \
					BITS_WITH_WMASK(2U, 0x3U, 12))

#define CPUL_PVTPLL_PATH_DEEP_SLOW	BITS_WITH_WMASK(0U, 0x1U, 14)
#define CPUL_PVTPLL_PATH_PVTPLL		BITS_WITH_WMASK(1U, 0x1U, 14)

/* cpub01 clk path */
#define CPUB01_CLK_PATH_NOR_XIN		BITS_WITH_WMASK(0U, 0x3U, 6)
#define CPUB01_CLK_PATH_NOR_GPLL	BITS_WITH_WMASK(1U, 0x3U, 6)
#define CPUB01_CLK_PATH_NOR_B0PLL	BITS_WITH_WMASK(2U, 0x3U, 6)

#define CPUB01_CLK_PATH_B0PLL		BITS_WITH_WMASK(0U, 0x3U, 13)
#define CPUB01_CLK_PATH_DIR_B0PLL	BITS_WITH_WMASK(1U, 0x3U, 13)
#define CPUB01_CLK_PATH_B0_PVTPLL	BITS_WITH_WMASK(2U, 0x3U, 13)

#define CPUB01_CLK_PATH_B1PLL		BITS_WITH_WMASK(0U, 0x3U, 5)
#define CPUB01_CLK_PATH_DIR_B1PLL	BITS_WITH_WMASK(1U, 0x3U, 5)
#define CPUB01_CLK_PATH_B1_PVTPLL	BITS_WITH_WMASK(2U, 0x3U, 5)

#define CPUB01_PVTPLL_PATH_DEEP_SLOW	BITS_WITH_WMASK(0U, 0x1U, 2)
#define CPUB01_PVTPLL_PATH_PVTPLL	BITS_WITH_WMASK(1U, 0x1U, 2)

#define CPUB_PCLK_PATH_100M		BITS_WITH_WMASK(0U, 0x3U, 0)
#define CPUB_PCLK_PATH_50M		BITS_WITH_WMASK(1U, 0x3U, 0)
#define CPUB_PCLK_PATH_24M		BITS_WITH_WMASK(2U, 0x3U, 0)

/* dsu clk path */
#define SCLK_DSU_PATH_NOR_B0PLL		BITS_WITH_WMASK(0U, 0x3U, 12)
#define SCLK_DSU_PATH_NOR_B1PLL		BITS_WITH_WMASK(1U, 0x3U, 12)
#define SCLK_DSU_PATH_NOR_LPLL		BITS_WITH_WMASK(2U, 0x3U, 12)
#define SCLK_DSU_PATH_NOR_GPLL		BITS_WITH_WMASK(3U, 0x3U, 12)

#define DSU_PVTPLL_PATH_DEEP_SLOW	BITS_WITH_WMASK(0U, 0x1U, 15)
#define DSU_PVTPLL_PATH_PVTPLL		BITS_WITH_WMASK(1U, 0x1U, 15)

#define SCLK_DSU_PATH_NOR_PLL		WMSK_BIT(0)
#define SCLK_DSU_PATH_PVTPLL		BIT_WITH_WMSK(0)

/* npu clk path */
#define NPU_CLK_PATH_NOR_GPLL		BITS_WITH_WMASK(0U, 0x7U, 7)
#define NPU_CLK_PATH_NOR_CPLL		BITS_WITH_WMASK(1U, 0x7U, 7)
#define NPU_CLK_PATH_NOR_AUPLL		BITS_WITH_WMASK(2U, 0x7U, 7)
#define NPU_CLK_PATH_NOR_NPLL		BITS_WITH_WMASK(3U, 0x7U, 7)
#define NPU_CLK_PATH_NOR_SPLL		BITS_WITH_WMASK(4U, 0x7U, 7)

#define NPU_CLK_PATH_NOR_PLL		WMSK_BIT(0)
#define NPU_CLK_PATH_PVTPLL		BIT_WITH_WMSK(0)

/* gpu clk path */
#define GPU_CLK_PATH_NOR_GPLL		BITS_WITH_WMASK(0U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_CPLL		BITS_WITH_WMASK(1U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_AUPLL		BITS_WITH_WMASK(2U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_NPLL		BITS_WITH_WMASK(3U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_SPLL		BITS_WITH_WMASK(4U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_PLL		WMSK_BIT(14)
#define GPU_CLK_PATH_PVTPLL		BIT_WITH_WMSK(14)

#define PVTPLL_NEED(type, length)	(((type) == PLL_SEL_PVT || \
					  (type) == PLL_SEL_AUTO) && \
					 (length))

struct pvtpll_table {
	unsigned int rate;
	uint32_t length;
	uint32_t ring_sel;
};

struct sys_clk_info_t {
	struct pvtpll_table *cpul_table;
	struct pvtpll_table *cpub01_table;
	struct pvtpll_table *cpub23_table;
	struct pvtpll_table *gpu_table;
	struct pvtpll_table *npu_table;
	unsigned int cpul_rate_count;
	unsigned int cpub01_rate_count;
	unsigned int cpub23_rate_count;
	unsigned int gpu_rate_count;
	unsigned int npu_rate_count;
	unsigned long cpul_rate;
	unsigned long dsu_rate;
	unsigned long cpub01_rate;
	unsigned long cpub23_rate;
	unsigned long gpu_rate;
	unsigned long npu_rate;
};

#define RK3588_SCMI_CLOCK(_id, _name, _data, _table, _cnt, _is_s)	\
{									\
	.id	= _id,							\
	.name = _name,							\
	.clk_ops = _data,						\
	.rate_table = _table,						\
	.rate_cnt = _cnt,						\
	.is_security = _is_s,						\
}

#define ROCKCHIP_PVTPLL(_rate, _sel, _len)				\
{									\
	.rate = _rate##U,						\
	.ring_sel = _sel,						\
	.length = _len,							\
}

static struct pvtpll_table rk3588_cpul_pvtpll_table[] = {
	/* rate_hz, ring_sel, length */
	ROCKCHIP_PVTPLL(1800000000, 1, 15),
	ROCKCHIP_PVTPLL(1704000000, 1, 15),
	ROCKCHIP_PVTPLL(1608000000, 1, 15),
	ROCKCHIP_PVTPLL(1416000000, 1, 15),
	ROCKCHIP_PVTPLL(1200000000, 1, 17),
	ROCKCHIP_PVTPLL(1008000000, 1, 22),
	ROCKCHIP_PVTPLL(816000000, 1, 32),
	ROCKCHIP_PVTPLL(600000000, 0, 0),
	ROCKCHIP_PVTPLL(408000000, 0, 0),
	{ /* sentinel */ },
};

static struct pvtpll_table rk3588_cpub0_pvtpll_table[] = {
	/* rate_hz, ring_sel, length */
	ROCKCHIP_PVTPLL(2400000000, 1, 11),
	ROCKCHIP_PVTPLL(2352000000, 1, 11),
	ROCKCHIP_PVTPLL(2304000000, 1, 11),
	ROCKCHIP_PVTPLL(2256000000, 1, 11),
	ROCKCHIP_PVTPLL(2208000000, 1, 11),
	ROCKCHIP_PVTPLL(2112000000, 1, 11),
	ROCKCHIP_PVTPLL(2016000000, 1, 11),
	ROCKCHIP_PVTPLL(1800000000, 1, 11),
	ROCKCHIP_PVTPLL(1608000000, 1, 11),
	ROCKCHIP_PVTPLL(1416000000, 1, 13),
	ROCKCHIP_PVTPLL(1200000000, 1, 17),
	ROCKCHIP_PVTPLL(1008000000, 1, 23),
	ROCKCHIP_PVTPLL(816000000, 1, 33),
	ROCKCHIP_PVTPLL(600000000, 0, 0),
	ROCKCHIP_PVTPLL(408000000, 0, 0),
	{ /* sentinel */ },
};

static struct
pvtpll_table rk3588_cpub1_pvtpll_table[ARRAY_SIZE(rk3588_cpub0_pvtpll_table)] = { 0 };

static struct pvtpll_table rk3588_gpu_pvtpll_table[] = {
	/* rate_hz, ring_sel, length */
	ROCKCHIP_PVTPLL(1000000000, 1, 12),
	ROCKCHIP_PVTPLL(900000000, 1, 12),
	ROCKCHIP_PVTPLL(800000000, 1, 12),
	ROCKCHIP_PVTPLL(700000000, 1, 13),
	ROCKCHIP_PVTPLL(600000000, 1, 17),
	ROCKCHIP_PVTPLL(500000000, 1, 25),
	ROCKCHIP_PVTPLL(400000000, 1, 38),
	ROCKCHIP_PVTPLL(300000000, 1, 55),
	ROCKCHIP_PVTPLL(200000000, 0, 0),
	{ /* sentinel */ },
};

static struct pvtpll_table rk3588_npu_pvtpll_table[] = {
	/* rate_hz, ring_sel, length */
	ROCKCHIP_PVTPLL(1000000000, 1, 12),
	ROCKCHIP_PVTPLL(900000000, 1, 12),
	ROCKCHIP_PVTPLL(800000000, 1, 12),
	ROCKCHIP_PVTPLL(700000000, 1, 13),
	ROCKCHIP_PVTPLL(600000000, 1, 17),
	ROCKCHIP_PVTPLL(500000000, 1, 25),
	ROCKCHIP_PVTPLL(400000000, 1, 38),
	ROCKCHIP_PVTPLL(300000000, 1, 55),
	ROCKCHIP_PVTPLL(200000000, 0, 0),
	{ /* sentinel */ },
};

static unsigned long rk3588_cpul_rates[] = {
	408000000, 600000000, 816000000, 1008000000,
	1200000000, 1416000000, 1608000000, 1800000063,
};

static unsigned long rk3588_cpub_rates[] = {
	408000000, 816000000, 1008000000, 1200000000,
	1416000000, 1608000000, 1800000000, 2016000000,
	2208000000, 2304000000, 2400000063
};

static unsigned long rk3588_gpu_rates[] = {
	200000000, 300000000, 400000000, 500000000,
	600000000, 700000000, 800000000, 900000000,
	1000000063
};

static unsigned long rk3588_sbus_rates[] = {
	24000000, 50000000, 100000000, 150000000, 200000000,
	250000000, 350000000, 700000000
};

static unsigned long rk3588_sdmmc_rates[] = {
	400000, 24000000, 50000000, 100000000, 150000000, 200000000,
	300000000, 400000000, 600000000, 700000000
};

static struct sys_clk_info_t sys_clk_info;
static int clk_scmi_dsu_set_rate(rk_scmi_clock_t *clock, unsigned long rate);

static struct pvtpll_table *rkclk_get_pvtpll_config(struct pvtpll_table *table,
						    unsigned int count,
						    unsigned int freq_hz)
{
	int i;

	for (i = 0; i < count; i++) {
		if (freq_hz == table[i].rate)
			return &table[i];
	}
	return NULL;
}

static int clk_cpul_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *pvtpll;
	int div;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	pvtpll = rkclk_get_pvtpll_config(sys_clk_info.cpul_table,
					 sys_clk_info.cpul_rate_count, rate);
	if (pvtpll == NULL)
		return SCMI_INVALID_PARAMETERS;

	/* set lpll */
	if (PVTPLL_NEED(type, pvtpll->length) != 0) {
		/* set clock gating interval */
		mmio_write_32(LITCOREGRF_BASE + RK3588_CPUL_PVTPLL_CON2,
			      0x00040000);
		/* set ring sel */
		mmio_write_32(LITCOREGRF_BASE + RK3588_CPUL_PVTPLL_CON0_L,
			      0x07000000 | (pvtpll->ring_sel << 8));
		/* set length */
		mmio_write_32(LITCOREGRF_BASE + RK3588_CPUL_PVTPLL_CON0_H,
			      0x003f0000 | pvtpll->length);
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(LITCOREGRF_BASE + RK3588_CPUL_PVTPLL_CON1,
			      0x18);
		/* enable pvtpll */
		mmio_write_32(LITCOREGRF_BASE + RK3588_CPUL_PVTPLL_CON0_L,
			      0x00020002);
		/* start monitor */
		mmio_write_32(LITCOREGRF_BASE + RK3588_CPUL_PVTPLL_CON0_L,
			      0x00010001);
		/* set corel mux pvtpll */
		mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(7),
			      CPUL_PVTPLL_PATH_PVTPLL);
		mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(6),
			      CPUL_CLK_PATH_PVTPLL);
		mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(7),
			      CPUL_CLK_PATH_PVTPLL);
		return 0;
	}

	/* set clk corel div */
	div = DIV_ROUND_UP(GPLL_RATE, rate) - 1;
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(6),
		      CLKDIV_5BITS_SHF(div, 0) | CLKDIV_5BITS_SHF(div, 7));
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(7),
		      CLKDIV_5BITS_SHF(div, 0) | CLKDIV_5BITS_SHF(div, 7));
	/* set corel mux gpll */
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(5),
		      CPUL_CLK_PATH_NOR_GPLL);
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(6),
		      CPUL_CLK_PATH_LPLL);
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(7),
		      CPUL_CLK_PATH_LPLL);

	return 0;
}

static int clk_scmi_cpul_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	ret = clk_cpul_set_rate(rate, PLL_SEL_AUTO);
	if (ret == 0) {
		sys_clk_info.cpul_rate = rate;
		ret = clk_scmi_dsu_set_rate(clock, rate);
	}

	return ret;
}

static unsigned long rk3588_lpll_get_rate(void)
{
	unsigned int m, p, s, k;
	uint64_t rate64 = 24000000, postdiv;
	int mode;

	mode = (mmio_read_32(DSUCRU_BASE + CRU_CLKSEL_CON(5)) >> 14) &
	       0x3;

	if (mode == 0)
		return rate64;

	m = (mmio_read_32(DSUCRU_BASE + CRU_PLL_CON(16)) >>
		 CRU_PLLCON0_M_SHIFT) &
		CRU_PLLCON0_M_MASK;
	p = (mmio_read_32(DSUCRU_BASE + CRU_PLL_CON(17)) >>
		    CRU_PLLCON1_P_SHIFT) &
		   CRU_PLLCON1_P_MASK;
	s = (mmio_read_32(DSUCRU_BASE + CRU_PLL_CON(17)) >>
		  CRU_PLLCON1_S_SHIFT) &
		 CRU_PLLCON1_S_MASK;
	k = (mmio_read_32(DSUCRU_BASE + CRU_PLL_CON(18)) >>
		    CRU_PLLCON2_K_SHIFT) &
		   CRU_PLLCON2_K_MASK;

	rate64 *= m;
	rate64 = rate64 / p;

	if (k != 0) {
		/* fractional mode */
		uint64_t frac_rate64 = 24000000 * k;

		postdiv = p * 65535;
		frac_rate64 = frac_rate64 / postdiv;
		rate64 += frac_rate64;
	}
	rate64 = rate64 >> s;

	return (unsigned long)rate64;
}

static unsigned long clk_scmi_cpul_get_rate(rk_scmi_clock_t *clock)
{
	int src, div;

	src = mmio_read_32(DSUCRU_BASE + CRU_CLKSEL_CON(6)) & 0x0060;
	src = src >> 5;
	if (src == 2) {
		return sys_clk_info.cpul_rate;
	} else {
		src = mmio_read_32(DSUCRU_BASE + CRU_CLKSEL_CON(5)) & 0xc000;
		src = src >> 14;
		div = mmio_read_32(DSUCRU_BASE + CRU_CLKSEL_CON(6)) & 0x1f;
		switch (src) {
		case 0:
			return 24000000;
		case 1:
			/* Make the return rate is equal to the set rate */
			if (sys_clk_info.cpul_rate)
				return sys_clk_info.cpul_rate;
			else
				return GPLL_RATE / (div + 1);
		case 2:
			return rk3588_lpll_get_rate();
		default:
			return 0;
		}
	}
}

static int clk_scmi_cpul_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static void clk_scmi_b0pll_disable(void)
{
	static bool is_b0pll_disabled;

	if (is_b0pll_disabled != 0)
		return;

	/* set coreb01 mux gpll */
	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(0),
		      CPUB01_CLK_PATH_NOR_GPLL);
	 /* pll enter slow mode */
	mmio_write_32(BIGCORE0CRU_BASE + CRU_MODE_CON0, CPU_PLL_PATH_SLOWMODE);
	/* set pll power down */
	mmio_write_32(BIGCORE0CRU_BASE + CRU_PLL_CON(1), CRU_PLL_POWER_DOWN);

	is_b0pll_disabled = true;
}

static int clk_cpub01_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *pvtpll;
	int div;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	pvtpll = rkclk_get_pvtpll_config(sys_clk_info.cpub01_table,
					 sys_clk_info.cpub01_rate_count, rate);
	if (pvtpll == NULL)
		return SCMI_INVALID_PARAMETERS;

	/* set b0pll */
	if (PVTPLL_NEED(type, pvtpll->length)) {
		/* set clock gating interval */
		mmio_write_32(BIGCORE0GRF_BASE + RK3588_CPUB_PVTPLL_CON2,
			      0x00040000);
		/* set ring sel */
		mmio_write_32(BIGCORE0GRF_BASE + RK3588_CPUB_PVTPLL_CON0_L,
			      0x07000000 | (pvtpll->ring_sel << 8));
		/* set length */
		mmio_write_32(BIGCORE0GRF_BASE + RK3588_CPUB_PVTPLL_CON0_H,
			      0x003f0000 | pvtpll->length);
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(BIGCORE0GRF_BASE + RK3588_CPUB_PVTPLL_CON1,
			      0x18);
		/* enable pvtpll */
		mmio_write_32(BIGCORE0GRF_BASE + RK3588_CPUB_PVTPLL_CON0_L,
			      0x00020002);
		/* start monitor */
		mmio_write_32(BIGCORE0GRF_BASE + RK3588_CPUB_PVTPLL_CON0_L,
			      0x00010001);
		/* set core mux pvtpll */
		mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(2),
			      CPUB01_PVTPLL_PATH_PVTPLL);
		mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(0),
			      CPUB01_CLK_PATH_B0_PVTPLL);
		mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(1),
			      CPUB01_CLK_PATH_B1_PVTPLL);
		goto out;
	}

	/* set clk coreb01 div */
	div = DIV_ROUND_UP(GPLL_RATE, rate) - 1;
	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(0),
		      CLKDIV_5BITS_SHF(div, 8));
	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(1),
		      CLKDIV_5BITS_SHF(div, 0));
	/* set coreb01 mux gpll */
	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(0),
		      CPUB01_CLK_PATH_NOR_GPLL);
	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(0),
		      CPUB01_CLK_PATH_B0PLL);
	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(1),
		      CPUB01_CLK_PATH_B1PLL);

out:
	clk_scmi_b0pll_disable();

	return 0;
}

static int clk_scmi_cpub01_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	ret = clk_cpub01_set_rate(rate, PLL_SEL_AUTO);
	if (ret == 0)
		sys_clk_info.cpub01_rate = rate;

	return ret;
}

static unsigned long rk3588_b0pll_get_rate(void)
{
	unsigned int m, p, s, k;
	uint64_t rate64 = 24000000, postdiv;
	int mode;

	mode = (mmio_read_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(0)) >> 6) &
	       0x3;

	if (mode == 0)
		return rate64;

	m = (mmio_read_32(BIGCORE0CRU_BASE + CRU_PLL_CON(0)) >>
		 CRU_PLLCON0_M_SHIFT) &
		CRU_PLLCON0_M_MASK;
	p = (mmio_read_32(BIGCORE0CRU_BASE + CRU_PLL_CON(1)) >>
		    CRU_PLLCON1_P_SHIFT) &
		   CRU_PLLCON1_P_MASK;
	s = (mmio_read_32(BIGCORE0CRU_BASE + CRU_PLL_CON(1)) >>
		  CRU_PLLCON1_S_SHIFT) &
		 CRU_PLLCON1_S_MASK;
	k = (mmio_read_32(BIGCORE0CRU_BASE + CRU_PLL_CON(2)) >>
		    CRU_PLLCON2_K_SHIFT) &
		   CRU_PLLCON2_K_MASK;

	rate64 *= m;
	rate64 = rate64 / p;

	if (k != 0) {
		/* fractional mode */
		uint64_t frac_rate64 = 24000000 * k;

		postdiv = p * 65535;
		frac_rate64 = frac_rate64 / postdiv;
		rate64 += frac_rate64;
	}
	rate64 = rate64 >> s;

	return (unsigned long)rate64;
}

static unsigned long clk_scmi_cpub01_get_rate(rk_scmi_clock_t *clock)
{
	int value, src, div;

	value = mmio_read_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(0));
	src = (value & 0x6000) >> 13;
	if (src == 2) {
		return sys_clk_info.cpub01_rate;
	} else {
		src = (value & 0x00c0) >> 6;
		div = (value & 0x1f00) >> 8;
		switch (src) {
		case 0:
			return 24000000;
		case 1:
			/* Make the return rate is equal to the set rate */
			if (sys_clk_info.cpub01_rate)
				return sys_clk_info.cpub01_rate;
			else
				return GPLL_RATE / (div + 1);
		case 2:
			return rk3588_b0pll_get_rate();
		default:
			return 0;
		}
	}
}

static int clk_scmi_cpub01_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static void clk_scmi_b1pll_disable(void)
{
	static bool is_b1pll_disabled;

	if (is_b1pll_disabled != 0)
		return;

	/* set coreb23 mux gpll */
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(0),
		      CPUB01_CLK_PATH_NOR_GPLL);
	 /* pll enter slow mode */
	mmio_write_32(BIGCORE1CRU_BASE + CRU_MODE_CON0, CPU_PLL_PATH_SLOWMODE);
	/* set pll power down */
	mmio_write_32(BIGCORE1CRU_BASE + CRU_PLL_CON(9), CRU_PLL_POWER_DOWN);

	is_b1pll_disabled = true;
}

static int clk_cpub23_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *pvtpll;
	int div;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	pvtpll = rkclk_get_pvtpll_config(sys_clk_info.cpub23_table,
					 sys_clk_info.cpub23_rate_count, rate);
	if (pvtpll == NULL)
		return SCMI_INVALID_PARAMETERS;

	/* set b1pll */
	if (PVTPLL_NEED(type, pvtpll->length)) {
		/* set clock gating interval */
		mmio_write_32(BIGCORE1GRF_BASE + RK3588_CPUB_PVTPLL_CON2,
			      0x00040000);
		/* set ring sel */
		mmio_write_32(BIGCORE1GRF_BASE + RK3588_CPUB_PVTPLL_CON0_L,
			      0x07000000 | (pvtpll->ring_sel << 8));
		/* set length */
		mmio_write_32(BIGCORE1GRF_BASE + RK3588_CPUB_PVTPLL_CON0_H,
			      0x003f0000 | pvtpll->length);
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(BIGCORE1GRF_BASE + RK3588_CPUB_PVTPLL_CON1,
			      0x18);
		/* enable pvtpll */
		mmio_write_32(BIGCORE1GRF_BASE + RK3588_CPUB_PVTPLL_CON0_L,
			      0x00020002);
		/* start monitor */
		mmio_write_32(BIGCORE1GRF_BASE + RK3588_CPUB_PVTPLL_CON0_L,
			      0x00010001);
		/* set core mux pvtpll */
		mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(2),
			      CPUB01_PVTPLL_PATH_PVTPLL);
		mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(0),
			      CPUB01_CLK_PATH_B0_PVTPLL);
		mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(1),
			      CPUB01_CLK_PATH_B1_PVTPLL);
		goto out;
	}

	/* set clk coreb23 div */
	div = DIV_ROUND_UP(GPLL_RATE, rate) - 1;
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(0),
		      CLKDIV_5BITS_SHF(div, 8));
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(1),
		      CLKDIV_5BITS_SHF(div, 0));
	/* set coreb23 mux gpll */
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(0),
		      CPUB01_CLK_PATH_NOR_GPLL);
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(0),
		      CPUB01_CLK_PATH_B0PLL);
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(1),
		      CPUB01_CLK_PATH_B1PLL);

out:
	clk_scmi_b1pll_disable();

	return 0;
}

static int clk_scmi_cpub23_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	ret = clk_cpub23_set_rate(rate, PLL_SEL_AUTO);
	if (ret == 0)
		sys_clk_info.cpub23_rate = rate;

	return ret;
}

static unsigned long rk3588_b1pll_get_rate(void)
{
	unsigned int m, p, s, k;
	uint64_t rate64 = 24000000, postdiv;
	int mode;

	mode = (mmio_read_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(0)) >> 6) &
	       0x3;

	if (mode == 0)
		return rate64;

	m = (mmio_read_32(BIGCORE1CRU_BASE + CRU_PLL_CON(8)) >>
		 CRU_PLLCON0_M_SHIFT) &
		CRU_PLLCON0_M_MASK;
	p = (mmio_read_32(BIGCORE1CRU_BASE + CRU_PLL_CON(9)) >>
		    CRU_PLLCON1_P_SHIFT) &
		   CRU_PLLCON1_P_MASK;
	s = (mmio_read_32(BIGCORE1CRU_BASE + CRU_PLL_CON(9)) >>
		  CRU_PLLCON1_S_SHIFT) &
		 CRU_PLLCON1_S_MASK;
	k = (mmio_read_32(BIGCORE1CRU_BASE + CRU_PLL_CON(10)) >>
		    CRU_PLLCON2_K_SHIFT) &
		   CRU_PLLCON2_K_MASK;

	rate64 *= m;
	rate64 = rate64 / p;

	if (k != 0) {
		/* fractional mode */
		uint64_t frac_rate64 = 24000000 * k;

		postdiv = p * 65535;
		frac_rate64 = frac_rate64 / postdiv;
		rate64 += frac_rate64;
	}
	rate64 = rate64 >> s;

	return (unsigned long)rate64;
}

static unsigned long clk_scmi_cpub23_get_rate(rk_scmi_clock_t *clock)
{
	int value, src, div;

	value = mmio_read_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(0));
	src = (value & 0x6000) >> 13;
	if (src == 2) {
		return sys_clk_info.cpub23_rate;
	} else {
		src = (value & 0x00c0) >> 6;
		div = (value & 0x1f00) >> 8;
		switch (src) {
		case 0:
			return 24000000;
		case 1:
			/* Make the return rate is equal to the set rate */
			if (sys_clk_info.cpub23_rate)
				return sys_clk_info.cpub23_rate;
			else
				return GPLL_RATE / (div + 1);
		case 2:
			return rk3588_b1pll_get_rate();
		default:
			return 0;
		}
	}
}

static int clk_scmi_cpub23_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_dsu_get_rate(rk_scmi_clock_t *clock)
{
	int src, div;

	src = mmio_read_32(DSUCRU_BASE + CRU_CLKSEL_CON(1)) & 0x1;
	if (src != 0) {
		return sys_clk_info.dsu_rate;
	} else {
		src = mmio_read_32(DSUCRU_BASE + CRU_CLKSEL_CON(0)) & 0x3000;
		src = src >> 12;
		div = mmio_read_32(DSUCRU_BASE + CRU_CLKSEL_CON(0)) & 0xf80;
		div = div >> 7;
		switch (src) {
		case 0:
			return rk3588_b0pll_get_rate() / (div + 1);
		case 1:
			return rk3588_b1pll_get_rate() / (div + 1);
		case 2:
			return rk3588_lpll_get_rate() / (div + 1);
		case 3:
			return GPLL_RATE / (div + 1);
		default:
			return 0;
		}
	}
}

static void clk_scmi_lpll_disable(void)
{
	static bool is_lpll_disabled;

	if (is_lpll_disabled)
		return;

	/* set corel mux gpll */
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(5),
		      CPUL_CLK_PATH_NOR_GPLL);
	/* set dsu mux gpll */
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(0),
		      SCLK_DSU_PATH_NOR_GPLL);
	/* pll enter slow mode */
	mmio_write_32(DSUCRU_BASE + CRU_MODE_CON0, CPU_PLL_PATH_SLOWMODE);
	/* set pll power down */
	mmio_write_32(DSUCRU_BASE + CRU_PLL_CON(17), CRU_PLL_POWER_DOWN);

	is_lpll_disabled = true;
}

static int clk_dsu_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *pvtpll;
	int div;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	pvtpll = rkclk_get_pvtpll_config(sys_clk_info.cpul_table,
					 sys_clk_info.cpul_rate_count, rate);
	if (pvtpll == NULL)
		return SCMI_INVALID_PARAMETERS;

	/* set pvtpll */
	if (PVTPLL_NEED(type, pvtpll->length)) {
		/* set clock gating interval */
		mmio_write_32(DSUGRF_BASE + RK3588_DSU_PVTPLL_CON2,
			      0x00040000);
		/* set ring sel */
		mmio_write_32(DSUGRF_BASE + RK3588_DSU_PVTPLL_CON0_L,
			      0x07000000 | (pvtpll->ring_sel << 8));
		/* set length */
		mmio_write_32(DSUGRF_BASE + RK3588_DSU_PVTPLL_CON0_H,
			      0x003f0000 | pvtpll->length);
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(DSUGRF_BASE + RK3588_DSU_PVTPLL_CON1,
			      0x18);
		/* enable pvtpll */
		mmio_write_32(DSUGRF_BASE + RK3588_DSU_PVTPLL_CON0_L,
			      0x00020002);
		/* start monitor */
		mmio_write_32(DSUGRF_BASE + RK3588_DSU_PVTPLL_CON0_L,
			      0x00010001);
		/* set dsu mux pvtpll */
		mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(7),
			      DSU_PVTPLL_PATH_PVTPLL);
		mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(1),
			      SCLK_DSU_PATH_PVTPLL);
		goto out;
	}
	/* set dsu div */
	div = DIV_ROUND_UP(GPLL_RATE, rate) - 1;
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(0),
		      CLKDIV_5BITS_SHF(div, 7));
	/* set dsu mux gpll */
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(0),
		      SCLK_DSU_PATH_NOR_GPLL);
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(1),
		      SCLK_DSU_PATH_NOR_PLL);

out:
	clk_scmi_lpll_disable();

	return 0;
}

static int clk_scmi_dsu_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	ret = clk_dsu_set_rate(rate, PLL_SEL_AUTO);

	if (ret == 0)
		sys_clk_info.dsu_rate = rate;
	return ret;
}

static int clk_scmi_dsu_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_gpu_get_rate(rk_scmi_clock_t *clock)
{
	int div, src;

	if ((mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(158)) & 0x4000) != 0) {
		return sys_clk_info.gpu_rate;
	} else {
		div = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(158)) & 0x1f;
		src = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(158)) & 0x00e0;
		src = src >> 5;
		switch (src) {
		case 0:
			/* Make the return rate is equal to the set rate */
			if (sys_clk_info.gpu_rate)
				return sys_clk_info.gpu_rate;
			else
				return GPLL_RATE / (div + 1);
		case 1:
			return CPLL_RATE / (div + 1);
		case 2:
			return AUPLL_RATE / (div + 1);
		case 3:
			return NPLL_RATE / (div + 1);
		case 4:
			return SPLL_RATE / (div + 1);
		default:
			return 0;
		}
	}
}

static int clk_gpu_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *pvtpll;
	int div;

	pvtpll = rkclk_get_pvtpll_config(sys_clk_info.gpu_table,
					 sys_clk_info.gpu_rate_count, rate);
	if (pvtpll == NULL)
		return SCMI_INVALID_PARAMETERS;

	if (PVTPLL_NEED(type, pvtpll->length)) {
		/* set clock gating interval */
		mmio_write_32(GPUGRF_BASE + RK3588_GPU_PVTPLL_CON2,
			      0x00040000);
		/* set ring sel */
		mmio_write_32(GPUGRF_BASE + RK3588_GPU_PVTPLL_CON0_L,
			      0x07000000 | (pvtpll->ring_sel << 8));
		/* set length */
		mmio_write_32(GPUGRF_BASE + RK3588_GPU_PVTPLL_CON0_H,
			      0x003f0000 | pvtpll->length);
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(GPUGRF_BASE + RK3588_GPU_PVTPLL_CON1,
			      0x18);
		/* enable pvtpll */
		mmio_write_32(GPUGRF_BASE + RK3588_GPU_PVTPLL_CON0_L,
			      0x00020002);
		/* start monitor */
		mmio_write_32(GPUGRF_BASE + RK3588_GPU_PVTPLL_CON0_L,
			      0x00010001);
		/* set gpu mux pvtpll */
		mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(158),
			      GPU_CLK_PATH_PVTPLL);
		return 0;
	}

	/* set gpu div */
	div = DIV_ROUND_UP(GPLL_RATE, rate);
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(158),
		      CLKDIV_5BITS_SHF(div - 1, 0));
	/* set gpu mux gpll */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(158),
		      GPU_CLK_PATH_NOR_GPLL);
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(158),
		      GPU_CLK_PATH_NOR_PLL);

	return 0;
}

static int clk_scmi_gpu_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	ret = clk_gpu_set_rate(rate, PLL_SEL_AUTO);
	if (ret == 0)
		sys_clk_info.gpu_rate = rate;

	return ret;
}

static int clk_scmi_gpu_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_npu_get_rate(rk_scmi_clock_t *clock)
{
	int div, src;

	if ((mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(74)) & 0x1) != 0) {
		return sys_clk_info.npu_rate;
	} else {
		div = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(73)) & 0x007c;
		div = div >> 2;
		src = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(73)) & 0x0380;
		src = src >> 7;
		switch (src) {
		case 0:
			/* Make the return rate is equal to the set rate */
			if (sys_clk_info.npu_rate != 0)
				return sys_clk_info.npu_rate;
			else
				return GPLL_RATE / (div + 1);
		case 1:
			return CPLL_RATE / (div + 1);
		case 2:
			return AUPLL_RATE / (div + 1);
		case 3:
			return NPLL_RATE / (div + 1);
		case 4:
			return SPLL_RATE / (div + 1);
		default:
			return 0;
		}
	}
}

static int clk_npu_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *pvtpll;
	int div;

	pvtpll = rkclk_get_pvtpll_config(sys_clk_info.npu_table,
					 sys_clk_info.npu_rate_count, rate);
	if (pvtpll == NULL)
		return SCMI_INVALID_PARAMETERS;

	if (PVTPLL_NEED(type, pvtpll->length)) {
		/* set clock gating interval */
		mmio_write_32(NPUGRF_BASE + RK3588_NPU_PVTPLL_CON2,
			      0x00040000);
		/* set ring sel */
		mmio_write_32(NPUGRF_BASE + RK3588_NPU_PVTPLL_CON0_L,
			      0x07000000 | (pvtpll->ring_sel << 8));
		/* set length */
		mmio_write_32(NPUGRF_BASE + RK3588_NPU_PVTPLL_CON0_H,
			      0x003f0000 | pvtpll->length);
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(NPUGRF_BASE + RK3588_NPU_PVTPLL_CON1,
			      0x18);
		/* enable pvtpll */
		mmio_write_32(NPUGRF_BASE + RK3588_NPU_PVTPLL_CON0_L,
			      0x00020002);
		/* start monitor */
		mmio_write_32(NPUGRF_BASE + RK3588_NPU_PVTPLL_CON0_L,
			      0x00010001);
		/* set npu mux pvtpll */
		mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(74),
			      NPU_CLK_PATH_PVTPLL);
		return 0;
	}

	/* set npu div */
	div = DIV_ROUND_UP(GPLL_RATE, rate);
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(73),
		      CLKDIV_5BITS_SHF(div - 1, 2));
	/* set npu mux gpll */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(73),
		      NPU_CLK_PATH_NOR_GPLL);
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(74),
		      NPU_CLK_PATH_NOR_PLL);

	return 0;
}

static int clk_scmi_npu_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	ret = clk_npu_set_rate(rate, PLL_SEL_AUTO);
	if (ret == 0)
		sys_clk_info.npu_rate = rate;

	return ret;
}

static int clk_scmi_npu_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_sbus_get_rate(rk_scmi_clock_t *clock)
{
	int div;

	if ((mmio_read_32(BUSSCRU_BASE + CRU_CLKSEL_CON(0)) & 0x0800) != 0) {
		div = mmio_read_32(BUSSCRU_BASE + CRU_CLKSEL_CON(0));
		div = (div & 0x03e0) >> 5;
		return SPLL_RATE / (div + 1);
	} else {
		return OSC_HZ;
	}
}

static int clk_scmi_sbus_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int div;

	if (rate == OSC_HZ) {
		mmio_write_32(BUSSCRU_BASE + CRU_CLKSEL_CON(0),
			      WMSK_BIT(11));
		return 0;
	}

	div = DIV_ROUND_UP(SPLL_RATE, rate);
	mmio_write_32(BUSSCRU_BASE + CRU_CLKSEL_CON(0),
		      CLKDIV_5BITS_SHF(div - 1, 5));
	mmio_write_32(BUSSCRU_BASE + CRU_CLKSEL_CON(0),
		      BIT_WITH_WMSK(11) | WMSK_BIT(10));
	return 0;
}

static int clk_scmi_sbus_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_pclk_sbus_get_rate(rk_scmi_clock_t *clock)
{
	int div;

	div = mmio_read_32(BUSSCRU_BASE + CRU_CLKSEL_CON(0));
	div = div & 0x001f;
	return SPLL_RATE / (div + 1);

}

static int clk_scmi_pclk_sbus_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int div;

	div = DIV_ROUND_UP(SPLL_RATE, rate);
	mmio_write_32(BUSSCRU_BASE + CRU_CLKSEL_CON(0),
		      CLKDIV_5BITS_SHF(div - 1, 0));
	return 0;
}

static int clk_scmi_pclk_sbus_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_cclk_sdmmc_get_rate(rk_scmi_clock_t *clock)
{
	int div;
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(3)) & 0x3000;
	src = src >> 12;
	div = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(3)) & 0x0fc0;
	div = div >> 6;
	if (src == 1) {
		return SPLL_RATE / (div + 1);
	} else if (src == 2) {
		return OSC_HZ / (div + 1);
	} else {
		return GPLL_RATE / (div + 1);
	}
}

static int clk_scmi_cclk_sdmmc_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int div;

	if ((OSC_HZ % rate) == 0) {
		div = DIV_ROUND_UP(OSC_HZ, rate);
		mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(3),
			      CLKDIV_6BITS_SHF(div - 1, 6) |
			      BITS_WITH_WMASK(2U, 0x3U, 12));
	} else if ((SPLL_RATE % rate) == 0) {
		div = DIV_ROUND_UP(SPLL_RATE, rate);
		mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(3),
			      CLKDIV_6BITS_SHF(div - 1, 6) |
			      BITS_WITH_WMASK(1U, 0x3U, 12));
	} else {
		div = DIV_ROUND_UP(GPLL_RATE, rate);
		mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(3),
			      CLKDIV_6BITS_SHF(div - 1, 6) |
			      BITS_WITH_WMASK(0U, 0x3U, 12));
	}

	return 0;
}

static int clk_scmi_cclk_sdmmc_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(3),
		      BITS_WITH_WMASK(!status, 0x1U, 4));
	return 0;
}

static unsigned long clk_scmi_dclk_sdmmc_get_rate(rk_scmi_clock_t *clock)
{
	int div;
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(3)) & 0x0020;
	div = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(3)) & 0x001f;
	if (src != 0) {
		return SPLL_RATE / (div + 1);
	} else {
		return GPLL_RATE / (div + 1);
	}
}

static int clk_scmi_dclk_sdmmc_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int div;

	if ((SPLL_RATE % rate) == 0) {
		div = DIV_ROUND_UP(SPLL_RATE, rate);
		mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(3),
			      CLKDIV_5BITS_SHF(div - 1, 0) |
			      BITS_WITH_WMASK(1U, 0x1U, 5));
	} else {
		div = DIV_ROUND_UP(GPLL_RATE, rate);
		mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(3),
			      CLKDIV_5BITS_SHF(div - 1, 0) |
			      BITS_WITH_WMASK(0U, 0x1U, 5));
	}
	return 0;
}

static int clk_scmi_dclk_sdmmc_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(3),
		      BITS_WITH_WMASK(!status, 0x1U, 1));
	return 0;
}

static unsigned long clk_scmi_aclk_secure_ns_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(1)) & 0x0003;
	switch (src) {
	case 0:
		return 350 * MHz;
	case 1:
		return 200 * MHz;
	case 2:
		return 100 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_aclk_secure_ns_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 350 * MHz)
		src = 0;
	else if (rate >= 200 * MHz)
		src = 1;
	else if (rate >= 100 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(src, 0x3U, 0));

	return 0;
}

static int clk_scmi_aclk_secure_ns_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_hclk_secure_ns_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(1)) & 0x000c;
	src = src >> 2;
	switch (src) {
	case 0:
		return 150 * MHz;
	case 1:
		return 100 * MHz;
	case 2:
		return 50 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_hclk_secure_ns_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 150 * MHz)
		src = 0;
	else if (rate >= 100 * MHz)
		src = 1;
	else if (rate >= 50 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(src, 0x3U, 2));
	return 0;
}

static int clk_scmi_hclk_secure_ns_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_tclk_wdt_get_rate(rk_scmi_clock_t *clock)
{
	return OSC_HZ;
}

static int clk_scmi_tclk_wdt_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(2),
		      BITS_WITH_WMASK(!status, 0x1U, 0));
	return 0;
}

static unsigned long clk_scmi_keyladder_core_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(2)) & 0x00c0;
	src = src >> 6;
	switch (src) {
	case 0:
		return 350 * MHz;
	case 1:
		return 233 * MHz;
	case 2:
		return 116 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_keyladder_core_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 350 * MHz)
		src = 0;
	else if (rate >= 233 * MHz)
		src = 1;
	else if (rate >= 116 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(2),
		      BITS_WITH_WMASK(src, 0x3U, 6));
	return 0;
}

static int clk_scmi_keyladder_core_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 9));
	return 0;
}

static unsigned long clk_scmi_keyladder_rng_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(2)) & 0x0300;
	src = src >> 8;
	switch (src) {
	case 0:
		return 175 * MHz;
	case 1:
		return 116 * MHz;
	case 2:
		return 58 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_keyladder_rng_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 175 * MHz)
		src = 0;
	else if (rate >= 116 * MHz)
		src = 1;
	else if (rate >= 58 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(2),
		      BITS_WITH_WMASK(src, 0x3U, 8));
	return 0;
}

static int clk_scmi_keyladder_rng_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 10));
	return 0;
}

static unsigned long clk_scmi_aclk_secure_s_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(1)) & 0x0030;
	src = src >> 4;
	switch (src) {
	case 0:
		return 350 * MHz;
	case 1:
		return 233 * MHz;
	case 2:
		return 116 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_aclk_secure_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 350 * MHz)
		src = 0;
	else if (rate >= 233 * MHz)
		src = 1;
	else if (rate >= 116 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(src, 0x3U, 4));
	return 0;
}

static int clk_scmi_aclk_secure_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_hclk_secure_s_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(1)) & 0x00c0;
	src = src >> 6;
	switch (src) {
	case 0:
		return 175 * MHz;
	case 1:
		return 116 * MHz;
	case 2:
		return 58 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_hclk_secure_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 175 * MHz)
		src = 0;
	else if (rate >= 116 * MHz)
		src = 1;
	else if (rate >= 58 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(src, 0x3U, 6));
	return 0;
}

static int clk_scmi_hclk_secure_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_pclk_secure_s_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(1)) & 0x0300;
	src = src >> 8;
	switch (src) {
	case 0:
		return 116 * MHz;
	case 1:
		return 58 * MHz;
	case 2:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_pclk_secure_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 116 * MHz)
		src = 0;
	else if (rate >= 58 * MHz)
		src = 1;
	else
		src = 2;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(src, 0x3U, 8));
	return 0;
}

static int clk_scmi_pclk_secure_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_crypto_rng_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(1)) & 0xc000;
	src = src >> 14;
	switch (src) {
	case 0:
		return 175 * MHz;
	case 1:
		return 116 * MHz;
	case 2:
		return 58 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_crypto_rng_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 175 * MHz)
		src = 0;
	else if (rate >= 116 * MHz)
		src = 1;
	else if (rate >= 58 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(src, 0x3U, 14));
	return 0;
}

static int clk_scmi_crypto_rng_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 1));

	return 0;
}

static unsigned long clk_scmi_crypto_core_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(1)) & 0x0c00;
	src = src >> 10;
	switch (src) {
	case 0:
		return 350 * MHz;
	case 1:
		return 233 * MHz;
	case 2:
		return 116 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_crypto_core_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 350 * MHz)
		src = 0;
	else if (rate >= 233 * MHz)
		src = 1;
	else if (rate >= 116 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(src, 0x3U, 10));
	return 0;
}

static int clk_scmi_crypto_core_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(0),
		      BITS_WITH_WMASK(!status, 0x1U, 15));

	return 0;
}

static unsigned long clk_scmi_crypto_pka_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(1)) & 0x3000;
	src = src >> 12;
	switch (src) {
	case 0:
		return 350 * MHz;
	case 1:
		return 233 * MHz;
	case 2:
		return 116 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_crypto_pka_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 350 * MHz)
		src = 0;
	else if (rate >= 233 * MHz)
		src = 1;
	else if (rate >= 116 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(1),
		      BITS_WITH_WMASK(src, 0x3U, 12));
	return 0;
}

static int clk_scmi_crypto_pka_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 0));

	return 0;
}

static unsigned long clk_scmi_spll_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(BUSSCRU_BASE + CRU_MODE_CON0) & 0x3;
	switch (src) {
	case 0:
		return OSC_HZ;
	case 1:
		return 702 * MHz;
	case 2:
		return 32768;
	default:
		return 0;
	}
}

static int clk_scmi_spll_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 700 * MHz)
		src = 1;
	else
		src = 0;

	mmio_write_32(BUSSCRU_BASE + CRU_MODE_CON0,
		      BITS_WITH_WMASK(0, 0x3U, 0));
	mmio_write_32(BUSSCRU_BASE + CRU_PLL_CON(137),
		      BITS_WITH_WMASK(2, 0x7U, 6));

	mmio_write_32(BUSSCRU_BASE + CRU_MODE_CON0,
		      BITS_WITH_WMASK(src, 0x3U, 0));
	return 0;
}

static int clk_scmi_spll_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_hclk_sd_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_hclk_secure_ns_get_rate(clock);
}

static int clk_scmi_hclk_sd_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(3),
		      BITS_WITH_WMASK(!status, 0x1U, 2));
	return 0;
}

static unsigned long clk_scmi_crypto_rng_s_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(2)) & 0x0030;
	src = src >> 4;
	switch (src) {
	case 0:
		return 175 * MHz;
	case 1:
		return 116 * MHz;
	case 2:
		return 58 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_crypto_rng_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 175 * MHz)
		src = 0;
	else if (rate >= 116 * MHz)
		src = 1;
	else if (rate >= 58 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(2),
		      BITS_WITH_WMASK(src, 0x3U, 4));
	return 0;
}

static int clk_scmi_crypto_rng_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 6));

	return 0;
}

static unsigned long clk_scmi_crypto_core_s_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(2)) & 0x3;
	src = src >> 0;
	switch (src) {
	case 0:
		return 350 * MHz;
	case 1:
		return 233 * MHz;
	case 2:
		return 116 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_crypto_core_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 350 * MHz)
		src = 0;
	else if (rate >= 233 * MHz)
		src = 1;
	else if (rate >= 116 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(2),
		      BITS_WITH_WMASK(src, 0x3U, 0));
	return 0;
}

static int clk_scmi_crypto_core_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 4));

	return 0;
}

static unsigned long clk_scmi_crypto_pka_s_get_rate(rk_scmi_clock_t *clock)
{
	uint32_t src;

	src = mmio_read_32(SCRU_BASE + CRU_CLKSEL_CON(2)) & 0x000c;
	src = src >> 2;
	switch (src) {
	case 0:
		return 350 * MHz;
	case 1:
		return 233 * MHz;
	case 2:
		return 116 * MHz;
	case 3:
		return OSC_HZ;
	default:
		return 0;
	}
}

static int clk_scmi_crypto_pka_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	uint32_t src;

	if (rate >= 350 * MHz)
		src = 0;
	else if (rate >= 233 * MHz)
		src = 1;
	else if (rate >= 116 * MHz)
		src = 2;
	else
		src = 3;

	mmio_write_32(SCRU_BASE + CRU_CLKSEL_CON(2),
		      BITS_WITH_WMASK(src, 0x3U, 2));
	return 0;
}

static int clk_scmi_crypto_pka_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 5));

	return 0;
}

static unsigned long clk_scmi_a_crypto_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_aclk_secure_s_get_rate(clock);
}

static int clk_scmi_a_crypto_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_aclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_a_crypto_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 7));

	return 0;
}

static unsigned long clk_scmi_h_crypto_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_hclk_secure_s_get_rate(clock);
}

static int clk_scmi_h_crypto_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_hclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_h_crypto_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 8));

	return 0;
}

static unsigned long clk_scmi_p_crypto_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_pclk_secure_s_get_rate(clock);
}

static int clk_scmi_p_crypto_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_pclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_p_crypto_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(2),
		      BITS_WITH_WMASK(!status, 0x1U, 13));

	return 0;
}

static unsigned long clk_scmi_a_keylad_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_aclk_secure_s_get_rate(clock);
}

static int clk_scmi_a_keylad_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_aclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_a_keylad_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 11));

	return 0;
}

static unsigned long clk_scmi_h_keylad_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_hclk_secure_s_get_rate(clock);
}

static int clk_scmi_h_keylad_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_hclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_h_keylad_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 12));

	return 0;
}

static unsigned long clk_scmi_p_keylad_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_pclk_secure_s_get_rate(clock);
}

static int clk_scmi_p_keylad_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_pclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_p_keylad_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(2),
		      BITS_WITH_WMASK(!status, 0x1U, 14));

	return 0;
}

static unsigned long clk_scmi_trng_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_hclk_secure_s_get_rate(clock);
}

static int clk_scmi_trng_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_hclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_trng_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(3),
		      BITS_WITH_WMASK(!status, 0x1U, 6));

	return 0;
}

static unsigned long clk_scmi_h_trng_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_hclk_secure_s_get_rate(clock);
}

static int clk_scmi_h_trng_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_hclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_h_trng_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(2),
		      BITS_WITH_WMASK(!status, 0x1U, 15));

	return 0;
}

static unsigned long clk_scmi_p_otpc_s_get_rate(rk_scmi_clock_t *clock)
{
	return clk_scmi_pclk_secure_s_get_rate(clock);
}

static int clk_scmi_p_otpc_s_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	return clk_scmi_pclk_secure_s_set_rate(clock, rate);
}

static int clk_scmi_p_otpc_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 13));

	return 0;
}

static unsigned long clk_scmi_otpc_s_get_rate(rk_scmi_clock_t *clock)
{
	return OSC_HZ;
}

static int clk_scmi_otpc_s_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(SCRU_BASE + CRU_CLKGATE_CON(1),
		      BITS_WITH_WMASK(!status, 0x1U, 14));
	return 0;
}

static unsigned long clk_scmi_otp_phy_get_rate(rk_scmi_clock_t *clock)
{
	return OSC_HZ;
}

static int clk_scmi_otp_phy_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(18),
		      BITS_WITH_WMASK(!status, 0x1U, 13));
	return 0;
}

static unsigned long clk_scmi_otpc_rd_get_rate(rk_scmi_clock_t *clock)
{
	return OSC_HZ;
}

static int clk_scmi_otpc_rd_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(18),
		      BITS_WITH_WMASK(!status, 0x1U, 12));
	return 0;
}

static unsigned long clk_scmi_otpc_arb_get_rate(rk_scmi_clock_t *clock)
{
	return OSC_HZ;
}

static int clk_scmi_otpc_arb_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(CRU_BASE + CRU_CLKGATE_CON(18),
		      BITS_WITH_WMASK(!status, 0x1U, 11));
	return 0;
}

static const struct rk_clk_ops clk_scmi_cpul_ops = {
	.get_rate = clk_scmi_cpul_get_rate,
	.set_rate = clk_scmi_cpul_set_rate,
	.set_status = clk_scmi_cpul_set_status,
};

static const struct rk_clk_ops clk_scmi_dsu_ops = {
	.get_rate = clk_scmi_dsu_get_rate,
	.set_rate = clk_scmi_dsu_set_rate,
	.set_status = clk_scmi_dsu_set_status,
};

static const struct rk_clk_ops clk_scmi_cpub01_ops = {
	.get_rate = clk_scmi_cpub01_get_rate,
	.set_rate = clk_scmi_cpub01_set_rate,
	.set_status = clk_scmi_cpub01_set_status,
};

static const struct rk_clk_ops clk_scmi_cpub23_ops = {
	.get_rate = clk_scmi_cpub23_get_rate,
	.set_rate = clk_scmi_cpub23_set_rate,
	.set_status = clk_scmi_cpub23_set_status,
};

static const struct rk_clk_ops clk_scmi_gpu_ops = {
	.get_rate = clk_scmi_gpu_get_rate,
	.set_rate = clk_scmi_gpu_set_rate,
	.set_status = clk_scmi_gpu_set_status,
};

static const struct rk_clk_ops clk_scmi_npu_ops = {
	.get_rate = clk_scmi_npu_get_rate,
	.set_rate = clk_scmi_npu_set_rate,
	.set_status = clk_scmi_npu_set_status,
};

static const struct rk_clk_ops clk_scmi_sbus_ops = {
	.get_rate = clk_scmi_sbus_get_rate,
	.set_rate = clk_scmi_sbus_set_rate,
	.set_status = clk_scmi_sbus_set_status,
};

static const struct rk_clk_ops clk_scmi_pclk_sbus_ops = {
	.get_rate = clk_scmi_pclk_sbus_get_rate,
	.set_rate = clk_scmi_pclk_sbus_set_rate,
	.set_status = clk_scmi_pclk_sbus_set_status,
};

static const struct rk_clk_ops clk_scmi_cclk_sdmmc_ops = {
	.get_rate = clk_scmi_cclk_sdmmc_get_rate,
	.set_rate = clk_scmi_cclk_sdmmc_set_rate,
	.set_status = clk_scmi_cclk_sdmmc_set_status,
};

static const struct rk_clk_ops clk_scmi_dclk_sdmmc_ops = {
	.get_rate = clk_scmi_dclk_sdmmc_get_rate,
	.set_rate = clk_scmi_dclk_sdmmc_set_rate,
	.set_status = clk_scmi_dclk_sdmmc_set_status,
};

static const struct rk_clk_ops clk_scmi_aclk_secure_ns_ops = {
	.get_rate = clk_scmi_aclk_secure_ns_get_rate,
	.set_rate = clk_scmi_aclk_secure_ns_set_rate,
	.set_status = clk_scmi_aclk_secure_ns_set_status,
};

static const struct rk_clk_ops clk_scmi_hclk_secure_ns_ops = {
	.get_rate = clk_scmi_hclk_secure_ns_get_rate,
	.set_rate = clk_scmi_hclk_secure_ns_set_rate,
	.set_status = clk_scmi_hclk_secure_ns_set_status,
};

static const struct rk_clk_ops clk_scmi_tclk_wdt_ops = {
	.get_rate = clk_scmi_tclk_wdt_get_rate,
	.set_status = clk_scmi_tclk_wdt_set_status,
};

static const struct rk_clk_ops clk_scmi_keyladder_core_ops = {
	.get_rate = clk_scmi_keyladder_core_get_rate,
	.set_rate = clk_scmi_keyladder_core_set_rate,
	.set_status = clk_scmi_keyladder_core_set_status,
};

static const struct rk_clk_ops clk_scmi_keyladder_rng_ops = {
	.get_rate = clk_scmi_keyladder_rng_get_rate,
	.set_rate = clk_scmi_keyladder_rng_set_rate,
	.set_status = clk_scmi_keyladder_rng_set_status,
};

static const struct rk_clk_ops clk_scmi_aclk_secure_s_ops = {
	.get_rate = clk_scmi_aclk_secure_s_get_rate,
	.set_rate = clk_scmi_aclk_secure_s_set_rate,
	.set_status = clk_scmi_aclk_secure_s_set_status,
};

static const struct rk_clk_ops clk_scmi_hclk_secure_s_ops = {
	.get_rate = clk_scmi_hclk_secure_s_get_rate,
	.set_rate = clk_scmi_hclk_secure_s_set_rate,
	.set_status = clk_scmi_hclk_secure_s_set_status,
};

static const struct rk_clk_ops clk_scmi_pclk_secure_s_ops = {
	.get_rate = clk_scmi_pclk_secure_s_get_rate,
	.set_rate = clk_scmi_pclk_secure_s_set_rate,
	.set_status = clk_scmi_pclk_secure_s_set_status,
};

static const struct rk_clk_ops clk_scmi_crypto_rng_ops = {
	.get_rate = clk_scmi_crypto_rng_get_rate,
	.set_rate = clk_scmi_crypto_rng_set_rate,
	.set_status = clk_scmi_crypto_rng_set_status,
};

static const struct rk_clk_ops clk_scmi_crypto_core_ops = {
	.get_rate = clk_scmi_crypto_core_get_rate,
	.set_rate = clk_scmi_crypto_core_set_rate,
	.set_status = clk_scmi_crypto_core_set_status,
};

static const struct rk_clk_ops clk_scmi_crypto_pka_ops = {
	.get_rate = clk_scmi_crypto_pka_get_rate,
	.set_rate = clk_scmi_crypto_pka_set_rate,
	.set_status = clk_scmi_crypto_pka_set_status,
};

static const struct rk_clk_ops clk_scmi_spll_ops = {
	.get_rate = clk_scmi_spll_get_rate,
	.set_rate = clk_scmi_spll_set_rate,
	.set_status = clk_scmi_spll_set_status,
};

static const struct rk_clk_ops clk_scmi_hclk_sd_ops = {
	.get_rate = clk_scmi_hclk_sd_get_rate,
	.set_status = clk_scmi_hclk_sd_set_status,
};

static const struct rk_clk_ops clk_scmi_crypto_rng_s_ops = {
	.get_rate = clk_scmi_crypto_rng_s_get_rate,
	.set_rate = clk_scmi_crypto_rng_s_set_rate,
	.set_status = clk_scmi_crypto_rng_s_set_status,
};

static const struct rk_clk_ops clk_scmi_crypto_core_s_ops = {
	.get_rate = clk_scmi_crypto_core_s_get_rate,
	.set_rate = clk_scmi_crypto_core_s_set_rate,
	.set_status = clk_scmi_crypto_core_s_set_status,
};

static const struct rk_clk_ops clk_scmi_crypto_pka_s_ops = {
	.get_rate = clk_scmi_crypto_pka_s_get_rate,
	.set_rate = clk_scmi_crypto_pka_s_set_rate,
	.set_status = clk_scmi_crypto_pka_s_set_status,
};

static const struct rk_clk_ops clk_scmi_a_crypto_s_ops = {
	.get_rate = clk_scmi_a_crypto_s_get_rate,
	.set_rate = clk_scmi_a_crypto_s_set_rate,
	.set_status = clk_scmi_a_crypto_s_set_status,
};

static const struct rk_clk_ops clk_scmi_h_crypto_s_ops = {
	.get_rate = clk_scmi_h_crypto_s_get_rate,
	.set_rate = clk_scmi_h_crypto_s_set_rate,
	.set_status = clk_scmi_h_crypto_s_set_status,
};

static const struct rk_clk_ops clk_scmi_p_crypto_s_ops = {
	.get_rate = clk_scmi_p_crypto_s_get_rate,
	.set_rate = clk_scmi_p_crypto_s_set_rate,
	.set_status = clk_scmi_p_crypto_s_set_status,
};

static const struct rk_clk_ops clk_scmi_a_keylad_s_ops = {
	.get_rate = clk_scmi_a_keylad_s_get_rate,
	.set_rate = clk_scmi_a_keylad_s_set_rate,
	.set_status = clk_scmi_a_keylad_s_set_status,
};

static const struct rk_clk_ops clk_scmi_h_keylad_s_ops = {
	.get_rate = clk_scmi_h_keylad_s_get_rate,
	.set_rate = clk_scmi_h_keylad_s_set_rate,
	.set_status = clk_scmi_h_keylad_s_set_status,
};

static const struct rk_clk_ops clk_scmi_p_keylad_s_ops = {
	.get_rate = clk_scmi_p_keylad_s_get_rate,
	.set_rate = clk_scmi_p_keylad_s_set_rate,
	.set_status = clk_scmi_p_keylad_s_set_status,
};

static const struct rk_clk_ops clk_scmi_trng_s_ops = {
	.get_rate = clk_scmi_trng_s_get_rate,
	.set_rate = clk_scmi_trng_s_set_rate,
	.set_status = clk_scmi_trng_s_set_status,
};

static const struct rk_clk_ops clk_scmi_h_trng_s_ops = {
	.get_rate = clk_scmi_h_trng_s_get_rate,
	.set_rate = clk_scmi_h_trng_s_set_rate,
	.set_status = clk_scmi_h_trng_s_set_status,
};

static const struct rk_clk_ops clk_scmi_p_otpc_s_ops = {
	.get_rate = clk_scmi_p_otpc_s_get_rate,
	.set_rate = clk_scmi_p_otpc_s_set_rate,
	.set_status = clk_scmi_p_otpc_s_set_status,
};

static const struct rk_clk_ops clk_scmi_otpc_s_ops = {
	.get_rate = clk_scmi_otpc_s_get_rate,
	.set_status = clk_scmi_otpc_s_set_status,
};

static const struct rk_clk_ops clk_scmi_otp_phy_ops = {
	.get_rate = clk_scmi_otp_phy_get_rate,
	.set_status = clk_scmi_otp_phy_set_status,
};

static const struct rk_clk_ops clk_scmi_otpc_rd_ops = {
	.get_rate = clk_scmi_otpc_rd_get_rate,
	.set_status = clk_scmi_otpc_rd_set_status,
};

static const struct rk_clk_ops clk_scmi_otpc_arb_ops = {
	.get_rate = clk_scmi_otpc_arb_get_rate,
	.set_status = clk_scmi_otpc_arb_set_status,
};

rk_scmi_clock_t clock_table[] = {
	RK3588_SCMI_CLOCK(SCMI_CLK_CPUL, "scmi_clk_cpul", &clk_scmi_cpul_ops, rk3588_cpul_rates, ARRAY_SIZE(rk3588_cpul_rates), false),
	RK3588_SCMI_CLOCK(SCMI_CLK_DSU, "scmi_clk_dsu", &clk_scmi_dsu_ops, rk3588_cpul_rates, ARRAY_SIZE(rk3588_cpul_rates), false),
	RK3588_SCMI_CLOCK(SCMI_CLK_CPUB01, "scmi_clk_cpub01", &clk_scmi_cpub01_ops, rk3588_cpub_rates, ARRAY_SIZE(rk3588_cpub_rates), false),
	RK3588_SCMI_CLOCK(SCMI_CLK_CPUB23, "scmi_clk_cpub23", &clk_scmi_cpub23_ops, rk3588_cpub_rates, ARRAY_SIZE(rk3588_cpub_rates), false),
	RK3588_SCMI_CLOCK(SCMI_CLK_DDR, "scmi_clk_ddr", NULL, NULL, 0, false),
	RK3588_SCMI_CLOCK(SCMI_CLK_GPU, "scmi_clk_gpu", &clk_scmi_gpu_ops, rk3588_gpu_rates, ARRAY_SIZE(rk3588_gpu_rates), false),
	RK3588_SCMI_CLOCK(SCMI_CLK_NPU, "scmi_clk_npu", &clk_scmi_npu_ops, rk3588_gpu_rates, ARRAY_SIZE(rk3588_gpu_rates), false),
	RK3588_SCMI_CLOCK(SCMI_CLK_SBUS, "scmi_clk_sbus", &clk_scmi_sbus_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_PCLK_SBUS, "scmi_pclk_sbus", &clk_scmi_pclk_sbus_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_CCLK_SD, "scmi_cclk_sd", &clk_scmi_cclk_sdmmc_ops, rk3588_sdmmc_rates, ARRAY_SIZE(rk3588_sdmmc_rates), false),
	RK3588_SCMI_CLOCK(SCMI_DCLK_SD, "scmi_dclk_sd", &clk_scmi_dclk_sdmmc_ops, rk3588_sdmmc_rates, ARRAY_SIZE(rk3588_sdmmc_rates), false),
	RK3588_SCMI_CLOCK(SCMI_ACLK_SECURE_NS, "scmi_aclk_se_ns", &clk_scmi_aclk_secure_ns_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), false),
	RK3588_SCMI_CLOCK(SCMI_HCLK_SECURE_NS, "scmi_hclk_se_ns", &clk_scmi_hclk_secure_ns_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), false),
	RK3588_SCMI_CLOCK(SCMI_TCLK_WDT, "scmi_tclk_wdt", &clk_scmi_tclk_wdt_ops, NULL, 0, false),
	RK3588_SCMI_CLOCK(SCMI_KEYLADDER_CORE, "scmi_keylad_c", &clk_scmi_keyladder_core_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_KEYLADDER_RNG, "scmi_keylad_r", &clk_scmi_keyladder_rng_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_ACLK_SECURE_S, "scmi_aclk_se_s", &clk_scmi_aclk_secure_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_HCLK_SECURE_S, "scmi_hclk_se_s", &clk_scmi_hclk_secure_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_PCLK_SECURE_S, "scmi_pclk_se_s", &clk_scmi_pclk_secure_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_CRYPTO_RNG, "scmi_crypto_r", &clk_scmi_crypto_rng_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), false),
	RK3588_SCMI_CLOCK(SCMI_CRYPTO_CORE, "scmi_crypto_c", &clk_scmi_crypto_core_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), false),
	RK3588_SCMI_CLOCK(SCMI_CRYPTO_PKA, "scmi_crypto_p", &clk_scmi_crypto_pka_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), false),
	RK3588_SCMI_CLOCK(SCMI_SPLL, "scmi_spll", &clk_scmi_spll_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), false),
	RK3588_SCMI_CLOCK(SCMI_HCLK_SD, "scmi_hclk_sd", &clk_scmi_hclk_sd_ops, NULL, 0, false),
	RK3588_SCMI_CLOCK(SCMI_CRYPTO_RNG_S, "scmi_crypto_r_s", &clk_scmi_crypto_rng_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_CRYPTO_CORE_S, "scmi_crypto_c_s", &clk_scmi_crypto_core_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_CRYPTO_PKA_S, "scmi_crypto_p_s", &clk_scmi_crypto_pka_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_A_CRYPTO_S, "scmi_a_crypto_s", &clk_scmi_a_crypto_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_H_CRYPTO_S, "scmi_h_crypto_s", &clk_scmi_h_crypto_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_P_CRYPTO_S, "scmi_p_crypto_s", &clk_scmi_p_crypto_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_A_KEYLADDER_S, "scmi_a_keylad_s", &clk_scmi_a_keylad_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_H_KEYLADDER_S, "scmi_h_keylad_s", &clk_scmi_h_keylad_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_P_KEYLADDER_S, "scmi_p_keylad_s", &clk_scmi_p_keylad_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_TRNG_S, "scmi_trng_s", &clk_scmi_trng_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_H_TRNG_S, "scmi_h_trng_s", &clk_scmi_h_trng_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_P_OTPC_S, "scmi_p_otpc_s", &clk_scmi_p_otpc_s_ops, rk3588_sbus_rates, ARRAY_SIZE(rk3588_sbus_rates), true),
	RK3588_SCMI_CLOCK(SCMI_OTPC_S, "scmi_otpc_s", &clk_scmi_otpc_s_ops, NULL, 0, true),
	RK3588_SCMI_CLOCK(SCMI_OTP_PHY, "scmi_otp_phy", &clk_scmi_otp_phy_ops, NULL, 0, false),
	RK3588_SCMI_CLOCK(SCMI_OTPC_AUTO_RD, "scmi_otpc_rd", &clk_scmi_otpc_rd_ops, NULL, 0, false),
	RK3588_SCMI_CLOCK(SCMI_OTPC_ARB, "scmi_otpc_arb", &clk_scmi_otpc_arb_ops, NULL, 0, false),
};

size_t rockchip_scmi_clock_count(unsigned int agent_id __unused)
{
	return ARRAY_SIZE(clock_table);
}

rk_scmi_clock_t *rockchip_scmi_get_clock(uint32_t agent_id __unused,
					 uint32_t clock_id)
{
	rk_scmi_clock_t *table = NULL;

	if (clock_id < ARRAY_SIZE(clock_table))
		table = &clock_table[clock_id];

	if (table && !table->is_security)
		return table;
	else
		return NULL;
}

void pvtplls_suspend(void)
{
	clk_cpul_set_rate(408000000, PLL_SEL_NOR);
	clk_dsu_set_rate(408000000, PLL_SEL_NOR);
	clk_cpub01_set_rate(408000000, PLL_SEL_NOR);
	clk_cpub23_set_rate(408000000, PLL_SEL_NOR);
}

void pvtplls_resume(void)
{
	clk_cpul_set_rate(sys_clk_info.cpul_rate, PLL_SEL_AUTO);
	clk_dsu_set_rate(sys_clk_info.dsu_rate, PLL_SEL_AUTO);
	clk_cpub01_set_rate(sys_clk_info.cpub01_rate, PLL_SEL_AUTO);
	clk_cpub23_set_rate(sys_clk_info.cpub23_rate, PLL_SEL_AUTO);
}

void sys_reset_pvtplls_prepare(void)
{
	clk_gpu_set_rate(100000000, PLL_SEL_NOR);
	clk_npu_set_rate(100000000, PLL_SEL_NOR);
	clk_cpul_set_rate(408000000, PLL_SEL_NOR);
	clk_cpub01_set_rate(408000000, PLL_SEL_NOR);
	clk_cpub23_set_rate(408000000, PLL_SEL_NOR);
	clk_dsu_set_rate(408000000, PLL_SEL_NOR);
}

void rockchip_clock_init(void)
{
	/* set gpll src div to 0 for cpul */
	mmio_write_32(DSUCRU_BASE + CRU_CLKSEL_CON(5), CLKDIV_5BITS_SHF(0U, 9));
	/* set gpll src div to 0 for cpub01 */
	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(0),
		      CLKDIV_5BITS_SHF(0U, 1));
	/* set gpll src div to 0 for cpu23 */
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(0),
		      CLKDIV_5BITS_SHF(0U, 1));

	mmio_write_32(BIGCORE0CRU_BASE + CRU_CLKSEL_CON(2),
		      CPUB_PCLK_PATH_50M);
	mmio_write_32(BIGCORE1CRU_BASE + CRU_CLKSEL_CON(2),
		      CPUB_PCLK_PATH_50M);

	mmio_write_32(DSUCRU_BASE + DSUCRU_CLKSEL_CON(4),
		      CLKDIV_5BITS_SHF(5U, 0));
	mmio_write_32(DSUCRU_BASE + DSUCRU_CLKSEL_CON(4),
		      BITS_WITH_WMASK(PCLK_DSU_ROOT_SEL_GPLL,
				      PCLK_DSU_ROOT_SEL_MASK,
				      PCLK_DSU_ROOT_SEL_SHIFT));

	sys_clk_info.cpul_table = rk3588_cpul_pvtpll_table;
	sys_clk_info.cpul_rate_count = ARRAY_SIZE(rk3588_cpul_pvtpll_table);
	sys_clk_info.cpub01_table = rk3588_cpub0_pvtpll_table;
	sys_clk_info.cpub01_rate_count = ARRAY_SIZE(rk3588_cpub0_pvtpll_table);
	sys_clk_info.cpub23_table = rk3588_cpub1_pvtpll_table;
	sys_clk_info.cpub23_rate_count = ARRAY_SIZE(rk3588_cpub1_pvtpll_table);
	memcpy(sys_clk_info.cpub23_table, sys_clk_info.cpub01_table,
	       sys_clk_info.cpub01_rate_count * sizeof(*sys_clk_info.cpub01_table));
	sys_clk_info.gpu_table = rk3588_gpu_pvtpll_table;
	sys_clk_info.gpu_rate_count = ARRAY_SIZE(rk3588_gpu_pvtpll_table);
	sys_clk_info.npu_table = rk3588_npu_pvtpll_table;
	sys_clk_info.npu_rate_count = ARRAY_SIZE(rk3588_npu_pvtpll_table);
}
