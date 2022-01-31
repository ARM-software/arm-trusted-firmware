/*
 * Copyright (c) 2013-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <plat/common/platform.h>

#include "psci_private.h"

/*
 * Helper functions for the CPU level spinlocks
 */
static inline void psci_spin_lock_cpu(unsigned int idx)
{
	spin_lock(&psci_cpu_pd_nodes[idx].cpu_lock);
}

static inline void psci_spin_unlock_cpu(unsigned int idx)
{
	spin_unlock(&psci_cpu_pd_nodes[idx].cpu_lock);
}

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
		      const entry_point_info_t *ep)
{
	int rc;
	aff_info_state_t target_aff_state;
	int ret = plat_core_pos_by_mpidr(target_cpu);
	unsigned int target_idx = (unsigned int)ret;

	/* Calling function must supply valid input arguments */
	assert(ret >= 0);
	assert(ep != NULL);


	/*
	 * This function must only be called on platforms where the
	 * CPU_ON platform hooks have been implemented.
	 */
	assert((psci_plat_pm_ops->pwr_domain_on != NULL) &&
	       (psci_plat_pm_ops->pwr_domain_on_finish != NULL));

	/* Protect against multiple CPUs trying to turn ON the same target CPU */
	psci_spin_lock_cpu(target_idx);

	/*
	 * Generic management: Ensure that the cpu is off to be
	 * turned on.
	 * Perform cache maintanence ahead of reading the target CPU state to
	 * ensure that the data is not stale.
	 * There is a theoretical edge case where the cache may contain stale
	 * data for the target CPU data - this can occur under the following
	 * conditions:
	 * - the target CPU is in another cluster from the current
	 * - the target CPU was the last CPU to shutdown on its cluster
	 * - the cluster was removed from coherency as part of the CPU shutdown
	 *
	 * In this case the cache maintenace that was performed as part of the
	 * target CPUs shutdown was not seen by the current CPU's cluster. And
	 * so the cache may contain stale data for the target CPU.
	 */
	flush_cpu_data_by_index(target_idx,
				psci_svc_cpu_data.aff_info_state);
	rc = cpu_on_validate_state(psci_get_aff_info_state_by_idx(target_idx));
	if (rc != PSCI_E_SUCCESS)
		goto exit;

	/*
	 * Call the cpu on handler registered by the Secure Payload Dispatcher
	 * to let it do any bookeeping. If the handler encounters an error, it's
	 * expected to assert within
	 */
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_on != NULL))
		psci_spd_pm->svc_on(target_cpu);

	/*
	 * Set the Affinity info state of the target cpu to ON_PENDING.
	 * Flush aff_info_state as it will be accessed with caches
	 * turned OFF.
	 */
	psci_set_aff_info_state_by_idx(target_idx, AFF_STATE_ON_PENDING);
	flush_cpu_data_by_index(target_idx,
				psci_svc_cpu_data.aff_info_state);

	/*
	 * The cache line invalidation by the target CPU after setting the
	 * state to OFF (see psci_do_cpu_off()), could cause the update to
	 * aff_info_state to be invalidated. Retry the update if the target
	 * CPU aff_info_state is not ON_PENDING.
	 */
	target_aff_state = psci_get_aff_info_state_by_idx(target_idx);
	if (target_aff_state != AFF_STATE_ON_PENDING) {
		assert(target_aff_state == AFF_STATE_OFF);
		psci_set_aff_info_state_by_idx(target_idx, AFF_STATE_ON_PENDING);
		flush_cpu_data_by_index(target_idx,
					psci_svc_cpu_data.aff_info_state);

		assert(psci_get_aff_info_state_by_idx(target_idx) ==
		       AFF_STATE_ON_PENDING);
	}

	/*
	 * Perform generic, architecture and platform specific handling.
	 */
	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	rc = psci_plat_pm_ops->pwr_domain_on(target_cpu);
	assert((rc == PSCI_E_SUCCESS) || (rc == PSCI_E_INTERN_FAIL));

	if (rc == PSCI_E_SUCCESS)
		/* Store the re-entry information for the non-secure world. */
		cm_init_context_by_index(target_idx, ep);
	else {
		/* Restore the state on error. */
		psci_set_aff_info_state_by_idx(target_idx, AFF_STATE_OFF);
		flush_cpu_data_by_index(target_idx,
					psci_svc_cpu_data.aff_info_state);
	}

exit:
	psci_spin_unlock_cpu(target_idx);
	return rc;
}

/*******************************************************************************
 * The following function finish an earlier power on request. They
 * are called by the common finisher routine in psci_common.c. The `state_info`
 * is the psci_power_state from which this CPU has woken up from.
 ******************************************************************************/
void psci_cpu_on_finish(unsigned int cpu_idx, const psci_power_state_t *state_info)
{
	/*
	 * Plat. management: Perform the platform specific actions
	 * for this cpu e.g. enabling the gic or zeroing the mailbox
	 * register. The actual state of this cpu has already been
	 * changed.
	 */
	psci_plat_pm_ops->pwr_domain_on_finish(state_info);

#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
	/*
	 * Arch. management: Enable data cache and manage stack memory
	 */
	psci_do_pwrup_cache_maintenance();
#endif

	/*
	 * Plat. management: Perform any platform specific actions which
	 * can only be done with the cpu and the cluster guaranteed to
	 * be coherent.
	 */
	if (psci_plat_pm_ops->pwr_domain_on_finish_late != NULL)
		psci_plat_pm_ops->pwr_domain_on_finish_late(state_info);

	/*
	 * All the platform specific actions for turning this cpu
	 * on have completed. Perform enough arch.initialization
	 * to run in the non-secure address space.
	 */
	psci_arch_setup();

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
	if ((psci_spd_pm != NULL) && (psci_spd_pm->svc_on_finish != NULL))
		psci_spd_pm->svc_on_finish(0);

	PUBLISH_EVENT(psci_cpu_on_finish);

	/* Populate the mpidr field within the cpu node array */
	/* This needs to be done only once */
	psci_cpu_pd_nodes[cpu_idx].mpidr = read_mpidr() & MPIDR_AFFINITY_MASK;

	/*
	 * Generic management: Now we just need to retrieve the
	 * information that we had stashed away during the cpu_on
	 * call to set this cpu on its way.
	 */
	cm_prepare_el3_exit_ns();
}
