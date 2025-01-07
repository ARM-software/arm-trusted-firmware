/*
 * Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <drivers/arm/gic.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <lib/pmf/pmf.h>
#include <lib/runtime_instr.h>
#include <plat/common/platform.h>

#include "psci_private.h"

/*******************************************************************************
 * This function does generic and platform specific operations after a wake-up
 * from standby/retention states at multiple power levels.
 ******************************************************************************/
static void psci_cpu_suspend_to_standby_finish(unsigned int end_pwrlvl,
					     psci_power_state_t *state_info)
{
	/*
	 * Plat. management: Allow the platform to do operations
	 * on waking up from retention.
	 */
	psci_plat_pm_ops->pwr_domain_suspend_finish(state_info);

	/* This loses its meaning when not suspending, reset so it's correct for OFF */
	psci_set_suspend_pwrlvl(PLAT_MAX_PWR_LVL);
}

/*******************************************************************************
 * This function does generic and platform specific suspend to power down
 * operations.
 ******************************************************************************/
static void psci_suspend_to_pwrdown_start(unsigned int idx,
					  unsigned int end_pwrlvl,
					  unsigned int max_off_lvl,
					  const psci_power_state_t *state_info)
{
	PUBLISH_EVENT_ARG(psci_suspend_pwrdown_start, &idx);

#if PSCI_OS_INIT_MODE
#ifdef PLAT_MAX_CPU_SUSPEND_PWR_LVL
	end_pwrlvl = PLAT_MAX_CPU_SUSPEND_PWR_LVL;
#else
	end_pwrlvl = PLAT_MAX_PWR_LVL;
#endif
#endif

	/* Save PSCI target power level for the suspend finisher handler */
	psci_set_suspend_pwrlvl(end_pwrlvl);

	/*
	 * Flush the target power level as it might be accessed on power up with
	 * Data cache disabled.
	 */
	psci_flush_cpu_data(psci_svc_cpu_data.target_pwrlvl);

	/*
	 * Call the cpu suspend handler registered by the Secure Payload
	 * Dispatcher to let it do any book-keeping. If the handler encounters an
	 * error, it's expected to assert within
	 */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_suspend != NULL))
		psci_spd_pm->svc_suspend(max_off_lvl);

#if !HW_ASSISTED_COHERENCY
	/*
	 * Plat. management: Allow the platform to perform any early
	 * actions required to power down the CPU. This might be useful for
	 * HW_ASSISTED_COHERENCY = 0 platforms that can safely perform these
	 * actions with data caches enabled.
	 */
	if (psci_plat_pm_ops->pwr_domain_suspend_pwrdown_early != NULL)
		psci_plat_pm_ops->pwr_domain_suspend_pwrdown_early(state_info);
#endif
	/*
	 * Arch. management. Initiate power down sequence.
	 */
	psci_pwrdown_cpu_start(max_off_lvl);
}

/*******************************************************************************
 * Top level handler which is called when a cpu wants to suspend its execution.
 * It is assumed that along with suspending the cpu power domain, power domains
 * at higher levels until the target power level will be suspended as well. It
 * coordinates with the platform to negotiate the target state for each of
 * the power domain level till the target power domain level. It then performs
 * generic, architectural, platform setup and state management required to
 * suspend that power domain level and power domain levels below it.
 * e.g. For a cpu that's to be suspended, it could mean programming the
 * power controller whereas for a cluster that's to be suspended, it will call
 * the platform specific code which will disable coherency at the interconnect
 * level if the cpu is the last in the cluster and also the program the power
 * controller.
 *
 * All the required parameter checks are performed at the beginning and after
 * the state transition has been done, no further error is expected and it is
 * not possible to undo any of the actions taken beyond that point.
 ******************************************************************************/
int psci_cpu_suspend_start(unsigned int idx,
			   unsigned int end_pwrlvl,
			   psci_power_state_t *state_info,
			   unsigned int is_power_down_state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int parent_nodes[PLAT_MAX_PWR_LVL] = {0};
	unsigned int max_off_lvl = 0;

	/*
	 * This function must only be called on platforms where the
	 * CPU_SUSPEND platform hooks have been implemented.
	 */
	assert((psci_plat_pm_ops->pwr_domain_suspend != NULL) &&
	       (psci_plat_pm_ops->pwr_domain_suspend_finish != NULL));

	/* Get the parent nodes */
	psci_get_parent_pwr_domain_nodes(idx, end_pwrlvl, parent_nodes);

	/*
	 * This function acquires the lock corresponding to each power
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_pwr_domain_locks(end_pwrlvl, parent_nodes);

	/*
	 * We check if there are any pending interrupts after the delay
	 * introduced by lock contention to increase the chances of early
	 * detection that a wake-up interrupt has fired.
	 */
	if (read_isr_el1() != 0U) {
		goto suspend_exit;
	}

#if PSCI_OS_INIT_MODE
	if (psci_suspend_mode == OS_INIT) {
		/*
		 * This function validates the requested state info for
		 * OS-initiated mode.
		 */
		rc = psci_validate_state_coordination(idx, end_pwrlvl, state_info);
		if (rc != PSCI_E_SUCCESS) {
			goto suspend_exit;
		}
	} else {
#endif
		/*
		 * This function is passed the requested state info and
		 * it returns the negotiated state info for each power level upto
		 * the end level specified.
		 */
		psci_do_state_coordination(idx, end_pwrlvl, state_info);
#if PSCI_OS_INIT_MODE
	}
#endif

#if PSCI_OS_INIT_MODE
	if (psci_plat_pm_ops->pwr_domain_validate_suspend != NULL) {
		rc = psci_plat_pm_ops->pwr_domain_validate_suspend(state_info);
		if (rc != PSCI_E_SUCCESS) {
			goto suspend_exit;
		}
	}
#endif

	/* Update the target state in the power domain nodes */
	psci_set_target_local_pwr_states(idx, end_pwrlvl, state_info);

#if ENABLE_PSCI_STAT
	/* Update the last cpu for each level till end_pwrlvl */
	psci_stats_update_pwr_down(idx, end_pwrlvl, state_info);
#endif

	if (is_power_down_state != 0U) {
		/*
		 * WHen CTX_INCLUDE_EL2_REGS is usnet, we're probably runnig
		 * with some SPD that assumes the core is going off so it
		 * doesn't bother saving NS's context. Do that here until we
		 * figure out a way to make this coherent.
		 */
#if FEAT_PABANDON
#if !CTX_INCLUDE_EL2_REGS
		cm_el1_sysregs_context_save(NON_SECURE);
#endif
#endif
		max_off_lvl = psci_find_max_off_lvl(state_info);
		psci_suspend_to_pwrdown_start(idx, end_pwrlvl, end_pwrlvl, state_info);
	}

#if USE_GIC_DRIVER
	/* turn the GIC off before we hand off to the platform */
	gic_cpuif_disable(idx);
#endif /* USE_GIC_DRIVER */

	/*
	 * Plat. management: Allow the platform to perform the
	 * necessary actions to turn off this cpu e.g. set the
	 * platform defined mailbox with the psci entrypoint,
	 * program the power controller etc.
	 */
	psci_plat_pm_ops->pwr_domain_suspend(state_info);

#if ENABLE_PSCI_STAT
	plat_psci_stat_accounting_start(state_info);
#endif

	/*
	 * Release the locks corresponding to each power level in the
	 * reverse order to which they were acquired.
	 */
	psci_release_pwr_domain_locks(end_pwrlvl, parent_nodes);

#if ENABLE_RUNTIME_INSTRUMENTATION
	/*
	 * Update the timestamp with cache off. We assume this
	 * timestamp can only be read from the current CPU and the
	 * timestamp cache line will be flushed before return to
	 * normal world on wakeup.
	 */
	PMF_CAPTURE_TIMESTAMP(rt_instr_svc,
	    RT_INSTR_ENTER_HW_LOW_PWR,
	    PMF_NO_CACHE_MAINT);
#endif

	if (is_power_down_state != 0U) {
		if (psci_plat_pm_ops->pwr_domain_pwr_down != NULL) {
			/* This function may not return */
			psci_plat_pm_ops->pwr_domain_pwr_down(state_info);
		}

		psci_pwrdown_cpu_end_wakeup(max_off_lvl);
	} else {
		/*
		 * We will reach here if only retention/standby states have been
		 * requested at multiple power levels. This means that the cpu
		 * context will be preserved.
		 */
		wfi();
	}

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_CAPTURE_TIMESTAMP(rt_instr_svc,
	    RT_INSTR_EXIT_HW_LOW_PWR,
	    PMF_NO_CACHE_MAINT);
#endif

	psci_acquire_pwr_domain_locks(end_pwrlvl, parent_nodes);
	/*
	 * Find out which retention states this CPU has exited from until the
	 * 'end_pwrlvl'. The exit retention state could be deeper than the entry
	 * state as a result of state coordination amongst other CPUs post wfi.
	 */
	psci_get_target_local_pwr_states(idx, end_pwrlvl, state_info);

#if ENABLE_PSCI_STAT
	plat_psci_stat_accounting_stop(state_info);
	psci_stats_update_pwr_up(idx, end_pwrlvl, state_info);
#endif

	/*
	 * Waking up means we've retained all context. Call the finishers to put
	 * the system back to a usable state.
	 */
	if (is_power_down_state != 0U) {
#if FEAT_PABANDON
		psci_cpu_suspend_to_powerdown_finish(idx, max_off_lvl, state_info);

#if !CTX_INCLUDE_EL2_REGS
		cm_el1_sysregs_context_restore(NON_SECURE);
#endif
#endif
	} else {
		psci_cpu_suspend_to_standby_finish(end_pwrlvl, state_info);
	}

#if USE_GIC_DRIVER
	/* Turn GIC on after platform has had a chance to do state management */
	gic_cpuif_enable(idx);
#endif /* USE_GIC_DRIVER */

	/*
	 * Set the requested and target state of this CPU and all the higher
	 * power domain levels for this CPU to run.
	 */
	psci_set_pwr_domains_to_run(idx, end_pwrlvl);

suspend_exit:
	psci_release_pwr_domain_locks(end_pwrlvl, parent_nodes);

	return rc;
}

/*******************************************************************************
 * The following functions finish an earlier suspend request. They
 * are called by the common finisher routine in psci_common.c. The `state_info`
 * is the psci_power_state from which this CPU has woken up from.
 ******************************************************************************/
void psci_cpu_suspend_to_powerdown_finish(unsigned int cpu_idx, unsigned int max_off_lvl, const psci_power_state_t *state_info)
{
	unsigned int counter_freq;

	/* Ensure we have been woken up from a suspended state */
	assert((psci_get_aff_info_state() == AFF_STATE_ON) &&
		(is_local_state_off(
			state_info->pwr_domain_state[PSCI_CPU_PWR_LVL]) != 0));

	/*
	 * Plat. management: Perform the platform specific actions
	 * before we change the state of the cpu e.g. enabling the
	 * gic or zeroing the mailbox register. If anything goes
	 * wrong then assert as there is no way to recover from this
	 * situation.
	 */
	psci_plat_pm_ops->pwr_domain_suspend_finish(state_info);

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	/* Arch. management: Enable the data cache, stack memory maintenance. */
	psci_do_pwrup_cache_maintenance();
#endif

#if USE_GIC_DRIVER
	/* GIC on after platform has had its say and MMU is on */
	gic_cpuif_enable(cpu_idx);
#endif /* USE_GIC_DRIVER */

	/* Re-init the cntfrq_el0 register */
	counter_freq = plat_get_syscnt_freq2();
	write_cntfrq_el0(counter_freq);

	/*
	 * Call the cpu suspend finish handler registered by the Secure Payload
	 * Dispatcher to let it do any bookeeping. If the handler encounters an
	 * error, it's expected to assert within
	 */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_suspend_finish != NULL)) {
		psci_spd_pm->svc_suspend_finish(max_off_lvl);
	}

	/* This loses its meaning when not suspending, reset so it's correct for OFF */
	psci_set_suspend_pwrlvl(PLAT_MAX_PWR_LVL);

	PUBLISH_EVENT_ARG(psci_suspend_pwrdown_finish, &cpu_idx);
}
