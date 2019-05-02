/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/arm/gicv2.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/arm/common/plat_arm.h>

#include <mcucfg.h>
#include <mt8173_def.h>
#include <mt_cpuxgpt.h> /* generic_timer_backup() */
#include <plat_private.h>
#include <power_tracer.h>
#include <rtc.h>
#include <scu.h>
#include <spm_hotplug.h>
#include <spm_mcdi.h>
#include <spm_suspend.h>

#define MTK_PWR_LVL0	0
#define MTK_PWR_LVL1	1
#define MTK_PWR_LVL2	2

/* Macros to read the MTK power domain state */
#define MTK_CORE_PWR_STATE(state)	(state)->pwr_domain_state[MTK_PWR_LVL0]
#define MTK_CLUSTER_PWR_STATE(state)	(state)->pwr_domain_state[MTK_PWR_LVL1]
#define MTK_SYSTEM_PWR_STATE(state)	((PLAT_MAX_PWR_LVL > MTK_PWR_LVL1) ?\
			(state)->pwr_domain_state[MTK_PWR_LVL2] : 0)

#if PSCI_EXTENDED_STATE_ID
/*
 *  The table storing the valid idle power states. Ensure that the
 *  array entries are populated in ascending order of state-id to
 *  enable us to use binary search during power state validation.
 *  The table must be terminated by a NULL entry.
 */
const unsigned int mtk_pm_idle_states[] = {
	/* State-id - 0x001 */
	mtk_make_pwrstate_lvl2(MTK_LOCAL_STATE_RUN, MTK_LOCAL_STATE_RUN,
		MTK_LOCAL_STATE_RET, MTK_PWR_LVL0, PSTATE_TYPE_STANDBY),
	/* State-id - 0x002 */
	mtk_make_pwrstate_lvl2(MTK_LOCAL_STATE_RUN, MTK_LOCAL_STATE_RUN,
		MTK_LOCAL_STATE_OFF, MTK_PWR_LVL0, PSTATE_TYPE_POWERDOWN),
	/* State-id - 0x022 */
	mtk_make_pwrstate_lvl2(MTK_LOCAL_STATE_RUN, MTK_LOCAL_STATE_OFF,
		MTK_LOCAL_STATE_OFF, MTK_PWR_LVL1, PSTATE_TYPE_POWERDOWN),
#if PLAT_MAX_PWR_LVL > MTK_PWR_LVL1
	/* State-id - 0x222 */
	mtk_make_pwrstate_lvl2(MTK_LOCAL_STATE_OFF, MTK_LOCAL_STATE_OFF,
		MTK_LOCAL_STATE_OFF, MTK_PWR_LVL2, PSTATE_TYPE_POWERDOWN),
#endif
	0,
};
#endif

struct core_context {
	unsigned long timer_data[8];
	unsigned int count;
	unsigned int rst;
	unsigned int abt;
	unsigned int brk;
};

struct cluster_context {
	struct core_context core[PLATFORM_MAX_CPUS_PER_CLUSTER];
};

/*
 * Top level structure to hold the complete context of a multi cluster system
 */
struct system_context {
	struct cluster_context cluster[PLATFORM_CLUSTER_COUNT];
};

/*
 * Top level structure which encapsulates the context of the entire system
 */
static struct system_context dormant_data[1];

static inline struct cluster_context *system_cluster(
						struct system_context *system,
						uint32_t clusterid)
{
	return &system->cluster[clusterid];
}

static inline struct core_context *cluster_core(struct cluster_context *cluster,
						uint32_t cpuid)
{
	return &cluster->core[cpuid];
}

static struct cluster_context *get_cluster_data(unsigned long mpidr)
{
	uint32_t clusterid;

	clusterid = (mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;

	return system_cluster(dormant_data, clusterid);
}

static struct core_context *get_core_data(unsigned long mpidr)
{
	struct cluster_context *cluster;
	uint32_t cpuid;

	cluster = get_cluster_data(mpidr);
	cpuid = mpidr & MPIDR_CPU_MASK;

	return cluster_core(cluster, cpuid);
}

static void mt_save_generic_timer(unsigned long *container)
{
	uint64_t ctl;
	uint64_t val;

	__asm__ volatile("mrs	%x0, cntkctl_el1\n\t"
			 "mrs	%x1, cntp_cval_el0\n\t"
			 "stp	%x0, %x1, [%2, #0]"
			 : "=&r" (ctl), "=&r" (val)
			 : "r" (container)
			 : "memory");

	__asm__ volatile("mrs	%x0, cntp_tval_el0\n\t"
			 "mrs	%x1, cntp_ctl_el0\n\t"
			 "stp	%x0, %x1, [%2, #16]"
			 : "=&r" (val), "=&r" (ctl)
			 : "r" (container)
			 : "memory");

	__asm__ volatile("mrs	%x0, cntv_tval_el0\n\t"
			 "mrs	%x1, cntv_ctl_el0\n\t"
			 "stp	%x0, %x1, [%2, #32]"
			 : "=&r" (val), "=&r" (ctl)
			 : "r" (container)
			 : "memory");
}

static void mt_restore_generic_timer(unsigned long *container)
{
	uint64_t ctl;
	uint64_t val;

	__asm__ volatile("ldp	%x0, %x1, [%2, #0]\n\t"
			 "msr	cntkctl_el1, %x0\n\t"
			 "msr	cntp_cval_el0, %x1"
			 : "=&r" (ctl), "=&r" (val)
			 : "r" (container)
			 : "memory");

	__asm__ volatile("ldp	%x0, %x1, [%2, #16]\n\t"
			 "msr	cntp_tval_el0, %x0\n\t"
			 "msr	cntp_ctl_el0, %x1"
			 : "=&r" (val), "=&r" (ctl)
			 : "r" (container)
			 : "memory");

	__asm__ volatile("ldp	%x0, %x1, [%2, #32]\n\t"
			 "msr	cntv_tval_el0, %x0\n\t"
			 "msr	cntv_ctl_el0, %x1"
			 : "=&r" (val), "=&r" (ctl)
			 : "r" (container)
			 : "memory");
}

static inline uint64_t read_cntpctl(void)
{
	uint64_t cntpctl;

	__asm__ volatile("mrs	%x0, cntp_ctl_el0"
			 : "=r" (cntpctl) : : "memory");

	return cntpctl;
}

static inline void write_cntpctl(uint64_t cntpctl)
{
	__asm__ volatile("msr	cntp_ctl_el0, %x0" : : "r"(cntpctl));
}

static void stop_generic_timer(void)
{
	/*
	 * Disable the timer and mask the irq to prevent
	 * suprious interrupts on this cpu interface. It
	 * will bite us when we come back if we don't. It
	 * will be replayed on the inbound cluster.
	 */
	uint64_t cntpctl = read_cntpctl();

	write_cntpctl(clr_cntp_ctl_enable(cntpctl));
}

static void mt_cpu_save(unsigned long mpidr)
{
	struct core_context *core;

	core = get_core_data(mpidr);
	mt_save_generic_timer(core->timer_data);

	/* disable timer irq, and upper layer should enable it again. */
	stop_generic_timer();
}

static void mt_cpu_restore(unsigned long mpidr)
{
	struct core_context *core;

	core = get_core_data(mpidr);
	mt_restore_generic_timer(core->timer_data);
}

static void mt_platform_save_context(unsigned long mpidr)
{
	/* mcusys_save_context: */
	mt_cpu_save(mpidr);
}

static void mt_platform_restore_context(unsigned long mpidr)
{
	/* mcusys_restore_context: */
	mt_cpu_restore(mpidr);
}

static void plat_cpu_standby(plat_local_state_t cpu_state)
{
	unsigned int scr;

	scr = read_scr_el3();
	write_scr_el3(scr | SCR_IRQ_BIT);
	isb();
	dsb();
	wfi();
	write_scr_el3(scr);
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static uintptr_t secure_entrypoint;

static int plat_power_domain_on(unsigned long mpidr)
{
	int rc = PSCI_E_SUCCESS;
	unsigned long cpu_id;
	unsigned long cluster_id;
	uintptr_t rv;

	cpu_id = mpidr & MPIDR_CPU_MASK;
	cluster_id = mpidr & MPIDR_CLUSTER_MASK;

	if (cluster_id)
		rv = (uintptr_t)&mt8173_mcucfg->mp1_rv_addr[cpu_id].rv_addr_lw;
	else
		rv = (uintptr_t)&mt8173_mcucfg->mp0_rv_addr[cpu_id].rv_addr_lw;

	mmio_write_32(rv, secure_entrypoint);
	INFO("mt_on[%ld:%ld], entry %x\n",
		cluster_id, cpu_id, mmio_read_32(rv));

	spm_hotplug_on(mpidr);
	return rc;
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * off. The level and mpidr determine the affinity instance. The 'state' arg.
 * allows the platform to decide whether the cluster is being turned off and
 * take apt actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to do flush a write to the global to prevent unpredictable results.
 ******************************************************************************/
static void plat_power_domain_off(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();

	/* Prevent interrupts from spuriously waking up this cpu */
	gicv2_cpuif_disable();

	spm_hotplug_off(mpidr);

	trace_power_flow(mpidr, CPU_DOWN);

	if (MTK_CLUSTER_PWR_STATE(state) == MTK_LOCAL_STATE_OFF) {
		/* Disable coherency if this cluster is to be turned off */
		plat_cci_disable();

		trace_power_flow(mpidr, CLUSTER_DOWN);
	}
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be
 * suspended. The level and mpidr determine the affinity instance. The 'state'
 * arg. allows the platform to decide whether the cluster is being turned off
 * and take apt actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to do flush a write to the global to prevent unpredictable results.
 ******************************************************************************/
static void plat_power_domain_suspend(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();
	unsigned long cluster_id;
	unsigned long cpu_id;
	uintptr_t rv;

	cpu_id = mpidr & MPIDR_CPU_MASK;
	cluster_id = mpidr & MPIDR_CLUSTER_MASK;

	if (cluster_id)
		rv = (uintptr_t)&mt8173_mcucfg->mp1_rv_addr[cpu_id].rv_addr_lw;
	else
		rv = (uintptr_t)&mt8173_mcucfg->mp0_rv_addr[cpu_id].rv_addr_lw;

	mmio_write_32(rv, secure_entrypoint);

	if (MTK_SYSTEM_PWR_STATE(state) != MTK_LOCAL_STATE_OFF) {
		spm_mcdi_prepare_for_off_state(mpidr, MTK_PWR_LVL0);
		if (MTK_CLUSTER_PWR_STATE(state) == MTK_LOCAL_STATE_OFF)
			spm_mcdi_prepare_for_off_state(mpidr, MTK_PWR_LVL1);
	}

	mt_platform_save_context(mpidr);

	/* Perform the common cluster specific operations */
	if (MTK_CLUSTER_PWR_STATE(state) == MTK_LOCAL_STATE_OFF) {
		/* Disable coherency if this cluster is to be turned off */
		plat_cci_disable();
	}

	if (MTK_SYSTEM_PWR_STATE(state) == MTK_LOCAL_STATE_OFF) {
		disable_scu(mpidr);
		generic_timer_backup();
		spm_system_suspend();
		/* Prevent interrupts from spuriously waking up this cpu */
		gicv2_cpuif_disable();
	}
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance has just been powered
 * on after being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
void mtk_system_pwr_domain_resume(void);

static void plat_power_domain_on_finish(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();

	assert(state->pwr_domain_state[MPIDR_AFFLVL0] == MTK_LOCAL_STATE_OFF);

	if ((PLAT_MAX_PWR_LVL > MTK_PWR_LVL1) &&
		(state->pwr_domain_state[MTK_PWR_LVL2] == MTK_LOCAL_STATE_OFF))
		mtk_system_pwr_domain_resume();

	if (state->pwr_domain_state[MPIDR_AFFLVL1] == MTK_LOCAL_STATE_OFF) {
		plat_cci_enable();
		trace_power_flow(mpidr, CLUSTER_UP);
	}

	if ((PLAT_MAX_PWR_LVL > MTK_PWR_LVL1) &&
		(state->pwr_domain_state[MTK_PWR_LVL2] == MTK_LOCAL_STATE_OFF))
		return;

	/* Enable the gic cpu interface */
	gicv2_cpuif_enable();
	gicv2_pcpu_distif_init();
	trace_power_flow(mpidr, CPU_UP);
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance has just been powered
 * on after having been suspended earlier. The level and mpidr determine the
 * affinity instance.
 ******************************************************************************/
static void plat_power_domain_suspend_finish(const psci_power_state_t *state)
{
	unsigned long mpidr = read_mpidr_el1();

	if (state->pwr_domain_state[MTK_PWR_LVL0] == MTK_LOCAL_STATE_RET)
		return;

	if (MTK_SYSTEM_PWR_STATE(state) == MTK_LOCAL_STATE_OFF) {
		/* Enable the gic cpu interface */
		plat_arm_gic_init();
		spm_system_suspend_finish();
		enable_scu(mpidr);
	}

	/* Perform the common cluster specific operations */
	if (MTK_CLUSTER_PWR_STATE(state) == MTK_LOCAL_STATE_OFF) {
		/* Enable coherency if this cluster was off */
		plat_cci_enable();
	}

	mt_platform_restore_context(mpidr);

	if (MTK_SYSTEM_PWR_STATE(state) != MTK_LOCAL_STATE_OFF) {
		spm_mcdi_finish_for_on_state(mpidr, MTK_PWR_LVL0);
		if (MTK_CLUSTER_PWR_STATE(state) == MTK_LOCAL_STATE_OFF)
			spm_mcdi_finish_for_on_state(mpidr, MTK_PWR_LVL1);
	}

	gicv2_pcpu_distif_init();
}

static void plat_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	assert(PLAT_MAX_PWR_LVL >= 2);

	for (int i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = MTK_LOCAL_STATE_OFF;
}

/*******************************************************************************
 * MTK handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_system_off(void)
{
	INFO("MTK System Off\n");

	rtc_bbpu_power_down();

	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static void __dead2 plat_system_reset(void)
{
	/* Write the System Configuration Control Register */
	INFO("MTK System Reset\n");

	mmio_clrsetbits_32(MTK_WDT_BASE,
		(MTK_WDT_MODE_DUAL_MODE | MTK_WDT_MODE_IRQ),
		MTK_WDT_MODE_KEY);
	mmio_setbits_32(MTK_WDT_BASE, (MTK_WDT_MODE_KEY | MTK_WDT_MODE_EXTEN));
	mmio_setbits_32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

#if !PSCI_EXTENDED_STATE_ID
static int plat_validate_power_state(unsigned int power_state,
					psci_power_state_t *req_state)
{
	int pstate = psci_get_pstate_type(power_state);
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int i;

	assert(req_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on power level 0
		 * Ignore any other power level.
		 */
		if (pwr_lvl != 0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MTK_PWR_LVL0] =
					MTK_LOCAL_STATE_RET;
	} else {
		for (i = 0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					MTK_LOCAL_STATE_OFF;
	}

	/*
	 * We expect the 'state id' to be zero.
	 */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}
#else
int plat_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	unsigned int state_id;
	int i;

	assert(req_state);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; !!mtk_pm_idle_states[i]; i++) {
		if (power_state == mtk_pm_idle_states[i])
			break;
	}

	/* Return error if entry not found in the idle state array */
	if (!mtk_pm_idle_states[i])
		return PSCI_E_INVALID_PARAMS;

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = state_id &
						MTK_LOCAL_PSTATE_MASK;
		state_id >>= MTK_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}
#endif

void mtk_system_pwr_domain_resume(void)
{
	console_switch_state(CONSOLE_FLAG_BOOT);

	/* Assert system power domain is available on the platform */
	assert(PLAT_MAX_PWR_LVL >= MTK_PWR_LVL2);

	plat_arm_gic_init();

	console_switch_state(CONSOLE_FLAG_RUNTIME);
}

static const plat_psci_ops_t plat_plat_pm_ops = {
	.cpu_standby			= plat_cpu_standby,
	.pwr_domain_on			= plat_power_domain_on,
	.pwr_domain_on_finish		= plat_power_domain_on_finish,
	.pwr_domain_off			= plat_power_domain_off,
	.pwr_domain_suspend		= plat_power_domain_suspend,
	.pwr_domain_suspend_finish	= plat_power_domain_suspend_finish,
	.system_off			= plat_system_off,
	.system_reset			= plat_system_reset,
	.validate_power_state		= plat_validate_power_state,
	.get_sys_suspend_power_state	= plat_get_sys_suspend_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_plat_pm_ops;
	secure_entrypoint = sec_entrypoint;
	return 0;
}

/*
 * The PSCI generic code uses this API to let the platform participate in state
 * coordination during a power management operation. It compares the platform
 * specific local power states requested by each cpu for a given power domain
 * and returns the coordinated target power state that the domain should
 * enter. A platform assigns a number to a local power state. This default
 * implementation assumes that the platform assigns these numbers in order of
 * increasing depth of the power state i.e. for two power states X & Y, if X < Y
 * then X represents a shallower power state than Y. As a result, the
 * coordinated target local power state for a power domain will be the minimum
 * of the requested local power states.
 */
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = PLAT_MAX_OFF_STATE, temp;

	assert(ncpu);

	do {
		temp = *states++;
		if (temp < target)
			target = temp;
	} while (--ncpu);

	return target;
}
