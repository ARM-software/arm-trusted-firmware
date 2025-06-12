/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
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

static uint16_t mt6xxx_regs[] = {
	[PMIF_INIT_DONE] =			0x0000,
	[PMIF_INF_EN] =				0x0024,
	[PMIF_ARB_EN] =				0x0150,
	[PMIF_IRQ_EVENT_EN_0] =			0x0420,
	[PMIF_IRQ_FLAG_0] =			0x0428,
	[PMIF_IRQ_CLR_0] =			0x042C,
	[PMIF_IRQ_EVENT_EN_2] =			0x0440,
	[PMIF_IRQ_FLAG_2] =			0x0448,
	[PMIF_IRQ_CLR_2] =			0x044C,
	[PMIF_WDT_CTRL] =			0x0470,
	[PMIF_WDT_EVENT_EN_1] =			0x047C,
	[PMIF_WDT_FLAG_1] =			0x0480,
	[PMIF_SWINF_2_ACC] =			0x0880,
	[PMIF_SWINF_2_WDATA_31_0] =		0x0884,
	[PMIF_SWINF_2_WDATA_63_32] =		0x0888,
	[PMIF_SWINF_2_RDATA_31_0] =		0x0894,
	[PMIF_SWINF_2_RDATA_63_32] =		0x0898,
	[PMIF_SWINF_2_VLD_CLR] =		0x08A4,
	[PMIF_SWINF_2_STA] =			0x08A8,
	[PMIF_SWINF_3_ACC] =			0x08C0,
	[PMIF_SWINF_3_WDATA_31_0] =		0x08C4,
	[PMIF_SWINF_3_WDATA_63_32] =		0x08C8,
	[PMIF_SWINF_3_RDATA_31_0] =		0x08D4,
	[PMIF_SWINF_3_RDATA_63_32] =		0x08D8,
	[PMIF_SWINF_3_VLD_CLR] =		0x08E4,
	[PMIF_SWINF_3_STA] =			0x08E8,
	/* hw mpu */
	[PMIF_PMIC_ALL_RGN_EN_1] =		0x09B0,
	[PMIF_PMIC_ALL_RGN_EN_2] =		0x0D30,
	[PMIF_PMIC_ALL_RGN_0_START] =		0x09B4,
	[PMIF_PMIC_ALL_RGN_0_END] =		0x09B8,
	[PMIF_PMIC_ALL_RGN_1_START] =		0x09BC,
	[PMIF_PMIC_ALL_RGN_1_END] =		0x09C0,
	[PMIF_PMIC_ALL_RGN_2_START] =		0x09C4,
	[PMIF_PMIC_ALL_RGN_2_END] =		0x09C8,
	[PMIF_PMIC_ALL_RGN_3_START] =		0x09CC,
	[PMIF_PMIC_ALL_RGN_3_END] =		0x09D0,
	[PMIF_PMIC_ALL_RGN_31_START] =		0x0D34,
	[PMIF_PMIC_ALL_RGN_31_END] =		0x0D38,
	[PMIF_PMIC_ALL_INVLD_SLVID] =		0x0AAC,
	[PMIF_PMIC_ALL_RGN_0_PER0] =		0x0AB0,
	[PMIF_PMIC_ALL_RGN_0_PER1] =		0x0AB4,
	[PMIF_PMIC_ALL_RGN_1_PER0] =		0x0AB8,
	[PMIF_PMIC_ALL_RGN_2_PER0] =		0x0AC0,
	[PMIF_PMIC_ALL_RGN_3_PER0] =		0x0AC8,
	[PMIF_PMIC_ALL_RGN_31_PER0] =		0x0E34,
	[PMIF_PMIC_ALL_RGN_31_PER1] =		0x0E38,
	[PMIF_PMIC_ALL_RGN_OTHERS_PER0] =	0x0BA8,
	[PMIF_PMIC_ALL_RGN_OTHERS_PER1] =	0x0BAC,
};

static uint16_t mt6xxx_spmi_regs[] = {
	[SPMI_OP_ST_CTRL] =		0x0000,
	[SPMI_GRP_ID_EN] =		0x0004,
	[SPMI_OP_ST_STA] =		0x0008,
	[SPMI_MST_SAMPL] =		0x000c,
	[SPMI_MST_REQ_EN] =		0x0010,
	[SPMI_RCS_CTRL] =		0x0014,
	[SPMI_SLV_3_0_EINT] =		0x0020,
	[SPMI_SLV_7_4_EINT] =		0x0024,
	[SPMI_SLV_B_8_EINT] =		0x0028,
	[SPMI_SLV_F_C_EINT] =		0x002c,
	[SPMI_REC_CTRL] =		0x0040,
	[SPMI_REC0] =			0x0044,
	[SPMI_REC1] =			0x0048,
	[SPMI_REC2] =			0x004c,
	[SPMI_REC3] =			0x0050,
	[SPMI_REC4] =			0x0054,
	[SPMI_REC_CMD_DEC] =		0x005c,
	[SPMI_DEC_DBG] =		0x00f8,
	[SPMI_MST_DBG] =		0x00fc,
};

struct pmif pmif_spmi_arb[] = {
	{
		.regs = mt6xxx_regs,
		.spmimst_regs = mt6xxx_spmi_regs,
		.mstid = SPMI_MASTER_0,
		.read_cmd = pmif_spmi_read_cmd,
		.write_cmd = pmif_spmi_write_cmd,
	},
	{
		.regs = mt6xxx_regs,
		.spmimst_regs = mt6xxx_spmi_regs,
		.mstid = SPMI_MASTER_1,
		.read_cmd = pmif_spmi_read_cmd,
		.write_cmd = pmif_spmi_write_cmd,
	},
	{
		.base = (unsigned int *)PMIF_SPMI_P_BASE,
		.regs = mt6xxx_regs,
		.spmimst_base = (unsigned int *)SPMI_MST_P_BASE,
		.spmimst_regs = mt6xxx_spmi_regs,
		.mstid = SPMI_MASTER_P_1,
		.read_cmd = pmif_spmi_read_cmd,
		.write_cmd = pmif_spmi_write_cmd,
	},
};

static struct spmi_device spmi_dev[] = {
	{
		.slvid = SPMI_SLAVE_7, /* MT6319 */
		.grpiden = 0x800,
		.type = BUCK_CPU,
		.type_id = BUCK_CPU_ID,
		.mstid = SPMI_MASTER_P_1,/* spmi-p */
		.hwcid_addr = 0x09,
		.hwcid_val = 0x15,
		.swcid_addr = 0x0B,
		.swcid_val = 0x15,
		.wpk_key_addr = 0x3A8,
		.wpk_key_val = 0x6315,
		.tma_key_addr = 0x39F,
		.tma_key_val = 0x9CEA,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_P_1],
	},
	{
		.slvid = SPMI_SLAVE_8, /* MT6319 */
		.grpiden = 0x800,
		.type = BUCK_CPU,
		.type_id = BUCK_CPU_ID,
		.mstid = SPMI_MASTER_P_1,/* spmi-p */
		.hwcid_addr = 0x09,
		.hwcid_val = 0x15,
		.swcid_addr = 0x0B,
		.swcid_val = 0x15,
		.wpk_key_addr = 0x3A8,
		.wpk_key_val = 0x6315,
		.tma_key_addr = 0x39F,
		.tma_key_val = 0x9CEA,
		.pmif_arb = &pmif_spmi_arb[SPMI_MASTER_P_1],
	},
};

int platform_pmif_spmi_init(void)
{
/*
 * The MT8189 chipset comes in two variants: MT8189G and MT8189H. The
 * MT8189G variant uses a single PMIC IC (MT6319), whereas the MT8189H
 * variant uses two PMIC ICs. To ensure driver compatibility, we utilize
 * the CPU ID and segment ID to accurately determine the required number
 * of SPMIF instances.
 */
	if (mmio_read_32((uintptr_t)CHIP_ID_REG) == MTK_CPU_ID_MT8189 &&
	    mmio_read_32((uintptr_t)CPU_SEG_ID_REG) == MTK_CPU_SEG_ID_MT8189G)
		spmi_device_register(spmi_dev, 1);
	else
		spmi_device_register(spmi_dev, ARRAY_SIZE(spmi_dev));

	return 0;
}
MTK_ARCH_INIT(platform_pmif_spmi_init);
