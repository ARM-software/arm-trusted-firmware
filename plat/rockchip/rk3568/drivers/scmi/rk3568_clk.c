/*
 * Copyright (c) 2024-2025, Rockchip Electronics Co., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/delay_timer.h>
#include <drivers/scmi.h>

#include "otp.h"
#include <plat_private.h>
#include <platform_def.h>
#include <rk3568_clk.h>
#include <scmi_clock.h>

enum pll_type_sel {
	PLL_SEL_AUTO, /* all plls (normal pll or pvtpll) */
	PLL_SEL_PVT,
	PLL_SEL_NOR,
	PLL_SEL_AUTO_NOR /* all normal plls (apll/gpll/npll) */
};

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define RK3568_CPU_PVTPLL_CON0	0x10
#define RK3568_GPU_PVTPLL_CON0	0x700
#define RK3568_NPU_PVTPLL_CON0	0x740

#define GPLL_RATE		1188000000
#define MAX_RATE_TABLE		16

#define CLKDIV_5BITS_SHF0(div)	BITS_WITH_WMASK(div, 0x1f, 0)
#define CLKDIV_5BITS_SHF8(div)	BITS_WITH_WMASK(div, 0x1f, 8)

#define CLKDIV_4BITS_SHF0(div)	BITS_WITH_WMASK(div, 0xf, 0)
#define CLKDIV_2BITS_SHF4(div)	BITS_WITH_WMASK(div, 0x3, 4)

/* core_i: from gpll or apll */
#define CLK_CORE_I_SEL_APLL	WMSK_BIT(6)
#define CLK_CORE_I_SEL_GPLL	BIT_WITH_WMSK(6)

/* clk_core:
 * from normal pll(core_i: gpll or apll) path or direct pass from apll
 */
#define CLK_CORE_SEL_CORE_I	WMSK_BIT(7)
#define CLK_CORE_SEL_APLL	BIT_WITH_WMSK(7)

/* cpu clk: form clk_core or pvtpll */
#define CLK_CORE_NDFT_CLK_CORE		WMSK_BIT(15)
#define CLK_CORE_NDFT_CLK_PVTPLL	BIT_WITH_WMSK(15)

/* clk_core_ndft path */
#define CLK_CORE_PATH_NOR_GPLL	(CLK_CORE_I_SEL_GPLL | CLK_CORE_SEL_CORE_I)
#define CLK_CORE_PATH_NOR_APLL	(CLK_CORE_I_SEL_APLL | CLK_CORE_SEL_CORE_I)
#define CLK_CORE_PATH_DIR_APLL	(CLK_CORE_SEL_APLL) /* from apll directly*/

/* cpu clk path */
#define CPU_CLK_PATH_NOR_GPLL	(CLK_CORE_PATH_NOR_GPLL | \
				 CLK_CORE_NDFT_CLK_CORE)
#define CPU_CLK_PATH_NOR_APLL	(CLK_CORE_PATH_NOR_APLL | \
				 CLK_CORE_NDFT_CLK_CORE)
#define CPU_CLK_PATH_DIR_APLL	(CLK_CORE_PATH_DIR_APLL | \
				 CLK_CORE_NDFT_CLK_CORE)
#define CPU_CLK_PATH_PVTPLL	 CLK_CORE_NDFT_CLK_PVTPLL

/* dsu clk path */
#define SCLK_PATH_NOR_APLL	(BITS_WITH_WMASK(0, 0x3, 8) | WMSK_BIT(15))
#define SCLK_PATH_NOR_GPLL	(BITS_WITH_WMASK(0x1, 0x3, 8) | WMSK_BIT(15))
#define SCLK_PATH_NOR_NPLL	BITS_WITH_WMASK(0x2, 0x3, 8) | WMSK_BIT(15)
#define SCLK_PATH_DIR_NPLL	BIT_WITH_WMSK(15)

/* npu clk path */
#define CLK_NPU_SRC_NPLL	WMSK_BIT(6)
#define CLK_NPU_SRC_GPLL	BIT_WITH_WMSK(6)

#define CLK_NPU_NP5_SRC_NPLL	WMSK_BIT(7)
#define CLK_NPU_NP5_SRC_GPLL	BIT_WITH_WMSK(7)

#define NPU_PRE_CLK_SEL_PLL_SRC	WMSK_BIT(8)
#define NPU_PRE_CLK_SEL_NP5	BIT_WITH_WMSK(8)

#define CLK_NPU_MUX_PLL_SRC	WMSK_BIT(15)
#define CLK_NPU_MUX_PVTPLL	BIT_WITH_WMSK(15)

#define NPU_PRE_CLK_PATH_NPLL	(CLK_NPU_SRC_NPLL | NPU_PRE_CLK_SEL_PLL_SRC)
#define NPU_PRE_CLK_PATH_GPLL	(CLK_NPU_SRC_GPLL | NPU_PRE_CLK_SEL_PLL_SRC)
#define NPU_PRE_CLK_PATH_NP5_NPLL	(CLK_NPU_NP5_SRC_NPLL | \
					 NPU_PRE_CLK_SEL_NP5)
#define NPU_PRE_CLK_PATH_NP5_GPLL	(CLK_NPU_NP5_SRC_GPLL | \
					 NPU_PRE_CLK_SEL_NP5)

#define NPU_CLK_PATH_NOR_NPLL	(NPU_PRE_CLK_PATH_NPLL | CLK_NPU_MUX_PLL_SRC)
#define NPU_CLK_PATH_NOR_GPLL	(NPU_PRE_CLK_PATH_GPLL | CLK_NPU_MUX_PLL_SRC)
#define NPU_CLK_PATH_NP5_NPLL	(NPU_PRE_CLK_PATH_NP5_NPLL | \
				 CLK_NPU_MUX_PLL_SRC)
#define NPU_CLK_PATH_NP5_GPLL	(NPU_PRE_CLK_PATH_NP5_GPLL | \
				 CLK_NPU_MUX_PLL_SRC)
#define NPU_CLK_PATH_PVTPLL	CLK_NPU_MUX_PVTPLL

/* gpu clk path */
#define GPU_CLK_PATH_NOR_MPLL	(WMSK_BIT(11) | BITS_WITH_WMASK(0, 0x3, 6))
#define GPU_CLK_PATH_NOR_GPLL	(WMSK_BIT(11) | BITS_WITH_WMASK(0x1, 0x3, 6))
#define GPU_CLK_PATH_NOR_CPLL	(WMSK_BIT(11) | BITS_WITH_WMASK(0x2, 0x3, 6))
#define GPU_CLK_PATH_NOR_NPLL	(WMSK_BIT(11) | BITS_WITH_WMASK(0x3, 0x3, 6))
#define GPU_CLK_PATH_PVTPLL		BIT_WITH_WMSK(11)

#define PVTPLL_NEED(type, length)	(((type) == PLL_SEL_PVT || \
					  (type) == PLL_SEL_AUTO) && \
					 (length))

#define RK3568_CPU_OPP_INFO_OFFSET	(OTP_S_BYTE_SIZE + 54)
#define RK3568_GPU_OPP_INFO_OFFSET	(OTP_S_BYTE_SIZE + 60)
#define RK3568_NPU_OPP_INFO_OFFSET	(OTP_S_BYTE_SIZE + 66)

struct sys_clk_info_t {
	unsigned long cpu_rate;
	unsigned long gpu_rate;
	unsigned long npu_rate;
};

struct otp_opp_info {
	uint16_t min_freq;
	uint16_t max_freq;
	uint8_t volt;
	uint8_t length;
} __packed __aligned(2);

struct pvtpll_table {
	unsigned int rate;
	uint32_t refdiv;
	uint32_t fbdiv;
	uint32_t postdiv1;
	uint32_t postdiv2;
	uint32_t dsmpd;
	uint32_t frac;
	uint32_t length;
};

#define ROCKCHIP_CPU_PVTPLL(_rate, _refdiv, _fbdiv, _postdiv1,	\
			    _postdiv2, _dsmpd, _frac, _length)	\
{								\
	.rate	= _rate##U,					\
	.refdiv = _refdiv,					\
	.fbdiv = _fbdiv,					\
	.postdiv1 = _postdiv1,					\
	.postdiv2 = _postdiv2,					\
	.dsmpd = _dsmpd,					\
	.frac = _frac,						\
	.length = _length,					\
}

#define ROCKCHIP_GPU_PVTPLL(_rate, _length)			\
{								\
	.rate = _rate##U,					\
	.length = _length,					\
}

static struct pvtpll_table rk3568_cpu_pvtpll_table[] = {
	ROCKCHIP_CPU_PVTPLL(1992000000, 1, 83, 1, 1, 1, 0, 0x33),
	ROCKCHIP_CPU_PVTPLL(1800000000, 1, 75, 1, 1, 1, 0, 0x33),
	ROCKCHIP_CPU_PVTPLL(1608000000, 1, 67, 1, 1, 1, 0, 0x3b),
	ROCKCHIP_CPU_PVTPLL(1416000000, 1, 118, 2, 1, 1, 0, 0x43),
	ROCKCHIP_CPU_PVTPLL(1200000000, 1, 100, 2, 1, 1, 0, 0x53),
	ROCKCHIP_CPU_PVTPLL(1104000000, 1, 92, 2, 1, 1, 0, 0x53),
	ROCKCHIP_CPU_PVTPLL(1008000000, 1, 84, 2, 1, 1, 0, 0x5b),
	ROCKCHIP_CPU_PVTPLL(816000000, 1, 68, 2, 1, 1, 0, 0),
	ROCKCHIP_CPU_PVTPLL(600000000, 1, 100, 4, 1, 1, 0, 0),
	ROCKCHIP_CPU_PVTPLL(408000000, 1, 68, 2, 2, 1, 0, 0),
	ROCKCHIP_CPU_PVTPLL(312000000, 1, 78, 6, 1, 1, 0, 0),
	ROCKCHIP_CPU_PVTPLL(216000000, 1, 72, 4, 2, 1, 0, 0),
	{ /* sentinel */ },
};

static struct pvtpll_table rk3568_gpu_pvtpll_table[] = {
	/* rate_hz, length */
	ROCKCHIP_GPU_PVTPLL(800000000, 0x1db),
	ROCKCHIP_GPU_PVTPLL(700000000, 0x1db),
	ROCKCHIP_GPU_PVTPLL(600000000, 0x1db),
	ROCKCHIP_GPU_PVTPLL(400000000, 0),
	ROCKCHIP_GPU_PVTPLL(300000000, 0),
	ROCKCHIP_GPU_PVTPLL(200000000, 0),
	{ /* sentinel */ },
};

static struct pvtpll_table rk3568_npu_pvtpll_table[] = {
	/* rate_hz, length */
	ROCKCHIP_GPU_PVTPLL(1000000000, 0xd3),
	ROCKCHIP_GPU_PVTPLL(900000000, 0xd3),
	ROCKCHIP_GPU_PVTPLL(800000000, 0xd3),
	ROCKCHIP_GPU_PVTPLL(700000000, 0xdb),
	ROCKCHIP_GPU_PVTPLL(600000000, 0xfb),
	ROCKCHIP_GPU_PVTPLL(400000000, 0),
	ROCKCHIP_GPU_PVTPLL(300000000, 0),
	ROCKCHIP_GPU_PVTPLL(200000000, 0),
	{ /* sentinel */ },
};

static unsigned long rk3568_cpu_rates[] = {
	216000000, 312000000, 408000000, 816000000,
	1008000000, 1200000000, 1416000000, 1608000000,
	1800000000, 1992000000
};

static unsigned long rk3568_gpu_rates[] = {
	100000000, 200000000, 300000000, 400000000,
	500000000, 600000000, 700000000, 800000000,
	900000000, 1000000000, 1100000000, 1200000000
};

static struct sys_clk_info_t sys_clk_info;

static bool check_otp_ecc_ok(uint32_t addr)
{
	int i;

	for (i = 0; i < sizeof(struct otp_opp_info); i++) {
		if (rk_otp_ns_ecc_flag(addr + i))
			return false;
	}

	return true;
}

static void rk3568_adjust_pvtpll_table(struct pvtpll_table *pvtpll,
				       unsigned int count,
				       uint16_t min_freq,
				       uint16_t max_freq,
				       uint8_t length)
{
	uint16_t freq;
	uint8_t cur_length;
	int i;

	if (length > 31)
		return;

	for (i = 0; i < count; i++) {
		if (!pvtpll[i].length)
			continue;
		cur_length = (pvtpll[i].length >> 3) & 0x1f;

		/*
		 * Max value of length is 31, so adjust length to
		 * make sure (cur_length + length) <= 31.
		 */
		if ((cur_length + length) > 31)
			length = 31 - cur_length;
		freq = pvtpll[i].rate / 1000000;
		if ((freq >= min_freq) && (freq <= max_freq))
			pvtpll[i].length += (length << 3);
	}
}

static unsigned int
rockchip_get_pvtpll_length(struct pvtpll_table *table, int count,
			   unsigned long rate)
{
	int i;

	for (i = 0; i < count; i++) {
		if (rate == table[i].rate)
			return table[i].length;
	}
	return 0;
}

static struct pvtpll_table *rkclk_get_pll_config(unsigned int freq_hz)
{
	unsigned int rate_count = ARRAY_SIZE(rk3568_cpu_pvtpll_table);
	int i;

	for (i = 0; i < rate_count; i++) {
		if (freq_hz == rk3568_cpu_pvtpll_table[i].rate)
			return &rk3568_cpu_pvtpll_table[i];
	}
	return NULL;
}

static int rk3568_apll_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *div;
	int delay = 2400;

	div = rkclk_get_pll_config(rate);
	if (div == NULL)
		return SCMI_INVALID_PARAMETERS;

	if (PVTPLL_NEED(type, div->length)) {
		/* set pvtpll length */
		mmio_write_32(CPUGRF_BASE + RK3568_CPU_PVTPLL_CON0,
			      0xffff0000);
		udelay(1);
		mmio_write_32(CPUGRF_BASE + RK3568_CPU_PVTPLL_CON0,
			      0xffff0000 | div->length);
		udelay(1);
		/* set core mux pvtpll */
		mmio_write_32(CRU_BASE + RK3568_CLK_SEL(0),
			      CPU_CLK_PATH_PVTPLL);
	}

	/* pll enter slow mode */
	mmio_write_32(CRU_BASE + 0xc0,
		      (RK3568_PLL_MODE_MASK <<
		       (16 + RK3568_PLL_MODE_SHIFT)) |
		      (RK3568_PLL_MODE_SLOWMODE << RK3568_PLL_MODE_SHIFT));
	/* update pll values */
	mmio_write_32(CRU_BASE + RK3568_PLLCON(0),
		      (RK3568_PLLCON0_FBDIV_MASK <<
		       (16 + RK3568_PLLCON0_FBDIV_SHIFT)) |
		      (div->fbdiv << RK3568_PLLCON0_FBDIV_SHIFT));
	mmio_write_32(CRU_BASE + RK3568_PLLCON(0),
		      (RK3568_PLLCON0_POSTDIV1_MASK <<
		       (16 + RK3568_PLLCON0_POSTDIV1_SHIFT)) |
		      (div->postdiv1 << RK3568_PLLCON0_POSTDIV1_SHIFT));
	mmio_write_32(CRU_BASE + RK3568_PLLCON(1),
		      (RK3568_PLLCON1_REFDIV_MASK <<
		       (16 + RK3568_PLLCON1_REFDIV_SHIFT)) |
		      (div->refdiv << RK3568_PLLCON1_REFDIV_SHIFT));
	mmio_write_32(CRU_BASE + RK3568_PLLCON(1),
		      (RK3568_PLLCON1_POSTDIV2_MASK <<
		       (16 + RK3568_PLLCON1_POSTDIV2_SHIFT)) |
		      (div->postdiv2 << RK3568_PLLCON1_POSTDIV2_SHIFT));
	mmio_write_32(CRU_BASE + RK3568_PLLCON(1),
		      (RK3568_PLLCON1_DSMPD_MASK <<
		       (16 + RK3568_PLLCON1_DSMPD_SHIFT)) |
		      (div->dsmpd << RK3568_PLLCON1_DSMPD_SHIFT));

	/* wait for the pll to lock */
	while (delay > 0) {
		if (mmio_read_32(CRU_BASE + RK3568_PLLCON(1)) &
		    RK3568_PLLCON1_LOCK_STATUS)
			break;
		udelay(1);
		delay--;
	}
	if (delay == 0)
		INFO("%s:ERROR: PLL WAIT LOCK FAILED\n", __func__);

	/* pll enter normal mode */
	mmio_write_32(CRU_BASE + 0xc0,
		      (RK3568_PLL_MODE_MASK << (16 + RK3568_PLL_MODE_SHIFT)) |
		      (RK3568_PLL_MODE_NORMAL << RK3568_PLL_MODE_SHIFT));

	return 0;
}

static unsigned long rk3568_apll_get_rate(void)
{
	unsigned int fbdiv, postdiv1, refdiv, postdiv2;
	uint64_t rate64 = 24000000;
	int mode;

	mode = (mmio_read_32(CRU_BASE + 0xc0) >> RK3568_PLL_MODE_SHIFT) &
	       RK3568_PLL_MODE_MASK;

	if (mode == RK3568_PLL_MODE_SLOWMODE)
		return rate64;

	fbdiv = (mmio_read_32(CRU_BASE + RK3568_PLLCON(0)) >>
		 RK3568_PLLCON0_FBDIV_SHIFT) &
		RK3568_PLLCON0_FBDIV_MASK;
	postdiv1 = (mmio_read_32(CRU_BASE + RK3568_PLLCON(0)) >>
		    RK3568_PLLCON0_POSTDIV1_SHIFT) &
		   RK3568_PLLCON0_POSTDIV1_MASK;
	refdiv = (mmio_read_32(CRU_BASE + RK3568_PLLCON(1)) >>
		  RK3568_PLLCON1_REFDIV_SHIFT) &
		 RK3568_PLLCON1_REFDIV_MASK;
	postdiv2 = (mmio_read_32(CRU_BASE + RK3568_PLLCON(1)) >>
		    RK3568_PLLCON1_POSTDIV2_SHIFT) &
		   RK3568_PLLCON1_POSTDIV2_MASK;

	rate64 *= fbdiv;
	rate64 = rate64 / refdiv;
	rate64 = rate64 / postdiv1;
	rate64 = rate64 / postdiv2;

	return (unsigned long)rate64;
}

static int clk_cpu_set_rate(unsigned long rate, enum pll_type_sel type)
{
	int div = 0, ret = 0;

	if (!rate)
		return SCMI_INVALID_PARAMETERS;

	/* set clk core div to 3 */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(0),
		      CLKDIV_5BITS_SHF8(2) | CLKDIV_5BITS_SHF0(2));
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(1),
		      CLKDIV_5BITS_SHF8(2) | CLKDIV_5BITS_SHF0(2));
	/* set atcore/gicclk div */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(3),
		      CLKDIV_5BITS_SHF8(7) | CLKDIV_5BITS_SHF0(7));
	/* set pclk/periph div */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(4),
		      CLKDIV_5BITS_SHF8(9) | CLKDIV_5BITS_SHF0(9));

	/* set dsu div to 4 */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(2), CLKDIV_4BITS_SHF0(3));

	/* set core mux gpll */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(0), CPU_CLK_PATH_NOR_GPLL);
	/* set dsu mux gpll */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(2), SCLK_PATH_NOR_GPLL);

	/* set apll */
	ret = rk3568_apll_set_rate(rate, type);
	if (ret < 0)
		return ret;

	/* set t core mux apll */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(0), CLK_CORE_PATH_DIR_APLL);

	div = DIV_ROUND_UP(rate, 300000000);
	div = div - 1;
	/* set atcore/gicclk div */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(3),
		      CLKDIV_5BITS_SHF8(div) | CLKDIV_5BITS_SHF0(div));
	/* set pclk/periph div */
	div = DIV_ROUND_UP(rate, 300000000);
	div = div - 1;
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(4),
		CLKDIV_5BITS_SHF8(div) | CLKDIV_5BITS_SHF0(div));

	if (rate >= 1608000000) {
		/* set dsu mux npll */
		mmio_write_32(CRU_BASE + RK3568_CLK_SEL(2),
			      SCLK_PATH_DIR_NPLL);
		/* set dsu div to 1 */
		mmio_write_32(CRU_BASE + RK3568_CLK_SEL(2),
			      CLKDIV_4BITS_SHF0(0));
	} else {
		/* set dsu mux apll */
		mmio_write_32(CRU_BASE + RK3568_CLK_SEL(2),
			      SCLK_PATH_NOR_APLL);
		/* set dsu div to 2 */
		mmio_write_32(CRU_BASE + RK3568_CLK_SEL(2),
			      CLKDIV_4BITS_SHF0(1));
	}

	/* set clk core div to 1 */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(0),
		      CLKDIV_5BITS_SHF8(0) | CLKDIV_5BITS_SHF0(0));
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(1),
		      CLKDIV_5BITS_SHF8(0) | CLKDIV_5BITS_SHF0(0));
	return ret;
}

static int clk_scmi_cpu_set_rate(struct rk_scmi_clock *clock, unsigned long rate)
{
	int ret;

	ret = clk_cpu_set_rate(rate, PLL_SEL_AUTO);

	if (!ret)
		sys_clk_info.cpu_rate = rate;

	return ret;
}

static unsigned long clk_scmi_cpu_get_rate(struct rk_scmi_clock *clock)
{
	return rk3568_apll_get_rate();
}

static int clk_scmi_cpu_set_status(struct rk_scmi_clock *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_gpu_get_rate(struct rk_scmi_clock *clock)
{
	int div;

	if (mmio_read_32(CRU_BASE + RK3568_CLK_SEL(6)) & 0x0800) {
		return 0;
	} else {
		div = mmio_read_32(CRU_BASE + RK3568_CLK_SEL(6));
		div = div & 0x000f;
		return GPLL_RATE / (div + 1);
	}
}

static int clk_gpu_set_rate(unsigned long rate, enum pll_type_sel type)
{
	unsigned int length;
	int div;

	if (!rate)
		return SCMI_INVALID_PARAMETERS;

	/* set gpu div 6 */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(6), CLKDIV_4BITS_SHF0(5));
	/* set gpu mux gpll */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(6), GPU_CLK_PATH_NOR_GPLL);

	/* set pvtpll ring */
	length = rockchip_get_pvtpll_length(rk3568_gpu_pvtpll_table,
					    ARRAY_SIZE(rk3568_gpu_pvtpll_table),
					    rate);
	if (PVTPLL_NEED(type, length)) {
		mmio_write_32(GRF_BASE + RK3568_GPU_PVTPLL_CON0,
			      0xffff0000);
		udelay(1);
		mmio_write_32(GRF_BASE + RK3568_GPU_PVTPLL_CON0,
			      0xffff0000 | length);
		udelay(1);
		/* set gpu mux pvtpll */
		mmio_write_32(CRU_BASE + RK3568_CLK_SEL(6),
			      GPU_CLK_PATH_PVTPLL);
	}

	div = DIV_ROUND_UP(GPLL_RATE, rate);
	/* set gpu div */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(6), CLKDIV_4BITS_SHF0((div - 1)));

	return 0;
}

static int clk_scmi_gpu_set_rate(struct rk_scmi_clock *clock, unsigned long rate)
{
	int ret;

	ret = clk_gpu_set_rate(rate, PLL_SEL_AUTO);

	if (!ret)
		sys_clk_info.gpu_rate = rate;
	return ret;
}

static int clk_scmi_gpu_set_status(struct rk_scmi_clock *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_npu_get_rate(struct rk_scmi_clock *clock)
{
	int div;

	if (mmio_read_32(CRU_BASE + RK3568_CLK_SEL(7)) & 0x8000) {
		return 0;
	} else {
		div = mmio_read_32(CRU_BASE + RK3568_CLK_SEL(7));
		div = div & 0x000f;
		return GPLL_RATE / (div + 1);
	}
}

static int clk_npu_set_rate(unsigned long rate, enum pll_type_sel type)
{
	unsigned int length;
	int div;

	if (!rate)
		return SCMI_INVALID_PARAMETERS;

	/* set npu div 6 */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(7),
		      CLKDIV_2BITS_SHF4(2) | CLKDIV_4BITS_SHF0(5));
	/* set npu mux gpll */
	mmio_write_32(CRU_BASE + RK3568_CLK_SEL(7),
		      NPU_CLK_PATH_NOR_GPLL | CLK_NPU_NP5_SRC_GPLL);

	/* set pvtpll ring */
	length = rockchip_get_pvtpll_length(rk3568_npu_pvtpll_table,
					    ARRAY_SIZE(rk3568_npu_pvtpll_table),
					    rate);
	if (PVTPLL_NEED(type, length)) {
		mmio_write_32(GRF_BASE + RK3568_NPU_PVTPLL_CON0,
			      0xffff0000);
		udelay(1);
		mmio_write_32(GRF_BASE + RK3568_NPU_PVTPLL_CON0,
			      0xffff0000 | length);
		udelay(1);
		/* set npu mux pvtpll */
		mmio_write_32(CRU_BASE + RK3568_CLK_SEL(7),
			      NPU_CLK_PATH_PVTPLL);
	} else {
		div = DIV_ROUND_UP(GPLL_RATE, rate);
		/* set gpu div */
		mmio_write_32(CRU_BASE + RK3568_CLK_SEL(7),
			      CLKDIV_4BITS_SHF0((div - 1)));
	}

	return 0;
}

static int clk_scmi_npu_set_rate(struct rk_scmi_clock *clock, unsigned long rate)
{
	int ret;

	ret = clk_npu_set_rate(rate, PLL_SEL_AUTO);

	if (!ret)
		sys_clk_info.npu_rate = rate;

	return ret;
}

static int clk_scmi_npu_set_status(struct rk_scmi_clock *clock, bool status)
{
	return 0;
}

static const struct rk_clk_ops clk_scmi_cpu_ops = {
	.get_rate = clk_scmi_cpu_get_rate,
	.set_rate = clk_scmi_cpu_set_rate,
	.set_status = clk_scmi_cpu_set_status,
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

struct rk_scmi_clock clock_table[] = {
	{
		.id = 0,
		.name = "clk_scmi_cpu",
		.clk_ops = &clk_scmi_cpu_ops,
		.rate_table = rk3568_cpu_rates,
		.rate_cnt = ARRAY_SIZE(rk3568_cpu_rates),
	},
	{
		.id = 1,
		.name = "clk_scmi_gpu",
		.clk_ops = &clk_scmi_gpu_ops,
		.rate_table = rk3568_gpu_rates,
		.rate_cnt = ARRAY_SIZE(rk3568_gpu_rates),
	},
	{
		.id = 2,
		.name = "clk_scmi_npu",
		.clk_ops = &clk_scmi_npu_ops,
		.rate_table = rk3568_gpu_rates,
		.rate_cnt = ARRAY_SIZE(rk3568_gpu_rates),
	},
};

size_t rockchip_scmi_clock_count(unsigned int agent_id __unused)
{
	return ARRAY_SIZE(clock_table);
}

rk_scmi_clock_t *rockchip_scmi_get_clock(uint32_t agent_id __unused,
					 uint32_t clock_id)
{
	if (clock_id < ARRAY_SIZE(clock_table))
		return &clock_table[clock_id];

	return NULL;
}

void pvtplls_suspend(void)
{
	clk_gpu_set_rate(100000000, PLL_SEL_NOR);
	clk_npu_set_rate(100000000, PLL_SEL_NOR);
	clk_cpu_set_rate(408000000, PLL_SEL_NOR);
}

void pvtplls_resume(void)
{
	clk_cpu_set_rate(sys_clk_info.cpu_rate, PLL_SEL_AUTO);
	clk_gpu_set_rate(sys_clk_info.gpu_rate, PLL_SEL_AUTO);
	clk_npu_set_rate(sys_clk_info.npu_rate, PLL_SEL_AUTO);
}

void sys_reset_pvtplls_prepare(void)
{
	clk_gpu_set_rate(100000000, PLL_SEL_NOR);
	clk_npu_set_rate(100000000, PLL_SEL_NOR);
	clk_cpu_set_rate(408000000, PLL_SEL_NOR);
}

void rockchip_clock_init(void)
{
	struct otp_opp_info cpu_opp_info, gpu_opp_info, npu_opp_info;
	int ret;

	ret = rk_otp_read(RK3568_CPU_OPP_INFO_OFFSET,
			  sizeof(cpu_opp_info),
			  (uint16_t *)&cpu_opp_info,
			  true);
	if (ret || !check_otp_ecc_ok(RK3568_CPU_OPP_INFO_OFFSET)) {
		INFO("get cpu_opp_info fail, use default config!\n");
		cpu_opp_info.min_freq = 1008;
		cpu_opp_info.max_freq = 1992;
		cpu_opp_info.volt = 50;
		cpu_opp_info.length = 4;
	}
	if (cpu_opp_info.length) {
		INFO("adjust cpu pvtpll: min=%uM, max=%uM, length=%u\n",
		     cpu_opp_info.min_freq, cpu_opp_info.max_freq, cpu_opp_info.length);

		rk3568_adjust_pvtpll_table(rk3568_cpu_pvtpll_table,
					   ARRAY_SIZE(rk3568_cpu_pvtpll_table),
					   cpu_opp_info.min_freq,
					   cpu_opp_info.max_freq,
					   cpu_opp_info.length);
	}

	ret = rk_otp_read(RK3568_GPU_OPP_INFO_OFFSET,
			  sizeof(gpu_opp_info),
			  (uint16_t *)&gpu_opp_info,
			  true);
	if (ret || !check_otp_ecc_ok(RK3568_GPU_OPP_INFO_OFFSET)) {
		INFO("get gpu_opp_info fail, use default config!\n");
		gpu_opp_info.min_freq = 600;
		gpu_opp_info.max_freq = 800;
		gpu_opp_info.volt = 50;
		gpu_opp_info.length = 6;
	}
	if (gpu_opp_info.length) {
		INFO("adjust gpu pvtpll: min=%uM, max=%uM, length=%u\n",
		     gpu_opp_info.min_freq, gpu_opp_info.max_freq, gpu_opp_info.length);

		rk3568_adjust_pvtpll_table(rk3568_gpu_pvtpll_table,
					   ARRAY_SIZE(rk3568_gpu_pvtpll_table),
					   gpu_opp_info.min_freq,
					   gpu_opp_info.max_freq,
					   gpu_opp_info.length);
	}

	ret = rk_otp_read(RK3568_NPU_OPP_INFO_OFFSET,
			  sizeof(npu_opp_info),
			  (uint16_t *)&npu_opp_info,
			  true);
	if (ret || !check_otp_ecc_ok(RK3568_NPU_OPP_INFO_OFFSET)) {
		INFO("get npu_opp_info fail, use default config!\n");
		npu_opp_info.min_freq = 600;
		npu_opp_info.max_freq = 1000;
		npu_opp_info.volt = 50;
		npu_opp_info.length = 6;
	}
	if (npu_opp_info.length) {
		INFO("adjust npu pvtpll: min=%uM, max=%uM, length=%u\n",
		     npu_opp_info.min_freq, npu_opp_info.max_freq, npu_opp_info.length);

		rk3568_adjust_pvtpll_table(rk3568_npu_pvtpll_table,
					   ARRAY_SIZE(rk3568_npu_pvtpll_table),
					   npu_opp_info.min_freq,
					   npu_opp_info.max_freq,
					   npu_opp_info.length);
	}
}
