/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#ifdef CONFIG_MTK_BOOKER
#include <drivers/booker.h>
#endif

#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/console.h>
#include <lib/psci/psci.h>
#include <lib/utils.h>
#include <mt_gic_v3.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <lib/mtk_init/mtk_init.h>
#include <lib/pm/mtk_pm.h>
#ifdef MTK_PUBEVENT_ENABLE
#include <vendor_pubsub_events.h>
#endif

#define IS_AFFLV_PUBEVENT(_pstate) \
	(_pstate & (MT_CPUPM_PWR_DOMAIN_MCUSYS | \
		    MT_CPUPM_PWR_DOMAIN_CLUSTER))

#ifdef MTK_PUBEVENT_ENABLE
#define MT_CPUPM_EVENT_PWR_ON(x) ({ \
	PUBLISH_EVENT_ARG(mt_cpupm_publish_pwr_on, \
			  (const void *)(x)); })

#define MT_CPUPM_EVENT_PWR_OFF(x) ({ \
	PUBLISH_EVENT_ARG(mt_cpupm_publish_pwr_off, \
			  (const void *)(x)); })

#define MT_CPUPM_EVENT_AFFLV_PWR_ON(x) ({ \
	PUBLISH_EVENT_ARG(mt_cpupm_publish_afflv_pwr_on, \
			  (const void *)(x)); })

#define MT_CPUPM_EVENT_AFFLV_PWR_OFF(x) ({ \
	PUBLISH_EVENT_ARG(mt_cpupm_publish_afflv_pwr_off, \
			  (const void *)(x)); })

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
#define coordinate_cluster(onoff) \
	write_clusterpwrdn_el1(onoff)
#define coordinate_cluster_pwron() \
	coordinate_cluster(1)
#define coordinate_cluster_pwroff() \
	coordinate_cluster(0)

/* default enable all function */
#define MTK_CPU_PWR_FN_MASK_DEFAULT	(0)

struct mtk_cpu_pwr_ctrl {
	unsigned int fn_mask;
	struct mtk_cpu_pm_ops *ops;
	struct mtk_cpu_smp_ops *smp;
};

static struct mtk_cpu_pwr_ctrl imtk_cpu_pwr = {
	.fn_mask = MTK_CPU_PWR_FN_MASK_DEFAULT,
	.ops = NULL,
};

#define IS_CPUIDLE_FN_ENABLE(x)	(imtk_cpu_pwr.ops && (imtk_cpu_pwr.fn_mask & (x)))
#define IS_CPUSMP_FN_ENABLE(x)	(imtk_cpu_pwr.smp && (imtk_cpu_pwr.fn_mask & (x)))

/* per-cpu power state */
static unsigned int cpu_power_state[PLATFORM_CORE_COUNT];

#define get_pwr_stateid(cpu) \
	psci_get_pstate_id(cpu_power_state[cpu])

#define GET_MEDIATEK_PSTATE(_domain, _psci_state, _state) ({ \
	int mret = 0; \
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_CPUPM_GET_PWR_STATE)) \
		mret = imtk_cpu_pwr.ops->get_pstate( \
			_domain, _psci_state, _state); \
	mret; })

static inline unsigned int get_pwr_afflv(const psci_power_state_t *state)
{
	for (int i = PLAT_MAX_PWR_LVL; i >= PSCI_CPU_PWR_LVL; i--) {
		if (is_local_state_run(state->pwr_domain_state[i]) == 0)
			return (unsigned int) i;
	}

	return PSCI_INVALID_PWR_LVL;
}

static void mcusys_pwr_on_common(const struct mtk_cpupm_pwrstate *state)
{
	mt_gic_distif_restore();
	mt_gic_rdistif_restore();

	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_RESUME_MCUSYS))
		imtk_cpu_pwr.ops->mcusys_resume(state);
}

static void mcusys_pwr_dwn_common(const struct mtk_cpupm_pwrstate *state)
{
#ifdef CONFIG_MTK_BOOKER
	booker_flush();
#endif

	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_SUSPEND_MCUSYS))
		imtk_cpu_pwr.ops->mcusys_suspend(state);

	mt_gic_rdistif_save();
	/* save gic context after cirq enable */
	mt_gic_distif_save();
}

static void cluster_pwr_on_common(const struct mtk_cpupm_pwrstate *state)
{
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_RESUME_CLUSTER))
		imtk_cpu_pwr.ops->cluster_resume(state);
}

static void cluster_pwr_dwn_common(const struct mtk_cpupm_pwrstate *state)
{
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_SUSPEND_CLUSTER))
		imtk_cpu_pwr.ops->cluster_suspend(state);
}

static void cpu_pwr_on_common(const struct mtk_cpupm_pwrstate *state,
				      unsigned int pstate)
{
	coordinate_cluster_pwron();

	mt_gic_cpuif_enable();
}

static void cpu_pwr_dwn_common(const struct mtk_cpupm_pwrstate *state,
				       unsigned int pstate)
{
	if (pstate & MT_CPUPM_PWR_DOMAIN_PERCORE_DSU)
		coordinate_cluster_pwroff();

	mt_gic_cpuif_disable();
}

static void cpu_pwr_resume(const struct mtk_cpupm_pwrstate *state,
				   unsigned int pstate)
{
	cpu_pwr_on_common(state, pstate);
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_RESUME_CORE))
		imtk_cpu_pwr.ops->cpu_resume(state);
}

static void cpu_pwr_suspend(const struct mtk_cpupm_pwrstate *state,
				    unsigned int pstate)
{
	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_SUSPEND_CORE))
		imtk_cpu_pwr.ops->cpu_suspend(state);
	cpu_pwr_dwn_common(state, pstate);
}

static void cpu_pwr_on(const struct mtk_cpupm_pwrstate *state,
			       unsigned int pstate)
{
	cpu_pwr_on_common(state, pstate);
	if (IS_CPUSMP_FN_ENABLE(MTK_CPUPM_FN_SMP_CORE_ON))
		imtk_cpu_pwr.smp->cpu_on(state);
}

static void cpu_pwr_off(const struct mtk_cpupm_pwrstate *state,
				unsigned int pstate)
{
	if (IS_CPUSMP_FN_ENABLE(MTK_CPUPM_FN_SMP_CORE_OFF))
		imtk_cpu_pwr.smp->cpu_off(state);
	cpu_pwr_dwn_common(state, pstate);
}

static int power_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	int cpu = plat_core_pos_by_mpidr(mpidr);
	uintptr_t entry = plat_pm_get_warm_entry();

	if (IS_CPUSMP_FN_ENABLE(MTK_CPUPM_FN_PWR_ON_CORE_PREPARE)) {
		int b_ret = MTK_CPUPM_E_FAIL;

		b_ret = imtk_cpu_pwr.smp->cpu_pwr_on_prepare(cpu, entry);

		if (b_ret)
			ret = PSCI_E_DENIED;
	}
	INFO("CPU %u power domain prepare on\n", cpu);
	return ret;
}

static void power_domain_on_finish(const psci_power_state_t *state)
{
	struct mt_cpupm_event_data nb;
	unsigned int pstate = (MT_CPUPM_PWR_DOMAIN_CORE |
			       MT_CPUPM_PWR_DOMAIN_PERCORE_DSU);
	struct mtk_cpupm_pwrstate pm_state = {
		.info = {
			.cpuid = plat_my_core_pos(),
			.mode = MTK_CPU_PM_SMP,
		},
		.pwr = {
			.afflv = get_pwr_afflv(state),
			.state_id = 0x0,
		},
	};

	mt_gic_pcpu_init();

	cpu_pwr_on(&pm_state, pstate);

	nb.cpuid = pm_state.info.cpuid;
	nb.pwr_domain = pstate;
	MT_CPUPM_EVENT_PWR_ON(&nb);
	INFO("CPU %u power domain on finished\n", pm_state.info.cpuid);
}

static void power_domain_off(const psci_power_state_t *state)
{
	struct mt_cpupm_event_data nb;
	unsigned int pstate = (MT_CPUPM_PWR_DOMAIN_CORE |
			      MT_CPUPM_PWR_DOMAIN_PERCORE_DSU);
	struct mtk_cpupm_pwrstate pm_state = {
		.info = {
			.cpuid = plat_my_core_pos(),
			.mode = MTK_CPU_PM_SMP,
		},
		.pwr = {
			.afflv = get_pwr_afflv(state),
			.state_id = 0x0,
		},
	};

	cpu_pwr_off(&pm_state, pstate);

	mt_gic_redistif_off();

	nb.cpuid = pm_state.info.cpuid;
	nb.pwr_domain = pstate;
	MT_CPUPM_EVENT_PWR_OFF(&nb);

	INFO("CPU %u power domain off\n", pm_state.info.cpuid);
}

static void power_domain_suspend(const psci_power_state_t *state)
{
	unsigned int pstate = 0;
	struct mt_cpupm_event_data nb;
	struct mtk_cpupm_pwrstate pm_state = {
		.info = {
			.cpuid = plat_my_core_pos(),
			.mode = MTK_CPU_PM_CPUIDLE,
		},
	};

	pm_state.pwr.state_id = get_pwr_stateid(pm_state.info.cpuid);
	pm_state.pwr.afflv = get_pwr_afflv(state);
	pm_state.pwr.raw = state;

	pstate = GET_MEDIATEK_PSTATE(CPUPM_PWR_OFF,
		cpu_power_state[pm_state.info.cpuid], &pm_state);

	cpu_pwr_suspend(&pm_state, pstate);

	if (pstate & MT_CPUPM_PWR_DOMAIN_CLUSTER)
		cluster_pwr_dwn_common(&pm_state);

	if (pstate & MT_CPUPM_PWR_DOMAIN_MCUSYS)
		mcusys_pwr_dwn_common(&pm_state);

	nb.cpuid = pm_state.info.cpuid;
	nb.pwr_domain = pstate;
	MT_CPUPM_EVENT_PWR_OFF(&nb);

	if (IS_AFFLV_PUBEVENT(pstate))
		MT_CPUPM_EVENT_AFFLV_PWR_OFF(&nb);
}

static void power_domain_suspend_finish(const psci_power_state_t *state)
{
	unsigned int pstate = 0;
	struct mt_cpupm_event_data nb;
	struct mtk_cpupm_pwrstate pm_state = {
		.info = {
			.cpuid = plat_my_core_pos(),
			.mode = MTK_CPU_PM_CPUIDLE,
		},
	};

	pm_state.pwr.state_id = get_pwr_stateid(pm_state.info.cpuid);
	pm_state.pwr.afflv = get_pwr_afflv(state);
	pm_state.pwr.raw = state;

	pstate = GET_MEDIATEK_PSTATE(CPUPM_PWR_ON,
			cpu_power_state[pm_state.info.cpuid], &pm_state);

	if (pstate & MT_CPUPM_PWR_DOMAIN_MCUSYS)
		mcusys_pwr_on_common(&pm_state);

	if (pstate & MT_CPUPM_PWR_DOMAIN_CLUSTER)
		cluster_pwr_on_common(&pm_state);

	cpu_pwr_resume(&pm_state, pstate);

	nb.cpuid = pm_state.info.cpuid;
	nb.pwr_domain = pstate;
	MT_CPUPM_EVENT_PWR_ON(&nb);

	if (IS_AFFLV_PUBEVENT(pstate))
		MT_CPUPM_EVENT_AFFLV_PWR_ON(&nb);
}

static int validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int i;
	unsigned int pstate = psci_get_pstate_type(power_state);
	int aff_lvl = psci_get_pstate_pwrlvl(power_state);
	unsigned int my_core_pos = plat_my_core_pos();

	if (!imtk_cpu_pwr.ops)
		return PSCI_E_INVALID_PARAMS;

	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_PWR_STATE_VALID)) {
		int ret = MTK_CPUPM_E_FAIL;

		ret = imtk_cpu_pwr.ops->pwr_state_valid(aff_lvl, pstate);
		if (ret)
			return PSCI_E_INVALID_PARAMS;
	}

	if (pstate == PSTATE_TYPE_STANDBY)
		req_state->pwr_domain_state[0] = PLAT_MAX_RET_STATE;
	else {
		for (i = PSCI_CPU_PWR_LVL; i <= aff_lvl; i++)
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
	cpu_power_state[my_core_pos] = power_state;
	return PSCI_E_SUCCESS;
}

#if CONFIG_MTK_SUPPORT_SYSTEM_SUSPEND
/* Mediatek PSCI power domain */
static void get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int lv = 0;
	unsigned int my_core_pos = plat_my_core_pos();

	for (lv = PSCI_CPU_PWR_LVL; lv <= PLAT_MAX_PWR_LVL; lv++)
		req_state->pwr_domain_state[lv] = PLAT_MAX_OFF_STATE;

	cpu_power_state[my_core_pos] = psci_make_powerstate(
						MT_PLAT_PWR_STATE_SUSPEND,
						PSTATE_TYPE_POWERDOWN,
						PLAT_MT_SYSTEM_SUSPEND);

	flush_dcache_range((uintptr_t)&cpu_power_state[my_core_pos],
			   sizeof(cpu_power_state[my_core_pos]));
}
#endif

static void pwr_domain_pwr_down_wfi(const psci_power_state_t *req_state)
{
	unsigned int cpu = plat_my_core_pos();
	int ret = MTK_CPUPM_E_NOT_SUPPORT;

	if (IS_CPUIDLE_FN_ENABLE(MTK_CPUPM_FN_PWR_DOMAIN_POWER_DOWN_WFI))
		ret = imtk_cpu_pwr.ops->pwr_domain_pwr_down_wfi(cpu);
	if (ret == MTK_CPUPM_E_OK)
		plat_panic_handler();
}

static void pm_smp_init(unsigned int cpu_id, uintptr_t entry_point)
{
	if (entry_point == 0) {
		ERROR("%s, warm_entry_point is null\n", __func__);
		panic();
	}
	if (IS_CPUSMP_FN_ENABLE(MTK_CPUPM_FN_SMP_INIT))
		imtk_cpu_pwr.smp->init(cpu_id, entry_point);
	INFO("[%s:%d] - Initialize finished\n", __func__, __LINE__);
}

static struct plat_pm_pwr_ctrl armv9_0_pwr_ops = {
	.pwr_domain_suspend = power_domain_suspend,
	.pwr_domain_suspend_finish = power_domain_suspend_finish,
	.validate_power_state = validate_power_state,
#if CONFIG_MTK_SUPPORT_SYSTEM_SUSPEND
	.get_sys_suspend_power_state = get_sys_suspend_power_state,
#endif
	.pwr_domain_pwr_down_wfi = pwr_domain_pwr_down_wfi,
};

struct plat_pm_smp_ctrl armv9_0_smp_ops = {
	.init = pm_smp_init,
	.pwr_domain_on = power_domain_on,
	.pwr_domain_off = power_domain_off,
	.pwr_domain_on_finish = power_domain_on_finish,
};

#define ISSUE_CPU_PM_REG_FAIL(_success) ({ \
	_success = 0; assert(0); })

#define CPM_PM_FN_CHECK(_fns, _ops, _id, _func, _cond_ex, _result, _flag) ({ \
	if ((_fns & _id)) { \
		if (_ops->_func && _cond_ex) \
			_flag |= _id; \
		else { \
			ISSUE_CPU_PM_REG_FAIL(_result); \
		} \
	} }) \

int plat_pm_invoke_func(enum mtk_cpu_pm_mode mode, unsigned int id, void *priv)
{
	int ret = MTK_CPUPM_E_ERR;

	if ((mode == MTK_CPU_PM_CPUIDLE) && imtk_cpu_pwr.ops &&
	    imtk_cpu_pwr.ops->invoke)
		ret = imtk_cpu_pwr.ops->invoke(id, priv);
	else if ((mode == MTK_CPU_PM_SMP) &&
		 imtk_cpu_pwr.smp &&
		 imtk_cpu_pwr.smp->invoke)
		ret = imtk_cpu_pwr.smp->invoke(id, priv);

	return ret;
}

int register_cpu_pm_ops(unsigned int fn_flags, struct mtk_cpu_pm_ops *ops)
{
	int success = 1;
	unsigned int fns = 0;

	if (!ops || imtk_cpu_pwr.ops) {
		ERROR("[%s:%d] register cpu_pm fail !!\n", __FILE__, __LINE__);
		return MTK_CPUPM_E_ERR;
	}
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_RESUME_CORE,
			cpu_resume, 1, success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SUSPEND_CORE,
			cpu_suspend, 1, success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_RESUME_CLUSTER,
			cluster_resume, 1, success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SUSPEND_CLUSTER,
			cluster_suspend, 1, success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_RESUME_MCUSYS,
			mcusys_resume, 1,
			success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SUSPEND_MCUSYS,
			mcusys_suspend, 1, success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_CPUPM_GET_PWR_STATE,
			get_pstate, 1, success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_PWR_STATE_VALID,
			pwr_state_valid, 1, success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_INIT,
			init, 1, success, fns);
	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_PWR_DOMAIN_POWER_DOWN_WFI,
			pwr_domain_pwr_down_wfi, 1, success, fns);
	if (success) {
		imtk_cpu_pwr.ops = ops;
		imtk_cpu_pwr.fn_mask |= fns;
		plat_pm_ops_setup_pwr(&armv9_0_pwr_ops);
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
	int success = 1;
	unsigned int fns = 0;

	if (!ops || imtk_cpu_pwr.smp) {
		ERROR("[%s:%d] register cpu_smp fail !!\n", __FILE__, __LINE__);
		return MTK_CPUPM_E_ERR;
	}

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SMP_INIT,
			init, 1, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_PWR_ON_CORE_PREPARE,
			cpu_pwr_on_prepare, 1, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SMP_CORE_ON,
			cpu_on, 1, success, fns);

	CPM_PM_FN_CHECK(fn_flags, ops, MTK_CPUPM_FN_SMP_CORE_OFF,
			cpu_off, 1, success, fns);

	if (success) {
		imtk_cpu_pwr.smp = ops;
		imtk_cpu_pwr.fn_mask |= fns;
		plat_pm_ops_setup_smp(&armv9_0_smp_ops);
		INFO("[%s:%d] CPU smp ops register success, support:0x%x\n",
		     __func__, __LINE__, fns);
	} else {
		ERROR("[%s:%d] register cpu_smp ops fail !, fn:0x%x\n",
		      __func__, __LINE__, fn_flags);
		assert(0);
	}
	return MTK_CPUPM_E_OK;
}
