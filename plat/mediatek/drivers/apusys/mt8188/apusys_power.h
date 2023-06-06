/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_POWER_H
#define APUSYS_POWER_H

#include <platform_def.h>

enum APU_CLKSRC_ID {
	PLL_CONN = 0, /* MNOC */
	PLL_UP,
	PLL_VPU,
	PLL_DLA,
	PLL_NUM,
};

enum APU_ARE_ID {
	APU_ARE0 = 0,
	APU_ARE1,
	APU_ARE2,
	APU_ARE_NUM,
};

enum APU_D4_SLV_CTRL {
	D4_SLV_OFF = 0,
	D4_SLV_ON,
};

enum APU_BACKUP_RESTORE_CTRL {
	APU_CTRL_BACKUP		= 0,
	APU_CTRL_RESTORE	= 1,
};

struct apu_restore_data {
	uint32_t reg;
	uint32_t data;
};

#define APU_POLL_STEP_US			(5)

#define OUT_CLK_FREQ_MIN			(1500)
#define BASIC_CLK_FREQ				(26)
#define DDS_SHIFT				(14)

#define APUPLL0_DEFAULT_FREQ			(900)
#define APUPLL1_DEFAULT_FREQ			(832)
#define APUPLL2_DEFAULT_FREQ			(700)
#define APUPLL3_DEFAULT_FREQ			(700)

#define APU_TOP_ON_POLLING_TIMEOUT_US		(10000)
#define APU_TOP_OFF_POLLING_TIMEOUT_US		(5 * APU_TOP_ON_POLLING_TIMEOUT_US)
#define APU_ARE_POLLING_TIMEOUT_US		(10000)

/* APU related reg */
#define APU_VCORE_BASE				(APU_RCX_VCORE_CONFIG)
#define APU_RCX_BASE				(APU_RCX_CONFIG)
#define APU_RPC_BASE				(APU_RPCTOP)
#define APU_PCU_BASE				(APU_PCUTOP)
#define APU_ARE0_BASE				(APU_ARETOP_ARE0)
#define APU_ARE1_BASE				(APU_ARETOP_ARE1)
#define APU_ARE2_BASE				(APU_ARETOP_ARE2)
#define APU_MBOX0_BASE				(APU_MBOX0)
#define APU_AO_CTL_BASE				(APU_AO_CTRL)
#define APU_PLL_BASE				(APU_PLL)
#define APU_ACC_BASE				(APU_ACC)
#define APU_ACX0_RPC_LITE_BASE			(APU_ACX0_RPC_LITE)

/* RPC offset define */
#define APU_RPC_TOP_CON				(0x0000)
#define APU_RPC_TOP_SEL				(0x0004)
#define APU_RPC_STATUS				(0x0014)
#define APU_RPC_TOP_SEL_1			(0x0018)
#define APU_RPC_HW_CON				(0x001c)
#define APU_RPC_INTF_PWR_RDY			(0x0044)
#define APU_RPC_SW_TYPE0			(0x0200)

/* RPC control */
#define SRAM_AOC_ISO_CLR			BIT(7)
#define BUCK_ELS_EN_SET				BIT(10)
#define BUCK_ELS_EN_CLR				BIT(11)
#define BUCK_AO_RST_B_SET			BIT(12)
#define BUCK_AO_RST_B_CLR			BIT(13)
#define BUCK_PROT_REQ_SET			BIT(14)
#define BUCK_PROT_REQ_CLR			BIT(15)
#define SW_TYPE					BIT(1)
#define RPC_CTRL				(0x0000009e)
#define RPC_TOP_CTRL				(0x0800501e)
#define RPC_TOP_CTRL1				BIT(20)
#define AFC_ENA					BIT(16)
#define REG_WAKEUP_SET				BIT(8)
#define REG_WAKEUP_CLR				BIT(12)
#define PWR_RDY					BIT(0)
#define PWR_OFF					(0)
#define RPC_STATUS_RDY				BIT(29)
#define RSV10					BIT(10)
#define CLR_IRQ					(0x6)
#define SLEEP_REQ				BIT(0)

/* PLL offset define */
#define PLL4H_PLL1_CON1				(0x000c)
#define PLL4H_PLL2_CON1				(0x001c)
#define PLL4H_PLL3_CON1				(0x002c)
#define PLL4H_PLL4_CON1				(0x003c)
#define PLL4HPLL_FHCTL_HP_EN			(0x0e00)
#define PLL4HPLL_FHCTL_CLK_CON			(0x0e08)
#define PLL4HPLL_FHCTL_RST_CON			(0x0e0c)
#define PLL4HPLL_FHCTL0_CFG			(0x0e3c)
#define PLL4HPLL_FHCTL0_DDS			(0x0e44)
#define PLL4HPLL_FHCTL1_CFG			(0x0e50)
#define PLL4HPLL_FHCTL1_DDS			(0x0e58)
#define PLL4HPLL_FHCTL2_CFG			(0x0e64)
#define PLL4HPLL_FHCTL2_DDS			(0x0e6c)
#define PLL4HPLL_FHCTL3_CFG			(0x0e78)
#define PLL4HPLL_FHCTL3_DDS			(0x0e80)

/* PLL control */
#define PLL4H_PLL_HP_EN				(0xf)
#define PLL4H_PLL_HP_CLKEN			(0xf)
#define PLL4H_PLL_HP_SWRSTB			(0xf)
#define FHCTL0_EN				BIT(0)
#define	SFSTR0_EN				BIT(2)
#define RG_PLL_POSDIV_MASK			(0x7)
#define RG_PLL_POSDIV_SFT			(24)
#define FHCTL_PLL_TGL_ORG			BIT(31)

/* ACC offset define */
#define APU_ACC_CONFG_SET0			(0x0000)
#define APU_ACC_CONFG_SET1			(0x0004)
#define APU_ACC_CONFG_SET2			(0x0008)
#define APU_ACC_CONFG_SET3			(0x000c)
#define APU_ACC_CONFG_CLR0			(0x0040)
#define APU_ACC_CONFG_CLR1			(0x0044)
#define APU_ACC_CONFG_CLR2			(0x0048)
#define APU_ACC_CONFG_CLR3			(0x004c)
#define APU_ACC_CLK_INV_EN_SET			(0x00e8)
#define APU_ACC_AUTO_CTRL_SET2			(0x0128)
#define APU_ACC_AUTO_CTRL_SET3			(0x012c)

/* ACC control */
#define CGEN_SOC				BIT(2)
#define HW_CTRL_EN				BIT(15)
#define CLK_REQ_SW_EN				BIT(8)
#define CLK_INV_EN				(0xaaa8)

/* ARE offset define */
#define APU_ARE_INI_CTRL			(0x0000)
#define APU_ARE_GLO_FSM				(0x0048)
#define APU_ARE_ENTRY0_SRAM_H			(0x0c00)
#define APU_ARE_ENTRY0_SRAM_L			(0x0800)
#define APU_ARE_ENTRY1_SRAM_H			(0x0c04)
#define APU_ARE_ENTRY1_SRAM_L			(0x0804)
#define APU_ARE_ENTRY2_SRAM_H			(0x0c08)
#define APU_ARE_ENTRY2_SRAM_L			(0x0808)

/* ARE control */
#define ARE_ENTRY_CFG_H				(0x00140000)
#define ARE0_ENTRY2_CFG_L			(0x004e0804)
#define ARE1_ENTRY2_CFG_L			(0x004e0806)
#define ARE2_ENTRY2_CFG_L			(0x004e0807)
#define ARE_GLO_FSM_IDLE			BIT(0)
#define ARE_ENTRY0_SRAM_H_INIT			(0x12345678)
#define ARE_ENTRY0_SRAM_L_INIT			(0x89abcdef)
#define ARE_ENTRY1_SRAM_H_INIT			(0xfedcba98)
#define ARE_ENTRY1_SRAM_L_INIT			(0x76543210)
#define ARE_CONFG_INI				BIT(2)

/* VCORE offset define */
#define APUSYS_VCORE_CG_CLR			(0x0008)

/* RCX offset define */
#define APU_RCX_CG_CLR				(0x0008)

/* SPM offset define */
#define APUSYS_BUCK_ISOLATION			(0x03ec)

/* SPM control*/
#define IPU_EXT_BUCK_ISO			(0x21)

/* apu_rcx_ao_ctrl  */
#define CSR_DUMMY_0_ADDR			(0x0024)

/* apu_rcx_ao_ctrl control */
#define VCORE_ARE_REQ				BIT(2)

/* xpu2apusys */
#define INFRA_FMEM_BUS_u_SI21_CTRL_0		(0x002c)
#define INFRA_FMEM_BUS_u_SI22_CTRL_0		(0x0044)
#define INFRA_FMEM_BUS_u_SI11_CTRL_0		(0x0048)
#define INFRA_FMEM_M6M7_BUS_u_SI24_CTRL_0	(0x01d0)

/* xpu2apusys */
#define INFRA_FMEM_BUS_u_SI21_CTRL_EN		BIT(12)
#define INFRA_FMEM_BUS_u_SI22_CTRL_EN		BIT(13)
#define INFRA_FMEM_BUS_u_SI11_CTRL_EN		BIT(11)
#define INFRA_FMEM_M6M7_BUS_u_SI24_CTRL_EN	BIT(15)

/* PCU offset define */
#define APU_PCU_CTRL_SET			(0x0000)
#define APU_PCU_BUCK_STEP_SEL			(0x0030)
#define APU_PCU_BUCK_ON_DAT0_L			(0x0080)
#define APU_PCU_BUCK_ON_DAT0_H			(0x0084)
#define APU_PCU_BUCK_ON_DAT1_L			(0x0088)
#define APU_PCU_BUCK_ON_DAT1_H			(0x008c)
#define APU_PCU_BUCK_OFF_DAT0_L			(0x00a0)
#define APU_PCU_BUCK_OFF_DAT0_H			(0x00a4)
#define APU_PCU_BUCK_OFF_DAT1_L			(0x00a8)
#define APU_PCU_BUCK_OFF_DAT1_H			(0x00ac)
#define APU_PCU_BUCK_ON_SLE0			(0x00c0)
#define APU_PCU_BUCK_ON_SLE1			(0x00c4)
#define APU_PCU_BUCK_ON_SETTLE_TIME		(0x012c)

/* PCU initial data */
#define MT6359P_RG_BUCK_VMODEM_EN_ADDR		(0x1688)
#define MT6359P_RG_LDO_VSRAM_MD_EN_ADDR		(0x1f2e)
#define BUCK_VAPU_PMIC_REG_EN_ADDR		MT6359P_RG_BUCK_VMODEM_EN_ADDR
#define BUCK_VAPU_SRAM_PMIC_REG_EN_ADDR		MT6359P_RG_LDO_VSRAM_MD_EN_ADDR

/* PCU control */
#define AUTO_BUCK_EN				BIT(16)
#define BUCK_ON_OFF_CMD_EN			(0x33)
#define BUCK_OFFSET_SFT				(16)
#define BUCK_ON_CMD				(0x1)
#define BUCK_OFF_CMD				(0x0)
#define CMD_OP					(0x4)

/* RPC lite offset define */
#define APU_RPC_SW_TYPE2			(0x0208)
#define APU_RPC_SW_TYPE3			(0x020c)
#define APU_RPC_SW_TYPE4			(0x0210)
#define APU_RPC_SW_TYPE5			(0x0214)
#define APU_RPC_SW_TYPE6			(0x0218)
#define APU_RPC_SW_TYPE7			(0x021c)
#define APU_RPC_SW_TYPE8			(0x0220)
#define APU_RPC_SW_TYPE9			(0x0224)

/* power flow sync */
#define PWR_FLOW_SYNC_REG			(0x0440)

#define CG_CLR					(0xffffffff)

int apusys_power_init(void);
int apusys_kernel_apusys_pwr_top_on(void);
int apusys_kernel_apusys_pwr_top_off(void);

#endif /* APUSYS_POWER_H */
