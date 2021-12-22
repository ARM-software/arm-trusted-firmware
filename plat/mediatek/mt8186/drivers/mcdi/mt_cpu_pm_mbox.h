/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_CPU_PM_MBOX_H__
#define __MT_CPU_PM_MBOX_H__

/* SSPM Mbox */
/* AP Write */
#define SSPM_MBOX_SPM_CMD	(0U)
#define SSPM_MBOX_SPM_ARGS1	(1U)
#define SSPM_MBOX_SPM_ARGS2	(2U)
#define SSPM_MBOX_SPM_ARGS3	(3U)
#define SSPM_MBOX_SPM_ARGS4	(4U)
#define SSPM_MBOX_SPM_ARGS5	(5U)
#define SSPM_MBOX_SPM_ARGS6	(6U)
#define SSPM_MBOX_SPM_ARGS7	(7U)
#define SSPM_MBOX_AP_READY	(17U)

#define SSPM_MBOX_SPM_CMD_SIZE	(8U)

void mtk_set_sspm_lp_cmd(void *buf, unsigned int size);
void mtk_clr_sspm_lp_cmd(unsigned int size);

/* MCUPM Mbox */
/* AP Write */
#define MCUPM_MBOX_AP_READY		(0U)
#define MCUPM_MBOX_RESERVED_1		(1U)
#define MCUPM_MBOX_RESERVED_2		(2U)
#define MCUPM_MBOX_RESERVED_3		(3U)
#define MCUPM_MBOX_PWR_CTRL_EN		(4U)
#define MCUPM_MBOX_L3_CACHE_MODE	(5U)
#define MCUPM_MBOX_BUCK_MODE		(6U)
#define MCUPM_MBOX_ARMPLL_MODE		(7U)
/* AP Read */
#define MCUPM_MBOX_TASK_STA		(8U)
#define MCUPM_MBOX_RESERVED_9		(9U)
#define MCUPM_MBOX_RESERVED_10		(10U)
#define MCUPM_MBOX_RESERVED_11		(11U)
/* CPC mode - Read/Write */
#define MCUPM_MBOX_WAKEUP_CPU		(12U)

/* Mbox Slot: APMCU_MCUPM_MBOX_PWR_CTRL_EN (4) */
#define MCUPM_MCUSYS_CTRL		(1U << 0)
#define MCUPM_BUCK_CTRL			(1U << 1)
#define MCUPM_ARMPLL_CTRL		(1U << 2)
#define MCUPM_PWR_CTRL_MASK		((1U << 3) - 1U)

/* Mbox Slot: APMCU_MCUPM_MBOX_L3_CACHE_MODE (5) */
#define MCUPM_L3_OFF_MODE		(0U) /* default */
#define MCUPM_L3_DORMANT_MODE		(1U)
#define NF_MCUPM_L3_MODE		(2U)

/* Mbox Slot: APMCU_MCUPM_MBOX_BUCK_MODE (6) */
#define MCUPM_BUCK_NORMAL_MODE		(0U) /* default */
#define MCUPM_BUCK_LP_MODE		(1U)
#define MCUPM_BUCK_OFF_MODE		(2U)
#define NF_MCUPM_BUCK_MODE		(3U)

/* Mbox Slot: APMCU_MCUPM_MBOX_ARMPLL_MODE (7) */
#define MCUPM_ARMPLL_ON			(0U) /* default */
#define MCUPM_ARMPLL_GATING		(1U)
#define MCUPM_ARMPLL_OFF		(2U)
#define NF_MCUPM_ARMPLL_MODE		(3U)

/* Mbox Slot: APMCU_MCUPM_MBOX_TASK_STA (9) */
#define MCUPM_TASK_UNINIT		(0U)
#define MCUPM_TASK_INIT			(1U)
#define MCUPM_TASK_INIT_FINISH		(2U)
#define MCUPM_TASK_WAIT			(3U)
#define MCUPM_TASK_RUN			(4U)
#define MCUPM_TASK_PAUSE		(5U)

void mtk_set_cpu_pm_pll_mode(unsigned int mode);
int mtk_get_cpu_pm_pll_mode(void);
void mtk_set_cpu_pm_buck_mode(unsigned int mode);
int mtk_get_cpu_pm_buck_mode(void);
void mtk_set_cpu_pm_preffered_cpu(unsigned int cpuid);
int mtk_set_cpu_pm_mbox_addr(uint64_t phy_addr);

#endif
