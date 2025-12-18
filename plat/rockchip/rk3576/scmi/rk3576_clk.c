// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#include <assert.h>
#include <errno.h>

#include <drivers/delay_timer.h>
#include <drivers/scmi.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <platform_def.h>

#include <plat_private.h>
#include <rk3576_clk.h>
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

#define RK3576_PVTPLL_RING_EN		0x00
#define RK3576_PVTPLL_RING0_LENGTH	0x04
#define RK3576_PVTPLL_RING1_LENGTH	0x08
#define RK3576_PVTPLL_RING2_LENGTH	0x0c
#define RK3576_PVTPLL_RING3_LENGTH	0x10
#define RK3576_PVTPLL_GCK_CFG		0x20
#define RK3576_PVTPLL_GCK_LEN		0x24
#define RK3576_PVTPLL_GCK_DIV		0x28
#define RK3576_PVTPLL_GCK_CAL_CNT	0x2c
#define RK3576_PVTPLL_GCK_REF_VAL	0x30
#define RK3576_PVTPLL_GCK_CFG_VAL	0x34
#define RK3576_PVTPLL_GCK_THR		0x38
#define RK3576_PVTPLL_GFREE_CON		0x3c
#define RK3576_PVTPLL_ADC_CFG		0x40
#define RK3576_PVTPLL_ADC_CAL_CNT	0x48
#define RK3576_PVTPLL_GCK_CNT		0x50
#define RK3576_PVTPLL_GCK_CNT_AVG	0x54
#define RK3576_PVTPLL_GCK_STATE		0x5c
#define RK3576_PVTPLL_ADC_CNT		0x60
#define RK3576_PVTPLL_ADC_CNT_AVG	0x68
#define RK3576_PVTPLL_VERSION		0x70
#define RK3576_PVTPLL_MAX_LENGTH	0x3f

#define GPLL_RATE			1188000000
#define CPLL_RATE			1000000000
#define SPLL_RATE			702000000
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

/* clk_core:
 * from normal pll(core_i: gpll or apll) path or direct pass from apll
 */

/* cpul clk path */
#define CPUL_CLK_PATH_NOR_GPLL		BITS_WITH_WMASK(1U, 0x3U, 12)
#define CPUL_CLK_PATH_NOR_LPLL		BITS_WITH_WMASK(0U, 0x3U, 12)
#define CPUL_CLK_PATH_NOR_PVTPLL	BITS_WITH_WMASK(2U, 0x3U, 12)

#define CPUL_CLK_PATH_LPLL		BITS_WITH_WMASK(0U, 0x3U, 6)
#define CPUL_CLK_PATH_DIR_LPLL		BITS_WITH_WMASK(2U, 0x3U, 6)
#define CPUL_CLK_PATH_PVTPLL		BITS_WITH_WMASK(1U, 0x3U, 6)

#define CPUL_PVTPLL_PATH_DEEP_SLOW	BITS_WITH_WMASK(0U, 0x1U, 13)
#define CPUL_PVTPLL_PATH_PVTPLL		BITS_WITH_WMASK(0x1, 0x1U, 13)

/* cpub clk path */
#define CPUB_CLK_PATH_NOR_GPLL		BITS_WITH_WMASK(1U, 0x3U, 12)
#define CPUB_CLK_PATH_NOR_BPLL		BITS_WITH_WMASK(0U, 0x3U, 12)
#define CPUB_CLK_PATH_NOR_PVTPLL	BITS_WITH_WMASK(2U, 0x3U, 12)

#define CPUB_CLK_PATH_BPLL		BITS_WITH_WMASK(0U, 0x3U, 14)
#define CPUB_CLK_PATH_DIR_BPLL		BITS_WITH_WMASK(2U, 0x3U, 14)
#define CPUB_CLK_PATH_PVTPLL		BITS_WITH_WMASK(1U, 0x3U, 14)

#define CPUB_PVTPLL_PATH_DEEP_SLOW	BITS_WITH_WMASK(0U, 0x1U, 5)
#define CPUB_PVTPLL_PATH_PVTPLL		BITS_WITH_WMASK(0x1, 0x1U, 5)

#define CPUB_PCLK_PATH_100M		BITS_WITH_WMASK(0U, 0x3U, 0)
#define CPUB_PCLK_PATH_50M		BITS_WITH_WMASK(1U, 0x3U, 0)
#define CPUB_PCLK_PATH_24M		BITS_WITH_WMASK(2U, 0x3U, 0)

/* cci clk path */
#define SCLK_CCI_PATH_XIN		BITS_WITH_WMASK(0U, 0x3U, 12)
#define SCLK_CCI_PATH_PVTPLL		BITS_WITH_WMASK(1U, 0x3U, 12)
#define SCLK_CCI_PATH_NOR_LPLL		BITS_WITH_WMASK(3U, 0x3U, 12)
#define SCLK_CCI_PATH_NOR_GPLL		BITS_WITH_WMASK(2U, 0x3U, 12)

#define CCI_PVTPLL_PATH_DEEP_SLOW	BITS_WITH_WMASK(0U, 0x1U, 14)
#define CCI_PVTPLL_PATH_PVTPLL		BITS_WITH_WMASK(1U, 0x1U, 14)

/* npu clk path */
#define NPU_CLK_PATH_NOR_GPLL		BITS_WITH_WMASK(0U, 0x7U, 7)
#define NPU_CLK_PATH_NOR_CPLL		BITS_WITH_WMASK(1U, 0x7U, 7)
#define NPU_CLK_PATH_NOR_AUPLL		BITS_WITH_WMASK(2U, 0x7U, 7)
#define NPU_CLK_PATH_NOR_SPLL		BITS_WITH_WMASK(3U, 0x7U, 7)

#define NPU_CLK_PATH_NOR_PLL		WMSK_BIT(15)
#define NPU_CLK_PATH_PVTPLL		BIT_WITH_WMSK(15)
#define NPU_PVTPLL_PATH_DEEP_SLOW	BITS_WITH_WMASK(0U, 0x1U, 9)
#define NPU_PVTPLL_PATH_PVTPLL		BITS_WITH_WMASK(1U, 0x1U, 9)

/* gpu clk path */
#define GPU_CLK_PATH_NOR_GPLL		BITS_WITH_WMASK(0U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_CPLL		BITS_WITH_WMASK(1U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_AUPLL		BITS_WITH_WMASK(2U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_SPLL		BITS_WITH_WMASK(3U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_LPLL		BITS_WITH_WMASK(4U, 0x7U, 5)
#define GPU_CLK_PATH_NOR_PLL		WMSK_BIT(8)
#define GPU_CLK_PATH_PVTPLL		BIT_WITH_WMSK(8)
#define GPU_PVTPLL_PATH_DEEP_SLOW	BITS_WITH_WMASK(0U, 0x1U, 9)
#define GPU_PVTPLL_PATH_PVTPLL		BITS_WITH_WMASK(1U, 0x1U, 9)

#define PVTPLL_NEED(type, length)	(((type) == PLL_SEL_PVT || \
					  (type) == PLL_SEL_AUTO) && \
					 (length))
/*
 * [0]:      set intermediate rate
 *           [1]: scaling up rate or scaling down rate
 * [1]:      add length for pvtpll
 *           [2:5]: length
 * [2]:      use low length for pvtpll
 * [3:5]:    reserved
 */
#define OPP_RATE_MASK			0x3f
#define OPP_INTERMEDIATE_RATE		BIT(0)
#define OPP_SCALING_UP_RATE		BIT(1)
#define OPP_ADD_LENGTH			BIT(1)
#define OPP_LENGTH_MASK			GENMASK_32(5, 2)
#define OPP_LENGTH_SHIFT		2
#define OPP_LENGTH_LOW			BIT(2)

#define PRATE(x) static const unsigned long const x[]
#define PINFO(x) static const uint32_t const x[]

PRATE(p_24m)			= { OSC_HZ };
PRATE(p_100m_24m)		= { 100 * MHz, OSC_HZ };
PRATE(p_350m_175m_116m_24m)	= { 350 * MHz, 175 * MHz, 116 * MHz, OSC_HZ };
PRATE(p_175m_116m_58m_24m)	= { 175 * MHz, 116 * MHz, 58 * MHz, OSC_HZ };
PRATE(p_116m_58m_24m)		= { 116 * MHz, 58 * MHz, OSC_HZ };
PRATE(p_pclk_secure_s)		= { PCLK_SECURE_S };
PRATE(p_hclk_secure_s)		= { HCLK_SECURE_S };
PRATE(p_aclk_secure_s)		= { ACLK_SECURE_S };
PRATE(p_hclk_vo0_s)		= { HCLK_VO0_S };
PRATE(p_pclk_vo0_s)		= { PCLK_VO0_S };
PRATE(p_hclk_vo1_s)		= { HCLK_VO1_S };
PRATE(p_pclk_vo1_s)		= { PCLK_VO1_S };

PINFO(clk_stimer0_root_info)	= { 0x27214004, 6, 1, 0, 0, 0, 0x27214028, 9 };
PINFO(clk_stimer1_root_info)	= { 0x27214004, 7, 1, 0, 0, 0, 0x2721402c, 1 };
PINFO(pclk_secure_s_info)	= { 0x27214004, 4, 2, 0, 0, 0, 0x27214028, 2 };
PINFO(hclk_secure_s_info)	= { 0x27214004, 2, 2, 0, 0, 0, 0x27214028, 1 };
PINFO(aclk_secure_s_info)	= { 0x27214004, 0, 2, 0, 0, 0, 0x27214028, 0 };
PINFO(clk_pka_crypto_s_info)	= { 0x27214004, 11, 2, 0, 0, 0, 0x27214030, 11 };
PINFO(hclk_vo1_s_info)		= { 0x27214010, 0, 2, 0, 0, 0, 0x27214038, 1 };
PINFO(pclk_vo1_s_info)		= { 0x27214010, 2, 2, 0, 0, 0, 0x27214038, 4 };
PINFO(hclk_vo0_s_info)		= { 0x27214018, 0, 2, 0, 0, 0, 0x2721403c, 1 };
PINFO(pclk_vo0_s_info)		= { 0x27214018, 2, 2, 0, 0, 0, 0x2721403c, 4 };
PINFO(pclk_klad_info)		= { 0, 0, 0, 0, 0, 0, 0x27214030, 7 };
PINFO(hclk_crypto_s_info)	= { 0, 0, 0, 0, 0, 0, 0x27214030, 8 };
PINFO(hclk_klad_info)		= { 0, 0, 0, 0, 0, 0, 0x27214030, 9 };
PINFO(aclk_crypto_s_info)	= { 0, 0, 0, 0, 0, 0, 0x27214030, 12 };
PINFO(hclk_trng_s_info)		= { 0, 0, 0, 0, 0, 0, 0x27214034, 0 };
PINFO(pclk_otpc_s_info)		= { 0, 0, 0, 0, 0, 0, 0x27214034, 3 };
PINFO(clk_otpc_s_info)		= { 0, 0, 0, 0, 0, 0, 0x27214034, 4 };
PINFO(pclk_wdt_s_info)		= { 0, 0, 0, 0, 0, 0, 0x27214034, 9 };
PINFO(tclk_wdt_s_info)		= { 0, 0, 0, 0, 0, 0, 0x27214034, 10 };
PINFO(pclk_hdcp1_trng_info)	= { 0, 0, 0, 0, 0, 0, 0x27214038, 0 };
PINFO(hclk_hdcp_key1_info)	= { 0, 0, 0, 0, 0, 0, 0x27214038, 3 };
PINFO(pclk_hdcp0_trng_info)	= { 0, 0, 0, 0, 0, 0, 0x2721403c, 0 };
PINFO(hclk_hdcp_key0_info)	= { 0, 0, 0, 0, 0, 0, 0x2721403c, 3 };
PINFO(pclk_edp_s_info)		= { 0, 0, 0, 0, 0, 0, 0x2721403c, 5 };

struct pvtpll_table {
	unsigned int rate;
	uint32_t length;
	uint32_t length_frac;
	uint32_t length_low;
	uint32_t length_low_frac;
	uint32_t ring_sel;
	uint32_t volt_sel_thr;
};

struct sys_clk_info_t {
	struct pvtpll_table *cpul_table;
	struct pvtpll_table *cci_table;
	struct pvtpll_table *cpub_table;
	struct pvtpll_table *gpu_table;
	struct pvtpll_table *npu_table;
	unsigned int cpul_rate_count;
	unsigned int cci_rate_count;
	unsigned int cpub_rate_count;
	unsigned int gpu_rate_count;
	unsigned int npu_rate_count;
	unsigned long cpul_rate;
	unsigned long cci_rate;
	unsigned long cpub_rate;
	unsigned long gpu_rate;
	unsigned long npu_rate;
};

struct otp_opp_info {
	uint16_t min_freq;
	uint16_t max_freq;
	uint8_t volt;
	uint8_t length;
} __packed;

#define RK3576_SCMI_CLOCK(_id, _name, _data, _table, _cnt, _is_s)	\
rk_scmi_clock_t _name = {						\
	.id	= _id,							\
	.name = #_name,							\
	.clk_ops = _data,						\
	.rate_table = _table,						\
	.rate_cnt = _cnt,						\
	.is_security = _is_s,						\
}

#define RK3576_SCMI_CLOCK_COM(_id, _name,  _parent_table, _info, _data,	\
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

#define ROCKCHIP_PVTPLL(_rate, _sel, _len, _len_frac)			\
{									\
	.rate = _rate##U,						\
	.ring_sel = _sel,						\
	.length = _len,							\
	.length_frac = _len_frac,					\
}

static struct pvtpll_table rk3576_cpul_pvtpll_table[] = {
	/* rate_hz, ring_sel, length, length_frac */
	ROCKCHIP_PVTPLL(2016000000, 0, 6, 0),
	ROCKCHIP_PVTPLL(1920000000, 0, 6, 1),
	ROCKCHIP_PVTPLL(1800000000, 0, 6, 1),
	ROCKCHIP_PVTPLL(1608000000, 0, 6, 1),
	ROCKCHIP_PVTPLL(1416000000, 0, 8, 0),
	ROCKCHIP_PVTPLL(1200000000, 0, 11, 0),
	ROCKCHIP_PVTPLL(1008000000, 0, 17, 0),
	ROCKCHIP_PVTPLL(816000000, 0, 26, 0),
	ROCKCHIP_PVTPLL(600000000, 0, 0, 0),
	ROCKCHIP_PVTPLL(408000000, 0, 0, 0),
	{ /* sentinel */ },
};

static struct pvtpll_table rk3576_cci_pvtpll_table[] = {
	/* cpul_rate_hz, ring_sel, length, length_frac */
	ROCKCHIP_PVTPLL(2016000000 / 2, 0, 27, 0),
	ROCKCHIP_PVTPLL(1920000000 / 2, 0, 28, 0),
	ROCKCHIP_PVTPLL(1800000000 / 2, 0, 28, 0),
	ROCKCHIP_PVTPLL(1608000000 / 2, 0, 30, 0),
	ROCKCHIP_PVTPLL(1416000000 / 2, 0, 34, 0),
	ROCKCHIP_PVTPLL(1200000000 / 2, 0, 34, 0),
	{ /* sentinel */ },
};

static struct pvtpll_table rk3576_cpub_pvtpll_table[] = {
	/* rate_hz, ring_sel, length, length_frac, length_low, length_low_frac */
	ROCKCHIP_PVTPLL(2208000000, 0, 4, 3),
	ROCKCHIP_PVTPLL(2112000000, 0, 5, 0),
	ROCKCHIP_PVTPLL(2016000000, 0, 5, 0),
	ROCKCHIP_PVTPLL(1800000000, 0, 5, 0),
	ROCKCHIP_PVTPLL(1608000000, 0, 5, 0),
	ROCKCHIP_PVTPLL(1416000000, 0, 7, 0),
	ROCKCHIP_PVTPLL(1200000000, 0, 11, 0),
	ROCKCHIP_PVTPLL(1008000000, 0, 17, 0),
	ROCKCHIP_PVTPLL(816000000, 0, 26, 0),
	ROCKCHIP_PVTPLL(600000000, 0, 0, 0),
	ROCKCHIP_PVTPLL(408000000, 0, 0, 0),
	{ /* sentinel */ },
};

static struct pvtpll_table rk3576_gpu_pvtpll_table[] = {
	/* rate_hz, ring_sel, length, length_frac, length_low, length_low_frac */
	ROCKCHIP_PVTPLL(900000000, 0, 20, 0),
	ROCKCHIP_PVTPLL(800000000, 0, 21, 0),
	ROCKCHIP_PVTPLL(700000000, 0, 21, 0),
	ROCKCHIP_PVTPLL(600000000, 0, 23, 0),
	ROCKCHIP_PVTPLL(500000000, 0, 32, 0),
	ROCKCHIP_PVTPLL(400000000, 0, 48, 0),
	ROCKCHIP_PVTPLL(300000000, 0, 63, 0),
	ROCKCHIP_PVTPLL(200000000, 0, 0, 0),
	{ /* sentinel */ },
};

static struct pvtpll_table rk3576_npu_pvtpll_table[] = {
	/* rate_hz, ring_sel, length, length_frac, length_low, length_low_frac */
	ROCKCHIP_PVTPLL(950000000, 0, 16, 0),
	ROCKCHIP_PVTPLL(900000000, 0, 17, 0),
	ROCKCHIP_PVTPLL(800000000, 0, 18, 0),
	ROCKCHIP_PVTPLL(700000000, 0, 22, 0),
	ROCKCHIP_PVTPLL(600000000, 0, 25, 0),
	ROCKCHIP_PVTPLL(500000000, 0, 35, 0),
	ROCKCHIP_PVTPLL(400000000, 0, 46, 0),
	ROCKCHIP_PVTPLL(300000000, 0, 63, 0),
	ROCKCHIP_PVTPLL(200000000, 0, 0, 0),
	{ /* sentinel */ },
};

static unsigned long rk3576_cpul_rates[] = {
	408000000, 600000000, 816000000, 1008000000,
	1200000000, 1416000000, 1608000000, 1800000000,
	2016000000, 2208000000, 2304000063
};

static unsigned long rk3576_cpub_rates[] = {
	408000000, 600000000, 816000000, 1008000000,
	1200000000, 1416000000, 1608000000, 1800000000,
	2016000000, 2208000000, 2304000000, 2400000063
};

static unsigned long rk3576_gpu_rates[] = {
	200000000, 300000000, 400000000, 500000000,
	600000000, 700000000, 800000000, 900000000,
	1000000063
};

static unsigned long rk3576_npu_rates[] = {
	200000000, 300000000, 400000000, 500000000,
	600000000, 700000000, 800000000, 900000000,
	1000000063
};

static unsigned long rk3576_common_rates[] = {
	400000, 24000000, 58000000, 100000000, 116000000, 175000000, 350000000,
};

static unsigned long rk3576_aclk_secure_s_rates[] = {
	116000000, 175000000, 350000000,
};

static int aclk_crypto_s_enable;
static int aclk_klad_enable;
static spinlock_t crypto_lock;
static bool cpub_suspended;

static struct sys_clk_info_t sys_clk_info;
static int clk_scmi_cci_set_rate(rk_scmi_clock_t *clock, unsigned long rate);

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

static int clk_scmi_set_low_length(struct pvtpll_table *pvtpll, unsigned int count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (pvtpll[i].length_low) {
			pvtpll[i].length = pvtpll[i].length_low;
			pvtpll[i].length_frac = pvtpll[i].length_low_frac;
		}
	}

	return 0;
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

	/*
	 *               |-\
	 * -----lpll-----|  \
	 *               |   \                                        |-\
	 * -----gpll-----|mux|--litcore unclean src--[div]--[autocs]--|  \
	 *               |   /                                        |   \
	 * --pvtpll src--|  /                           --pvtpll src--|mux|--litcore--
	 *               |-/                                          |   /
	 *                                       --litcore clean src--|  /
	 *                                                            |-/
	 */
	if (PVTPLL_NEED(type, pvtpll->length)) {
		/* set ring sel and length */
		mmio_write_32(PVTPLL_LITCORE_BASE + RK3576_PVTPLL_GCK_LEN,
			      0x1dff0000 |
			      (pvtpll->ring_sel << 10) |
			      (pvtpll->length << 2) |
			      (pvtpll->length_frac));
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(PVTPLL_LITCORE_BASE + RK3576_PVTPLL_GCK_CAL_CNT, 0x18);
		/* enable pvtpll */
		mmio_write_32(PVTPLL_LITCORE_BASE + RK3576_PVTPLL_GCK_CFG, 0x00220022);
		/* start pvtpll */
		mmio_write_32(PVTPLL_LITCORE_BASE + RK3576_PVTPLL_GCK_CFG,  0x00230023);

		/* set pvtpll_src parent from 24MHz/32KHz to pvtpll */
		mmio_write_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(1),
			      CPUL_PVTPLL_PATH_PVTPLL);

		/* set litcore unclean_src parent to pvtpll_src */
		mmio_write_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(0),
			      CPUL_CLK_PATH_NOR_PVTPLL);
		/*
		 * set litcore parent from pvtpll_src to unclean_src,
		 * because autocs is on litcore unclean_src.
		 */
		mmio_write_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(1),
			      CPUL_CLK_PATH_LPLL);
		/* set litcore unclean_src div to 0 */
		mmio_write_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(0),
			      CLKDIV_5BITS_SHF(0, 7));

		return 0;
	}
	/* set litcore unclean_src div */
	div = DIV_ROUND_UP(GPLL_RATE, rate) - 1;
	mmio_write_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(0),
		      CLKDIV_5BITS_SHF(div, 7));
	/* set litcore unclean_src parent to gpll */
	mmio_write_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(0),
		      CPUL_CLK_PATH_NOR_GPLL);
	/* set litcore parent to unclean_src */
	mmio_write_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(1),
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
		ret = clk_scmi_cci_set_rate(clock, rate / 2);
	}

	return ret;
}

static unsigned long rk3576_lpll_get_rate(void)
{
	unsigned int m, p, s, k;
	uint64_t rate64 = 24000000, postdiv;
	int mode;

	mode = mmio_read_32(LITTLE_CRU_BASE + CRU_MODE_CON) &
	       0x3;

	if (mode == 0)
		return rate64;

	m = (mmio_read_32(CCI_CRU_BASE + CRU_PLL_CON(16)) >>
		 CRU_PLLCON0_M_SHIFT) &
		CRU_PLLCON0_M_MASK;
	p = (mmio_read_32(CCI_CRU_BASE + CRU_PLL_CON(17)) >>
		    CRU_PLLCON1_P_SHIFT) &
		   CRU_PLLCON1_P_MASK;
	s = (mmio_read_32(CCI_CRU_BASE + CRU_PLL_CON(17)) >>
		  CRU_PLLCON1_S_SHIFT) &
		 CRU_PLLCON1_S_MASK;
	k = (mmio_read_32(CCI_CRU_BASE + CRU_PLL_CON(18)) >>
		    CRU_PLLCON2_K_SHIFT) &
		   CRU_PLLCON2_K_MASK;

	rate64 *= m;
	rate64 = rate64 / p;

	if (k != 0) {
		/* fractional mode */
		uint64_t frac_rate64 = 24000000 * k;

		postdiv = p * 65536;
		frac_rate64 = frac_rate64 / postdiv;
		rate64 += frac_rate64;
	}
	rate64 = rate64 >> s;

	return (unsigned long)rate64;
}

static unsigned long clk_scmi_cpul_get_rate(rk_scmi_clock_t *clock)
{
	int src, div;

	src = mmio_read_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(1)) & 0x00c0;
	src = src >> 6;
	if (src == 1)
		return sys_clk_info.cpul_rate;

	src = mmio_read_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(0)) & 0x3000;
	src = src >> 12;
	div = mmio_read_32(LITTLE_CRU_BASE + LCORE_CRU_CLKSEL_CON(6)) & 0x0f80;
	div = div >> 7;
	switch (src) {
	case 0:
		return rk3576_lpll_get_rate();
	case 1:
		/* Make the return rate is equal to the set rate */
		if (sys_clk_info.cpul_rate != 0)
			return sys_clk_info.cpul_rate;
		else
			return GPLL_RATE / (div + 1);
	case 2:
		return sys_clk_info.cpul_rate;
	default:
		return 0;
	}
}

static int clk_scmi_cpul_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static int clk_cpub_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *pvtpll;
	int div;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	pvtpll = rkclk_get_pvtpll_config(sys_clk_info.cpub_table,
					 sys_clk_info.cpub_rate_count, rate);
	if (pvtpll == NULL)
		return SCMI_INVALID_PARAMETERS;

	/*
	 *               |-\
	 * -----bpll-----|  \
	 *               |   \                                        |-\
	 * -----gpll-----|mux|--bigcore unclean src--[div]--[autocs]--|  \
	 *               |   /                                        |   \
	 * --pvtpll src--|  /                           --pvtpll src--|mux|--bigcore--
	 *               |-/                                          |   /
	 *                                       --bigcore clean src--|  /
	 *                                                            |-/
	 */
	if (PVTPLL_NEED(type, pvtpll->length) != 0) {
		/* set ring sel and length */
		mmio_write_32(PVTPLL_BIGCORE_BASE + RK3576_PVTPLL_GCK_LEN,
			      0x1dff0000 |
			      (pvtpll->ring_sel << 10) |
			      (pvtpll->length << 2) |
			      (pvtpll->length_frac));
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(PVTPLL_BIGCORE_BASE + RK3576_PVTPLL_GCK_CAL_CNT, 0x18);
		/* enable pvtpll */
		mmio_write_32(PVTPLL_BIGCORE_BASE + RK3576_PVTPLL_GCK_CFG, 0x00220022);
		/* start pvtpll */
		mmio_write_32(PVTPLL_BIGCORE_BASE + RK3576_PVTPLL_GCK_CFG, 0x00230023);

		/* set pvtpll_src parent from 24MHz/32KHz to pvtpll */
		mmio_write_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(2),
			      CPUB_PVTPLL_PATH_PVTPLL);

		/* set bigcore unclean_src parent to pvtpll_src */
		mmio_write_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(1),
			      CPUB_CLK_PATH_NOR_PVTPLL);
		/*
		 * set bigcore parent from pvtpll_src to unclean_src,
		 * because autocs is on bigcore unclean_src.
		 */
		mmio_write_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(1),
			      CPUB_CLK_PATH_BPLL);

		/* set bigcore unclean_src div to 0 */
		mmio_write_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(1),
			      CLKDIV_5BITS_SHF(0, 7));

		return 0;
	}

	/* set bigcore unclean_src div */
	div = DIV_ROUND_UP(GPLL_RATE, rate) - 1;
	mmio_write_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(1),
		      CLKDIV_5BITS_SHF(div, 7));
	/* set bigcore unclean_src parent to gpll */
	mmio_write_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(1),
		      CPUB_CLK_PATH_NOR_GPLL);
	/* set bigcore parent to unclean_src */
	mmio_write_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(1),
		      CPUB_CLK_PATH_BPLL);

	return 0;
}

static int clk_scmi_cpub_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	if ((rate & OPP_LENGTH_LOW) != 0) {
		clk_scmi_set_low_length(sys_clk_info.cpub_table,
					sys_clk_info.cpub_rate_count);
		return 0;
	}

	ret = clk_cpub_set_rate(rate, PLL_SEL_AUTO);
	if (ret == 0)
		sys_clk_info.cpub_rate = rate;

	return ret;
}

static unsigned long rk3576_bpll_get_rate(void)
{
	unsigned int m, p, s, k;
	uint64_t rate64 = 24000000, postdiv;
	int mode;

	mode = mmio_read_32(CRU_BASE + CRU_MODE_CON) &
	       0x3;

	if (mode == 0)
		return rate64;

	m = (mmio_read_32(CRU_BASE + CRU_PLL_CON(0)) >>
		 CRU_PLLCON0_M_SHIFT) &
		CRU_PLLCON0_M_MASK;
	p = (mmio_read_32(CRU_BASE + CRU_PLL_CON(1)) >>
		    CRU_PLLCON1_P_SHIFT) &
		   CRU_PLLCON1_P_MASK;
	s = (mmio_read_32(CRU_BASE + CRU_PLL_CON(1)) >>
		  CRU_PLLCON1_S_SHIFT) &
		 CRU_PLLCON1_S_MASK;
	k = (mmio_read_32(CRU_BASE + CRU_PLL_CON(2)) >>
		    CRU_PLLCON2_K_SHIFT) &
		   CRU_PLLCON2_K_MASK;

	rate64 *= m;
	rate64 = rate64 / p;

	if (k != 0) {
		/* fractional mode */
		uint64_t frac_rate64 = 24000000 * k;

		postdiv = p * 65536;
		frac_rate64 = frac_rate64 / postdiv;
		rate64 += frac_rate64;
	}
	rate64 = rate64 >> s;

	return (unsigned long)rate64;
}

static unsigned long clk_scmi_cpub_get_rate(rk_scmi_clock_t *clock)
{
	int value, src, div;

	if (cpub_suspended != 0)
		return sys_clk_info.cpub_rate;

	value = mmio_read_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(1));
	src = (value & 0xc000) >> 14;
	if (src == 1)
		return sys_clk_info.cpub_rate;

	value = mmio_read_32(BIGCORE_CRU_BASE + BCORE_CRU_CLKSEL_CON(1));
	src = (value & 0x3000) >> 12;
	div = (value & 0x0f80) >> 7;
	switch (src) {
	case 0:
		return rk3576_bpll_get_rate();
	case 1:
		/* Make the return rate is equal to the set rate */
		if (sys_clk_info.cpub_rate != 0)
			return sys_clk_info.cpub_rate;
		else
			return GPLL_RATE / (div + 1);
	case 2:
		return sys_clk_info.cpub_rate;
	default:
		return 0;
	}
}

static int clk_scmi_cpub_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_cci_get_rate(rk_scmi_clock_t *clock)
{
	int src, div;

	src = mmio_read_32(CCI_CRU_BASE + CCICRU_CLKSEL_CON(4)) & 0x3000;
	src = src >> 12;
	if (src == 1)
		return sys_clk_info.cci_rate;

	div = mmio_read_32(CCI_CRU_BASE + CCICRU_CLKSEL_CON(4)) & 0xf80;
	div = div >> 7;
	switch (src) {
	case 0:
		return OSC_HZ;
	case 1:
		return sys_clk_info.cci_rate;
	case 2:
		return GPLL_RATE / (div + 1);
	case 3:
		return rk3576_lpll_get_rate() / (div + 1);
	default:
		return 0;
	}
}

static int clk_cci_set_rate(unsigned long rate, enum pll_type_sel type)
{
	struct pvtpll_table *pvtpll;
	uint32_t pvtpll_en = 0;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	pvtpll = rkclk_get_pvtpll_config(sys_clk_info.cci_table,
					 sys_clk_info.cci_rate_count, rate);

	/* set pvtpll */
	if ((pvtpll != 0) && (PVTPLL_NEED(type, pvtpll->length) != 0)) {
		/* set ring sel and length */
		mmio_write_32(PVTPLL_CCI_BASE + RK3576_PVTPLL_GCK_LEN,
			      0x1dff0000 |
			      (pvtpll->ring_sel << 10) |
			      (pvtpll->length << 2) |
			      (pvtpll->length_frac));
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(PVTPLL_CCI_BASE + RK3576_PVTPLL_GCK_CAL_CNT, 0x18);
		/* enable pvtpll */
		pvtpll_en = mmio_read_32(PVTPLL_CCI_BASE + RK3576_PVTPLL_GCK_CFG);
		if (pvtpll_en && 0x22 != 0x22)
			mmio_write_32(PVTPLL_CCI_BASE + RK3576_PVTPLL_GCK_CFG, 0x00220022);
		/* start pvtpll */
		mmio_write_32(PVTPLL_CCI_BASE + RK3576_PVTPLL_GCK_CFG, 0x00230023);

		/* set cci mux pvtpll */
		mmio_write_32(CCI_CRU_BASE + CCICRU_CLKSEL_CON(4),
			      CCI_PVTPLL_PATH_PVTPLL);
		mmio_write_32(CCI_CRU_BASE + CCICRU_CLKSEL_CON(4),
			      SCLK_CCI_PATH_PVTPLL);
		mmio_write_32(CCI_CRU_BASE + CCICRU_CLKSEL_CON(4),
			      CLKDIV_5BITS_SHF(0, 7));
		sys_clk_info.cci_rate = rate;
		return 0;
	}
	sys_clk_info.cci_rate = 594000000;
	/* set cci div */
	mmio_write_32(CCI_CRU_BASE + CCICRU_CLKSEL_CON(4),
		      CLKDIV_5BITS_SHF(1, 7));
	/* set cci mux gpll */
	mmio_write_32(CCI_CRU_BASE + CCICRU_CLKSEL_CON(4),
		      SCLK_CCI_PATH_NOR_GPLL);

	return 0;
}

static int clk_scmi_cci_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	return clk_cci_set_rate(rate, PLL_SEL_AUTO);
}

static int clk_scmi_cci_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static unsigned long clk_scmi_gpu_get_rate(rk_scmi_clock_t *clock)
{
	int div, src;

	if ((mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(165)) & 0x100) != 0)
		return sys_clk_info.gpu_rate;

	div = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(165)) & 0x1f;
	src = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(165)) & 0x00e0;
	src = src >> 5;
	switch (src) {
	case 0:
		/* Make the return rate is equal to the set rate */
		if (sys_clk_info.gpu_rate != 0)
			return sys_clk_info.gpu_rate;
		else
			return GPLL_RATE / (div + 1);
	case 1:
		return CPLL_RATE / (div + 1);
	case 2:
		return AUPLL_RATE / (div + 1);
	case 3:
		return SPLL_RATE / (div + 1);
	case 4:
		return rk3576_lpll_get_rate() / (div + 1);
	default:
		return 0;
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

	if (PVTPLL_NEED(type, pvtpll->length) != 0) {
		/* set ring sel and length */
		mmio_write_32(PVTPLL_GPU_BASE + RK3576_PVTPLL_GCK_LEN,
			      0x1dff0000 |
			      (pvtpll->ring_sel << 10) |
			      (pvtpll->length << 2) |
			      (pvtpll->length_frac));
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(PVTPLL_GPU_BASE + RK3576_PVTPLL_GCK_CAL_CNT, 0x18);
		/* enable pvtpll */
		mmio_write_32(PVTPLL_GPU_BASE + RK3576_PVTPLL_GCK_CFG, 0x00220022);
		/* start pvtpll */
		mmio_write_32(PVTPLL_GPU_BASE + RK3576_PVTPLL_GCK_CFG, 0x00230023);
		/* set gpu mux pvtpll */
		mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(165),
			      GPU_PVTPLL_PATH_PVTPLL);
		mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(165),
			      GPU_CLK_PATH_PVTPLL);
		return 0;
	}

	/* set gpu div */
	div = DIV_ROUND_UP(GPLL_RATE, rate);
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(165),
		      CLKDIV_5BITS_SHF(div - 1, 0));
	/* set gpu mux gpll */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(165),
		      GPU_CLK_PATH_NOR_GPLL);
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(165),
		      GPU_CLK_PATH_NOR_PLL);

	return 0;
}

static int clk_scmi_gpu_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	if ((rate & OPP_LENGTH_LOW) != 0) {
		clk_scmi_set_low_length(sys_clk_info.gpu_table,
					sys_clk_info.gpu_rate_count);
		return 0;
	}

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
	int div, src, div_src;

	if ((mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(86)) & 0x8000) != 0)
		return sys_clk_info.npu_rate;

	div_src = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(86)) & 0x07c;
	div_src = div_src >> 2;
	src = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(86)) & 0x0180;
	src = src >> 7;
	div = mmio_read_32(CRU_BASE + CRU_CLKSEL_CON(86)) & 0x7c00;
	div = div >> 10;
	switch (src) {
	case 0:
		/* Make the return rate is equal to the set rate */
		if (sys_clk_info.npu_rate != 0)
			return sys_clk_info.npu_rate;
		else
			return GPLL_RATE / (div_src + 1)  / (div + 1);
	case 1:
		return CPLL_RATE / (div_src + 1)  / (div + 1);
	case 2:
		return AUPLL_RATE / (div_src + 1)  / (div + 1);
	case 3:
		return SPLL_RATE / (div_src + 1)  / (div + 1);
	default:
		return 0;
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

	if (PVTPLL_NEED(type, pvtpll->length) != 0) {
		/* set ring sel and length */
		mmio_write_32(PVTPLL_NPU_BASE + RK3576_PVTPLL_GCK_LEN,
			      0x1dff0000 |
			      (pvtpll->ring_sel << 10) |
			      (pvtpll->length << 2) |
			      (pvtpll->length_frac));
		/* set cal cnt = 24, T = 1us */
		mmio_write_32(PVTPLL_NPU_BASE + RK3576_PVTPLL_GCK_CAL_CNT, 0x18);
		/* enable pvtpll */
		mmio_write_32(PVTPLL_NPU_BASE + RK3576_PVTPLL_GCK_CFG, 0x00220022);
		/* start pvtpll */
		mmio_write_32(PVTPLL_NPU_BASE + RK3576_PVTPLL_GCK_CFG, 0x00230023);
		/* set npu mux pvtpll */
		mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(86),
			      NPU_PVTPLL_PATH_PVTPLL);
		mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(86),
			      NPU_CLK_PATH_PVTPLL);
		mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(86),
			      CLKDIV_5BITS_SHF(0, 10));
		return 0;
	}

	/* set npu div */
	div = DIV_ROUND_UP(GPLL_RATE, rate);
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(86),
		      CLKDIV_5BITS_SHF(div - 1, 2));
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(86),
		      CLKDIV_5BITS_SHF(0, 10));
	/* set npu mux gpll */
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(86),
		      NPU_CLK_PATH_NOR_GPLL);
	mmio_write_32(CRU_BASE + CRU_CLKSEL_CON(86),
		      NPU_CLK_PATH_NOR_PLL);

	return 0;
}

static int clk_scmi_npu_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	int ret;

	if (rate == 0)
		return SCMI_INVALID_PARAMETERS;

	if ((rate & OPP_LENGTH_LOW) != 0) {
		clk_scmi_set_low_length(sys_clk_info.npu_table,
					sys_clk_info.npu_rate_count);
		return 0;
	}

	ret = clk_npu_set_rate(rate, PLL_SEL_AUTO);
	if (ret == 0)
		sys_clk_info.npu_rate = rate;

	return ret;
}

static int clk_scmi_npu_set_status(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

int clk_scmi_crypto_set_status(rk_scmi_clock_t *clock, bool status)
{
	spin_lock(&crypto_lock);

	if (clock->id == ACLK_CRYPTO_S)
		aclk_crypto_s_enable = status;
	else
		aclk_klad_enable = status;

	if ((aclk_crypto_s_enable != 0) || (aclk_klad_enable != 0))
		clk_scmi_common_set_status(clock, 1);
	else
		clk_scmi_common_set_status(clock, 0);

	spin_unlock(&crypto_lock);
	return 0;
}

static int clk_scmi_common_set_status_critical(rk_scmi_clock_t *clock, bool status)
{
	return 0;
}

static const struct rk_clk_ops clk_scmi_cpul_ops = {
	.get_rate = clk_scmi_cpul_get_rate,
	.set_rate = clk_scmi_cpul_set_rate,
	.set_status = clk_scmi_cpul_set_status,
};

static const struct rk_clk_ops clk_scmi_cci_ops = {
	.get_rate = clk_scmi_cci_get_rate,
	.set_rate = clk_scmi_cci_set_rate,
	.set_status = clk_scmi_cci_set_status,
};

static const struct rk_clk_ops clk_scmi_cpub_ops = {
	.get_rate = clk_scmi_cpub_get_rate,
	.set_rate = clk_scmi_cpub_set_rate,
	.set_status = clk_scmi_cpub_set_status,
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

static const struct rk_clk_ops clk_scmi_ops_com_critical = {
	.get_rate = clk_scmi_common_get_rate,
	.set_rate = clk_scmi_common_set_rate,
	.set_status = clk_scmi_common_set_status_critical,
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

static const struct rk_clk_ops clk_scmi_ops_crypto = {
	.get_rate = clk_scmi_common_get_rate,
	.set_status = clk_scmi_crypto_set_status,
};

RK3576_SCMI_CLOCK(ARMCLK_L, scmi_armclkl, &clk_scmi_cpul_ops, rk3576_cpul_rates, ARRAY_SIZE(rk3576_cpul_rates), false);
RK3576_SCMI_CLOCK(ACLK_CCI_ROOT, scmi_aclk_cci, &clk_scmi_cci_ops, rk3576_cpul_rates, ARRAY_SIZE(rk3576_cpul_rates), false);
RK3576_SCMI_CLOCK(ARMCLK_B, scmi_armclkb, &clk_scmi_cpub_ops, rk3576_cpub_rates, ARRAY_SIZE(rk3576_cpub_rates), false);
RK3576_SCMI_CLOCK(CLK_GPU, scmi_clk_gpu, &clk_scmi_gpu_ops, rk3576_gpu_rates, ARRAY_SIZE(rk3576_gpu_rates), false);
RK3576_SCMI_CLOCK(CLK_RKNN_DSU0, scmi_clk_npu, &clk_scmi_npu_ops, rk3576_npu_rates, ARRAY_SIZE(rk3576_npu_rates), false);
RK3576_SCMI_CLOCK_COM(CLK_STIMER0_ROOT, clk_stimer0_rt, p_100m_24m, clk_stimer0_root_info, &clk_scmi_ops_com_critical, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(CLK_STIMER1_ROOT, clk_stimer1_rt, p_100m_24m, clk_stimer1_root_info, &clk_scmi_ops_com_critical, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(PCLK_SECURE_S, pclk_secure_s, p_116m_58m_24m, pclk_secure_s_info, &clk_scmi_ops_com_critical, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(HCLK_SECURE_S, hclk_secure_s, p_175m_116m_58m_24m, hclk_secure_s_info, &clk_scmi_ops_com_critical, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(ACLK_SECURE_S, aclk_secure_s, p_350m_175m_116m_24m, aclk_secure_s_info, &clk_scmi_ops_com_critical, rk3576_aclk_secure_s_rates, false, false);
RK3576_SCMI_CLOCK_COM(CLK_PKA_CRYPTO_S, clk_pka_crypt_s, p_350m_175m_116m_24m, clk_pka_crypto_s_info, &clk_scmi_ops_com, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(HCLK_VO1_S, hclk_vo1_s, p_175m_116m_58m_24m, hclk_vo1_s_info, &clk_scmi_ops_com_critical, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(PCLK_VO1_S, pclk_vo1_s, p_116m_58m_24m, pclk_vo1_s_info, &clk_scmi_ops_com_critical, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(HCLK_VO0_S, hclk_vo0_s, p_175m_116m_58m_24m, hclk_vo0_s_info, &clk_scmi_ops_com_critical, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(PCLK_VO0_S, pclk_vo0_s, p_116m_58m_24m, pclk_vo0_s_info, &clk_scmi_ops_com_critical, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(PCLK_KLAD, pclk_klad, p_pclk_secure_s, pclk_klad_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(HCLK_CRYPTO_S, hclk_crypto_s, p_hclk_secure_s, hclk_crypto_s_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(HCLK_KLAD, hclk_klad, p_hclk_secure_s, hclk_klad_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(ACLK_CRYPTO_S, aclk_crypto_s, p_aclk_secure_s, aclk_crypto_s_info, &clk_scmi_ops_crypto, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(HCLK_TRNG_S, hclk_trng_s, p_hclk_secure_s, hclk_trng_s_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(PCLK_OTPC_S, plk_otpc_s, p_pclk_secure_s, pclk_otpc_s_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(CLK_OTPC_S, clk_otpc_s, p_24m, clk_otpc_s_info, &clk_scmi_ops_gate, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(PCLK_WDT_S, pclk_wdt_s, p_pclk_secure_s, pclk_wdt_s_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(TCLK_WDT_S, tclk_wdt_s, p_24m, tclk_wdt_s_info, &clk_scmi_ops_gate, rk3576_common_rates, false, true);
RK3576_SCMI_CLOCK_COM(PCLK_HDCP0_TRNG, pclk_hdcp0_trng, p_pclk_vo0_s, pclk_hdcp0_trng_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(PCLK_HDCP1_TRNG, pclk_hdcp1_trng, p_pclk_vo1_s, pclk_hdcp1_trng_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(HCLK_HDCP_KEY0, hclk_hdcp_key0, p_hclk_vo0_s, hclk_hdcp_key0_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(HCLK_HDCP_KEY1, hclk_hdcp_key1, p_hclk_vo1_s, hclk_hdcp_key1_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(PCLK_EDP_S, pclk_edp_s, p_pclk_vo0_s, pclk_edp_s_info, &clk_scmi_ops_gate, rk3576_common_rates, true, true);
RK3576_SCMI_CLOCK_COM(ACLK_KLAD, aclk_klad, p_aclk_secure_s, aclk_crypto_s_info, &clk_scmi_ops_crypto, rk3576_common_rates, true, true);

rk_scmi_clock_t *clock_table[] = {
	[ARMCLK_L]		= &scmi_armclkl,
	[ACLK_CCI_ROOT]		= &scmi_aclk_cci,
	[ARMCLK_B]		= &scmi_armclkb,
	[CLK_GPU]		= &scmi_clk_gpu,
	[CLK_RKNN_DSU0]		= &scmi_clk_npu,
	[CLK_STIMER0_ROOT]	= &clk_stimer0_rt,
	[CLK_STIMER1_ROOT]	= &clk_stimer1_rt,
	[PCLK_SECURE_S]		= &pclk_secure_s,
	[HCLK_SECURE_S]		= &hclk_secure_s,
	[ACLK_SECURE_S]		= &aclk_secure_s,
	[CLK_PKA_CRYPTO_S]	= &clk_pka_crypt_s,
	[HCLK_VO1_S]		= &hclk_vo1_s,
	[PCLK_VO1_S]		= &pclk_vo1_s,
	[HCLK_VO0_S]		= &hclk_vo0_s,
	[PCLK_VO0_S]		= &pclk_vo0_s,
	[PCLK_KLAD]		= &pclk_klad,
	[HCLK_CRYPTO_S]		= &hclk_crypto_s,
	[HCLK_KLAD]		= &hclk_klad,
	[ACLK_CRYPTO_S]		= &aclk_crypto_s,
	[HCLK_TRNG_S]		= &hclk_trng_s,
	[PCLK_OTPC_S]		= &plk_otpc_s,
	[CLK_OTPC_S]		= &clk_otpc_s,
	[PCLK_WDT_S]		= &pclk_wdt_s,
	[TCLK_WDT_S]		= &tclk_wdt_s,
	[PCLK_HDCP0_TRNG]	= &pclk_hdcp0_trng,
	[PCLK_HDCP1_TRNG]	= &pclk_hdcp1_trng,
	[HCLK_HDCP_KEY0]	= &hclk_hdcp_key0,
	[HCLK_HDCP_KEY1]	= &hclk_hdcp_key1,
	[PCLK_EDP_S]		= &pclk_edp_s,
	[ACLK_KLAD]		= &aclk_klad,
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

void pvtplls_cpub_suspend(void)
{
	clk_cpub_set_rate(408000000, PLL_SEL_NOR);
	cpub_suspended = true;
}

void pvtplls_cpub_resume(void)
{
	cpub_suspended = false;
	clk_cpub_set_rate(sys_clk_info.cpub_rate, PLL_SEL_AUTO);
}

void pvtplls_suspend(void)
{
	clk_cpul_set_rate(408000000, PLL_SEL_NOR);
	clk_cci_set_rate(408000000, PLL_SEL_NOR);
	clk_cpub_set_rate(408000000, PLL_SEL_NOR);
}

void pvtplls_resume(void)
{
	clk_cpul_set_rate(sys_clk_info.cpul_rate, PLL_SEL_AUTO);
	clk_cci_set_rate(sys_clk_info.cci_rate, PLL_SEL_AUTO);
	clk_cpub_set_rate(sys_clk_info.cpub_rate, PLL_SEL_AUTO);
}

void sys_reset_pvtplls_prepare(void)
{
	clk_gpu_set_rate(200000000, PLL_SEL_NOR);
	clk_npu_set_rate(200000000, PLL_SEL_NOR);
	clk_cpul_set_rate(408000000, PLL_SEL_NOR);
	clk_cci_set_rate(408000000, PLL_SEL_NOR);
	clk_cpub_set_rate(408000000, PLL_SEL_NOR);
}

int rockchip_opteed_clk_set_rate(uint64_t clk_idx, uint64_t rate)
{
	rk_scmi_clock_t *table;

	if (clk_idx > CLK_NR_CLKS) {
		INFO("%s: clk-%ld, %ld not supported\n", __func__, clk_idx, rate);
		return SCMI_INVALID_PARAMETERS;
	}

	table = rockchip_scmi_get_clock(0, clk_idx);
	if (table != NULL)
		table->clk_ops->set_rate(table, rate);

	return 0;
}

int rockchip_opteed_clk_get_rate(uint64_t clk_idx, uint64_t *rate)
{
	rk_scmi_clock_t *table;

	if (clk_idx > CLK_NR_CLKS) {
		INFO("%s: clk-%ld not supported\n", __func__, clk_idx);
		return SCMI_INVALID_PARAMETERS;
	}

	table = rockchip_scmi_get_clock(0, clk_idx);
	if (table != NULL)
		*rate = (uint64_t)table->clk_ops->get_rate(table);
	return 0;
}

int rockchip_opteed_clk_enable(uint64_t clk_idx, uint64_t enable)
{
	rk_scmi_clock_t *table;

	if (clk_idx > CLK_NR_CLKS) {
		INFO("%s: clk-%ld, %ld not supported\n", __func__, clk_idx, enable);
		return SCMI_INVALID_PARAMETERS;
	}

	table = rockchip_scmi_get_clock(0, clk_idx);
	if (table != NULL) {
		if (enable != 0) {
			table->clk_ops->set_status(table, enable);
			table->enable_count++;
		} else {
			if (table->enable_count == 0)
				return 0;
			if (--table->enable_count > 0)
				return 0;
			table->clk_ops->set_status(table, enable);
		}
	}
	return 0;
}

#define RK3576_CPUB_OPP_INFO_OFFSET	48
#define RK3576_CPUL_OPP_INFO_OFFSET	54
#define RK3576_CCI_OPP_INFO_OFFSET	60
#define RK3576_NPU_OPP_INFO_OFFSET	66
#define RK3576_GPU_OPP_INFO_OFFSET	72

static void rockchip_init_pvtpll_table(void)
{
	sys_clk_info.cpul_table = rk3576_cpul_pvtpll_table;
	sys_clk_info.cpul_rate_count = ARRAY_SIZE(rk3576_cpul_pvtpll_table);
	sys_clk_info.cci_table = rk3576_cci_pvtpll_table;
	sys_clk_info.cci_rate_count = ARRAY_SIZE(rk3576_cci_pvtpll_table);
	sys_clk_info.cpub_table = rk3576_cpub_pvtpll_table;
	sys_clk_info.cpub_rate_count = ARRAY_SIZE(rk3576_cpub_pvtpll_table);
	sys_clk_info.gpu_table = rk3576_gpu_pvtpll_table;
	sys_clk_info.gpu_rate_count = ARRAY_SIZE(rk3576_gpu_pvtpll_table);
	sys_clk_info.npu_table = rk3576_npu_pvtpll_table;
	sys_clk_info.npu_rate_count = ARRAY_SIZE(rk3576_npu_pvtpll_table);
}

void rockchip_clock_init(void)
{
	rockchip_init_pvtpll_table();
}

static int pvtpll_get_clk(uint64_t clock_id, struct pvtpll_table **table,
			  unsigned int *count)
{
	switch (clock_id) {
	case ARMCLK_L:
		*table = sys_clk_info.cpul_table;
		*count = sys_clk_info.cpul_rate_count;
		break;
	case ARMCLK_B:
		*table = sys_clk_info.cpub_table;
		*count = sys_clk_info.cpub_rate_count;
		break;
	case CLK_GPU:
		*table = sys_clk_info.gpu_table;
		*count = sys_clk_info.gpu_rate_count;
		break;
	case CLK_RKNN_DSU0:
		*table = sys_clk_info.npu_table;
		*count = sys_clk_info.npu_rate_count;
		break;
	default:
		return -1;
	}

	if ((*table == NULL) || (*count == 0))
		return -1;

	return 0;
}

int pvtpll_volt_sel_adjust(uint64_t clock_id, uint64_t volt_sel)
{
	struct pvtpll_table *table = NULL;
	uint32_t delta_len = 0;
	unsigned int count = 0;
	int i;

	if (pvtpll_get_clk(clock_id, &table, &count) != 0)
		return -1;

	for (i = 0; i < count; i++) {
		if (table[i].volt_sel_thr == 0)
			continue;
		if (volt_sel >= table[i].volt_sel_thr) {
			delta_len = volt_sel - table[i].volt_sel_thr + 1;
			table[i].length += delta_len;
			if (table[i].length > RK3576_PVTPLL_MAX_LENGTH)
				table[i].length = RK3576_PVTPLL_MAX_LENGTH;
		}
	}

	return 0;
}
