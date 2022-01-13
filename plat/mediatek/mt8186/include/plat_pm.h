/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PM_H
#define PLAT_PM_H

#include <lib/utils_def.h>

#ifndef __ASSEMBLY__
extern uintptr_t mtk_suspend_footprint_addr;
extern uintptr_t mtk_suspend_timestamp_addr;

#define MT_PLAT_PWR_STATE_CPU			U(1)
#define MT_PLAT_PWR_STATE_CLUSTER		U(2)
#define MT_PLAT_PWR_STATE_MCUSYS		U(3)
#define MT_PLAT_PWR_STATE_SUSPEND2IDLE		U(8)
#define MT_PLAT_PWR_STATE_SYSTEM_SUSPEND	U(9)

#define MTK_LOCAL_STATE_RUN			U(0)
#define MTK_LOCAL_STATE_RET			U(1)
#define MTK_LOCAL_STATE_OFF			U(2)

#define MTK_AFFLVL_CPU				U(0)
#define MTK_AFFLVL_CLUSTER			U(1)
#define MTK_AFFLVL_MCUSYS			U(2)
#define MTK_AFFLVL_SYSTEM			U(3)

void mtk_suspend_footprint_log(int idx);
void mtk_suspend_timestamp_log(int idx);

int mt_cluster_ops(int cputop_mpx, int mode, int state);
int mt_core_ops(int cpux, int state);

#define IS_CLUSTER_OFF_STATE(s)					\
		is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_CLUSTER])
#define IS_MCUSYS_OFF_STATE(s)					\
		is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_MCUSYS])
#define IS_SYSTEM_SUSPEND_STATE(s)				\
		is_local_state_off(s->pwr_domain_state[MTK_AFFLVL_SYSTEM])

/* SMC secure magic number */
#define SPM_LP_SMC_MAGIC	(0xDAF10000)

#define IS_SPM_LP_SMC(_type, _id)	(_id == (SPM_LP_SMC_MAGIC | _type))

enum mtk_suspend_mode {
	MTK_MCDI_MODE		= 1U,
	MTK_IDLEDRAM_MODE	= 2U,
	MTK_IDLESYSPLL_MODE	= 3U,
	MTK_IDLEBUS26M_MODE	= 4U,
	MTK_SUSPEND_MODE	= 5U,
};
#endif

enum mt8169_idle_model {
	IDLE_MODEL_START		= 0U,
	IDLE_MODEL_RESOURCE_HEAD	= IDLE_MODEL_START,
	IDLE_MODEL_BUS26M		= IDLE_MODEL_RESOURCE_HEAD,
	IDLE_MODEL_SYSPLL		= 1U,
	IDLE_MODEL_DRAM			= 2U,
	IDLE_MODEL_NUM			= 3U,
};

#define footprint_addr(cpu)		(mtk_suspend_footprint_addr + (cpu << 2))
#define timestamp_addr(cpu, idx)	(mtk_suspend_timestamp_addr +	\
					 ((cpu * MTK_SUSPEND_TIMESTAMP_MAX + idx) << 3))

#define MTK_SUSPEND_FOOTPRINT_ENTER_CPUIDLE		(0U)
#define MTK_SUSPEND_FOOTPRINT_BEFORE_ATF		(1U)
#define MTK_SUSPEND_FOOTPRINT_ENTER_ATF			(2U)
#define MTK_SUSPEND_FOOTPRINT_RESERVE_P1		(3U)
#define MTK_SUSPEND_FOOTPRINT_RESERVE_P2		(4U)
#define MTK_SUSPEND_FOOTPRINT_ENTER_SPM_SUSPEND		(5U)
#define MTK_SUSPEND_FOOTPRINT_LEAVE_SPM_SUSPEND		(6U)
#define MTK_SUSPEND_FOOTPRINT_BEFORE_WFI		(7U)
#define MTK_SUSPEND_FOOTPRINT_AFTER_WFI			(8U)
#define MTK_SUSPEND_FOOTPRINT_BEFORE_MMU		(9U)
#define MTK_SUSPEND_FOOTPRINT_AFTER_MMU			(10U)
#define MTK_SUSPEND_FOOTPRINT_ENTER_SPM_SUSPEND_FINISH	(11U)
#define MTK_SUSPEND_FOOTPRINT_LEAVE_SPM_SUSPEND_FINISH	(12U)
#define MTK_SUSPEND_FOOTPRINT_LEAVE_ATF			(13U)
#define MTK_SUSPEND_FOOTPRINT_AFTER_ATF			(14U)
#define MTK_SUSPEND_FOOTPRINT_LEAVE_CPUIDLE		(15U)

#define MTK_SUSPEND_TIMESTAMP_ENTER_CPUIDLE		(0U)
#define MTK_SUSPEND_TIMESTAMP_BEFORE_ATF		(1U)
#define MTK_SUSPEND_TIMESTAMP_ENTER_ATF			(2U)
#define MTK_SUSPEND_TIMESTAMP_BEFORE_L2_FLUSH		(3U)
#define MTK_SUSPEND_TIMESTAMP_AFTER_L2_FLUSH		(4U)
#define MTK_SUSPEND_TIMESTAMP_ENTER_SPM_SUSPEND		(5U)
#define MTK_SUSPEND_TIMESTAMP_LEAVE_SPM_SUSPEND		(6U)
#define MTK_SUSPEND_TIMESTAMP_GIC_P1			(7U)
#define MTK_SUSPEND_TIMESTAMP_GIC_P2			(8U)
#define MTK_SUSPEND_TIMESTAMP_BEFORE_WFI		(9U)
#define MTK_SUSPEND_TIMESTAMP_AFTER_WFI			(10U)
#define MTK_SUSPEND_TIMESTAMP_RESERVE_P1		(11U)
#define MTK_SUSPEND_TIMESTAMP_RESERVE_P2		(12U)
#define MTK_SUSPEND_TIMESTAMP_GIC_P3			(13U)
#define MTK_SUSPEND_TIMESTAMP_GIC_P4			(14U)
#define MTK_SUSPEND_TIMESTAMP_ENTER_SPM_SUSPEND_FINISH	(15U)
#define MTK_SUSPEND_TIMESTAMP_LEAVE_SPM_SUSPEND_FINISH	(16U)
#define MTK_SUSPEND_TIMESTAMP_LEAVE_ATF			(17U)
#define MTK_SUSPEND_TIMESTAMP_AFTER_ATF			(18U)
#define MTK_SUSPEND_TIMESTAMP_LEAVE_CPUIDLE		(19U)
#define MTK_SUSPEND_TIMESTAMP_MAX			(20U)

/*
 * definition platform power state menas.
 * PLAT_MT_SYSTEM_SUSPEND	- system suspend pwr level
 * PLAT_MT_CPU_SUSPEND_CLUSTER	- cluster off pwr level
 */
#define PLAT_MT_SYSTEM_SUSPEND		PLAT_MAX_OFF_STATE
#define PLAT_MT_CPU_SUSPEND_CLUSTER	PLAT_MAX_RET_STATE

#define IS_PLAT_SYSTEM_SUSPEND(aff)	(aff == PLAT_MT_SYSTEM_SUSPEND)
#define IS_PLAT_SYSTEM_RETENTION(aff)	(aff >= PLAT_MAX_RET_STATE)

#define IS_PLAT_SUSPEND2IDLE_ID(stateid)\
	(stateid == MT_PLAT_PWR_STATE_SUSPEND2IDLE)

#define IS_PLAT_SUSPEND_ID(stateid)			\
	((stateid == MT_PLAT_PWR_STATE_SUSPEND2IDLE)	\
	|| (stateid == MT_PLAT_PWR_STATE_SYSTEM_SUSPEND))

#endif /* PLAT_PM_H */
