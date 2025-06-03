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

#define CLK_SCP_CFG_0 (CKSYS_BASE + 0x200)
#define CLK_SCP_CFG_1 (CKSYS_BASE + 0x210)
#define INFRA_BUS_DCM_CTRL (INFRACFG_AO_BASE + 0x070)
#define RG_AXI_DCM_DIS_EN BIT(21)
#define RG_PLLCK_SEL_NO_SPM BIT(22)
#define MT_SPM_TIME_GET(tm) ({ (tm) = el3_uptime(); })
#define SPM_FW_NO_RESUME 1
#define MCUSYS_MTCMOS_ON 0
#define WAKEUP_LOG_ON 0
#define PMIC_ONLV 1

#define MT_SPM_USING_SRCLKEN_RC
/* SPM extern operand definition */
#define MT_SPM_EX_OP_CLR_26M_RECORD BIT(0)
#define MT_SPM_EX_OP_SET_WDT BIT(1)
#define MT_SPM_EX_OP_NON_GENERIC_RESOURCE_REQ BIT(2)
#define MT_SPM_EX_OP_SET_SUSPEND_MODE BIT(3)
#define MT_SPM_EX_OP_SET_IS_ADSP BIT(4)
#define MT_SPM_EX_OP_SRCLKEN_RC_BBLPM BIT(5)
#define MT_SPM_EX_OP_HW_S1_DETECT BIT(6)
#define MT_SPM_EX_OP_TRACE_LP BIT(7)
#define MT_SPM_EX_OP_TRACE_SUSPEND BIT(8)
#define MT_SPM_EX_OP_TRACE_TIMESTAMP_EN BIT(9)
#define MT_SPM_EX_OP_TIME_CHECK BIT(10)
#define MT_SPM_EX_OP_TIME_OBS BIT(11)
#define MT_SPM_EX_OP_SET_IS_USB_HEADSET BIT(12)
#define MT_SPM_EX_OP_SET_IS_FM_AUDIO BIT(13)
#define MT_SPM_EX_OP_DEVICES_SAVE BIT(14)
#define MT_SPM_EX_OP_DISABLE_VCORE_LP BIT(15)

#define MT_BUS26M_EXT_LP_26M_ON_MODE \
	(MT_SPM_EX_OP_SET_IS_ADSP | MT_SPM_EX_OP_SET_IS_FM_AUDIO)

#define MT_VCORE_EXT_LP_VCORE_ON_MODE                                 \
	(MT_SPM_EX_OP_SET_IS_ADSP | MT_SPM_EX_OP_SET_IS_USB_HEADSET | \
	 MT_SPM_EX_OP_SET_IS_FM_AUDIO)

/* EN SPM INFRA DEBUG OUT */
#define DEBUGSYS_DEBUG_EN_REG (DBGSYS_DEM_BASE + 0x94)

/* INFRA_AO_DEBUG_CON */
#define INFRA_AO_DBG_CON0 (INFRACFG_AO_BASE + 0x500)
#define INFRA_AO_DBG_CON1 (INFRACFG_AO_BASE + 0x504)
#define INFRA_AO_DBG_CON2 (INFRACFG_AO_BASE + 0x508)
#define INFRA_AO_DBG_CON3 (INFRACFG_AO_BASE + 0x50C)

#endif /* MT_SPM_H */
