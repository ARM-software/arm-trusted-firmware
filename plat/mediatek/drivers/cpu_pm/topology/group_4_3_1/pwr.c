/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "../inc/pwr_topology.h"
#include <lib/pm/mtk_pm.h>
#include <lpm/mt_lp_rm.h>

#define GROUP_CPU_ALL		0x7
#define GROUP_0_CPUID_MAX	3
#define GROUP_1_CPUID_MAX	6
#define GROUP_2_CPUID_MAX	7

#define IS_ALL_GROUP_OFF(_mask) ((_mask & GROUP_CPU_ALL) == GROUP_CPU_ALL)

#define GET_GROUPID(_cpuid, _gid) ({ \
	if (_cpuid <= GROUP_0_CPUID_MAX) \
		_gid = 0; \
	else if (_cpuid <= GROUP_1_CPUID_MAX) \
		_gid = 1; \
	else \
		_gid = 2; })

#define GET_GROUPMASK(_cpuid, _gmask) ({ \
	if (_cpuid <= GROUP_0_CPUID_MAX) \
		_gmask = BIT(0); \
	else if (_cpuid <= GROUP_1_CPUID_MAX) \
		_gmask = BIT(1);\
	else if (_cpuid <= GROUP_2_CPUID_MAX) \
		_gmask = BIT(2); \
	else \
		_gmask = 0; })

#ifdef MT_CPU_PM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(mt_pwr_lock);
#define plat_pwr_lock_init() bakery_lock_init(&mt_pwr_lock)
#define plat_pwr_lock() bakery_lock_get(&mt_pwr_lock)
#define plat_pwr_unlock() bakery_lock_release(&mt_pwr_lock)
#else
spinlock_t mt_pwr_lock;
#define plat_pwr_lock_init()
#define plat_pwr_lock() spin_lock(&mt_pwr_lock)
#define plat_pwr_unlock() spin_unlock(&mt_pwr_lock)
#endif /* MT_CPU_PM_USING_BAKERY_LOCK */

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN && \
!CPU_PM_DOMAIN_CORE_ONLY
static unsigned int cpu_groupmask;
#endif

unsigned int pwr_domain_coordination(enum pwr_domain_status pwr,
				     const mtk_pstate_type psci_state,
				     const struct mtk_cpupm_pwrstate *state,
				     afflv_prepare fn)
{
	unsigned int pstate = MT_CPUPM_PWR_DOMAIN_CORE;

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN && \
!CPU_PM_DOMAIN_CORE_ONLY
	struct pwr_toplogy tp;

	if (state->pwr.afflv >= PLAT_MT_CPU_SUSPEND_CLUSTER) {
		unsigned int fgmask;

		if (state->info.cpuid >= PLATFORM_CORE_COUNT)
			assert(0);

		GET_GROUPMASK(state->info.cpuid, tp.cur_group_bit);

		plat_pwr_lock();
		if ((pwr == PWR_DOMAIN_OFF) || (pwr == PWR_DOMAIN_SMP_OFF)) {
			tp.group = (cpu_groupmask | tp.cur_group_bit);
			fgmask = tp.group;
		} else {
			tp.group = (cpu_groupmask & ~tp.cur_group_bit);
			fgmask = cpu_groupmask;
		}
		fn(1, state, &tp);
		cpu_groupmask = tp.group;

		plat_pwr_unlock();
		if (IS_ALL_GROUP_OFF(fgmask))
			pstate |= MT_CPUPM_PWR_DOMAIN_CLUSTER;
	}

	/* Skip to process smp */
	if (pwr > PWR_DOMAIN_OFF)
		return pstate;

	if (psci_get_pstate_pwrlvl(psci_state) >= PLAT_MT_CPU_SUSPEND_CLUSTER)
		pstate |= MT_CPUPM_PWR_DOMAIN_PERCORE_DSU;

	if (IS_PLAT_MCUSYSOFF_AFFLV(state->pwr.afflv)) {
		int ret = MTK_CPUPM_E_OK;

		if (fn)
			ret = fn(state->pwr.afflv, state, &tp);

		if (ret == MTK_CPUPM_E_OK)
			pstate |= MT_CPUPM_PWR_DOMAIN_MCUSYS;
	}
#endif
	return pstate;
}

void pwr_topology_init(void)
{
#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN && \
!CPU_PM_DOMAIN_CORE_ONLY
	cpu_groupmask = GROUP_CPU_ALL;
#endif

	plat_pwr_lock_init();
}
