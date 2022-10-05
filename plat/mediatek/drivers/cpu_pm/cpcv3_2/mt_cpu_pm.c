/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <lib/spinlock.h>

#include <lib/mtk_init/mtk_init.h>
#include <lib/pm/mtk_pm.h>
#include "mt_cpu_pm.h"
#include "mt_cpu_pm_cpc.h"
#include "mt_cpu_pm_mbox.h"
#include <mt_lp_rm.h>
#include "mt_smp.h"
#include <mtk_mmap_pool.h>
#include <platform_def.h>

/*
 * The locker must use the bakery locker when cache turns off.
 * Using spin_lock will gain better performance.
 */
#ifdef MT_CPU_PM_USING_BAKERY_LOCK
DEFINE_BAKERY_LOCK(mt_cpu_pm_lock);
#define plat_cpu_pm_lock_init()	bakery_lock_init(&mt_cpu_pm_lock)
#define plat_cpu_pm_lock()	bakery_lock_get(&mt_cpu_pm_lock)
#define plat_cpu_pm_unlock()	bakery_lock_release(&mt_cpu_pm_lock)
#else
spinlock_t mt_cpu_pm_lock;
#define plat_cpu_pm_lock_init()
#define plat_cpu_pm_lock()	spin_lock(&mt_cpu_pm_lock)
#define plat_cpu_pm_unlock()	spin_unlock(&mt_cpu_pm_lock)
#endif

enum mt_pwr_node {
	MT_PWR_NONMCUSYS = 0,
	MT_PWR_MCUSYS_PDN,
	MT_PWR_SUSPEND,
	MT_PWR_SYSTEM_MEM,
	MT_PWR_SYSTEM_PLL,
	MT_PWR_SYSTEM_BUS,
	MT_PWR_MAX,
};

#define CPU_PM_DEPD_INIT	BIT(0)
#define CPU_PM_DEPD_READY	BIT(1)
#define CPU_PM_PLAT_READY	BIT(2)

#ifdef CPU_PM_TINYSYS_SUPPORT
#define CPU_PM_INIT_READY	(CPU_PM_DEPD_INIT | CPU_PM_DEPD_READY)
#define CPU_PM_LP_READY		(CPU_PM_INIT_READY | CPU_PM_PLAT_READY)
#else
#define CPU_PM_LP_READY		(CPU_PM_PLAT_READY)
#endif

#if CONFIG_MTK_PM_SUPPORT

#if CONFIG_MTK_CPU_SUSPEND_EN || CONFIG_MTK_SMP_EN
static void cpupm_cpu_resume_common(const struct mtk_cpupm_pwrstate *state)
{
	CPU_PM_ASSERT(state != NULL);
	mtk_cpc_core_on_hint_clr(state->info.cpuid);
}
#endif

#if CONFIG_MTK_SMP_EN
static int cpupm_cpu_pwr_on_prepare(unsigned int cpu, uintptr_t entry)
{
	struct cpu_pwr_ctrl pwr_ctrl;

	PER_CPU_PWR_CTRL(pwr_ctrl, cpu);
	mt_smp_core_bootup_address_set(&pwr_ctrl, entry);
	mt_smp_core_init_arch(0, cpu, 1, &pwr_ctrl);

	return mt_smp_power_core_on(cpu, &pwr_ctrl);
}

static void cpupm_cpu_resume_smp(const struct mtk_cpupm_pwrstate *state)
{
	CPU_PM_ASSERT(state != NULL);

	plat_cpu_pm_lock();
	mmio_clrbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG,
			GIC_WAKEUP_IGNORE(state->info.cpuid));
	plat_cpu_pm_unlock();
	cpupm_cpu_resume_common(state);
}

static void cpupm_cpu_suspend_smp(const struct mtk_cpupm_pwrstate *state)
{
	struct cpu_pwr_ctrl pwr_ctrl;

	CPU_PM_ASSERT(state != NULL);

	PER_CPU_PWR_CTRL(pwr_ctrl, state->info.cpuid);
	mt_smp_power_core_off(&pwr_ctrl);
	mmio_setbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG,
			GIC_WAKEUP_IGNORE(state->info.cpuid));
}

static void cpupm_smp_init(unsigned int cpu, uintptr_t sec_entrypoint)
{
	unsigned int reg;
	struct mtk_cpupm_pwrstate state = {
		.info = {
			.cpuid = cpu,
			.mode = MTK_CPU_PM_SMP,
		},
		.pwr = {
			.afflv = 0,
			.state_id = 0,
		},
	};

	reg = mmio_read_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG);
	if ((reg & CPC_MCUSYS_CPC_RESET_PWR_ON_EN) != 0) {
		INFO("[%s:%d][CPU_PM] reset pwr on is enabled then clear it!\n",
		     __func__, __LINE__);
		mmio_clrbits_32(CPC_MCUSYS_CPC_FLOW_CTRL_CFG, CPC_MCUSYS_CPC_RESET_PWR_ON_EN);
	}

	cpupm_cpu_pwr_on_prepare(cpu, sec_entrypoint);
	cpupm_cpu_resume_smp(&state);
}

static struct mtk_cpu_smp_ops cpcv3_2_cpu_smp = {
	.init = cpupm_smp_init,
	.cpu_pwr_on_prepare = cpupm_cpu_pwr_on_prepare,
	.cpu_on = cpupm_cpu_resume_smp,
	.cpu_off = cpupm_cpu_suspend_smp,
};

#endif /* CONFIG_MTK_SMP_EN */

#if CONFIG_MTK_CPU_SUSPEND_EN
#define CPUPM_READY_MS		(40000)
#define CPUPM_ARCH_TIME_MS(ms)	(ms * 1000 * SYS_COUNTER_FREQ_IN_MHZ)
#define CPUPM_BOOTUP_TIME_THR	CPUPM_ARCH_TIME_MS(CPUPM_READY_MS)

static int mt_pwr_nodes[MT_PWR_MAX];
static int plat_mt_lp_cpu_rc;
static unsigned int cpu_pm_status;
static unsigned int plat_prev_stateid;

static int mcusys_prepare_suspend(const struct mtk_cpupm_pwrstate *state)
{
	unsigned int stateid = state->pwr.state_id;

	if (mtk_cpc_mcusys_off_prepare() != CPC_SUCCESS) {
		goto mt_pwr_mcusysoff_break;
	}

	if (!IS_PLAT_SUSPEND_ID(stateid)) {
		if (mt_pwr_nodes[MT_PWR_SYSTEM_MEM] != 0) {
			stateid = MT_PLAT_PWR_STATE_SYSTEM_MEM;
		} else if (mt_pwr_nodes[MT_PWR_SYSTEM_PLL] != 0) {
			stateid = MT_PLAT_PWR_STATE_SYSTEM_PLL;
		} else if (mt_pwr_nodes[MT_PWR_SYSTEM_BUS] != 0) {
			stateid = MT_PLAT_PWR_STATE_SYSTEM_BUS;
		} else if (mt_pwr_nodes[MT_PWR_SUSPEND] != 0) {
			stateid = MT_PLAT_PWR_STATE_SUSPEND;
		} else {
			stateid = MT_PLAT_PWR_STATE_MCUSYS;
		}
	}

	plat_prev_stateid = stateid;
	plat_mt_lp_cpu_rc = mt_lp_rm_find_and_run_constraint(0, state->info.cpuid, stateid, NULL);

	if (plat_mt_lp_cpu_rc < 0) {
		goto mt_pwr_mcusysoff_reflect;
	}

#ifdef CPU_PM_TINYSYS_SUPPORT
	mtk_set_cpu_pm_preffered_cpu(state->info.cpuid);
#endif
	return MTK_CPUPM_E_OK;

mt_pwr_mcusysoff_reflect:
	mtk_cpc_mcusys_off_reflect();
mt_pwr_mcusysoff_break:
	plat_mt_lp_cpu_rc = -1;

	return MTK_CPUPM_E_FAIL;
}

static int mcusys_prepare_resume(const struct mtk_cpupm_pwrstate *state)
{
	if (plat_mt_lp_cpu_rc < 0) {
		return MTK_CPUPM_E_FAIL;
	}

	mt_lp_rm_reset_constraint(plat_mt_lp_cpu_rc, state->info.cpuid, plat_prev_stateid);
	mtk_cpc_mcusys_off_reflect();
	return MTK_CPUPM_E_OK;
}

static unsigned int cpupm_do_pstate_off(const mtk_pstate_type psci_state,
					const struct mtk_cpupm_pwrstate *state)
{
	unsigned int pstate = MT_CPUPM_PWR_DOMAIN_CORE;

	if (!state || (state->pwr.afflv > PLAT_MAX_PWR_LVL)) {
		CPU_PM_ASSERT(0);
	}

	switch (state->pwr.state_id) {
	case MT_PLAT_PWR_STATE_SYSTEM_MEM:
		mt_pwr_nodes[MT_PWR_SYSTEM_MEM] += 1;
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_PLL:
		mt_pwr_nodes[MT_PWR_SYSTEM_PLL] += 1;
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_BUS:
		mt_pwr_nodes[MT_PWR_SYSTEM_BUS] += 1;
		break;
	case MT_PLAT_PWR_STATE_SUSPEND:
		mt_pwr_nodes[MT_PWR_SUSPEND] += 1;
		break;
	default:
		if (!IS_MT_PLAT_PWR_STATE_MCUSYS(state->pwr.state_id) &&
		    !IS_PLAT_SYSTEM_SUSPEND(state->pwr.afflv)) {
			plat_cpu_pm_lock();
			mt_pwr_nodes[MT_PWR_NONMCUSYS] += 1;
			flush_dcache_range((uintptr_t)&mt_pwr_nodes[MT_PWR_NONMCUSYS],
					   sizeof(mt_pwr_nodes[MT_PWR_NONMCUSYS]));
			plat_cpu_pm_unlock();
		}
		break;
	}

	if ((mt_pwr_nodes[MT_PWR_NONMCUSYS] == 0) && IS_PLAT_MCUSYSOFF_AFFLV(state->pwr.afflv)) {
		/* Prepare to power down mcusys */
		if (mcusys_prepare_suspend(state) == MTK_CPUPM_E_OK) {
			mt_pwr_nodes[MT_PWR_MCUSYS_PDN] += 1;
			flush_dcache_range((uintptr_t)&mt_pwr_nodes[MT_PWR_MCUSYS_PDN],
					   sizeof(mt_pwr_nodes[MT_PWR_MCUSYS_PDN]));
			pstate |= (MT_CPUPM_PWR_DOMAIN_MCUSYS | MT_CPUPM_PWR_DOMAIN_CLUSTER);
		}
	}

	if (state->pwr.afflv >= PLAT_MT_CPU_SUSPEND_CLUSTER) {
		pstate |= MT_CPUPM_PWR_DOMAIN_CLUSTER;
	}

	if (psci_get_pstate_pwrlvl(psci_state) >= PLAT_MT_CPU_SUSPEND_CLUSTER) {
		pstate |= MT_CPUPM_PWR_DOMAIN_PERCORE_DSU;
	}

	return pstate;
}

static unsigned int cpupm_do_pstate_on(const mtk_pstate_type psci_state,
				       const struct mtk_cpupm_pwrstate *state)
{
	unsigned int pstate = MT_CPUPM_PWR_DOMAIN_CORE;

	CPU_PM_ASSERT(state != NULL);

	if (state->pwr.afflv > PLAT_MAX_PWR_LVL) {
		CPU_PM_ASSERT(0);
	}

	if (mt_pwr_nodes[MT_PWR_MCUSYS_PDN] != 0) {
		mt_pwr_nodes[MT_PWR_MCUSYS_PDN] = 0;
		flush_dcache_range((uintptr_t)&mt_pwr_nodes[MT_PWR_MCUSYS_PDN],
				   sizeof(mt_pwr_nodes[MT_PWR_MCUSYS_PDN]));
		pstate |= (MT_CPUPM_PWR_DOMAIN_MCUSYS | MT_CPUPM_PWR_DOMAIN_CLUSTER);
		mcusys_prepare_resume(state);
	}

	if (state->pwr.afflv >= PLAT_MT_CPU_SUSPEND_CLUSTER) {
		pstate |= MT_CPUPM_PWR_DOMAIN_CLUSTER;
	}

	switch (state->pwr.state_id) {
	case MT_PLAT_PWR_STATE_SYSTEM_MEM:
		mt_pwr_nodes[MT_PWR_SYSTEM_MEM] -= 1;
		CPU_PM_ASSERT(mt_pwr_nodes[MT_PWR_SYSTEM_MEM] >= 0);
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_PLL:
		mt_pwr_nodes[MT_PWR_SYSTEM_PLL] -= 1;
		CPU_PM_ASSERT(mt_pwr_nodes[MT_PWR_SYSTEM_PLL] >= 0);
		break;
	case MT_PLAT_PWR_STATE_SYSTEM_BUS:
		mt_pwr_nodes[MT_PWR_SYSTEM_BUS] -= 1;
		CPU_PM_ASSERT(mt_pwr_nodes[MT_PWR_SYSTEM_BUS] >= 0);
		break;
	case MT_PLAT_PWR_STATE_SUSPEND:
		mt_pwr_nodes[MT_PWR_SUSPEND] -= 1;
		CPU_PM_ASSERT(mt_pwr_nodes[MT_PWR_SUSPEND] >= 0);
		break;
	default:
		if (!IS_MT_PLAT_PWR_STATE_MCUSYS(state->pwr.state_id) &&
		    !IS_PLAT_SYSTEM_SUSPEND(state->pwr.afflv)) {
			plat_cpu_pm_lock();
			mt_pwr_nodes[MT_PWR_NONMCUSYS] -= 1;
			flush_dcache_range((uintptr_t)&mt_pwr_nodes[MT_PWR_NONMCUSYS],
					   sizeof(mt_pwr_nodes[MT_PWR_NONMCUSYS]));
			plat_cpu_pm_unlock();
		}
		break;
	}

	if (IS_PLAT_SYSTEM_SUSPEND(state->pwr.afflv) ||
	    (IS_PLAT_SYSTEM_RETENTION(state->pwr.afflv) && (mt_pwr_nodes[MT_PWR_SUSPEND] > 0))) {
		mtk_cpc_time_sync();
	}

	if (mt_pwr_nodes[MT_PWR_NONMCUSYS] < 0) {
		CPU_PM_ASSERT(0);
	}

	pstate |= MT_CPUPM_PWR_DOMAIN_PERCORE_DSU;

	return pstate;
}

static void cpupm_cpu_resume(const struct mtk_cpupm_pwrstate *state)
{
	cpupm_cpu_resume_common(state);
}

static void cpupm_mcusys_resume(const struct mtk_cpupm_pwrstate *state)
{
	assert(state != NULL);
}

static void cpupm_mcusys_suspend(const struct mtk_cpupm_pwrstate *state)
{
	assert(state != NULL);
}

static unsigned int cpupm_get_pstate(enum mt_cpupm_pwr_domain domain,
				     const mtk_pstate_type psci_state,
				     const struct mtk_cpupm_pwrstate *state)
{
	unsigned int pstate = 0;

	if (state == NULL) {
		return 0;
	}

	if (state->info.mode == MTK_CPU_PM_SMP) {
		pstate = MT_CPUPM_PWR_DOMAIN_CORE;
	} else {
		if (domain == CPUPM_PWR_OFF) {
			pstate = cpupm_do_pstate_off(psci_state, state);
		} else if (domain == CPUPM_PWR_ON) {
			pstate = cpupm_do_pstate_on(psci_state, state);
		} else {
			INFO("[%s:%d][CPU_PM] unknown pwr domain :%d\n",
			     __func__, __LINE__, domain);
			assert(0);
		}
	}
	return pstate;
}

static int cpupm_init(void)
{
	int ret = MTK_CPUPM_E_OK;

#ifdef CPU_PM_TINYSYS_SUPPORT
	int status;

	if ((cpu_pm_status & CPU_PM_INIT_READY) == CPU_PM_INIT_READY) {
		return MTK_CPUPM_E_OK;
	}

	if (!(cpu_pm_status & CPU_PM_DEPD_INIT)) {
		status = mtk_lp_depd_condition(CPUPM_MBOX_WAIT_DEV_INIT);
		if (status == 0) {
			plat_cpu_pm_lock();
			cpu_pm_status |= CPU_PM_DEPD_INIT;
			plat_cpu_pm_unlock();
		}
	}

	if ((cpu_pm_status & CPU_PM_DEPD_INIT) && !(cpu_pm_status & CPU_PM_DEPD_READY)) {
		status = mtk_lp_depd_condition(CPUPM_MBOX_WAIT_TASK_READY);
		if (status == 0) {
			plat_cpu_pm_lock();
			cpu_pm_status |= CPU_PM_DEPD_READY;
			plat_cpu_pm_unlock();
		}
	}

	ret = ((cpu_pm_status & CPU_PM_INIT_READY) == CPU_PM_INIT_READY) ?
	      MTK_CPUPM_E_OK : MTK_CPUPM_E_FAIL;
#endif
	return ret;
}

static int cpupm_pwr_state_valid(unsigned int afflv, unsigned int state)
{
	if (cpu_pm_status == CPU_PM_LP_READY) {
		return MTK_CPUPM_E_OK;
	}

	if (cpupm_init() != MTK_CPUPM_E_OK) {
		return MTK_CPUPM_E_FAIL;
	}

	if (read_cntpct_el0() >= (uint64_t)CPUPM_BOOTUP_TIME_THR) {
		plat_cpu_pm_lock();
		cpu_pm_status |= CPU_PM_PLAT_READY;
		plat_cpu_pm_unlock();
	}

	if (!IS_PLAT_SYSTEM_SUSPEND(afflv) && (cpu_pm_status & CPU_PM_PLAT_READY) == 0) {
		return MTK_CPUPM_E_FAIL;
	}

	return MTK_CPUPM_E_OK;
}

static struct mtk_cpu_pm_ops cpcv3_2_mcdi = {
	.get_pstate = cpupm_get_pstate,
	.pwr_state_valid = cpupm_pwr_state_valid,
	.cpu_resume = cpupm_cpu_resume,
	.mcusys_suspend = cpupm_mcusys_suspend,
	.mcusys_resume = cpupm_mcusys_resume,
};
#endif /* CONFIG_MTK_CPU_SUSPEND_EN */

#endif /* CONFIG_MTK_PM_SUPPORT */

/*
 * Depend on mtk pm methodology, the psci op init must
 * be invoked after cpu pm to avoid initialization fail.
 */
int mt_plat_cpu_pm_init(void)
{
	plat_cpu_pm_lock_init();

	mtk_cpc_init();
#if CONFIG_MTK_PM_SUPPORT

#if CONFIG_MTK_CPU_SUSPEND_EN
	register_cpu_pm_ops(CPU_PM_FN, &cpcv3_2_mcdi);
#endif /* CONFIG_MTK_CPU_SUSPEND_EN */

#if CONFIG_MTK_SMP_EN
	register_cpu_smp_ops(CPU_PM_FN, &cpcv3_2_cpu_smp);
#endif /* CONFIG_MTK_SMP_EN */

#endif /* CONFIG_MTK_PM_SUPPORT */

	INFO("[%s:%d] - CPU PM INIT finished\n", __func__, __LINE__);
	return 0;
}
MTK_ARCH_INIT(mt_plat_cpu_pm_init);

static const mmap_region_t cpu_pm_mmap[] MTK_MMAP_SECTION = {
#ifdef CPU_PM_TINYSYS_SUPPORT
#if CONFIG_MTK_PM_SUPPORT && CONFIG_MTK_CPU_SUSPEND_EN
	MAP_REGION_FLAT(CPU_EB_TCM_BASE, CPU_EB_TCM_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#endif
#endif
	{0}
};
DECLARE_MTK_MMAP_REGIONS(cpu_pm_mmap);
