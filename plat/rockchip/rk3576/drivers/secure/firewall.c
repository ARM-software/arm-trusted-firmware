// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#include <assert.h>
#include <errno.h>

#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include <firewall.h>
#include <soc.h>

enum {
	FW_NS_A_S_A = 0x0,
	FW_NS_A_S_NA = 0x1,
	FW_NS_NA_S_A = 0x2,
	FW_NS_NA_S_NA = 0x3,
};

/* group type */
enum {
	FW_GRP_TYPE_INV = 0,
	FW_GRP_TYPE_DDR_RGN = 1,
	FW_GRP_TYPE_SYSMEM_RGN = 2,
	FW_GRP_TYPE_CBUF_RGN = 3,
	FW_GRP_TYPE_SLV = 4,
	FW_GRP_TYPE_DM = 5,
};

enum {
	FW_SLV_TYPE_INV = 0,
	FW_MST_TYPE_INV = 0,
	FW_SLV_TYPE_BUS = 1,
	FW_SLV_TYPE_TOP = 2,
	FW_SLV_TYPE_CENTER = 3,
	FW_SLV_TYPE_CCI = 4,
	FW_SLV_TYPE_PHP = 5,
	FW_SLV_TYPE_GPU = 6,
	FW_SLV_TYPE_NPU = 7,
	FW_SLV_TYPE_PMU = 8,
	FW_MST_TYPE_SYS = 9,
	FW_MST_TYPE_PMU = 10,
};

#define FW_ID(type, id)			\
	((((type) & 0xff) << 16) | ((id) & 0xffff))

#define FW_MST_ID(type, id)		FW_ID(type, id)
#define FW_SLV_ID(type, id)		FW_ID(type, id)
#define FW_GRP_ID(type, id)		FW_ID(type, id)

/* group id */
#define FW_GRP_ID_DDR_RGN(id)		FW_GRP_ID(FW_GRP_TYPE_DDR_RGN, id)
#define FW_GRP_ID_SYSMEM_RGN(id)	FW_GRP_ID(FW_GRP_TYPE_SYSMEM_RGN, id)
#define FW_GRP_ID_CBUF(id)		FW_GRP_ID(FW_GRP_TYPE_CBUF_RGN, id)
#define FW_GRP_ID_SLV(id)		FW_GRP_ID(FW_GRP_TYPE_SLV, id)
#define FW_GRP_ID_DM(id)		FW_GRP_ID(FW_GRP_TYPE_DM, id)

#define FW_GRP_ID_SLV_CNT		8
#define FW_GRP_ID_DM_CNT		8

#define FW_GET_ID(id)			((id) & 0xffff)
#define FW_GET_TYPE(id)			(((id) >> 16) & 0xff)

#define FW_INVLID_MST_ID		FW_MST_ID(FW_MST_TYPE_INV, 0)
#define FW_INVLID_SLV_ID		FW_SLV_ID(FW_SLV_TYPE_INV, 0)

typedef struct {
	uint32_t domain[FW_SGRF_MST_DOMAIN_CON_CNT];
	uint32_t pmu_domain;
	uint32_t bus_slv_grp[FW_SGRF_BUS_SLV_CON_CNT];
	uint32_t top_slv_grp[FW_SGRF_TOP_SLV_CON_CNT];
	uint32_t center_slv_grp[FW_SGRF_CENTER_SLV_CON_CNT];
	uint32_t cci_slv_grp[FW_SGRF_CCI_SLV_CON_CNT];
	uint32_t php_slv_grp[FW_SGRF_PHP_SLV_CON_CNT];
	uint32_t gpu_slv_grp;
	uint32_t npu_slv_grp[FW_SGRF_NPU_SLV_CON_CNT];
	uint32_t pmu_slv_grp[FW_PMU_SGRF_SLV_CON_CNT];
	uint32_t ddr_rgn[FW_SGRF_DDR_RGN_CNT];
	uint32_t ddr_size;
	uint32_t ddr_con;
	uint32_t sysmem_rgn[FW_SGRF_SYSMEM_RGN_CNT];
	uint32_t sysmem_con;
	uint32_t cbuf_rgn[FW_SGRF_CBUF_RGN_CNT];
	uint32_t cbuf_con;
	uint32_t ddr_lookup[FW_SGRF_DDR_LOOKUP_CNT];
	uint32_t sysmem_lookup[FW_SGRF_SYSMEM_LOOKUP_CNT];
	uint32_t cbuf_lookup[FW_SGRF_CBUF_LOOKUP_CNT];
	uint32_t slv_lookup[FW_SGRF_SLV_LOOKUP_CNT];
	uint32_t pmu_slv_lookup[FW_PMU_SGRF_SLV_LOOKUP_CNT];
} fw_config_t;

static fw_config_t fw_config_buf;

/****************************************************************************
 * Access rights between domains and groups are as follows:
 *
 * 00: NS access,     S access
 * 01: NS access,     S not access
 * 10: NS not access, S access
 * 11: NS not access, S not access
 * |---------------------------------------------------------|
 * |                 | d0 | d1 | d2 | d3 | d4 | d5 | d6 | d7 |
 * |---------------------------------------------------------|
 * | slave g0        | 00 | 00 | 11 | 11 | 11 | 11 | 11 | 00 |
 * |---------------------------------------------------------|
 * | slave g1        | 10 | 11 | 11 | 11 | 11 | 11 | 11 | 10 |
 * |---------------------------------------------------------|
 * | slave g2~7      | 11 | 11 | 11 | 11 | 11 | 11 | 11 | 11 |
 * |---------------------------------------------------------|
 * | ddr region 0~15 | 10 | 11 | 11 | 11 | 11 | 11 | 11 | 10 |
 * |---------------------------------------------------------|
 * | sram region 0~3 | 10 | 11 | 11 | 11 | 11 | 11 | 11 | 10 |
 * |---------------------------------------------------------|
 * | cbuf region 0~7 | 10 | 11 | 11 | 11 | 11 | 11 | 11 | 10 |
 * |---------------------------------------------------------|
 *
 * PS:
 * Domain 0/1/7 NS/S can access group 0.
 * Domain 0/1/7 NS and Domain1 NS/S can't access group 1, domain 0/7 S can access.
 * Other domains NS/S can't access all groups.
 *
 * Domain 0/7 NS can't access ddr/sram/cbuf region and Domain 0/7 S can access.
 * Other domains NS/S can't access ddr/sram/cbuf region.
 *
 ******************************************************************************/

/* Masters in dm1 */
static const int dm1_mst[] = {
	FW_MST_ID(FW_MST_TYPE_SYS, 1), /* keylad_apbm */
	FW_MST_ID(FW_MST_TYPE_SYS, 2), /* dft2apbm */
	FW_MST_ID(FW_MST_TYPE_SYS, 11), /* dma2ddr */
	FW_MST_ID(FW_MST_TYPE_SYS, 12), /* dmac0 */
	FW_MST_ID(FW_MST_TYPE_SYS, 13), /* dmac1 */
	FW_MST_ID(FW_MST_TYPE_SYS, 14), /* dmac2 */
	FW_MST_ID(FW_MST_TYPE_SYS, 19), /* gpu */
	FW_MST_ID(FW_MST_TYPE_SYS, 31), /* vop_m0 */
	FW_MST_ID(FW_MST_TYPE_SYS, 32), /* vop_m1 */
	FW_MST_ID(FW_MST_TYPE_SYS, 36), /* bus_mcu */
	FW_MST_ID(FW_MST_TYPE_SYS, 38), /* npu_mcu */
	FW_MST_ID(FW_MST_TYPE_SYS, 56), /* dap_lite */

	FW_INVLID_MST_ID
};

/* Slaves in group1 */
static const int sec_slv[] = {
	FW_SLV_ID(FW_SLV_TYPE_TOP, 28), /* crypto_s */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 29), /* keyladder_s */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 30), /* rkrng_s */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 33), /* jtag_lock */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 34), /* otp_s */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 35), /* otpmsk */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 37), /* scru_s */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 38), /* sys_sgrf */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 39), /* bootrom */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 41), /* wdts */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 44), /* sevice_secure */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 61), /* timers0_ch0~5 */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 62),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 63),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 64),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 65),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 66),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 67), /* timers1_ch0~5 */
	FW_SLV_ID(FW_SLV_TYPE_TOP, 68),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 69),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 70),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 71),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 72),
	FW_SLV_ID(FW_SLV_TYPE_TOP, 73), /* sys_fw */

	FW_SLV_ID(FW_SLV_TYPE_CENTER, 3),  /* ddr grf */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 4),  /* ddr ctl0 */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 5),  /* ddr ctl1 */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 6),  /* ddr phy0 */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 7),  /* ddr0 cru */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 8),  /* ddr phy1 */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 9),  /* ddr1 cru */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 15), /* ddr wdt */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 19), /* service ddr */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 58), /* ddr timer ch0 */
	FW_SLV_ID(FW_SLV_TYPE_CENTER, 59), /* ddr timer ch1 */

	FW_SLV_ID(FW_SLV_TYPE_PMU, 1),	/* pmu mem */
	FW_SLV_ID(FW_SLV_TYPE_PMU, 15), /* pmu1_scru */
	FW_SLV_ID(FW_SLV_TYPE_PMU, 30), /* osc chk */
	FW_SLV_ID(FW_SLV_TYPE_PMU, 31), /* pmu0_sgrf */
	FW_SLV_ID(FW_SLV_TYPE_PMU, 32), /* pmu1_sgrf */
	FW_SLV_ID(FW_SLV_TYPE_PMU, 34), /* scramble key */
	FW_SLV_ID(FW_SLV_TYPE_PMU, 36), /* pmu remap */
	FW_SLV_ID(FW_SLV_TYPE_PMU, 43), /* pmu fw */

	FW_INVLID_SLV_ID
};

static void fw_buf_sys_mst_dm_cfg(int mst_id, uint32_t dm_id)
{
	int sft = (mst_id & 0x7) << 2;

	fw_config_buf.domain[mst_id >> 3] &= ~(0xf << sft);
	fw_config_buf.domain[mst_id >> 3] |= (dm_id & 0xf) << sft;
}

static void fw_buf_pmu_mst_dm_cfg(int mst_id, uint32_t dm_id)
{
	int sft = (mst_id & 0x7) << 2;

	fw_config_buf.pmu_domain &= ~(0xf << sft);
	fw_config_buf.pmu_domain |= (dm_id & 0xf) << sft;
}

void fw_buf_mst_dm_cfg(int mst_id, uint32_t dm_id)
{
	int type = FW_GET_TYPE(mst_id);

	mst_id = FW_GET_ID(mst_id);

	switch (type) {
	case FW_MST_TYPE_SYS:
		fw_buf_sys_mst_dm_cfg(mst_id, dm_id);
		break;
	case FW_MST_TYPE_PMU:
		fw_buf_pmu_mst_dm_cfg(mst_id, dm_id);
		break;

	default:
		ERROR("%s: unknown FW_DOMAIN_TYPE (0x%x)\n", __func__, type);
		break;
	}
}

static void fw_buf_ddr_lookup_cfg(int rgn_id, int dm_id, uint32_t priv)
{
	int sft = (dm_id << 1) + (rgn_id & 0x1) * 16;

	fw_config_buf.ddr_lookup[rgn_id >> 1] &= ~(0x3 << sft);
	fw_config_buf.ddr_lookup[rgn_id >> 1] |= (priv & 0x3) << sft;
}

static void fw_buf_sysmem_lookup_cfg(int rgn_id, int dm_id, uint32_t priv)
{
	int sft = (dm_id << 1) + (rgn_id & 0x1) * 16;

	fw_config_buf.sysmem_lookup[rgn_id >> 1] &= ~(0x3 << sft);
	fw_config_buf.sysmem_lookup[rgn_id >> 1] |= (priv & 0x3) << sft;
}

static void fw_buf_cbuf_lookup_cfg(int rgn_id, int dm_id, uint32_t priv)
{
	int sft = (dm_id << 1) + (rgn_id & 0x1) * 16;

	fw_config_buf.cbuf_lookup[rgn_id >> 1] &= ~(0x3 << sft);
	fw_config_buf.cbuf_lookup[rgn_id >> 1] |= (priv & 0x3) << sft;
}

static void fw_buf_slv_lookup_cfg(int grp_id, int dm_id, uint32_t priv)
{
	int sft = (dm_id << 1) + (grp_id & 0x1) * 16;

	fw_config_buf.slv_lookup[grp_id >> 1] &= ~(0x3 << sft);
	fw_config_buf.slv_lookup[grp_id >> 1] |= (priv & 0x3) << sft;
}

static void fw_buf_pmu_slv_lookup_cfg(int grp_id, int dm_id, uint32_t priv)
{
	int sft = (dm_id << 1) + (grp_id & 0x1) * 16;

	fw_config_buf.pmu_slv_lookup[grp_id >> 1] &= ~(0x3 << sft);
	fw_config_buf.pmu_slv_lookup[grp_id >> 1] |= (priv & 0x3) << sft;
}

void fw_buf_grp_lookup_cfg(int grp_id, int dm_id, uint32_t priv)
{
	uint32_t type = FW_GET_TYPE(grp_id);

	grp_id = FW_GET_ID(grp_id);

	switch (type) {
	case FW_GRP_TYPE_DDR_RGN:
		fw_buf_ddr_lookup_cfg(grp_id, dm_id, priv);
		break;
	case FW_GRP_TYPE_SYSMEM_RGN:
		fw_buf_sysmem_lookup_cfg(grp_id, dm_id, priv);
		break;
	case FW_GRP_TYPE_CBUF_RGN:
		fw_buf_cbuf_lookup_cfg(grp_id, dm_id, priv);
		break;
	case FW_GRP_TYPE_SLV:
		fw_buf_slv_lookup_cfg(grp_id, dm_id, priv);
		fw_buf_pmu_slv_lookup_cfg(grp_id, dm_id, priv);
		break;

	default:
		ERROR("%s: unknown FW_LOOKUP_TYPE (0x%x)\n", __func__, type);
		break;
	}
}

static void fw_buf_bus_slv_grp_cfg(int slv_id, int grp_id)
{
	int sft = slv_id % 5 << 2;

	fw_config_buf.bus_slv_grp[slv_id / 5] &= ~(0xf << sft);
	fw_config_buf.bus_slv_grp[slv_id / 5] |= (grp_id & 0xf) << sft;
}

static void fw_buf_top_slv_grp_cfg(int slv_id, int grp_id)
{
	int sft = slv_id % 5 << 2;

	fw_config_buf.top_slv_grp[slv_id / 5] &= ~(0xf << sft);
	fw_config_buf.top_slv_grp[slv_id / 5] |= (grp_id & 0xf) << sft;
}

static void fw_buf_center_slv_grp_cfg(int slv_id, int grp_id)
{
	int sft = slv_id % 5 << 2;

	fw_config_buf.center_slv_grp[slv_id / 5] &= ~(0xf << sft);
	fw_config_buf.center_slv_grp[slv_id / 5] |= (grp_id & 0xf) << sft;
}

static void fw_buf_cci_slv_grp_cfg(int slv_id, int grp_id)
{
	int sft = slv_id % 5 << 2;

	fw_config_buf.cci_slv_grp[slv_id / 5] &= ~(0xf << sft);
	fw_config_buf.cci_slv_grp[slv_id / 5] |= (grp_id & 0xf) << sft;
}

static void fw_buf_php_slv_grp_cfg(int slv_id, int grp_id)
{
	int sft = slv_id % 5 << 2;

	fw_config_buf.php_slv_grp[slv_id / 5] &= ~(0xf << sft);
	fw_config_buf.php_slv_grp[slv_id / 5] |= (grp_id & 0xf) << sft;
}

static void fw_buf_gpu_slv_grp_cfg(int slv_id, int grp_id)
{
	int sft = slv_id % 5 << 2;

	fw_config_buf.gpu_slv_grp &= ~(0xf << sft);
	fw_config_buf.gpu_slv_grp |= (grp_id & 0xf) << sft;
}

static void fw_buf_npu_slv_grp_cfg(int slv_id, int grp_id)
{
	int sft = slv_id % 5 << 2;

	fw_config_buf.npu_slv_grp[slv_id / 5] &= ~(0xf << sft);
	fw_config_buf.npu_slv_grp[slv_id / 5] |= (grp_id & 0xf) << sft;
}

static void fw_buf_pmu_slv_grp_cfg(int slv_id, int grp_id)
{
	int sft = slv_id % 5 << 2;

	fw_config_buf.pmu_slv_grp[slv_id / 5] &= ~(0xf << sft);
	fw_config_buf.pmu_slv_grp[slv_id / 5] |= (grp_id & 0xf) << sft;
}

void fw_buf_slv_grp_cfg(int slv_id, int grp_id)
{
	int type = FW_GET_TYPE(slv_id);

	slv_id = FW_GET_ID(slv_id);
	grp_id = FW_GET_ID(grp_id);

	switch (type) {
	case FW_SLV_TYPE_BUS:
		fw_buf_bus_slv_grp_cfg(slv_id, grp_id);
		break;
	case FW_SLV_TYPE_TOP:
		fw_buf_top_slv_grp_cfg(slv_id, grp_id);
		break;
	case FW_SLV_TYPE_CENTER:
		fw_buf_center_slv_grp_cfg(slv_id, grp_id);
		break;
	case FW_SLV_TYPE_CCI:
		fw_buf_cci_slv_grp_cfg(slv_id, grp_id);
		break;
	case FW_SLV_TYPE_PHP:
		fw_buf_php_slv_grp_cfg(slv_id, grp_id);
		break;
	case FW_SLV_TYPE_GPU:
		fw_buf_gpu_slv_grp_cfg(slv_id, grp_id);
		break;
	case FW_SLV_TYPE_NPU:
		fw_buf_npu_slv_grp_cfg(slv_id, grp_id);
		break;
	case FW_SLV_TYPE_PMU:
		fw_buf_pmu_slv_grp_cfg(slv_id, grp_id);
		break;

	default:
		ERROR("%s: unknown FW_SLV_TYPE (0x%x)\n", __func__, type);
		break;
	}
}

void fw_buf_add_msts(const int *mst_ids, int dm_id)
{
	int i;

	for (i = 0; FW_GET_TYPE(mst_ids[i]) != FW_INVLID_SLV_ID; i++)
		fw_buf_mst_dm_cfg(mst_ids[i], dm_id);
}

void fw_buf_add_slvs(const int *slv_ids, int grp_id)
{
	int i;

	for (i = 0; FW_GET_TYPE(slv_ids[i]) != FW_INVLID_SLV_ID; i++)
		fw_buf_slv_grp_cfg(slv_ids[i], grp_id);
}

/* unit: Mb */
void fw_buf_ddr_size_cfg(uint64_t base_mb, uint64_t top_mb, int id)
{
	fw_config_buf.ddr_size = RG_MAP_SECURE(top_mb, base_mb);
	fw_config_buf.ddr_con |= BIT(16);
}

/* unit: Mb */
void fw_buf_ddr_rgn_cfg(uint64_t base_mb, uint64_t top_mb, int rgn_id)
{
	fw_config_buf.ddr_rgn[rgn_id] = RG_MAP_SECURE(top_mb, base_mb);
	fw_config_buf.ddr_con |= BIT(rgn_id);
}

/* Unit: kb */
void fw_buf_sysmem_rgn_cfg(uint64_t base_kb, uint64_t top_kb, int rgn_id)
{
	fw_config_buf.sysmem_rgn[rgn_id] = RG_MAP_SRAM_SECURE(top_kb, base_kb);
	fw_config_buf.sysmem_con |= BIT(rgn_id);
}

static void fw_domain_init(void)
{
	int i;

	/* select to domain0 by default */
	for (i = 0; i < FW_SGRF_MST_DOMAIN_CON_CNT; i++)
		fw_config_buf.domain[i] = 0x0;

	/* select to domain0 by default */
	fw_config_buf.pmu_domain = 0x0;
}

static void fw_slv_grp_init(void)
{
	int i;

	/* select to group0 by default */
	for (i = 0; i < FW_SGRF_BUS_SLV_CON_CNT; i++)
		fw_config_buf.bus_slv_grp[i] = 0x0;

	for (i = 0; i < FW_SGRF_TOP_SLV_CON_CNT; i++)
		fw_config_buf.top_slv_grp[i] = 0x0;

	for (i = 0; i < FW_SGRF_CENTER_SLV_CON_CNT; i++)
		fw_config_buf.center_slv_grp[i] = 0x0;

	for (i = 0; i < FW_SGRF_CCI_SLV_CON_CNT; i++)
		fw_config_buf.cci_slv_grp[i] = 0x0;

	for (i = 0; i < FW_SGRF_PHP_SLV_CON_CNT; i++)
		fw_config_buf.php_slv_grp[i] = 0x0;

	fw_config_buf.gpu_slv_grp = 0x0;

	for (i = 0; i < FW_SGRF_NPU_SLV_CON_CNT; i++)
		fw_config_buf.npu_slv_grp[i] = 0x0;
}

static void fw_region_init(void)
{
	/* Use FW_DDR_RGN0_REG to config 1024~1025M space to secure */
	fw_buf_ddr_rgn_cfg(1024, 1025, 0);

	/* Use FW_SYSMEM_RGN0_REG to config 0~32k space to secure */
	fw_buf_sysmem_rgn_cfg(0, 32, 0);
}

static void fw_lookup_init(void)
{
	int i;

	/*
	 * Domain 0/7 NS can't access ddr/sram/cbuf region and Domain 0/7 S can access.
	 * Other domains NS/S can't access ddr/sram/cbuf region.
	 */
	for (i = 0; i < FW_SGRF_DDR_LOOKUP_CNT; i++)
		fw_config_buf.ddr_lookup[i] = 0xbffebffe;

	for (i = 0; i < FW_SGRF_SYSMEM_LOOKUP_CNT; i++)
		fw_config_buf.sysmem_lookup[i] = 0xbffebffe;

	for (i = 0; i < FW_SGRF_CBUF_LOOKUP_CNT; i++)
		fw_config_buf.cbuf_lookup[i] = 0xbffebffe;

	/*
	 * Domain 0/1/7 NS/S can access group 0.
	 * Domain 0/1/7 NS and Domain1 NS/S can't access group 1, domain 0/7 S can access.
	 * Other domains NS/S can't access all groups.
	 */
	fw_config_buf.slv_lookup[0] = 0xbffe3ff0;
	fw_config_buf.slv_lookup[1] = 0xffffffff;
	fw_config_buf.slv_lookup[2] = 0xffffffff;
	fw_config_buf.slv_lookup[3] = 0xffffffff;

	/*
	 * Domain 0/1/7 NS/S can access group 0.
	 * Domain 0/1/7 NS and Domain1 NS/S can't access group 1, domain 0/7 S can access.
	 * Other domains NS/S can't access all groups.
	 */
	fw_config_buf.pmu_slv_lookup[0] = 0xbffe3ff0;
	fw_config_buf.pmu_slv_lookup[1] = 0xffffffff;
	fw_config_buf.pmu_slv_lookup[2] = 0xffffffff;
	fw_config_buf.pmu_slv_lookup[3] = 0xffffffff;
}

static void fw_config_buf_flush(void)
{
	int i;

	/* domain */
	for (i = 0; i < FW_SGRF_MST_DOMAIN_CON_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_MST_DOMAIN_CON(i),
			      fw_config_buf.domain[i]);

	mmio_write_32(PMU1SGRF_FW_BASE + FW_PMU_SGRF_DOMAIN_CON,
		      fw_config_buf.pmu_domain);

	/* slave group */
	for (i = 0; i < FW_SGRF_BUS_SLV_CON_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_BUS_SLV_CON(i),
			      fw_config_buf.bus_slv_grp[i]);

	for (i = 0; i < FW_SGRF_TOP_SLV_CON_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_TOP_SLV_CON(i),
			      fw_config_buf.top_slv_grp[i]);

	for (i = 0; i < FW_SGRF_CENTER_SLV_CON_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_CENTER_SLV_CON(i),
			      fw_config_buf.center_slv_grp[i]);

	for (i = 0; i < FW_SGRF_CCI_SLV_CON_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_CCI_SLV_CON(i),
			      fw_config_buf.cci_slv_grp[i]);

	for (i = 0; i < FW_SGRF_PHP_SLV_CON_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_PHP_SLV_CON(i),
			      fw_config_buf.php_slv_grp[i]);

	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_GPU_SLV_CON,
		      fw_config_buf.gpu_slv_grp);

	for (i = 0; i < FW_SGRF_NPU_SLV_CON_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_NPU_SLV_CON(i),
			      fw_config_buf.npu_slv_grp[i]);

	for (i = 0; i < FW_PMU_SGRF_SLV_CON_CNT; i++)
		mmio_write_32(PMU1SGRF_FW_BASE + FW_PMU_SGRF_SLV_CON(i),
			      fw_config_buf.pmu_slv_grp[i]);

	/* region */
	for (i = 0; i < FW_SGRF_DDR_RGN_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_DDR_RGN(i),
			      fw_config_buf.ddr_rgn[i]);

	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_DDR_SIZE, fw_config_buf.ddr_size);

	for (i = 0; i < FW_SGRF_SYSMEM_RGN_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_SYSMEM_RGN(i),
			      fw_config_buf.sysmem_rgn[i]);

	for (i = 0; i < FW_SGRF_CBUF_RGN_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_CBUF_RGN(i),
			      fw_config_buf.cbuf_rgn[i]);

	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_DDR_CON, fw_config_buf.ddr_con);
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_SYSMEM_CON, fw_config_buf.sysmem_con);
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_CBUF_CON, fw_config_buf.cbuf_con);

	dsb();
	isb();

	/* lookup */
	for (i = 0; i < FW_SGRF_DDR_LOOKUP_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_DDR_LOOKUP(i),
			      fw_config_buf.ddr_lookup[i]);

	for (i = 0; i < FW_SGRF_SYSMEM_LOOKUP_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_SYSMEM_LOOKUP(i),
			      fw_config_buf.sysmem_lookup[i]);

	for (i = 0; i < FW_SGRF_CBUF_LOOKUP_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_CBUF_LOOKUP(i),
			      fw_config_buf.cbuf_lookup[i]);

	for (i = 0; i < FW_SGRF_SLV_LOOKUP_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_SLV_LOOKUP(i),
			      fw_config_buf.slv_lookup[i]);

	for (i = 0; i < FW_PMU_SGRF_SLV_LOOKUP_CNT; i++)
		mmio_write_32(PMU1SGRF_FW_BASE + FW_PMU_SGRF_SLV_LOOKUP(i),
			      fw_config_buf.pmu_slv_lookup[i]);

	dsb();
	isb();
}

static __pmusramfunc void pmusram_udelay(uint32_t us)
{
	uint64_t orig;
	uint64_t to_wait;

	orig = read_cntpct_el0();
	to_wait = read_cntfrq_el0() * us / 1000000;

	while (read_cntpct_el0() - orig <= to_wait)
		;
}

__pmusramfunc void pmusram_fw_update_msk(uint32_t msk)
{
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_KEYUPD_CON0,
		      BITS_WITH_WMASK(0, 0x3ff, 0));
	dsb();

	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_KEYUPD_CON0,
		      BITS_WITH_WMASK(msk, msk, 0));
	dsb();
	isb();
	pmusram_udelay(20);
	dsb();
	isb();
}

__pmusramfunc void pmusram_all_fw_bypass(void)
{
	int i;

	/* disable regions */
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_DDR_CON, 0);
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_SYSMEM_CON, 0);
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_CBUF_CON, 0);

	for (i = 0; i < FW_SGRF_DDR_LOOKUP_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_DDR_LOOKUP(i), 0x0);

	for (i = 0; i < FW_SGRF_SYSMEM_LOOKUP_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_SYSMEM_LOOKUP(i), 0x0);

	for (i = 0; i < FW_SGRF_CBUF_LOOKUP_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_CBUF_LOOKUP(i), 0x0);

	for (i = 0; i < FW_SGRF_SLV_LOOKUP_CNT; i++)
		mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_SLV_LOOKUP(i), 0x0);

	for (i = 0; i < FW_PMU_SGRF_SLV_LOOKUP_CNT; i++)
		mmio_write_32(PMU1SGRF_FW_BASE + FW_PMU_SGRF_SLV_LOOKUP(i), 0x0);

	dsb();

	pmusram_fw_update_msk(0x3ff);
}

void fw_init(void)
{
	/* Enable all fw auto-update */
	mmio_write_32(SYS_SGRF_FW_BASE + FW_SGRF_KEYUPD_CON1, 0x03ff03ff);

	pmusram_all_fw_bypass();

	fw_domain_init();
	fw_slv_grp_init();
	fw_region_init();

	fw_buf_add_slvs(sec_slv, 1);
	fw_buf_add_msts(dm1_mst, 1);

	fw_lookup_init();

	fw_config_buf_flush();
	pmusram_fw_update_msk(0x3ff);
}
