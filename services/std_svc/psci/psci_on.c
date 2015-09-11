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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <debug.h>
#include <context_mgmt.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stddef.h>
#include "psci_private.h"

/*******************************************************************************
 * This function checks whether a cpu which has been requested to be turned on
 * is OFF to begin with.
 ******************************************************************************/
static int cpu_on_validate_state(aff_info_state_t aff_state)
{
	if (aff_state == AFF_STATE_ON)
		return PSCI_E_ALREADY_ON;

	if (aff_state == AFF_STATE_ON_PENDING)
		return PSCI_E_ON_PENDING;

	assert(aff_state == AFF_STATE_OFF);
	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * This function sets the aff_info_state in the per-cpu data of the CPU
 * specified by cpu_idx
 ******************************************************************************/
static void psci_set_aff_info_state_by_idx(unsigned int cpu_idx,
					   aff_info_state_t aff_state)
{

	set_cpu_data_by_index(cpu_idx,
			      psci_svc_cpu_data.aff_info_state,
			      aff_state);

	/*
	 * Flush aff_info_state as it will be accessed with caches turned OFF.
	 */
	flush_cpu_data_by_index(cpu_idx, psci_svc_cpu_data.aff_info_state);
}

/*******************************************************************************
 * Generic handler which is called to physically power on a cpu identified by
 * its mpidr. It performs the generic, architectural, platform setup and state
 * management to power on the target cpu e.g. it will ensure that
 * enough information is stashed for it to resume execution in the non-secure
 * security state.
 *
 * The state of all the relevant power domains are changed after calling the
 * platform handler as it can return error.
 ******************************************************************************/
int psci_cpu_on_start(u_register_t target_cpu,
		      entry_point_info_t *ep,
		      unsigned int end_pwrlvl)
{
	int rc;
	unsigned int target_idx = plat_core_pos_by_mpidr(target_cpu);

	/*
	 * This function must only be called on platforms where the
	 * CPU_ON platform hooks have been implemented.
	 */
	assert(psci_plat_pm_ops->pwr_domain_on &&
			psci_plat_pm_ops->pwr_domain_on_finish);

	/* Protect against multiple CPUs trying to turn ON the same target CPU */
	psci_spin_lock_cpu(target_idx);

	/*
	 * Generic management: Ensure that the cpu is off to be
	 * turned on.
	 */
	rc = cpu_on_validate_state(psci_get_aff_info_state_by_idx(target_idx));
	if (rc != PSCI_E_SUCCESS)
		goto exit;

	/*
	 * Call the cpu on handler registered by the Secure Payload Dispatcher
	 * to let it do any bookeeping. If the handler encounters an error, it's
	 * expected to assert within
	 */
	if (psci_spd_pm && psci_spd_pm->svc_on)
		psci_spd_pm->svc_on(target_cpu);

	/*
	 * Set the Affinity info state of the target cpu to ON_PENDING.
	 */
	psci_set_aff_info_state_by_idx(target_idx, AFF_STATE_ON_PENDING);

	/*
	 * Perform generic, architecture and platform specific handling.
	 */
	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	rc = psci_plat_pm_ops->pwr_domain_on(target_cpu);
	assert(rc == PSCI_E_SUCCESS || rc == PSCI_E_INTERN_FAIL);

	if (rc == PSCI_E_SUCCESS)
		/* Store the re-entry information for the non-secure world. */
		cm_init_context_by_index(target_idx, ep);
	else
		/* Restore the state on error. */
		psci_set_aff_info_state_by_idx(target_idx, AFF_STATE_OFF);

exit:
	psci_spin_unlock_cpu(target_idx);
	return rc;
}

/*******************************************************************************
 * The following function finish an earlier power on request. They
 * are called by the common finisher routine in psci_common.c. The `state_info`
 * is the psci_power_state from which this CPU has woken up from.
 ******************************************************************************/
void psci_cpu_on_finish(unsigned int cpu_idx,
			psci_power_state_t *state_info)
{
	/*
	 * Plat. management: Perform the platform specific actions
	 * for this cpu e.g. enabling the gic or zeroing the mailbox
	 * register. The actual state of this cpu has already been
	 * changed.
	 */
	psci_plat_pm_ops->pwr_domain_on_finish(state_info);

	/*
	 * Arch. management: Enable data cache and manage stack memory
	 */
	psci_do_pwrup_cache_maintenance();

	/*
	 * All the platform specific actions for turning this cpu
	 * on have completed. Perform enough arch.initialization
	 * to run in the non-secure address space.
	 */
	bl31_arch_setup();

	/*
	 * Lock the CPU spin lock to make sure that the context initialization
	 * is done. Since the lock is only used in this function to create
	 * a synchronization point with cpu_on_start(), it can be released
	 * immediately.
	 */
	psci_spin_lock_cpu(cpu_idx);
	psci_spin_unlock_cpu(cpu_idx);

	/* Ensure we have been explicitly woken up by another cpu */
	assert(psci_get_aff_info_state() == AFF_STATE_ON_PENDING);

	/*
	 * Call the cpu on finish handler registered by the Secure Payload
	 * Dispatcher to let it do any bookeeping. If the handler encounters an
	 * error, it's expected to assert within
	 */
	if (psci_spd_pm && psci_spd_pm->svc_on_finish)
		psci_spd_pm->svc_on_finish(0);

	/* Populate the mpidr field within the cpu node array */
	/* This needs to be done only once */
	psci_cpu_pd_nodes[cpu_idx].mpidr = read_mpidr() & MPIDR_AFFINITY_MASK;

	/*
	 * Generic management: Now we just need to retrieve the
	 * information that we had stashed away during the cpu_on
	 * call to set this cpu on its way.
	 */
	cm_prepare_el3_exit(NON_SECURE);
}
