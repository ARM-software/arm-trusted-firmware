/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
static int cpu_on_validate_state(unsigned int psci_state)
{
	if (psci_state == PSCI_STATE_ON || psci_state == PSCI_STATE_SUSPEND)
		return PSCI_E_ALREADY_ON;

	if (psci_state == PSCI_STATE_ON_PENDING)
		return PSCI_E_ON_PENDING;

	assert(psci_state == PSCI_STATE_OFF);
	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Generic handler which is called to physically power on a cpu identified by
 * its mpidr. It performs the generic, architectural, platform setup and state
 * management to power on the target cpu e.g. it will ensure that
 * enough information is stashed for it to resume execution in the non-secure
 * security state.
 *
 * The state of all the relevant affinity levels is changed after calling the
 * platform handler as it can return error.
 ******************************************************************************/
int psci_afflvl_on(unsigned long target_cpu,
		   entry_point_info_t *ep,
		   int end_afflvl)
{
	int rc;
	mpidr_aff_map_nodes_t target_cpu_nodes;
	unsigned long psci_entrypoint;

	/*
	 * This function must only be called on platforms where the
	 * CPU_ON platform hooks have been implemented.
	 */
	assert(psci_plat_pm_ops->affinst_on &&
			psci_plat_pm_ops->affinst_on_finish);

	/*
	 * Collect the pointers to the nodes in the topology tree for
	 * each affinity instance in the mpidr. If this function does
	 * not return successfully then either the mpidr or the affinity
	 * levels are incorrect.
	 */
	rc = psci_get_aff_map_nodes(target_cpu,
				    MPIDR_AFFLVL0,
				    end_afflvl,
				    target_cpu_nodes);
	assert(rc == PSCI_E_SUCCESS);

	/*
	 * This function acquires the lock corresponding to each affinity
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_afflvl_locks(MPIDR_AFFLVL0,
				  end_afflvl,
				  target_cpu_nodes);

	/*
	 * Generic management: Ensure that the cpu is off to be
	 * turned on.
	 */
	rc = cpu_on_validate_state(psci_get_state(
				    target_cpu_nodes[MPIDR_AFFLVL0]));
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
	 * This function updates the state of each affinity instance
	 * corresponding to the mpidr in the range of affinity levels
	 * specified.
	 */
	psci_do_afflvl_state_mgmt(MPIDR_AFFLVL0,
				   end_afflvl,
				   target_cpu_nodes,
				   PSCI_STATE_ON_PENDING);

	/*
	 * Perform generic, architecture and platform specific handling.
	 */
	/* Set the secure world (EL3) re-entry point after BL1 */
	psci_entrypoint = (unsigned long) psci_aff_on_finish_entry;

	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	rc = psci_plat_pm_ops->affinst_on(target_cpu,
				    psci_entrypoint,
				    MPIDR_AFFLVL0);
	assert(rc == PSCI_E_SUCCESS || rc == PSCI_E_INTERN_FAIL);

	if (rc == PSCI_E_SUCCESS)
		/* Store the re-entry information for the non-secure world. */
		cm_init_context(target_cpu, ep);
	else
		/* Restore the state on error. */
		psci_do_afflvl_state_mgmt(MPIDR_AFFLVL0,
					  end_afflvl,
					  target_cpu_nodes,
					  PSCI_STATE_OFF);
exit:
	/*
	 * This loop releases the lock corresponding to each affinity level
	 * in the reverse order to which they were acquired.
	 */
	psci_release_afflvl_locks(MPIDR_AFFLVL0,
				  end_afflvl,
				  target_cpu_nodes);

	return rc;
}

/*******************************************************************************
 * The following function finish an earlier affinity power on request. They
 * are called by the common finisher routine in psci_common.c.
 ******************************************************************************/
void psci_afflvl_on_finisher(aff_map_node_t *node[], int afflvl)
{
	assert(node[afflvl]->level == afflvl);

	/* Ensure we have been explicitly woken up by another cpu */
	assert(psci_get_state(node[MPIDR_AFFLVL0]) == PSCI_STATE_ON_PENDING);

	/*
	 * Plat. management: Perform the platform specific actions
	 * for this cpu e.g. enabling the gic or zeroing the mailbox
	 * register. The actual state of this cpu has already been
	 * changed.
	 */
	psci_plat_pm_ops->affinst_on_finish(afflvl);

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
	 * Call the cpu on finish handler registered by the Secure Payload
	 * Dispatcher to let it do any bookeeping. If the handler encounters an
	 * error, it's expected to assert within
	 */
	if (psci_spd_pm && psci_spd_pm->svc_on_finish)
		psci_spd_pm->svc_on_finish(0);

	/*
	 * Generic management: Now we just need to retrieve the
	 * information that we had stashed away during the cpu_on
	 * call to set this cpu on its way.
	 */
	cm_prepare_el3_exit(NON_SECURE);

	/* Clean caches before re-entering normal world */
	dcsw_op_louis(DCCSW);
}

