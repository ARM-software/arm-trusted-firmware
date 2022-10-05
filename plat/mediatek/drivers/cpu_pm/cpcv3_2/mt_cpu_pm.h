/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_CPU_PM_H
#define MT_CPU_PM_H

#include <assert.h>
#include <mcucfg.h>
#include <platform_def.h>

/*
 * After ARM v8.2, the cache will turn off automatically when powering down CPU. Therefore, there
 * is no doubt to use the spin_lock here.
 */
#if !HW_ASSISTED_COHERENCY
#define MT_CPU_PM_USING_BAKERY_LOCK
#endif

#define CPU_PM_FN (MTK_CPUPM_FN_CPUPM_GET_PWR_STATE | \
		   MTK_CPUPM_FN_PWR_STATE_VALID | \
		   MTK_CPUPM_FN_PWR_ON_CORE_PREPARE | \
		   MTK_CPUPM_FN_RESUME_CORE | \
		   MTK_CPUPM_FN_SUSPEND_MCUSYS | \
		   MTK_CPUPM_FN_RESUME_MCUSYS | \
		   MTK_CPUPM_FN_SMP_INIT | \
		   MTK_CPUPM_FN_SMP_CORE_ON | \
		   MTK_CPUPM_FN_SMP_CORE_OFF)

#define CPU_PM_ASSERT(_cond) ({ \
	if (!(_cond)) { \
		INFO("[%s:%d] - %s\n", __func__, __LINE__, #_cond); \
		panic(); \
	} })

#define CPC_PWR_MASK_MCUSYS_MP0		(0xC001)

#define PER_CPU_PWR_DATA(ctrl, cluster, core) \
	do { \
		ctrl.rvbaraddr_l = CORE_RVBRADDR_##cluster##_##core##_L; \
		ctrl.arch_addr = MCUCFG_MP0_CLUSTER_CFG5; \
		ctrl.pwpr = SPM_MP##cluster##_CPU##core##_PWR_CON; \
	} while (0)

#define PER_CPU_PWR_CTRL(ctrl, cpu) ({ \
	switch (cpu) { \
	case 0: \
		PER_CPU_PWR_DATA(ctrl, 0, 0); \
		break; \
	case 1: \
		PER_CPU_PWR_DATA(ctrl, 0, 1); \
		break; \
	case 2: \
		PER_CPU_PWR_DATA(ctrl, 0, 2); \
		break; \
	case 3: \
		PER_CPU_PWR_DATA(ctrl, 0, 3); \
		break; \
	case 4: \
		PER_CPU_PWR_DATA(ctrl, 0, 4); \
		break; \
	case 5: \
		PER_CPU_PWR_DATA(ctrl, 0, 5); \
		break; \
	case 6: \
		PER_CPU_PWR_DATA(ctrl, 0, 6); \
		break; \
	case 7: \
		PER_CPU_PWR_DATA(ctrl, 0, 7); \
		break; \
	default: \
		assert(0); \
		break; \
	} })


/* MCUSYS DREQ BIG VPROC ISO control */
#define DREQ20_BIG_VPROC_ISO		(MCUCFG_BASE + 0xad8c)

/* Definition about bootup address for each core CORE_RVBRADDR_clusterid_cpuid */
#define CORE_RVBRADDR_0_0_L		(MCUCFG_BASE + 0xc900)
#define CORE_RVBRADDR_0_1_L		(MCUCFG_BASE + 0xc908)
#define CORE_RVBRADDR_0_2_L		(MCUCFG_BASE + 0xc910)
#define CORE_RVBRADDR_0_3_L		(MCUCFG_BASE + 0xc918)
#define CORE_RVBRADDR_0_4_L		(MCUCFG_BASE + 0xc920)
#define CORE_RVBRADDR_0_5_L		(MCUCFG_BASE + 0xc928)
#define CORE_RVBRADDR_0_6_L		(MCUCFG_BASE + 0xc930)
#define CORE_RVBRADDR_0_7_L		(MCUCFG_BASE + 0xc938)
#define MCUCFG_MP0_CLUSTER_CFG5		(MCUCFG_BASE + 0xc8e4)

struct cpu_pwr_ctrl {
	unsigned int rvbaraddr_l;
	unsigned int arch_addr;
	unsigned int pwpr;
};

#define MCUSYS_STATUS_PDN		BIT(0)
#define MCUSYS_STATUS_CPUSYS_PROTECT	BIT(8)
#define MCUSYS_STATUS_MCUSYS_PROTECT	BIT(9)

/* cpu_pm function ID */
enum mt_cpu_pm_user_id {
	MCUSYS_STATUS,
	CPC_COMMAND,
};

/* cpu_pm lp function ID */
enum mt_cpu_pm_lp_smc_id {
	LP_CPC_COMMAND,
	IRQS_REMAIN_ALLOC,
	IRQS_REMAIN_CTRL,
	IRQS_REMAIN_IRQ,
	IRQS_REMAIN_WAKEUP_CAT,
	IRQS_REMAIN_WAKEUP_SRC,
};

#endif /* MT_CPU_PM_H */
