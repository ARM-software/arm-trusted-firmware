/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_POWER_H
#define APUSYS_POWER_H

#include <platform_def.h>

#define CFG_APU_ARDCM_ENABLE	(0)
#define CFG_CTL_RPC_BY_CE	(1)

#define APUPLL0_DEFAULT_FREQ	(800)
#define APUPLL1_DEFAULT_FREQ	(960)
#define APUPLL2_DEFAULT_FREQ	(1200)
#define APUPLL3_DEFAULT_FREQ	(1230)

enum t_acx_id {
	D_ACX0 = 0,
	ACX0,
	ACX1,
	ACX2,
	CLUSTER_NUM,
	RCX,
};

enum rcx_ao_range {
	RCX_AO_BEGIN = 0,
	PLL_ENTRY_BEGIN = 0,
	PLL_ENTRY_END = 27,
	ACC_ENTRY_BEGIN = 28,
	ACC_ENTRY_END = 37,
	RCX_AO_END = 37,
};

#define SYS_VLP			(0x000000)
#define SYS_SPM			(0x000000)
#define APU_RCX			(0x020000)
#define APU_RCX_DLA		(0x040000)
#define APU_ARE			(0x0a0000)
#define APU_ARE_REG		(0x0b0000)
#define APU_VCORE		(0x0e0000)
#define APU_MD32_MBOX		(0x0e1000)
#define APU_RPC			(0x0f0000)
#define APU_PCU			(0x0f1000)
#define APU_AO_CTL		(0x0f2000)
#define APU_ACC			(0x0f3000)
#define APU_PLL			(0x0f6000)
#define APU_RPCTOP_MDLA		(0x0F7400)
#define APU_ACX0		(0x100000)
#define APU_ACX0_RPC_LITE	(0x140000)
#define APU_ACX1		(0x200000)
#define APU_ACX1_RPC_LITE	(0x240000)
#define APU_ACX2		(0x300000)
#define APU_ACX2_RPC_LITE	(0x340000)

/* APU GRP offset define */
#define APU_GRP_0_BASE		(0x0000)
#define APU_GRP_1_BASE		(0x0400)
#define APU_GRP_2_BASE		(0x0800)
#define APU_GRP_3_BASE		(0x0C00)

#define MDLA_PLL_BASE		APU_GRP_0_BASE
#define MVPU_PLL_BASE		APU_GRP_1_BASE
#define MNOC_PLL_BASE		APU_GRP_2_BASE
#define UP_PLL_BASE		APU_GRP_3_BASE

#define MDLA_ACC_BASE		APU_GRP_0_BASE
#define MVPU_ACC_BASE		APU_GRP_1_BASE
#define MNOC_ACC_BASE		APU_GRP_2_BASE
#define UP_ACC_BASE		APU_GRP_3_BASE

/* RPC / RPC_LITE control */
#define APU_RPC_SW_TYPE0_OFF	(0x200)
#define APU_RPC_SW_TYPE1_OFF	(0x204)
#define APU_RPC_SW_TYPE2_OFF	(0x208)
#define APU_RPC_SW_TYPE3_OFF	(0x20C)
#define APU_RPC_SW_TYPE4_OFF	(0x210)
#define SW_TYPE_MVPU_MDLA_RV	BIT(0)
#define CE_ENABLE		BIT(10)
#define BUCK_PROT_SEL		BIT(20)
#define RPC_TYPE_INIT_VAL	(0x18)
#define TOP_SEL_VAL		(0xB2)
#define RPC_TOP_SEL_VAL		(0xB800D50F)

#define APUSYS_AO_CTL		(APUSYS_BASE + APU_AO_CTL)
#define APUSYS_RPC		(APUSYS_BASE + APU_RPC)
#define APUSYS_ACC		(APUSYS_BASE + APU_ACC)
#define APUSYS_PLL		(APUSYS_BASE + APU_PLL)
#define APUSYS_PCU		(APUSYS_BASE + APU_PCU)

/* ARE control */
#define ARE_VCORE_EN		BIT(20)
#define ARE_RCX_AO_EN		BIT(21)
#define ARE_VCORE_OFF		(20)
#define ARE_CONF_START		(0x04)
#define ARE_CONF_END		(0x6C)
#define ARE_REG_SIZE		(4)

/* ACC offset */
#define APU_ACC_CONFG_SET0	(0x000)
#define APU_ACC_CONFG_CLR0	(0x010)
#define APU_ACC_AUTO_CTRL_SET0	(0x084)
#define APU_ARDCM_CTRL0		(0x100)
#define APU_ARDCM_CTRL1		(0x104)

/* ACC control */
#define APU_ARDCM_CTRL0_VAL_0	(0x00000016)
#define APU_ARDCM_CTRL0_VAL_1	(0x00000036)
#define APU_ARDCM_CTRL1_VAL_0	(0x00001006)
#define APU_ARDCM_CTRL1_VAL_1	(0x07F0F006)
#define CGEN_SOC		BIT(2)
#define CLK_REQ_SW_EN		BIT(8)
#define HW_CTRL_EN		BIT(15)

/* APU PLL1C offset */
#define RG_PLLGP_LVR_REFSEL	(0x204)
#define PLL1C_PLL1_CON1		(0x20C)
#define PLL1CPLL_FHCTL_HP_EN	(0x300)
#define PLL1CPLL_FHCTL_CLK_CON	(0x308)
#define PLL1CPLL_FHCTL_RST_CON	(0x30C)
#define PLL1CPLL_FHCTL0_CFG	(0x314)
#define PLL1CPLL_FHCTL0_DDS	(0x31C)

/* PLL control */
#define RG_PLLGP_LVR_REFSEL_VAL	(0x3)
#define FHCTL_CTRL		(0x1)
#define FHCTL_NO_RESET		(0x1)
#define FHCTL_CLKEN		(0x1)
#define FHCTL_HOPPING_EN	BIT(0)
#define FHCTL_SFSTR0_EN		BIT(2)
#define RG_PLL_SDM_PCW_CHG_OFF	(31)
#define RG_PLL_POSDIV_OFF	(24)
#define FHCTL0_PLL_TGL_ORG	(31)

/* RPC offset define */
#define APU_RPC_TOP_SEL		(0x0004)
#define APU_RPC_TOP_SEL_1	(0x0018)
#define APU_RPC_HW_CON		(0x001C)
#define APU_RPC_STATUS_1	(0x0034)
#define APU_RPC_INTF_PWR_RDY	(0x0044)

/* RPC control */
#define SRAM_AOC_LHENB_SET	BIT(4)
#define	SRAM_AOC_ISO_SET	BIT(6)
#define SRAM_AOC_ISO_CLR	BIT(7)
#define PLL_AOC_ISO_EN_SET	BIT(8)
#define PLL_AOC_ISO_EN_CLR	BIT(9)
#define BUCK_ELS_EN_SET		BIT(10)
#define BUCK_ELS_EN_CLR		BIT(11)
#define BUCK_AO_RST_B_SET	BIT(12)
#define BUCK_AO_RST_B_CLR	BIT(13)
#define BUCK_PROT_REQ_SET	BIT(14)
#define BUCK_PROT_REQ_CLR	BIT(15)

/* mt6373_vbuck2 */
#define MT6373_SLAVE_ID				(0x5)
#define MT6373_RG_BUCK_VBUCK2_SET		(0x241)
#define MT6373_RG_BUCK_VBUCK2_CLR		(0x242)
#define MT6373_RG_BUCK_VBUCK2_EN_SHIFT		(2)
#define MT6373_RG_BUCK_VBUCK2_VOSEL_ADDR	(0x24e)

/* PCU initial data */
#define APU_PCUTOP_CTRL_SET		(0x0)
#define APU_PCU_BUCK_STEP_SEL		(0x0030)
#define APU_PCU_BUCK_ON_DAT0_L		(0x0080)
#define APU_PCU_BUCK_ON_DAT0_H		(0x0084)
#define APU_PCU_BUCK_ON_DAT1_L		(0x0088)
#define APU_PCU_BUCK_ON_DAT1_H		(0x008C)
#define APU_PCU_BUCK_OFF_DAT0_L		(0x00A0)
#define APU_PCU_BUCK_OFF_DAT0_H		(0x00A4)
#define APU_PCU_BUCK_ON_SLE0		(0x00C0)
#define APU_PCU_BUCK_ON_SLE1		(0x00C4)
#define VAPU_BUCK_ON_SETTLE_TIME	(0x00C8)
#define APU_PCU_PMIC_TAR_BUF1		(0x0190)
#define APU_PCU_PMIC_TAR_BUF2		(0x0194)
#define APU_PCU_PMIC_CMD		(0x0184)
#define APU_PCU_PMIC_IRQ		(0x0180)

/* PCU control */
#define PMIC_CMD_IRQ			BIT(0)
#define PMIC_IRQ_EN			BIT(2)
#define AUTO_BUCK_EN			BIT(3)
#define PMIC_PMIFID_OFF			(3)
#define PMIC_SLVID_OFF			(4)
#define PCU_CMD_OP_W			(0x7)
#define PMIC_OFF_ADDR_OFF		(16)
#define PMIC_CMD_EN			(0x1)
#define BUCK_STEP_SEL_VAL		(0x13)
#define PCU_BUCK_OFF_CMD		(0x7)

/* sram_core: mt6363_vbuck4 */
#define MT6363_RG_BUCK_VBUCK4_VOSEL_ADDR	(0x250)

/* sub_pmic */
#define BUCK_VAPU_PMIC_ID		MT6373_SLAVE_ID
#define BUCK_VAPU_PMIC_REG_VOSEL_ADDR	MT6373_RG_BUCK_VBUCK2_VOSEL_ADDR
#define BUCK_VAPU_PMIC_REG_EN_SET_ADDR	MT6373_RG_BUCK_VBUCK2_SET
#define BUCK_VAPU_PMIC_REG_EN_CLR_ADDR	MT6373_RG_BUCK_VBUCK2_CLR
#define BUCK_VAPU_PMIC_REG_EN_SHIFT	MT6373_RG_BUCK_VBUCK2_EN_SHIFT

/* vlp offset define */
#define APUSYS_AO_SRAM_CONFIG		(0x70)
#define APUSYS_AO_SRAM_SET		(0x74)
#define APUSYS_AO_SRAM_CLR		(0x78)

#define APUSYS_AO_SRAM_EN		(0x1)

#define ARE_ENTRIES(x, y)		((((y) - (x)) + 1) * 2)
#define ARE_ENTRY(x)			(((x) * 2) + 36)
#define ARE_RCX_AO_CONFIG		(0x0014)
#define ARE_RCX_AO_CONFIG_HIGH_OFF	(16)

#define APU_ACE_HW_FLAG_DIS		(APUSYS_CE_BASE + 0x05D4)
#define APU_ACE_DIS_FLAG_VAL		(0xffff7ff8)

#define OUT_CLK_FREQ_MIN		(1500)
#define DDS_SHIFT			(14)
#define BASIC_CLK_FREQ			(26)

int apusys_power_init(void);

#endif /* APUSYS_POWER_H */
