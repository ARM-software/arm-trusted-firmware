/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mtk_iommu_plat.h>
#include <mtk_mmap_pool.h>
#include <platform_def.h>

/* mm iommu */
#define SMI_L0_ID		(0)
#define SMI_L1_ID		(1)
#define SMI_L2_ID		(2)
#define SMI_L3_ID		(3)
#define SMI_L4_ID		(4)
#define SMI_L5_ID		(5)
#define SMI_L6_ID		(6)
#define SMI_L7_ID		(7)
#define SMI_L9_ID		(8)
#define SMI_L10_ID		(9)
#define SMI_L11A_ID		(10)
#define SMI_L11B_ID		(11)
#define SMI_L11C_ID		(12)
#define SMI_L12_ID		(13)
#define SMI_L13_ID		(14)
#define SMI_L14_ID		(15)
#define SMI_L15_ID		(16)
#define SMI_L16A_ID		(17)
#define SMI_L16B_ID		(18)
#define SMI_L17A_ID		(19)
#define SMI_L17B_ID		(20)
#define SMI_L19_ID		(21)
#define SMI_L21_ID		(22)
#define SMI_L23_ID		(23)
#define SMI_L27_ID		(24)
#define SMI_L28_ID		(25)

/* infra iommu */
#define PERI_MST_PROT		(0x710)
#define PERICFG_AO_IOMMU_1	(0x714)
#define MMU_DEV_PCIE_0		(0)
#define IFR_CFG_GROUP_NUM	(1)

static struct mtk_smi_larb_config mt8188_larb_cfg[SMI_LARB_NUM] = {
	[SMI_L0_ID] = LARB_CFG_ENTRY(SMI_LARB_0_BASE, 7, 0),
	[SMI_L1_ID] = LARB_CFG_ENTRY(SMI_LARB_1_BASE, 7, 0),
	[SMI_L2_ID] = LARB_CFG_ENTRY(SMI_LARB_2_BASE, 5, 0),
	[SMI_L3_ID] = LARB_CFG_ENTRY(SMI_LARB_3_BASE, 7, 0),
	[SMI_L4_ID] = LARB_CFG_ENTRY(SMI_LARB_4_BASE, 7, 0),
	[SMI_L5_ID] = LARB_CFG_ENTRY(SMI_LARB_5_BASE, 8, 0),
	[SMI_L6_ID] = LARB_CFG_ENTRY(SMI_LARB_6_BASE, 4, 0),
	[SMI_L7_ID] = LARB_CFG_ENTRY(SMI_LARB_7_BASE, 3, 0),
	[SMI_L9_ID] = LARB_CFG_ENTRY(SMI_LARB_9_BASE, 25, 0),
	[SMI_L10_ID] = LARB_CFG_ENTRY(SMI_LARB_10_BASE, 20, 0),
	[SMI_L11A_ID] = LARB_CFG_ENTRY(SMI_LARB_11A_BASE, 30, 0),
	[SMI_L11B_ID] = LARB_CFG_ENTRY(SMI_LARB_11B_BASE, 30, 0),
	[SMI_L11C_ID] = LARB_CFG_ENTRY(SMI_LARB_11C_BASE, 30, 0),
	[SMI_L12_ID] = LARB_CFG_ENTRY(SMI_LARB_12_BASE, 16, 0),
	[SMI_L13_ID] = LARB_CFG_ENTRY(SMI_LARB_13_BASE, 24, 0),
	[SMI_L14_ID] = LARB_CFG_ENTRY(SMI_LARB_14_BASE, 23, 0),
	[SMI_L15_ID] = LARB_CFG_ENTRY(SMI_LARB_15_BASE, 19, 0),
	[SMI_L16A_ID] = LARB_CFG_ENTRY(SMI_LARB_16A_BASE, 17, 0),
	[SMI_L16B_ID] = LARB_CFG_ENTRY(SMI_LARB_16B_BASE, 17, 0),
	[SMI_L17A_ID] = LARB_CFG_ENTRY(SMI_LARB_17A_BASE, 7, 0),
	[SMI_L17B_ID] = LARB_CFG_ENTRY(SMI_LARB_17B_BASE, 7, 0),
	/* venc nbm ports (5/6/11/15/16/17) to sram */
	[SMI_L19_ID] = LARB_CFG_ENTRY_WITH_PATH(SMI_LARB_19_BASE, 27, 0, 0x38860),
	[SMI_L21_ID] = LARB_CFG_ENTRY(SMI_LARB_21_BASE, 11, 0),
	[SMI_L23_ID] = LARB_CFG_ENTRY(SMI_LARB_23_BASE, 9, 0),
	[SMI_L27_ID] = LARB_CFG_ENTRY(SMI_LARB_27_BASE, 4, 0),
	[SMI_L28_ID] = LARB_CFG_ENTRY(SMI_LARB_28_BASE, 0, 0),
};

static bool is_protected;

static uint32_t mt8188_ifr_mst_cfg_base[IFR_CFG_GROUP_NUM] = {
	PERICFG_AO_BASE,
};
static uint32_t mt8188_ifr_mst_cfg_offs[IFR_CFG_GROUP_NUM] = {
	PERICFG_AO_IOMMU_1,
};
static struct mtk_ifr_mst_config mt8188_ifr_mst_cfg[MMU_DEV_NUM] = {
	[MMU_DEV_PCIE_0] = IFR_MST_CFG_ENTRY(0, 18),
};

struct mtk_smi_larb_config *g_larb_cfg = &mt8188_larb_cfg[0];
struct mtk_ifr_mst_config *g_ifr_mst_cfg = &mt8188_ifr_mst_cfg[0];
uint32_t *g_ifr_mst_cfg_base = &mt8188_ifr_mst_cfg_base[0];
uint32_t *g_ifr_mst_cfg_offs = &mt8188_ifr_mst_cfg_offs[0];

/* Protect infra iommu enable setting registers as secure access. */
void mtk_infra_iommu_enable_protect(void)
{
	if (!is_protected) {
		mmio_write_32(PERICFG_AO_BASE + PERI_MST_PROT, 0xffffffff);
		is_protected = true;
	}
}
