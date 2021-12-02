/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/gpio.h>
#include <lib/psci/psci.h>

#include <mt_gic_v3.h>
#include <mtspmc.h>
#include <plat/common/platform.h>
#include <plat_dfd.h>
#include <plat_mtk_lpm.h>
#include <plat_params.h>
#include <plat_pm.h>
#include <pmic.h>
#include <rtc.h>

/*
 * Cluster state request:
 * [0] : The CPU requires cluster power down
 * [1] : The CPU requires cluster power on
 */
#define coordinate_cluster(onoff)	write_clusterpwrdn_el1(onoff)
#define coordinate_cluster_pwron()	coordinate_cluster(1)
#define coordinate_cluster_pwroff()	coordinate_cluster(0)

/* platform secure entry point */
static uintptr_t secure_entrypoint;
/* per-CPU power state */
static unsigned int plat_power_state[PLATFORM_CORE_COUNT];

/* platform CPU power domain - ops */
static const struct mt_lpm_tz *plat_mt_pm;

static inline int plat_mt_pm_invoke(int (*func)(unsigned int cpu,
						const psci_power_state_t *state),
				    int cpu, const psci_power_state_t *state)
{
	int ret = -1;

	if (func != NULL) {
		ret = func(cpu, state);
	}
	return ret;
}

/*
 * Common MTK_platform operations to power on/off a
 * CPU in response to a CPU_ON, CPU_OFF or CPU_SUSPEND request.
 */
static void plat_cpu_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	assert(cpu == plat_my_core_pos());
	assert(plat_mt_pm != NULL);

	(void)plat_mt_pm_invoke(plat_mt_pm->pwr_cpu_dwn, cpu, state);

	if ((psci_get_pstate_pwrlvl(req_pstate) >= MTK_AFFLVL_CLUSTER) ||
			(req_pstate == 0U)) { /* hotplug off */
		coordinate_cluster_pwroff();
	}

	/* Prevent interrupts from spuriously waking up this CPU */
	mt_gic_rdistif_save();
	gicv3_cpuif_disable(cpu);
	gicv3_rdistif_off(cpu);
}

static void plat_cpu_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	assert(cpu == plat_my_core_pos());
	assert(plat_mt_pm != NULL);

	(void)plat_mt_pm_invoke(plat_mt_pm->pwr_cpu_on, cpu, state);

	coordinate_cluster_pwron();

	/*
	 * If mcusys does power down before then restore
	 * all CPUs' GIC Redistributors
	 */
	if (IS_MCUSYS_OFF_STATE(state)) {
		mt_gic_rdistif_restore_all();
	} else {
		gicv3_rdistif_on(cpu);
		gicv3_cpuif_enable(cpu);
		mt_gic_rdistif_init();
		mt_gic_rdistif_restore();
	}
}

/*
 * Common MTK_platform operations to power on/off a
 * cluster in response to a CPU_ON, CPU_OFF or CPU_SUSPEND request.
 */
static void plat_cluster_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	assert(cpu == plat_my_core_pos());
	assert(plat_mt_pm != NULL);

	if (plat_mt_pm_invoke(plat_mt_pm->pwr_cluster_dwn, cpu, state) != 0) {
		coordinate_cluster_pwron();

		/*
		 * TODO:
		 * Return on fail and add a 'return' here before
		 * adding any code following the if-block.
		 */
	}
}

static void plat_cluster_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	assert(cpu == plat_my_core_pos());
	assert(plat_mt_pm != NULL);

	if (plat_mt_pm_invoke(plat_mt_pm->pwr_cluster_on, cpu, state) != 0) {
		/*
		 * TODO:
		 * return on fail and add a 'return' here before
		 * adding any code following the if-block.
		 */
	}
}

/*
 * Common MTK_platform operations to power on/off a
 * mcusys in response to a CPU_ON, CPU_OFF or CPU_SUSPEND request.
 */
static void plat_mcusys_pwrdwn_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	assert(cpu == plat_my_core_pos());
	assert(plat_mt_pm != NULL);

	if (plat_mt_pm_invoke(plat_mt_pm->pwr_mcusys_dwn, cpu, state) != 0) {
		return;		/* return on fail */
	}

	mt_gic_distif_save();
	gic_sgi_save_all();
}

static void plat_mcusys_pwron_common(unsigned int cpu,
		const psci_power_state_t *state, unsigned int req_pstate)
{
	assert(cpu == plat_my_core_pos());
	assert(plat_mt_pm != NULL);

	if (plat_mt_pm_invoke(plat_mt_pm->pwr_mcusys_on, cpu, state) != 0) {
		/* return on fail */
		return;
	}

	mt_gic_init();
	mt_gic_distif_restore();
	gic_sgi_restore_all();

	dfd_resume();

	(void)plat_mt_pm_invoke(plat_mt_pm->pwr_mcusys_on_finished, cpu, state);
}

/* plat_psci_ops implementation */
static void plat_cpu_standby(plat_local_state_t cpu_state)
{
	uint64_t scr;

	scr = read_scr_el3();
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);

	isb();
	dsb();
	wfi();

	write_scr_el3(scr);
}

static int plat_power_domain_on(u_register_t mpidr)
{
	unsigned int cpu = (unsigned int)plat_core_pos_by_mpidr(mpidr);
	unsigned int cluster = 0U;

	if (cpu >= PLATFORM_CORE_COUNT) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (!spm_get_cluster_powerstate(cluster)) {
		spm_poweron_cluster(cluster);
	}

	/* init CPU reset arch as AARCH64 */
	mcucfg_init_archstate(cluster, cpu, true);
	mcucfg_set_bootaddr(cluster, cpu, secure_entrypoint);
	spm_poweron_cpu(cluster, cpu);

	return PSCI_E_SUCCESS;
}

static void plat_power_domain_on_finish(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();
	unsigned int cpu = (unsigned int)plat_core_pos_by_mpidr(mpidr);

	assert(cpu < PLATFORM_CORE_COUNT);

	/* Allow IRQs to wakeup this core in IDLE flow */
	mcucfg_enable_gic_wakeup(0U, cpu);

	if (IS_CLUSTER_OFF_STATE(state)) {
		plat_cluster_pwron_common(cpu, state, 0U);
	}

	plat_cpu_pwron_common(cpu, state, 0U);
}

static void plat_power_domain_off(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();
	unsigned int cpu = (unsigned int)plat_core_pos_by_mpidr(mpidr);

	assert(cpu < PLATFORM_CORE_COUNT);

	plat_cpu_pwrdwn_common(cpu, state, 0U);
	spm_poweroff_cpu(0U, cpu);

	/* prevent unintended IRQs from waking up the hot-unplugged core */
	mcucfg_disable_gic_wakeup(0U, cpu);

	if (IS_CLUSTER_OFF_STATE(state)) {
		plat_cluster_pwrdwn_common(cpu, state, 0U);
	}
}

static void plat_power_domain_suspend(const psci_power_state_t *state)
{
	unsigned int cpu = plat_my_core_pos();

	assert(cpu < PLATFORM_CORE_COUNT);
	assert(plat_mt_pm != NULL);

	(void)plat_mt_pm_invoke(plat_mt_pm->pwr_prompt, cpu, state);

	/* Perform the common CPU specific operations */
	plat_cpu_pwrdwn_common(cpu, state, plat_power_state[cpu]);

	if (IS_CLUSTER_OFF_STATE(state)) {
		/* Perform the common cluster specific operations */
		plat_cluster_pwrdwn_common(cpu, state, plat_power_state[cpu]);
	}

	if (IS_MCUSYS_OFF_STATE(state)) {
		/* Perform the common mcusys specific operations */
		plat_mcusys_pwrdwn_common(cpu, state, plat_power_state[cpu]);
	}
}

static void plat_power_domain_suspend_finish(const psci_power_state_t *state)
{
	unsigned int cpu = plat_my_core_pos();

	assert(cpu < PLATFORM_CORE_COUNT);
	assert(plat_mt_pm != NULL);

	if (IS_MCUSYS_OFF_STATE(state)) {
		/* Perform the common mcusys specific operations */
		plat_mcusys_pwron_common(cpu, state, plat_power_state[cpu]);
	}

	if (IS_CLUSTER_OFF_STATE(state)) {
		/* Perform the common cluster specific operations */
		plat_cluster_pwron_common(cpu, state, plat_power_state[cpu]);
	}

	/* Perform the common CPU specific operations */
	plat_cpu_pwron_common(cpu, state, plat_power_state[cpu]);

	(void)plat_mt_pm_invoke(plat_mt_pm->pwr_reflect, cpu, state);
}

static int plat_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	unsigned int pstate = psci_get_pstate_type(power_state);
	unsigned int aff_lvl = psci_get_pstate_pwrlvl(power_state);
	unsigned int cpu = plat_my_core_pos();

	if (aff_lvl > PLAT_MAX_PWR_LVL) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (pstate == PSTATE_TYPE_STANDBY) {
		req_state->pwr_domain_state[0] = PLAT_MAX_RET_STATE;
	} else {
		unsigned int i;
		unsigned int pstate_id = psci_get_pstate_id(power_state);
		plat_local_state_t s = MTK_LOCAL_STATE_OFF;

		/* Use pstate_id to be power domain state */
		if (pstate_id > s) {
			s = (plat_local_state_t)pstate_id;
		}

		for (i = 0U; i <= aff_lvl; i++) {
			req_state->pwr_domain_state[i] = s;
		}
	}

	plat_power_state[cpu] = power_state;
	return PSCI_E_SUCCESS;
}

static void plat_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int lv;
	unsigned int cpu = plat_my_core_pos();

	for (lv = PSCI_CPU_PWR_LVL; lv <= PLAT_MAX_PWR_LVL; lv++) {
		req_state->pwr_domain_state[lv] = PLAT_MAX_OFF_STATE;
	}

	plat_power_state[cpu] =
			psci_make_powerstate(
				MT_PLAT_PWR_STATE_SYSTEM_SUSPEND,
				PSTATE_TYPE_POWERDOWN, PLAT_MAX_PWR_LVL);

	flush_dcache_range((uintptr_t)
			&plat_power_state[cpu],
			sizeof(plat_power_state[cpu]));
}

/*******************************************************************************
 * MTK handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_mtk_system_reset(void)
{
	struct bl_aux_gpio_info *gpio_reset = plat_get_mtk_gpio_reset();

	INFO("MTK System Reset\n");

	gpio_set_value(gpio_reset->index, gpio_reset->polarity);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

static void __dead2 plat_mtk_system_off(void)
{
	INFO("MTK System Off\n");

	rtc_power_off_sequence();
	pmic_power_off();

	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static const plat_psci_ops_t plat_psci_ops = {
	.cpu_standby			= plat_cpu_standby,
	.pwr_domain_on			= plat_power_domain_on,
	.pwr_domain_on_finish		= plat_power_domain_on_finish,
	.pwr_domain_off			= plat_power_domain_off,
	.pwr_domain_suspend		= plat_power_domain_suspend,
	.pwr_domain_suspend_finish	= plat_power_domain_suspend_finish,
	.validate_power_state		= plat_validate_power_state,
	.get_sys_suspend_power_state	= plat_get_sys_suspend_power_state,
	.system_off			= plat_mtk_system_off,
	.system_reset			= plat_mtk_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_psci_ops;
	secure_entrypoint = sec_entrypoint;

	/*
	 * init the warm reset config for boot CPU
	 * reset arch as AARCH64
	 * reset addr as function bl31_warm_entrypoint()
	 */
	mcucfg_init_archstate(0U, 0U, true);
	mcucfg_set_bootaddr(0U, 0U, secure_entrypoint);

	spmc_init();
	plat_mt_pm = mt_plat_cpu_pm_init();

	return 0;
}
