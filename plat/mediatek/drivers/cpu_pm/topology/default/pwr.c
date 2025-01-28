/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <lib/spinlock.h>
#include <platform_def.h>

#include "../inc/pwr_topology.h"
#include <lib/pm/mtk_pm.h>
#include <lpm/mt_lp_rm.h>

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
#endif

enum mt_pwr_domain_node {
	MT_PWR_NONMCUSYS = 0,
	MT_PWR_MCUSYS_PDN,
	MT_PWR_MAX
};

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN && \
!CPU_PM_DOMAIN_CORE_ONLY
static int mt_pwr_domain_st[MT_PWR_MAX];
#endif

#define ALL_IN_ONE_GROUP	0xFF
unsigned int pwr_domain_coordination(enum pwr_domain_status pwr,
				     const mtk_pstate_type psci_state,
				     const struct mtk_cpupm_pwrstate *state,
				     afflv_prepare fn)
{
	unsigned int pstate = 0;

#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN && \
!CPU_PM_DOMAIN_CORE_ONLY
	unsigned int is_flush = 0;
	struct pwr_toplogy tp = {
		.cur_group_bit = ALL_IN_ONE_GROUP,
		.group = ALL_IN_ONE_GROUP,
	};

	/* Skip to process smp */
	if (pwr > PWR_DOMAIN_OFF)
		return pstate;

	if (!IS_MT_PLAT_PWR_STATE_MCUSYS(state->pwr.state_id)) {
		plat_pwr_lock();
		if (pwr == PWR_DOMAIN_OFF)
			mt_pwr_domain_st[MT_PWR_NONMCUSYS] += 1;
		else
			mt_pwr_domain_st[MT_PWR_NONMCUSYS] -= 1;
		flush_dcache_range(
			(uintptr_t)&mt_pwr_domain_st[MT_PWR_NONMCUSYS],
			sizeof(mt_pwr_domain_st[MT_PWR_NONMCUSYS]));
		plat_pwr_unlock();
	}

	plat_pwr_lock();
	if (state->pwr.afflv >= PLAT_MT_CPU_SUSPEND_CLUSTER)
		pstate |= MT_CPUPM_PWR_DOMAIN_CLUSTER;

	if (psci_get_pstate_pwrlvl(psci_state) >= PLAT_MT_CPU_SUSPEND_CLUSTER)
		pstate |= MT_CPUPM_PWR_DOMAIN_PERCORE_DSU;

	if (pwr == PWR_DOMAIN_OFF) {
		if (IS_PLAT_MCUSYSOFF_AFFLV(state->pwr.afflv) &&
		    (mt_pwr_domain_st[MT_PWR_NONMCUSYS] == 0)) {
			int ret = MTK_CPUPM_E_OK;

			tp.group = ALL_IN_ONE_GROUP;
			if (fn)
				ret = fn(1, state, &tp);

			if (ret == MTK_CPUPM_E_OK) {
				pstate |= MT_CPUPM_PWR_DOMAIN_MCUSYS;
				mt_pwr_domain_st[MT_PWR_MCUSYS_PDN] += 1;
				is_flush = 1;
			}
		}
	} else {
		if (mt_pwr_domain_st[MT_PWR_MCUSYS_PDN]) {
			tp.group = 0x0;
			if (fn)
				fn(1, state, &tp);
			pstate |= MT_CPUPM_PWR_DOMAIN_MCUSYS;
			mt_pwr_domain_st[MT_PWR_MCUSYS_PDN] -= 1;
			is_flush = 1;
		}
		if (mt_pwr_domain_st[MT_PWR_NONMCUSYS] < 0)
			assert(0);
	}

	if (is_flush)
		flush_dcache_range(
			(uintptr_t)&mt_pwr_domain_st[MT_PWR_MCUSYS_PDN],
			sizeof(mt_pwr_domain_st[MT_PWR_MCUSYS_PDN]));

	plat_pwr_unlock();
#endif
	return pstate;
}

void pwr_topology_init(void)
{
	plat_pwr_lock_init();
}
