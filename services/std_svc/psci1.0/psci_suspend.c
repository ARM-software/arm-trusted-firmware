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
 * This function saves the power state parameter passed in the current PSCI
 * cpu_suspend call in the per-cpu data array.
 ******************************************************************************/
void psci_set_suspend_power_state(unsigned int power_state)
{
	set_cpu_data(psci_svc_cpu_data.power_state, power_state);
	flush_cpu_data(psci_svc_cpu_data.power_state);
}

/*******************************************************************************
 * This function gets the power level till which the current cpu could be
 * powered down during a cpu_suspend call. Returns PSCI_INVALID_DATA if the
 * power state is invalid.
 ******************************************************************************/
int psci_get_suspend_pwrlvl(void)
{
	unsigned int power_state;

	power_state = get_cpu_data(psci_svc_cpu_data.power_state);

	return ((power_state == PSCI_INVALID_DATA) ?
		power_state : psci_get_pstate_pwrlvl(power_state));
}

/*******************************************************************************
 * This function gets the state id of the current cpu from the power state
 * parameter saved in the per-cpu data array. Returns PSCI_INVALID_DATA if the
 * power state saved is invalid.
 ******************************************************************************/
int psci_get_suspend_stateid(void)
{
	unsigned int power_state;

	power_state = get_cpu_data(psci_svc_cpu_data.power_state);

	return ((power_state == PSCI_INVALID_DATA) ?
		power_state : psci_get_pstate_id(power_state));
}

/*******************************************************************************
 * This function gets the state id of the cpu specified by the cpu index
 * from the power state parameter saved in the per-cpu data array. Returns
 * PSCI_INVALID_DATA if the power state saved is invalid.
 ******************************************************************************/
int psci_get_suspend_stateid_by_idx(unsigned long cpu_idx)
{
	unsigned int power_state;

	power_state = get_cpu_data_by_index(cpu_idx,
					    psci_svc_cpu_data.power_state);

	return ((power_state == PSCI_INVALID_DATA) ?
		power_state : psci_get_pstate_id(power_state));
}

/*******************************************************************************
 * Top level handler which is called when a cpu wants to suspend its execution.
 * It is assumed that along with suspending the cpu power domain, power domains
 * at higher levels until the target power level will be suspended as well.
 * It finds the highest level where a domain has to be suspended by traversing
 * the node information and then performs generic, architectural, platform
 * setup and state management required to suspend that power domain and domains
 * below it. * e.g. For a cpu that's to be suspended, it could mean programming
 * the power controller whereas for a cluster that's to be suspended, it will
 * call the platform specific code which will disable coherency at the
 * interconnect level if the cpu is the last in the cluster and also the
 * program the power controller.
 *
 * All the required parameter checks are performed at the beginning and after
 * the state transition has been done, no further error is expected and it is
 * not possible to undo any of the actions taken beyond that point.
 ******************************************************************************/
void psci_cpu_suspend_start(entry_point_info_t *ep, int end_pwrlvl)
{
	int skip_wfi = 0;
	unsigned int max_phys_off_pwrlvl, idx = plat_my_core_pos();
	unsigned long psci_entrypoint;

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
	 * Call the cpu suspend handler registered by the Secure Payload
	 * Dispatcher to let it do any bookeeping. If the handler encounters an
	 * error, it's expected to assert within
	 */
	if (psci_spd_pm && psci_spd_pm->svc_suspend)
		psci_spd_pm->svc_suspend(0);

	/*
	 * This function updates the state of each power domain instance
	 * corresponding to the cpu index in the range of power levels
	 * specified.
	 */
	psci_do_state_coordination(end_pwrlvl,
				   idx,
				   PSCI_STATE_SUSPEND);

	max_phys_off_pwrlvl = psci_find_max_phys_off_pwrlvl(end_pwrlvl,
							    idx);
	assert(max_phys_off_pwrlvl != PSCI_INVALID_DATA);

	/*
	 * Store the re-entry information for the non-secure world.
	 */
	cm_init_my_context(ep);

	/* Set the secure world (EL3) re-entry point after BL1 */
	psci_entrypoint = (unsigned long) psci_cpu_suspend_finish_entry;

	/*
	 * Arch. management. Perform the necessary steps to flush all
	 * cpu caches.
	 */
	psci_do_pwrdown_cache_maintenance(max_phys_off_pwrlvl);

	/*
	 * Plat. management: Allow the platform to perform the
	 * necessary actions to turn off this cpu e.g. set the
	 * platform defined mailbox with the psci entrypoint,
	 * program the power controller etc.
	 */
	psci_plat_pm_ops->pwr_domain_suspend(psci_entrypoint,
					max_phys_off_pwrlvl);

exit:
	/*
	 * Release the locks corresponding to each power level in the
	 * reverse order to which they were acquired.
	 */
	psci_release_pwr_domain_locks(end_pwrlvl,
				      idx);
	if (!skip_wfi)
		psci_power_down_wfi();
}

/*******************************************************************************
 * The following functions finish an earlier suspend request. They
 * are called by the common finisher routine in psci_common.c.
 ******************************************************************************/
void psci_cpu_suspend_finish(unsigned int cpu_idx, int max_off_pwrlvl)
{
	int32_t suspend_level;
	uint64_t counter_freq;

	/* Ensure we have been woken up from a suspended state */
	assert(psci_get_state(cpu_idx, PSCI_CPU_PWR_LVL)
				== PSCI_STATE_SUSPEND);

	/*
	 * Plat. management: Perform the platform specific actions
	 * before we change the state of the cpu e.g. enabling the
	 * gic or zeroing the mailbox register. If anything goes
	 * wrong then assert as there is no way to recover from this
	 * situation.
	 */
	psci_plat_pm_ops->pwr_domain_suspend_finish(max_off_pwrlvl);

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
		suspend_level = psci_get_suspend_pwrlvl();
		assert (suspend_level != PSCI_INVALID_DATA);
		psci_spd_pm->svc_suspend_finish(suspend_level);
	}

	/* Invalidate the suspend context for the node */
	psci_set_suspend_power_state(PSCI_INVALID_DATA);

	/*
	 * Generic management: Now we just need to retrieve the
	 * information that we had stashed away during the suspend
	 * call to set this cpu on its way.
	 */
	cm_prepare_el3_exit(NON_SECURE);

	/* Clean caches before re-entering normal world */
	dcsw_op_louis(DCCSW);
}
