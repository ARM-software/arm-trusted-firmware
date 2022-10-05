/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_CPU_PM_MBOX_H
#define MT_CPU_PM_MBOX_H

#include <lib/utils_def.h>

/* MCUPM Mbox */
/* AP Write */
#define MCUPM_MBOX_AP_READY		(0)
#define MCUPM_MBOX_RESERVED_1		(1)
#define MCUPM_MBOX_RESERVED_2		(2)
#define MCUPM_MBOX_RESERVED_3		(3)
#define MCUPM_MBOX_PWR_CTRL_EN		(4)
#define MCUPM_MBOX_L3_CACHE_MODE	(5)
#define MCUPM_MBOX_BUCK_MODE		(6)
#define MCUPM_MBOX_ARMPLL_MODE		(7)
/* AP Read */
#define MCUPM_MBOX_TASK_STA		(8)
#define MCUPM_MBOX_RESERVED_9		(9)
#define MCUPM_MBOX_RESERVED_10		(10)
#define MCUPM_MBOX_RESERVED_11		(11)
#define MCUPM_MBOX_WAKEUP_CPU		(12)

/* Mbox Slot: APMCU_MCUPM_MBOX_PWR_CTRL_EN (4) */
#define MCUPM_MCUSYS_CTRL		BIT(0)
#define MCUPM_BUCK_CTRL			BIT(1)
#define MCUPM_ARMPLL_CTRL		BIT(2)
#define MCUPM_CM_CTRL			BIT(3)
#define MCUPM_PWR_CTRL_MASK		(BIT(3) - 1)

/* Mbox Slot: APMCU_MCUPM_MBOX_L3_CACHE_MODE (5) */
#define MCUPM_L3_OFF_MODE		(0) /* default */
#define MCUPM_L3_DORMANT_MODE		(1)
#define NF_MCUPM_L3_MODE		(2)

/* Mbox Slot: APMCU_MCUPM_MBOX_BUCK_MODE (6) */
#define MCUPM_BUCK_NORMAL_MODE		(0) /* default */
#define MCUPM_BUCK_LP_MODE		(1)
#define MCUPM_BUCK_OFF_MODE		(2)
#define NF_MCUPM_BUCK_MODE		(3)

/* Mbox Slot: APMCU_MCUPM_MBOX_ARMPLL_MODE (7) */
#define MCUPM_ARMPLL_ON			(0) /* default */
#define MCUPM_ARMPLL_GATING		(1)
#define MCUPM_ARMPLL_OFF		(2)
#define NF_MCUPM_ARMPLL_MODE		(3)

/* Mbox Slot: APMCU_MCUPM_MBOX_TASK_STA (9) */
#define MCUPM_TASK_UNINIT		(0)
#define MCUPM_TASK_INIT			(1)
#define MCUPM_TASK_INIT_FINISH		(2)
#define MCUPM_TASK_WAIT			(3)
#define MCUPM_TASK_RUN			(4)
#define MCUPM_TASK_PAUSE		(5)


void mtk_set_mcupm_pll_mode(unsigned int mode);
int mtk_get_mcupm_pll_mode(void);

void mtk_set_mcupm_buck_mode(unsigned int mode);
int mtk_get_mcupm_buck_mode(void);

void mtk_set_cpu_pm_preffered_cpu(unsigned int cpuid);
unsigned int mtk_get_cpu_pm_preffered_cpu(void);

enum cpupm_mbox_depd_type {
	CPUPM_MBOX_WAIT_DEV_INIT,
	CPUPM_MBOX_WAIT_TASK_READY,
};

int mtk_lp_depd_condition(enum cpupm_mbox_depd_type type);

#endif /* MT_CPU_PM_MBOX_H */
