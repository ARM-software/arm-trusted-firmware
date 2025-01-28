/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>

#include <drivers/spmi/pmif_common.h>
#include <drivers/spmi/pmif_v1/pmif.h>
#include <drivers/spmi/spmi_common.h>
#include <drivers/spmi/spmi_sw.h>
#include <drivers/spmi_api.h>
#include <lib/mtk_init/mtk_init.h>
#include <mtk_mmap_pool.h>

#define SPMI_GROUP_ID	0xB
#define SPMI_DEBUG	0

static const mmap_region_t pmif_spmi_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(PMIF_SPMI_M_BASE, PMIF_SPMI_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(SPMI_MST_M_BASE, SPMI_MST_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMIF_SPMI_P_BASE, PMIF_SPMI_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	{0}
};
DECLARE_MTK_MMAP_REGIONS(pmif_spmi_mmap);

static uint16_t mt6xxx_spmi_regs[] = {
	[SPMI_OP_ST_CTRL]   = 0x0000,
	[SPMI_GRP_ID_EN]    = 0x0004,
	[SPMI_OP_ST_STA]    = 0x0008,
	[SPMI_MST_SAMPL]    = 0x000C,
	[SPMI_MST_REQ_EN]   = 0x0010,
	[SPMI_RCS_CTRL]     = 0x0014,
	[SPMI_SLV_3_0_EINT] = 0x0020,
	[SPMI_SLV_7_4_EINT] = 0x0024,
	[SPMI_SLV_B_8_EINT] = 0x0028,
	[SPMI_SLV_F_C_EINT] = 0x002C,
	[SPMI_REC_CTRL]     = 0x0040,
	[SPMI_REC0]         = 0x0044,
	[SPMI_REC1]         = 0x0048,
	[SPMI_REC2]         = 0x004C,
	[SPMI_REC3]         = 0x0050,
	[SPMI_REC4]         = 0x0054,
	[SPMI_REC_CMD_DEC]  = 0x005C,
	[SPMI_DEC_DBG]      = 0x00F8,
	[SPMI_MST_DBG]      = 0x00FC,
};

static uint16_t mt6xxx_regs[] = {
	[PMIF_INIT_DONE]                = 0x0000,
	[PMIF_INF_EN]                   = 0x0024,
	[PMIF_ARB_EN]                   = 0x0150,
	[PMIF_IRQ_EVENT_EN_0]           = 0x0420,
	[PMIF_IRQ_FLAG_0]               = 0x0428,
	[PMIF_IRQ_CLR_0]                = 0x042C,
	[PMIF_IRQ_EVENT_EN_2]           = 0x0440,
	[PMIF_IRQ_FLAG_2]               = 0x0448,
	[PMIF_IRQ_CLR_2]                = 0x044C,
	[PMIF_WDT_CTRL]                 = 0x0470,
	[PMIF_WDT_EVENT_EN_1]           = 0x047C,
	[PMIF_WDT_FLAG_1]               = 0x0480,
	[PMIF_SWINF_2_ACC]              = 0x0880,
	[PMIF_SWINF_2_WDATA_31_0]       = 0x0884,
	[PMIF_SWINF_2_WDATA_63_32]      = 0x0888,
	[PMIF_SWINF_2_RDATA_31_0]       = 0x0894,
	[PMIF_SWINF_2_RDATA_63_32]      = 0x0898,
	[PMIF_SWINF_2_VLD_CLR]          = 0x08A4,
	[PMIF_SWINF_2_STA]              = 0x08A8,
	[PMIF_SWINF_3_ACC]              = 0x08C0,
	[PMIF_SWINF_3_WDATA_31_0]       = 0x08C4,
	[PMIF_SWINF_3_WDATA_63_32]      = 0x08C8,
	[PMIF_SWINF_3_RDATA_31_0]       = 0x08D4,
	[PMIF_SWINF_3_RDATA_63_32]      = 0x08D8,
	[PMIF_SWINF_3_VLD_CLR]          = 0x08E4,
	[PMIF_SWINF_3_STA]              = 0x08E8,
	/* hw mpu */
	[PMIF_PMIC_ALL_RGN_EN_1]        = 0x09B0,
	[PMIF_PMIC_ALL_RGN_EN_2]        = 0x0D30,
	[PMIF_PMIC_ALL_RGN_0_START]     = 0x09B4,
	[PMIF_PMIC_ALL_RGN_0_END]       = 0x09B8,
	[PMIF_PMIC_ALL_RGN_1_START]     = 0x09BC,
	[PMIF_PMIC_ALL_RGN_1_END]       = 0x09C0,
	[PMIF_PMIC_ALL_RGN_2_START]     = 0x09C4,
	[PMIF_PMIC_ALL_RGN_2_END]       = 0x09C8,
	[PMIF_PMIC_ALL_RGN_3_START]     = 0x09CC,
	[PMIF_PMIC_ALL_RGN_3_END]       = 0x09D0,
	[PMIF_PMIC_ALL_RGN_31_START]    = 0x0D34,
	[PMIF_PMIC_ALL_RGN_31_END]      = 0x0D38,
	[PMIF_PMIC_ALL_INVLD_SLVID]     = 0x0AAC,
	[PMIF_PMIC_ALL_RGN_0_PER0]      = 0x0AB0,
	[PMIF_PMIC_ALL_RGN_0_PER1]      = 0x0AB4,
	[PMIF_PMIC_ALL_RGN_1_PER0]      = 0x0AB8,
	[PMIF_PMIC_ALL_RGN_2_PER0]      = 0x0AC0,
	[PMIF_PMIC_ALL_RGN_3_PER0]      = 0x0AC8,
	[PMIF_PMIC_ALL_RGN_31_PER0]     = 0x0DB4,
	[PMIF_PMIC_ALL_RGN_31_PER1]     = 0x0DB8,
	[PMIF_PMIC_ALL_RGN_OTHERS_PER0] = 0x0BA8,
	[PMIF_PMIC_ALL_RGN_OTHERS_PER1] = 0x0BAC,
};

struct pmif pmif_spmi_arb[] = {
	{
		.base = (void *)PMIF_SPMI_M_BASE,
		.regs = mt6xxx_regs,
		.spmimst_base = (void *)SPMI_MST_M_BASE,
		.spmimst_regs = mt6xxx_spmi_regs,
		.mstid = SPMI_MASTER_0,
		.read_cmd = pmif_spmi_read_cmd,
		.write_cmd = pmif_spmi_write_cmd,
	}, {
		.base = (void *)PMIF_SPMI_M_BASE,
		.regs = mt6xxx_regs,
		.spmimst_base = (void *)SPMI_MST_M_BASE,
		.spmimst_regs = mt6xxx_spmi_regs,
		.mstid = SPMI_MASTER_1,
		.read_cmd = pmif_spmi_read_cmd,
		.write_cmd = pmif_spmi_write_cmd,
	}, {
		.base = (void *)PMIF_SPMI_P_BASE,
		.regs = mt6xxx_regs,
		.spmimst_base = (void *)SPMI_MST_P_BASE,
		.spmimst_regs = mt6xxx_spmi_regs,
		.mstid = SPMI_MASTER_P_1,
		.read_cmd = pmif_spmi_read_cmd,
		.write_cmd = pmif_spmi_write_cmd,
	},
};

static struct spmi_device spmi_dev[] = {
	{
		.slvid = SPMI_SLAVE_4,
		.grpiden = 0x1 << SPMI_GROUP_ID,
		.mstid = SPMI_MASTER_1,
		.hwcid_addr = 0x09,
		.hwcid_val = 0x63,
		.swcid_addr = 0x0B,
		.swcid_val = 0x63,
		.wpk_key_addr = 0x3A7,
		.wpk_key_val = 0x9C,
		.wpk_key_h_val = 0x9C,
		.tma_key_addr = 0x39E,
		.tma_key_val = 0x9C,
		.tma_key_h_val = 0x9C,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_1],
	}, {
		.slvid = SPMI_SLAVE_9,
		.grpiden = 0x1 << SPMI_GROUP_ID,
		.mstid = SPMI_MASTER_1,
		.hwcid_addr = 0x09,
		.hwcid_val = 0x85,
		.swcid_addr = 0x0B,
		.swcid_val = 0x85,
		.wpk_key_addr = 0x3AA,
		.wpk_key_val = 0x30,
		.wpk_key_h_val = 0x63,
		.tma_key_addr = 0x39E,
		.tma_key_val = 0x7A,
		.tma_key_h_val = 0x99,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_1],
	}, {
		.slvid = SPMI_SLAVE_5,
		.grpiden = 0x800,
		.mstid = SPMI_MASTER_1,/* spmi-m */
		.hwcid_addr = 0x09,
		.hwcid_val = 0x73,
		.swcid_addr = 0x0B,
		.swcid_val = 0x73,
		.wpk_key_addr = 0x3A7,
		.wpk_key_val = 0x8C,
		.wpk_key_h_val = 0x9C,
		.tma_key_addr = 0x39E,
		.tma_key_val = 0x8C,
		.tma_key_h_val = 0x9C,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_1],
	}, {
		.slvid = SPMI_SLAVE_14, /* MT6379 */
		.grpiden = 0x800,
		.mstid = SPMI_MASTER_1,/* spmi-m */
		.hwcid_addr = 0x00,
		.hwcid_val = 0x70,
		.hwcid_mask = 0xF0,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_1],
	}, {
		.slvid = SPMI_SLAVE_6, /* MT6316 */
		.grpiden = 0x800,
		.mstid = SPMI_MASTER_P_1,/* spmi-m */
		.hwcid_addr = 0x209,
		.hwcid_val = 0x16,
		.swcid_addr = 0x20B,
		.swcid_val = 0x16,
		.wpk_key_addr = 0x3B1,
		.wpk_key_val = 0xE9,
		.wpk_key_h_val = 0xE6,
		.tma_key_addr = 0x3A8,
		.tma_key_val = 0xE9,
		.tma_key_h_val = 0xE6,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_P_1],
	}, {
		.slvid = SPMI_SLAVE_7,
		.grpiden = 0x800,
		.mstid = SPMI_MASTER_P_1,/* spmi-m */
		.hwcid_addr = 0x209,
		.hwcid_val = 0x16,
		.swcid_addr = 0x20B,
		.swcid_val = 0x16,
		.wpk_key_addr = 0x3B1,
		.wpk_key_val = 0xE9,
		.wpk_key_h_val = 0xE6,
		.tma_key_addr = 0x3A8,
		.tma_key_val = 0xE9,
		.tma_key_h_val = 0xE6,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_P_1],
	}, {
		.slvid = SPMI_SLAVE_8,
		.grpiden = 0x800,
		.mstid = SPMI_MASTER_P_1,/* spmi-m */
		.hwcid_addr = 0x209,
		.hwcid_val = 0x16,
		.swcid_addr = 0x20B,
		.swcid_val = 0x16,
		.wpk_key_addr = 0x3B1,
		.wpk_key_val = 0xE9,
		.wpk_key_h_val = 0xE6,
		.tma_key_addr = 0x3A8,
		.tma_key_val = 0xE9,
		.tma_key_h_val = 0xE6,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_P_1],
	}, {
		.slvid = SPMI_SLAVE_15,
		.grpiden = 0x800,
		.mstid = SPMI_MASTER_P_1,/* spmi-m */
		.hwcid_addr = 0x209,
		.hwcid_val = 0x16,
		.swcid_addr = 0x20B,
		.swcid_val = 0x16,
		.wpk_key_addr = 0x3B1,
		.wpk_key_val = 0xE9,
		.wpk_key_h_val = 0xE6,
		.tma_key_addr = 0x3A8,
		.tma_key_val = 0xE9,
		.tma_key_h_val = 0xE6,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_P_1],
	},
};

#if SPMI_DEBUG
static void spmi_read_check(struct spmi_device *dev)
{
	uint8_t rdata = 0;

	spmi_ext_register_readl(dev, dev->hwcid_addr, &rdata, 1);

	if (dev->hwcid_mask) {
		if ((rdata & dev->hwcid_mask) == (dev->hwcid_val & dev->hwcid_mask))
			SPMI_INFO("%s pass, slvid:%d rdata = 0x%x\n", __func__,
				dev->slvid, rdata);
		else
			SPMI_ERR("%s fail, slvid:%d rdata = 0x%x\n", __func__,
				dev->slvid, rdata);
	} else {
		if (rdata == dev->hwcid_val)
			SPMI_INFO("%s pass, slvid:%d rdata = 0x%x\n", __func__,
				dev->slvid, rdata);
		else
			SPMI_ERR("%s fail, slvid:%d rdata = 0x%x\n", __func__,
				dev->slvid, rdata);
	}
}

void spmi_test(void)
{
	for (int k = 0; k < ARRAY_SIZE(spmi_dev); k++)
		spmi_read_check(&spmi_dev[k]);
}
#endif

int platform_pmif_spmi_init(void)
{
	spmi_device_register(spmi_dev, ARRAY_SIZE(spmi_dev));

#if SPMI_DEBUG
	spmi_test();
#endif
	return 0;
}
MTK_ARCH_INIT(platform_pmif_spmi_init);
