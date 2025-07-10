/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_H
#define MT_SPM_H

#include <stdint.h>
#include <stdio.h>

#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lp_rq.h>
#include <mt_spm_common_v1.h>

#define CLK_SCP_CFG_0		(CKSYS_BASE + 0x1A0)
#define INFRA_BUS_DCM_CTRL	(INFRACFG_AO_BASE + 0x070)
#define RG_AXI_DCM_DIS_EN	BIT(21)
#define RG_PLLCK_SEL_NO_SPM	BIT(22)

#define MT_SPM_TIME_GET(tm)	({ (tm) = el3_uptime(); })

#define SPM_FW_NO_RESUME	1
#define MCUSYS_MTCMOS_ON	0
#define WAKEUP_LOG_ON		0

#define MT_SPM_USING_SRCLKEN_RC
/* SPM extern operand definition */
#define MT_SPM_EX_OP_CLR_26M_RECORD		BIT(0)
#define MT_SPM_EX_OP_SET_WDT			BIT(1)
#define MT_SPM_EX_OP_NON_GENERIC_RESOURCE_REQ	BIT(2)
#define MT_SPM_EX_OP_SET_SUSPEND_MODE		BIT(3)
#define MT_SPM_EX_OP_SET_IS_ADSP		BIT(4)
#define MT_SPM_EX_OP_SRCLKEN_RC_BBLPM		BIT(5)
#define MT_SPM_EX_OP_HW_S1_DETECT		BIT(6)
#define MT_SPM_EX_OP_TRACE_LP			BIT(7)
#define MT_SPM_EX_OP_TRACE_SUSPEND		BIT(8)
#define MT_SPM_EX_OP_TRACE_TIMESTAMP_EN		BIT(9)
#define MT_SPM_EX_OP_TIME_CHECK			BIT(10)
#define MT_SPM_EX_OP_TIME_OBS			BIT(11)
#define MT_SPM_EX_OP_SET_IS_USB_HEADSET		BIT(12)
#define MT_SPM_EX_OP_SET_IS_FM_AUDIO		BIT(13)
#define MT_SPM_EX_OP_DEVICES_SAVE		BIT(14)
#define MT_SPM_EX_OP_NOTIFY_INFRA_OFF		BIT(15)

#define MT_BUS26M_EXT_LP_26M_ON_MODE	(MT_SPM_EX_OP_SET_IS_ADSP | \
					 MT_SPM_EX_OP_SET_IS_FM_AUDIO)

#define MT_VCORE_EXT_LP_VCORE_ON_MODE	(MT_SPM_EX_OP_SET_IS_ADSP | \
					 MT_SPM_EX_OP_SET_IS_FM_AUDIO)

/* EN SPM INFRA DEBUG OUT */
#define DEBUGSYS_DEBUG_EN_REG	(DBGSYS_DEM_BASE + 0x94)

/* INFRA_AO_DEBUG_CON */
#define INFRA_AO_DBG_CON0	(INFRACFG_AO_BASE + 0x500)
#define INFRA_AO_DBG_CON1	(INFRACFG_AO_BASE + 0x504)
#define INFRA_AO_DBG_CON2	(INFRACFG_AO_BASE + 0x508)
#define INFRA_AO_DBG_CON3	(INFRACFG_AO_BASE + 0x50C)

/* SPM init. related registers */
#define VLP_AO_APC_CON			(VLP_AO_DEVAPC_APB_BASE + 0xF00)
#define VLP_AO_MAS_SEC_0		(VLP_AO_DEVAPC_APB_BASE + 0xA00)
#define SCP_CFGREG_PERI_BUS_CTRL0	(SCP_CFGREG_BASE + 0x24)
#define MODULE_SW_CG_0_MASK		(INFRACFG_AO_BASE + 0x060)
#define VLP_DBG_MON_SEL0_ADDR		(VLPCFG_BUS_BASE + 0x108)
#define VLP_DBG_MON_SEL1_ADDR		(VLPCFG_BUS_BASE + 0x10C)
#define VLP_CLKSQ_CON1			(VLP_CKSYS_BASE + 0x224)
#define VLP_AP_PLL_CON3			(VLP_CKSYS_BASE + 0x264)

/* SPM SRAM Data */
#define SPM_SRAM_TIMESTAMP_START	(SPM_SRAM_BASE + 0xF80)
#define SPM_SRAM_TIMESTAMP_END		(SPM_SRAM_BASE + 0xFFC)
#define SPM_SRAM_TIMESTAMP_SIZE \
	(((SPM_SRAM_TIMESTAMP_END - SPM_SRAM_TIMESTAMP_START) >> 2) + 1)

struct spm_lp_scen;

int mt_spm_common_sodi_get_spm_pcm_flag(uint32_t  *lp, uint32_t idx);
void mt_spm_common_sodi_en(bool en);
int mt_spm_common_sodi_get_spm_lp(struct spm_lp_scen **lp);
void mt_spm_set_common_sodi_pwrctr(void);
void mt_spm_set_common_sodi_pcm_flags(void);
extern struct pwr_ctrl spm_init_ctrl;

#endif /* MT_SPM_H */
