/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <bl_common.h>
#include <arch.h>
#include <arch_helpers.h>
#include <context.h>
#include <context_mgmt.h>
#include <cpu_data.h>
#include <debug.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stddef.h>
#include "psci_private.h"

/*******************************************************************************
 * This function does generic and platform specific operations after a wake-up
 * from standby/retention states at multiple power levels.
 ******************************************************************************/
static void psci_suspend_to_standby_finisher(unsigned int cpu_idx,
					     psci_power_state_t *state_info,
					     unsigned int end_pwrlvl)
{
	psci_acquire_pwr_domain_locks(end_pwrlvl,
				cpu_idx);

	/*
	 * Plat. management: Allow the platform to do operations
	 * on waking up from retention.
	 */
	psci_plat_pm_ops->pwr_domain_suspend_finish(state_info);

	/*
	 * Set the requested and target state of this CPU and all the higher
	 * power domain levels for this CPU to run.
	 */
	psci_set_pwr_domains_to_run(end_pwrlvl);

	psci_release_pwr_domain_locks(end_pwrlvl,
				cpu_idx);
}

/*******************************************************************************
 * This function does generic and platform specific suspend to power down
 * operations.
 ******************************************************************************/
static void psci_suspend_to_pwrdown_start(unsigned int end_pwrlvl,
					  entry_point_info_t *ep,
					  psci_power_state_t *state_info)
{
	unsigned int max_off_lvl = psci_find_max_off_lvl(state_info);

	/* Save PSCI target power level for the suspend finisher handler */
	psci_set_suspend_pwrlvl(end_pwrlvl);

	/*
	 * Flush the target power level as it will be accessed on power up with
	 * Data cache disabled.
	 */
	flush_cpu_data(psci_svc_cpu_data.target_pwrlvl);

	/*
	 * Call the cpu suspend handler registered by the Secure Payload
	 * Dispatcher to let it do any book-keeping. If the handler encounters an
	 * error, it's expected to assert within
	 */
	if (psci_spd_pm && psci_spd_pm->svc_suspend)
		psci_spd_pm->svc_suspend(max_off_lvl);

	/*
	 * Store the re-entry information for the non-secure world.
	 */
	cm_init_my_context(ep);

	/*
	 * Arch. management. Perform the necessary steps to flush all
	 * cpu caches. Currently we assume that the power level correspond
	 * the cache level.
	 * TODO : Introduce a mechanism to query the cache level to flush
	 * and the cpu-ops power down to perform from the platform.
	 */
	psci_do_pwrdown_cache_maintenance(max_off_lvl);
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
void psci_cpu_suspend_start(entry_point_info_t *ep,
			    unsigned int end_pwrlvl,
			    psci_power_state_t *state_info,
			    unsigned int is_power_down_state)
{
	int skip_wfi = 0;
	unsigned int idx = plat_my_core_pos();

	/*
	 * This function must only be called on platforms where the
	 * CPU_SUSPEND platform hooks have been implemented.
	 */
	assert(psci_plat_pm_ops->pwr_domain_suspend &&
			psci_plat_pm_ops->pwr_domain_suspend_finish);

	/*
	 * This function acquires the lock corresponding to each power
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_pwr_domain_locks(end_pwrlvl,
				      idx);

	/*
	 * We check if there are any pending interrupts after the delay
	 * introduced by lock contention to increase the chances of early
	 * detection that a wake-up interrupt has fired.
	 */
	if (read_isr_el1()) {
		skip_wfi = 1;
		goto exit;
	}

	/*
	 * This function is passed the requested state info and
	 * it returns the negotiated state info for each power level upto
	 * the end level specified.
	 */
	psci_do_state_coordination(end_pwrlvl, state_info);

	if (is_power_down_state)
		psci_suspend_to_pwrdown_start(end_pwrlvl, ep, state_info);

	/*
	 * Plat. management: Allow the platform to perform the
	 * necessary actions to turn off this cpu e.g. set the
	 * platform defined mailbox with the psci entrypoint,
	 * program the power controller etc.
	 */
	psci_plat_pm_ops->pwr_domain_suspend(state_info);

exit:
	/*
	 * Release the locks corresponding to each power level in the
	 * reverse order to which they were acquired.
	 */
	psci_release_pwr_domain_locks(end_pwrlvl,
				  idx);
	if (skip_wfi)
		return;

	if (is_power_down_state)
		psci_power_down_wfi();

	/*
	 * We will reach here if only retention/standby states have been
	 * requested at multiple power levels. This means that the cpu
	 * context will be preserved.
	 */
	wfi();

	/*
	 * After we wake up from context retaining suspend, call the
	 * context retaining suspend finisher.
	 */
	psci_suspend_to_standby_finisher(idx, state_info, end_pwrlvl);
}

/*******************************************************************************
 * The following functions finish an earlier suspend request. They
 * are called by the common finisher routine in psci_common.c. The `state_info`
 * is the psci_power_state from which this CPU has woken up from.
 ******************************************************************************/
void psci_cpu_suspend_finish(unsigned int cpu_idx,
			     psci_power_state_t *state_info)
{
	unsigned long long counter_freq;
	unsigned int max_off_lvl;

	/* Ensure we have been woken up from a suspended state */
	assert(psci_get_aff_info_state() == AFF_STATE_ON && is_local_state_off(\
			state_info->pwr_domain_state[PSCI_CPU_PWR_LVL]));

	/*
	 * Plat. management: Perform the platform specific actions
	 * before we change the state of the cpu e.g. enabling the
	 * gic or zeroing the mailbox register. If anything goes
	 * wrong then assert as there is no way to recover from this
	 * situation.
	 */
	psci_plat_pm_ops->pwr_domain_suspend_finish(state_info);

	/*
	 * Arch. management: Enable the data cache, manage stack memory and
	 * restore the stashed EL3 architectural context from the 'cpu_context'
	 * structure for this cpu.
	 */
	psci_do_pwrup_cache_maintenance();

	/* Re-init the cntfrq_el0 register */
	counter_freq = plat_get_syscnt_freq();
	write_cntfrq_el0(counter_freq);

	/*
	 * Call the cpu suspend finish handler registered by the Secure Payload
	 * Dispatcher to let it do any bookeeping. If the handler encounters an
	 * error, it's expected to assert within
	 */
	if (psci_spd_pm && psci_spd_pm->svc_suspend) {
		max_off_lvl = psci_find_max_off_lvl(state_info);
		assert (max_off_lvl != PSCI_INVALID_PWR_LVL);
		psci_spd_pm->svc_suspend_finish(max_off_lvl);
	}

	/* Invalidate the suspend level for the cpu */
	psci_set_suspend_pwrlvl(PSCI_INVALID_PWR_LVL);

	/*
	 * Generic management: Now we just need to retrieve the
	 * information that we had stashed away during the suspend
	 * call to set this cpu on its way.
	 */
	cm_prepare_el3_exit(NON_SECURE);
}
