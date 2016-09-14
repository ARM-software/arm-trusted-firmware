/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <arm_gic.h>
#include <assert.h>
#include <bakery_lock.h>
#include <cci.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <mcucfg.h>
#include <mmio.h>
#include <platform_def.h>
#include <plat_private.h>
#include <power_tracer.h>
#include <psci.h>
#include <scu.h>

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

static void stop_generic_timer(void)
{
	/*
	 * Disable the timer and mask the irq to prevent
	 * suprious interrupts on this cpu interface. It
	 * will bite us when we come back if we don't. It
	 * will be replayed on the inbound cluster.
	 */
	uint64_t cntpctl = read_cntp_ctl_el0();

	write_cntp_ctl_el0(clr_cntp_ctl_enable(cntpctl));
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

/*******************************************************************************
* Private function which is used to determine if any platform actions
* should be performed for the specified affinity instance given its
* state. Nothing needs to be done if the 'state' is not off or if this is not
* the highest affinity level which will enter the 'state'.
*******************************************************************************/
static int32_t plat_do_plat_actions(unsigned int afflvl, unsigned int state)
{
	unsigned int max_phys_off_afflvl;

	assert(afflvl <= MPIDR_AFFLVL2);

	if (state != PSCI_STATE_OFF)
		return -EAGAIN;

	/*
	 * Find the highest affinity level which will be suspended and postpone
	 * all the platform specific actions until that level is hit.
	 */
	max_phys_off_afflvl = psci_get_max_phys_off_afflvl();
	assert(max_phys_off_afflvl != PSCI_INVALID_DATA);
	if (afflvl != max_phys_off_afflvl)
		return -EAGAIN;

	return 0;
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to enter
 * standby.
 ******************************************************************************/
static void plat_affinst_standby(unsigned int power_state)
{
	unsigned int target_afflvl;

	/* Sanity check the requested state */
	target_afflvl = psci_get_pstate_afflvl(power_state);

	/*
	 * It's possible to enter standby only on affinity level 0 i.e. a cpu
	 * on the MTK_platform. Ignore any other affinity level.
	 */
	if (target_afflvl == MPIDR_AFFLVL0) {
		/*
		 * Enter standby state. dsb is good practice before using wfi
		 * to enter low power states.
		 */
		dsb();
		wfi();
	}
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static int plat_affinst_on(unsigned long mpidr,
		    unsigned long sec_entrypoint,
		    unsigned int afflvl,
		    unsigned int state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned long cpu_id;
	unsigned long cluster_id;
	uintptr_t rv;

	/*
	 * It's possible to turn on only affinity level 0 i.e. a cpu
	 * on the MTK_platform. Ignore any other affinity level.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		return rc;

	cpu_id = mpidr & MPIDR_CPU_MASK;
	cluster_id = mpidr & MPIDR_CLUSTER_MASK;

	if (cluster_id)
		rv = (uintptr_t)&mt6795_mcucfg->mp1_rv_addr[cpu_id].rv_addr_lw;
	else
		rv = (uintptr_t)&mt6795_mcucfg->mp0_rv_addr[cpu_id].rv_addr_lw;

	mmio_write_32(rv, sec_entrypoint);
	INFO("mt_on[%ld:%ld], entry %x\n",
		cluster_id, cpu_id, mmio_read_32(rv));

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
static void plat_affinst_off(unsigned int afflvl, unsigned int state)
{
	unsigned long mpidr = read_mpidr_el1();

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	/* Prevent interrupts from spuriously waking up this cpu */
	plat_mt_gic_cpuif_disable();

	trace_power_flow(mpidr, CPU_DOWN);

	if (afflvl != MPIDR_AFFLVL0) {
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
static void plat_affinst_suspend(unsigned long sec_entrypoint,
			  unsigned int afflvl,
			  unsigned int state)
{
	unsigned long mpidr = read_mpidr_el1();
	unsigned long cluster_id;
	unsigned long cpu_id;
	uintptr_t rv;

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	cpu_id = mpidr & MPIDR_CPU_MASK;
	cluster_id = mpidr & MPIDR_CLUSTER_MASK;

	if (cluster_id)
		rv = (uintptr_t)&mt6795_mcucfg->mp1_rv_addr[cpu_id].rv_addr_lw;
	else
		rv = (uintptr_t)&mt6795_mcucfg->mp0_rv_addr[cpu_id].rv_addr_lw;

	mmio_write_32(rv, sec_entrypoint);

	if (afflvl >= MPIDR_AFFLVL0)
		mt_platform_save_context(mpidr);

	/* Perform the common cluster specific operations */
	if (afflvl >= MPIDR_AFFLVL1) {
		/* Disable coherency if this cluster is to be turned off */
		plat_cci_disable();
		disable_scu(mpidr);

		trace_power_flow(mpidr, CLUSTER_SUSPEND);
	}

	if (afflvl >= MPIDR_AFFLVL2) {
		/* Prevent interrupts from spuriously waking up this cpu */
		plat_mt_gic_cpuif_disable();
	}
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance has just been powered
 * on after being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
static void plat_affinst_on_finish(unsigned int afflvl, unsigned int state)
{
	unsigned long mpidr = read_mpidr_el1();

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	/* Perform the common cluster specific operations */
	if (afflvl >= MPIDR_AFFLVL1) {
		enable_scu(mpidr);

		/* Enable coherency if this cluster was off */
		plat_cci_enable();
		trace_power_flow(mpidr, CLUSTER_UP);
	}

	/* Enable the gic cpu interface */
	plat_mt_gic_cpuif_enable();
	plat_mt_gic_pcpu_init();
	trace_power_flow(mpidr, CPU_UP);
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance has just been powered
 * on after having been suspended earlier. The level and mpidr determine the
 * affinity instance.
 ******************************************************************************/
static void plat_affinst_suspend_finish(unsigned int afflvl, unsigned int state)
{
	unsigned long mpidr = read_mpidr_el1();

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	if (afflvl >= MPIDR_AFFLVL2) {
		/* Enable the gic cpu interface */
		plat_mt_gic_init();
		plat_mt_gic_cpuif_enable();
	}

	/* Perform the common cluster specific operations */
	if (afflvl >= MPIDR_AFFLVL1) {
		enable_scu(mpidr);

		/* Enable coherency if this cluster was off */
		plat_cci_enable();
		trace_power_flow(mpidr, CLUSTER_UP);
	}

	if (afflvl >= MPIDR_AFFLVL0)
		mt_platform_restore_context(mpidr);

	plat_mt_gic_pcpu_init();
}

static unsigned int plat_get_sys_suspend_power_state(void)
{
	/* StateID: 0, StateType: 1(power down), PowerLevel: 2(system) */
	return psci_make_powerstate(0, 1, 2);
}

/*******************************************************************************
 * MTK handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_system_off(void)
{
	INFO("MTK System Off\n");
	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static void __dead2 plat_system_reset(void)
{
	/* Write the System Configuration Control Register */
	INFO("MTK System Reset\n");

	mmio_clrbits_32(MTK_WDT_BASE,
		(MTK_WDT_MODE_DUAL_MODE | MTK_WDT_MODE_IRQ));
	mmio_setbits_32(MTK_WDT_BASE, (MTK_WDT_MODE_KEY | MTK_WDT_MODE_EXTEN));
	mmio_setbits_32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t plat_plat_pm_ops = {
	.affinst_standby		= plat_affinst_standby,
	.affinst_on			= plat_affinst_on,
	.affinst_off			= plat_affinst_off,
	.affinst_suspend		= plat_affinst_suspend,
	.affinst_on_finish		= plat_affinst_on_finish,
	.affinst_suspend_finish		= plat_affinst_suspend_finish,
	.system_off			= plat_system_off,
	.system_reset			= plat_system_reset,
	.get_sys_suspend_power_state	= plat_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops & initialize the mtk_platform power
 * controller
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &plat_plat_pm_ops;
	return 0;
}
