/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <lib/psci/psci.h>
#include <lib/utils.h>
#ifdef MTK_PUBEVENT_ENABLE
#include <vendor_pubsub_events.h>
#endif
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <dfd.h>
#include <lib/mtk_init/mtk_init.h>
#include <lib/pm/mtk_pm.h>
#include <mt_gic_v3.h>
#include <platform_def.h>

#define IS_AFFLV_PUBEVENT(_pstate) \
	((_pstate & (MT_CPUPM_PWR_DOMAIN_MCUSYS | MT_CPUPM_PWR_DOMAIN_CLUSTER)) != 0)

#ifdef MTK_PUBEVENT_ENABLE
#define MT_CPUPM_EVENT_PWR_ON(x) ({ \
	PUBLISH_EVENT_ARG(mt_cpupm_publish_pwr_on, (const void *)(x)); })

#define MT_CPUPM_EVENT_PWR_OFF(x) ({ \
	PUBLISH_EVENT_ARG(mt_cpupm_publish_pwr_off, (const void *)(x)); })

#define MT_CPUPM_EVENT_AFFLV_PWR_ON(x) ({ \
	PUBLISH_EVENT_ARG(mt_cpupm_publish_afflv_pwr_on, (const void *)(x)); })

#define MT_CPUPM_EVENT_AFFLV_PWR_OFF(x) ({ \
	PUBLISH_EVENT_ARG(mt_cpupm_publish_afflv_pwr_off, (const void *)(x)); })

#else
#define MT_CPUPM_EVENT_PWR_ON(x) ({ (void)x; })
#define MT_CPUPM_EVENT_PWR_OFF(x) ({ (void)x; })
#define MT_CPUPM_EVENT_AFFLV_PWR_ON(x) ({ (void)x; })
#define MT_CPUPM_EVENT_AFFLV_PWR_OFF(x) ({ (void)x; })
#endif

/*
 * The cpu require to cluster power stattus
 * [0] : The cpu require cluster power down
 * [1] : The cpu require cluster power on
 */
#define coordinate_cluster(onoff) write_clusterpwrdn_el1(onoff)
#define coordinate_cluster_pwron() coordinate_cluster(1)
#define coordinate_cluster_pwroff() coordinate_cluster(0)

/* defaultly disable all functions */
#define MTK_CPUPM_FN_MASK_DEFAULT	(0)

struct mtk_cpu_pwr_ctrl {
	unsigned int fn_mask;
	struct mtk_cpu_pm_ops *ops;
	struct mtk_cpu_smp_ops *smp;
};

static struct mtk_cpu_pwr_ctrl mtk_cpu_pwr = {
	.fn_mask = MTK_CPUPM_FN_MASK_DEFAULT,
	.ops = NULL,
};

#define IS_CPUIDLE_FN_ENABLE(x)	((mtk_cpu_pwr.ops != NULL) && ((mtk_cpu_pwr.fn_mask & x) != 0))
#define IS_CPUSMP_FN_ENABLE(x)	((mtk_cpu_pwr.smp != NULL) && ((mtk_cpu_pwr.fn_mask & x) != 0))

/* per-cpu power state */
static unsigned int armv8_2_power_state[PLATFORM_CORE_COUNT];

#define armv8_2_get_pwr_stateid(cpu) psci_get_pstate_id(armv8_2_power_state[cpu])

static unsigned int get_mediatek_pstate(unsigned int domain, unsigned int psci_state,
					struct mtk_cpupm_pwrstate *state)
{
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_CPUPM_GET_PWR_STATE)) {
		return mtk_cpu_pwr.ops->get_pstate(domain, psci_state, state);
	}

	return 0;
}

unsigned int armv8_2_get_pwr_afflv(const psci_power_state_t *state_info)
{
	int i;

	for (i = (int)PLAT_MAX_PWR_LVL; i >= (int)PSCI_CPU_PWR_LVL; i--) {
		if (is_local_state_run(state_info->pwr_domain_state[i]) == 0) {
			return (unsigned int) i;
		}
	}

	return PSCI_INVALID_PWR_LVL;
}

/* MediaTek mcusys power on control interface */
static void armv8_2_mcusys_pwr_on_common(const struct mtk_cpupm_pwrstate *state)
{
	gicv3_distif_init();
	mt_gic_distif_restore();
	gic_sgi_restore_all();

	dfd_resume();

	/* Add code here that behavior before system enter mcusys'on */
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_RESUME_MCUSYS)) {
		mtk_cpu_pwr.ops->mcusys_resume(state);
	}
}

/* MediaTek mcusys power down control interface */
static void armv8_2_mcusys_pwr_dwn_common(const struct mtk_cpupm_pwrstate *state)
{
	mt_gic_distif_save();
	gic_sgi_save_all();

	/* Add code here that behaves before entering mcusys off */
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_SUSPEND_MCUSYS)) {
		mtk_cpu_pwr.ops->mcusys_suspend(state);
	}
}

/* MediaTek Cluster power on control interface */
static void armv8_2_cluster_pwr_on_common(const struct mtk_cpupm_pwrstate *state)
{
	/* Add code here that behavior before system enter cluster'on */
#if defined(MTK_CM_MGR) && !defined(MTK_FPGA_EARLY_PORTING)
	/* init cpu stall counter */
	init_cpu_stall_counter_all();
#endif

	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_RESUME_CLUSTER)) {
		mtk_cpu_pwr.ops->cluster_resume(state);
	}
}

/* MediaTek Cluster power down control interface */
static void armv8_2_cluster_pwr_dwn_common(const struct mtk_cpupm_pwrstate *state)
{
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_SUSPEND_CLUSTER)) {
		mtk_cpu_pwr.ops->cluster_suspend(state);
	}
}

/* MediaTek CPU power on control interface */
static void armv8_2_cpu_pwr_on_common(const struct mtk_cpupm_pwrstate *state, unsigned int pstate)
{
	coordinate_cluster_pwron();

	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());

	/* If MCUSYS has been powered down then restore GIC redistributor for all CPUs. */
	if (IS_PLAT_SYSTEM_RETENTION(state->pwr.afflv)) {
		mt_gic_rdistif_restore_all();
	} else {
		mt_gic_rdistif_restore();
	}
}

/* MediaTek CPU power down control interface */
static void armv8_2_cpu_pwr_dwn_common(const struct mtk_cpupm_pwrstate *state, unsigned int pstate)
{
	if ((pstate & MT_CPUPM_PWR_DOMAIN_PERCORE_DSU) != 0) {
		coordinate_cluster_pwroff();
	}

	mt_gic_rdistif_save();
	gicv3_cpuif_disable(plat_my_core_pos());
	gicv3_rdistif_off(plat_my_core_pos());
}

static void armv8_2_cpu_pwr_resume(const struct mtk_cpupm_pwrstate *state, unsigned int pstate)
{
	armv8_2_cpu_pwr_on_common(state, pstate);
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_RESUME_CORE)) {
		mtk_cpu_pwr.ops->cpu_resume(state);
	}
}

static void armv8_2_cpu_pwr_suspend(const struct mtk_cpupm_pwrstate *state, unsigned int pstate)
{
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_SUSPEND_CORE)) {
		mtk_cpu_pwr.ops->cpu_suspend(state);
	}
	armv8_2_cpu_pwr_dwn_common(state, pstate);
}

static void armv8_2_cpu_pwr_on(const struct mtk_cpupm_pwrstate *state, unsigned int pstate)
{
	armv8_2_cpu_pwr_on_common(state, pstate);

	if (IS_CPUSMP_FN_ENABLE(MTK_CPUPM_FN_SMP_CORE_ON)) {
		mtk_cpu_pwr.smp->cpu_on(state);
	}
}

static void armv8_2_cpu_pwr_off(const struct mtk_cpupm_pwrstate *state, unsigned int pstate)
{
	if (IS_CPUSMP_FN_ENABLE(MTK_CPUPM_FN_SMP_CORE_OFF)) {
		mtk_cpu_pwr.smp->cpu_off(state);
	}
	armv8_2_cpu_pwr_dwn_common(state, pstate);
}

/* MediaTek PSCI power domain */
static int armv8_2_power_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	int cpu = plat_core_pos_by_mpidr(mpidr);
	uintptr_t entry = plat_pm_get_warm_entry();

	if (IS_CPUSMP_FN_ENABLE(MTK_CPUPM_FN_PWR_ON_CORE_PREPARE)) {
		if (mtk_cpu_pwr.smp->cpu_pwr_on_prepare(cpu, entry) != 0) {
			ret = PSCI_E_DENIED;
		}
	}
	INFO("CPU %u power domain prepare on\n", cpu);
	return ret;
}

/* MediaTek PSCI power domain */
static void armv8_2_power_domain_on_finish(const psci_power_state_t *state)
{
	struct mt_cpupm_event_data nb;
	unsigned int pstate = (MT_CPUPM_PWR_DOMAIN_CORE | MT_CPUPM_PWR_DOMAIN_PERCORE_DSU);
	struct mtk_cpupm_pwrstate pm_state = {
		.info = {
			.cpuid = plat_my_core_pos(),
			.mode = MTK_CPU_PM_SMP,
		},
		.pwr = {
			.afflv = armv8_2_get_pwr_afflv(state),
			.state_id = 0x0,
		},
	};

	armv8_2_cpu_pwr_on(&pm_state, pstate);

	nb.cpuid = pm_state.info.cpuid;
	nb.pwr_domain = pstate;
	MT_CPUPM_EVENT_PWR_ON(&nb);

	INFO("CPU %u power domain on finished\n", pm_state.info.cpuid);
}

/* MediaTek PSCI power domain */
static void armv8_2_power_domain_off(const psci_power_state_t *state)
{
	struct mt_cpupm_event_data nb;
	unsigned int pstate = (MT_CPUPM_PWR_DOMAIN_CORE | MT_CPUPM_PWR_DOMAIN_PERCORE_DSU);
	struct mtk_cpupm_pwrstate pm_state = {
		.info = {
			.cpuid = plat_my_core_pos(),
			.mode = MTK_CPU_PM_SMP,
		},
		.pwr = {
			.afflv = armv8_2_get_pwr_afflv(state),
			.state_id = 0x0,
		},
	};
	armv8_2_cpu_pwr_off(&pm_state, pstate);

	nb.cpuid = pm_state.info.cpuid;
	nb.pwr_domain = pstate;
	MT_CPUPM_EVENT_PWR_OFF(&nb);

	INFO("CPU %u power domain off\n", pm_state.info.cpuid);
}

/* MediaTek PSCI power domain */
static void armv8_2_power_domain_suspend(const psci_power_state_t *state)
{
	unsigned int pstate = 0;
	struct mt_cpupm_event_data nb;
	struct mtk_cpupm_pwrstate pm_state = {
		.info = {
			.cpuid = plat_my_core_pos(),
			.mode = MTK_CPU_PM_CPUIDLE,
		},
	};

	pm_state.pwr.state_id = armv8_2_get_pwr_stateid(pm_state.info.cpuid);
	pm_state.pwr.afflv = armv8_2_get_pwr_afflv(state);
	pm_state.pwr.raw = state;

	pstate = get_mediatek_pstate(CPUPM_PWR_OFF,
				     armv8_2_power_state[pm_state.info.cpuid], &pm_state);

	armv8_2_cpu_pwr_suspend(&pm_state, pstate);

	if ((pstate & MT_CPUPM_PWR_DOMAIN_CLUSTER) != 0) {
		armv8_2_cluster_pwr_dwn_common(&pm_state);
	}

	if ((pstate & MT_CPUPM_PWR_DOMAIN_MCUSYS) != 0) {
		armv8_2_mcusys_pwr_dwn_common(&pm_state);
	}

	nb.cpuid = pm_state.info.cpuid;
	nb.pwr_domain = pstate;
	MT_CPUPM_EVENT_PWR_OFF(&nb);

	if (IS_AFFLV_PUBEVENT(pstate)) {
		MT_CPUPM_EVENT_AFFLV_PWR_OFF(&nb);
	}
}

/* MediaTek PSCI power domain */
static void armv8_2_power_domain_suspend_finish(const psci_power_state_t *state)
{
	unsigned int pstate = 0;
	struct mt_cpupm_event_data nb;
	struct mtk_cpupm_pwrstate pm_state = {
		.info = {
			.cpuid = plat_my_core_pos(),
			.mode = MTK_CPU_PM_CPUIDLE,
		},
	};

	pm_state.pwr.state_id = armv8_2_get_pwr_stateid(pm_state.info.cpuid);
	pm_state.pwr.afflv = armv8_2_get_pwr_afflv(state);
	pm_state.pwr.raw = state;

	pstate = get_mediatek_pstate(CPUPM_PWR_ON,
				     armv8_2_power_state[pm_state.info.cpuid], &pm_state);

	if ((pstate & MT_CPUPM_PWR_DOMAIN_MCUSYS) != 0) {
		armv8_2_mcusys_pwr_on_common(&pm_state);
	}

	if ((pstate & MT_CPUPM_PWR_DOMAIN_CLUSTER) != 0) {
		armv8_2_cluster_pwr_on_common(&pm_state);
	}

	armv8_2_cpu_pwr_resume(&pm_state, pstate);

	nb.cpuid = pm_state.info.cpuid;
	nb.pwr_domain = pstate;
	MT_CPUPM_EVENT_PWR_ON(&nb);

	if (IS_AFFLV_PUBEVENT(pstate)) {
		MT_CPUPM_EVENT_AFFLV_PWR_ON(&nb);
	}
}

/* MediaTek PSCI power domain */
static int armv8_2_validate_power_state(unsigned int power_state, psci_power_state_t *req_state)
{
	unsigned int i;
	unsigned int pstate = psci_get_pstate_type(power_state);
	unsigned int aff_lvl = psci_get_pstate_pwrlvl(power_state);
	unsigned int my_core_pos = plat_my_core_pos();

	if (mtk_cpu_pwr.ops == NULL) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_PWR_STATE_VALID)) {
		if (mtk_cpu_pwr.ops->pwr_state_valid(aff_lvl, pstate) != 0) {
			return PSCI_E_INVALID_PARAMS;
		}
	}

	if (pstate == PSTATE_TYPE_STANDBY) {
		req_state->pwr_domain_state[0] = PLAT_MAX_RET_STATE;
	} else {
		for (i = PSCI_CPU_PWR_LVL; i <= aff_lvl; i++) {
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
		}
	}
	armv8_2_power_state[my_core_pos] = power_state;

	return PSCI_E_SUCCESS;
}

/* MediaTek PSCI power domain */
#if CONFIG_MTK_SUPPORT_SYSTEM_SUSPEND
static void armv8_2_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;
	int ret;
	unsigned int power_state;
	unsigned int my_core_pos = plat_my_core_pos();

	ret = mtk_cpu_pwr.ops->pwr_state_valid(PLAT_MAX_PWR_LVL,
						PSTATE_TYPE_POWERDOWN);

	if (ret != MTK_CPUPM_E_OK) {
		/* Avoid suspend due to platform is not ready. */
		req_state->pwr_domain_state[PSCI_CPU_PWR_LVL] =
						PLAT_MAX_RET_STATE;
		for (i = PSCI_CPU_PWR_LVL + 1; i <= PLAT_MAX_PWR_LVL; i++) {
			req_state->pwr_domain_state[i] = PSCI_LOCAL_STATE_RUN;
		}

		power_state = psci_make_powerstate(0, PSTATE_TYPE_STANDBY, PSCI_CPU_PWR_LVL);
	} else {
		for (i = PSCI_CPU_PWR_LVL; i <= PLAT_MAX_PWR_LVL; i++) {
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
		}

		power_state = psci_make_powerstate(MT_PLAT_PWR_STATE_SUSPEND,
						   PSTATE_TYPE_POWERDOWN, PLAT_MAX_PWR_LVL);
	}

	armv8_2_power_state[my_core_pos] = power_state;
	flush_dcache_range((uintptr_t)&armv8_2_power_state[my_core_pos],
			   sizeof(armv8_2_power_state[my_core_pos]));
}
#endif
static void armv8_2_pm_smp_init(unsigned int cpu_id, uintptr_t entry_point)
{
	if (entry_point == 0) {
		ERROR("%s, warm_entry_point is null\n", __func__);
		panic();
	}
	if (IS_CPUSMP_FN_ENABLE(MTK_CPUPM_FN_SMP_INIT)) {
		mtk_cpu_pwr.smp->init(cpu_id, entry_point);
	}
	INFO("[%s:%d] - Initialize finished\n", __func__, __LINE__);
}

static struct plat_pm_pwr_ctrl armv8_2_pwr_ops = {
	.pwr_domain_suspend = armv8_2_power_domain_suspend,
	.pwr_domain_suspend_finish = armv8_2_power_domain_suspend_finish,
	.validate_power_state = armv8_2_validate_power_state,
#if CONFIG_MTK_SUPPORT_SYSTEM_SUSPEND
	.get_sys_suspend_power_state = armv8_2_get_sys_suspend_power_state,
#endif
};

struct plat_pm_smp_ctrl armv8_2_smp_ops = {
	.init = armv8_2_pm_smp_init,
	.pwr_domain_on = armv8_2_power_domain_on,
	.pwr_domain_off = armv8_2_power_domain_off,
	.pwr_domain_on_finish = armv8_2_power_domain_on_finish,
};

#define ISSUE_CPU_PM_REG_FAIL(_success) ({ _success = false; assert(0); })

#define CPM_PM_FN_CHECK(_fns, _ops, _id, _func, _result, _flag) ({ \
	if ((_fns & _id)) { \
		if (_ops->_func) \
			_flag |= _id; \
		else { \
			ISSUE_CPU_PM_REG_FAIL(_result); \
		} \
	} })

int register_cpu_pm_ops(unsigned int fn_flags, struct mtk_cpu_pm_ops *ops)
{
	bool success = true;
	unsigned int fns = 0;

	if ((ops == NULL) || (mtk_cpu_pwr.ops != NULL)) {
		ERROR("[%s:%d] register cpu_pm fail !!\n", __FILE__, __LINE__);
		return MTK_CPUPM_E_ERR;
	}

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_RESUME_CORE,
			cpu_resume, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SUSPEND_CORE,
			cpu_suspend, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_RESUME_CLUSTER,
			cluster_resume, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SUSPEND_CLUSTER,
			cluster_suspend, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_RESUME_MCUSYS,
			mcusys_resume, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SUSPEND_MCUSYS,
			mcusys_suspend, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_CPUPM_GET_PWR_STATE,
			get_pstate, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_PWR_STATE_VALID,
			pwr_state_valid, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_INIT,
			init, success, fns);

	if (success) {
		mtk_cpu_pwr.ops = ops;
		mtk_cpu_pwr.fn_mask |= fns;
		plat_pm_ops_setup_pwr(&armv8_2_pwr_ops);
		INFO("[%s:%d] CPU pwr ops register success, support:0x%x\n",
		     __func__, __LINE__, fns);
	} else {
		ERROR("[%s:%d] register cpu_pm ops fail !, fn:0x%x\n",
		      __func__, __LINE__, fn_flags);
		assert(0);
	}
	return MTK_CPUPM_E_OK;
}

int register_cpu_smp_ops(unsigned int fn_flags, struct mtk_cpu_smp_ops *ops)
{
	bool success = true;
	unsigned int fns = 0;

	if ((ops == NULL) || (mtk_cpu_pwr.smp != NULL)) {
		ERROR("[%s:%d] register cpu_smp fail !!\n", __FILE__, __LINE__);
		return MTK_CPUPM_E_ERR;
	}

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SMP_INIT,
			init, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_PWR_ON_CORE_PREPARE,
			cpu_pwr_on_prepare, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SMP_CORE_ON,
			cpu_on, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SMP_CORE_OFF,
			cpu_off, success, fns);

	if (success == true) {
		mtk_cpu_pwr.smp = ops;
		mtk_cpu_pwr.fn_mask |= fns;
		plat_pm_ops_setup_smp(&armv8_2_smp_ops);
		INFO("[%s:%d] CPU smp ops register success, support:0x%x\n",
		     __func__, __LINE__, fns);
	} else {
		ERROR("[%s:%d] register cpu_smp ops fail !, fn:0x%x\n",
		      __func__, __LINE__, fn_flags);
		assert(0);
	}
	return MTK_CPUPM_E_OK;
}
