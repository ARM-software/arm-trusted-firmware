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
#include <context_mgmt.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stddef.h>
#include "psci_private.h"

typedef int (*afflvl_on_handler_t)(unsigned long,
				 aff_map_node_t *,
				 unsigned long,
				 unsigned long);

/*******************************************************************************
 * This function checks whether a cpu which has been requested to be turned on
 * is OFF to begin with.
 ******************************************************************************/
static int cpu_on_validate_state(aff_map_node_t *node)
{
	unsigned int psci_state;

	/* Get the raw psci state */
	psci_state = psci_get_state(node);

	if (psci_state == PSCI_STATE_ON || psci_state == PSCI_STATE_SUSPEND)
		return PSCI_E_ALREADY_ON;

	if (psci_state == PSCI_STATE_ON_PENDING)
		return PSCI_E_ON_PENDING;

	assert(psci_state == PSCI_STATE_OFF);
	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler routine to turn a cpu on. It takes care of any generic, architectural
 * or platform specific setup required.
 * TODO: Split this code across separate handlers for each type of setup?
 ******************************************************************************/
static int psci_afflvl0_on(unsigned long target_cpu,
			   aff_map_node_t *cpu_node,
			   unsigned long ns_entrypoint,
			   unsigned long context_id)
{
	unsigned int index, plat_state;
	unsigned long psci_entrypoint;
	int rc;

	/* Sanity check to safeguard against data corruption */
	assert(cpu_node->level == MPIDR_AFFLVL0);

	/*
	 * Generic management: Ensure that the cpu is off to be
	 * turned on
	 */
	rc = cpu_on_validate_state(cpu_node);
	if (rc != PSCI_E_SUCCESS)
		return rc;

	/*
	 * Call the cpu on handler registered by the Secure Payload Dispatcher
	 * to let it do any bookeeping. If the handler encounters an error, it's
	 * expected to assert within
	 */
	if (psci_spd_pm && psci_spd_pm->svc_on)
		psci_spd_pm->svc_on(target_cpu);

	/*
	 * Arch. management: Derive the re-entry information for
	 * the non-secure world from the non-secure state from
	 * where this call originated.
	 */
	index = cpu_node->data;
	rc = psci_set_ns_entry_info(index, ns_entrypoint, context_id);
	if (rc != PSCI_E_SUCCESS)
		return rc;

	/* Set the secure world (EL3) re-entry point after BL1 */
	psci_entrypoint = (unsigned long) psci_aff_on_finish_entry;

	/* State management: Set this cpu's state as ON PENDING */
	psci_set_state(cpu_node, PSCI_STATE_ON_PENDING);

	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	if (psci_plat_pm_ops->affinst_on) {

		/* Get the current physical state of this cpu */
		plat_state = psci_get_phys_state(cpu_node);
		rc = psci_plat_pm_ops->affinst_on(target_cpu,
						  psci_entrypoint,
						  ns_entrypoint,
						  cpu_node->level,
						  plat_state);
	}

	return rc;
}

/*******************************************************************************
 * Handler routine to turn a cluster on. It takes care or any generic, arch.
 * or platform specific setup required.
 * TODO: Split this code across separate handlers for each type of setup?
 ******************************************************************************/
static int psci_afflvl1_on(unsigned long target_cpu,
			   aff_map_node_t *cluster_node,
			   unsigned long ns_entrypoint,
			   unsigned long context_id)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int plat_state;
	unsigned long psci_entrypoint;

	assert(cluster_node->level == MPIDR_AFFLVL1);

	/*
	 * There is no generic and arch. specific cluster
	 * management required
	 */

	/* State management: Is not required while turning a cluster on */

	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	if (psci_plat_pm_ops->affinst_on) {
		plat_state = psci_get_phys_state(cluster_node);
		psci_entrypoint = (unsigned long) psci_aff_on_finish_entry;
		rc = psci_plat_pm_ops->affinst_on(target_cpu,
						  psci_entrypoint,
						  ns_entrypoint,
						  cluster_node->level,
						  plat_state);
	}

	return rc;
}

/*******************************************************************************
 * Handler routine to turn a cluster of clusters on. It takes care or any
 * generic, arch. or platform specific setup required.
 * TODO: Split this code across separate handlers for each type of setup?
 ******************************************************************************/
static int psci_afflvl2_on(unsigned long target_cpu,
			   aff_map_node_t *system_node,
			   unsigned long ns_entrypoint,
			   unsigned long context_id)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int plat_state;
	unsigned long psci_entrypoint;

	/* Cannot go beyond affinity level 2 in this psci imp. */
	assert(system_node->level == MPIDR_AFFLVL2);

	/*
	 * There is no generic and arch. specific system management
	 * required
	 */

	/* State management: Is not required while turning a system on */

	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	if (psci_plat_pm_ops->affinst_on) {
		plat_state = psci_get_phys_state(system_node);
		psci_entrypoint = (unsigned long) psci_aff_on_finish_entry;
		rc = psci_plat_pm_ops->affinst_on(target_cpu,
						  psci_entrypoint,
						  ns_entrypoint,
						  system_node->level,
						  plat_state);
	}

	return rc;
}

/* Private data structure to make this handlers accessible through indexing */
static const afflvl_on_handler_t psci_afflvl_on_handlers[] = {
	psci_afflvl0_on,
	psci_afflvl1_on,
	psci_afflvl2_on,
};

/*******************************************************************************
 * This function takes an array of pointers to affinity instance nodes in the
 * topology tree and calls the on handler for the corresponding affinity
 * levels
 ******************************************************************************/
static int psci_call_on_handlers(mpidr_aff_map_nodes_t target_cpu_nodes,
				 int start_afflvl,
				 int end_afflvl,
				 unsigned long target_cpu,
				 unsigned long entrypoint,
				 unsigned long context_id)
{
	int rc = PSCI_E_INVALID_PARAMS, level;
	aff_map_node_t *node;

	for (level = end_afflvl; level >= start_afflvl; level--) {
		node = target_cpu_nodes[level];
		if (node == NULL)
			continue;

		/*
		 * TODO: In case of an error should there be a way
		 * of undoing what we might have setup at higher
		 * affinity levels.
		 */
		rc = psci_afflvl_on_handlers[level](target_cpu,
						    node,
						    entrypoint,
						    context_id);
		if (rc != PSCI_E_SUCCESS)
			break;
	}

	return rc;
}

/*******************************************************************************
 * Generic handler which is called to physically power on a cpu identified by
 * its mpidr. It traverses through all the affinity levels performing generic,
 * architectural, platform setup and state management e.g. for a cpu that is
 * to be powered on, it will ensure that enough information is stashed for it
 * to resume execution in the non-secure security state.
 *
 * The state of all the relevant affinity levels is changed after calling the
 * affinity level specific handlers as their actions would depend upon the state
 * the affinity level is currently in.
 *
 * The affinity level specific handlers are called in descending order i.e. from
 * the highest to the lowest affinity level implemented by the platform because
 * to turn on affinity level X it is neccesary to turn on affinity level X + 1
 * first.
 ******************************************************************************/
int psci_afflvl_on(unsigned long target_cpu,
		   unsigned long entrypoint,
		   unsigned long context_id,
		   int start_afflvl,
		   int end_afflvl)
{
	int rc = PSCI_E_SUCCESS;
	mpidr_aff_map_nodes_t target_cpu_nodes;
	unsigned long mpidr = read_mpidr() & MPIDR_AFFINITY_MASK;

	/*
	 * Collect the pointers to the nodes in the topology tree for
	 * each affinity instance in the mpidr. If this function does
	 * not return successfully then either the mpidr or the affinity
	 * levels are incorrect.
	 */
	rc = psci_get_aff_map_nodes(target_cpu,
				    start_afflvl,
				    end_afflvl,
				    target_cpu_nodes);
	if (rc != PSCI_E_SUCCESS)
		return rc;


	/*
	 * This function acquires the lock corresponding to each affinity
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_afflvl_locks(mpidr,
				  start_afflvl,
				  end_afflvl,
				  target_cpu_nodes);

	/* Perform generic, architecture and platform specific handling. */
	rc = psci_call_on_handlers(target_cpu_nodes,
				   start_afflvl,
				   end_afflvl,
				   target_cpu,
				   entrypoint,
				   context_id);

	/*
	 * This loop releases the lock corresponding to each affinity level
	 * in the reverse order to which they were acquired.
	 */
	psci_release_afflvl_locks(mpidr,
				  start_afflvl,
				  end_afflvl,
				  target_cpu_nodes);

	return rc;
}

/*******************************************************************************
 * The following functions finish an earlier affinity power on request. They
 * are called by the common finisher routine in psci_common.c.
 ******************************************************************************/
static unsigned int psci_afflvl0_on_finish(unsigned long mpidr,
					   aff_map_node_t *cpu_node)
{
	unsigned int index, plat_state, state, rc = PSCI_E_SUCCESS;

	assert(cpu_node->level == MPIDR_AFFLVL0);

	/* Ensure we have been explicitly woken up by another cpu */
	state = psci_get_state(cpu_node);
	assert(state == PSCI_STATE_ON_PENDING);

	/*
	 * Plat. management: Perform the platform specific actions
	 * for this cpu e.g. enabling the gic or zeroing the mailbox
	 * register. The actual state of this cpu has already been
	 * changed.
	 */
	if (psci_plat_pm_ops->affinst_on_finish) {

		/* Get the physical state of this cpu */
		plat_state = get_phys_state(state);
		rc = psci_plat_pm_ops->affinst_on_finish(mpidr,
							 cpu_node->level,
							 plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	/*
	 * Arch. management: Turn on mmu & restore architectural state
	 */
	bl31_plat_enable_mmu();

	/*
	 * All the platform specific actions for turning this cpu
	 * on have completed. Perform enough arch.initialization
	 * to run in the non-secure address space.
	 */
	bl31_arch_setup();

	/*
	 * Use the more complex exception vectors to enable SPD
	 * initialisation. SP_EL3 should point to a 'cpu_context'
	 * structure. The calling cpu should have set the
	 * context already
	 */
	assert(cm_get_context(mpidr, NON_SECURE));
	cm_set_next_eret_context(NON_SECURE);
	cm_init_pcpu_ptr_cache();
	write_vbar_el3((uint64_t) runtime_exceptions);

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
	 * call to set this cpu on its way. First get the index
	 * for restoring the re-entry info
	 */
	index = cpu_node->data;
	psci_get_ns_entry_info(index);

	/* State management: mark this cpu as on */
	psci_set_state(cpu_node, PSCI_STATE_ON);

	/* Clean caches before re-entering normal world */
	dcsw_op_louis(DCCSW);

	return rc;
}

static unsigned int psci_afflvl1_on_finish(unsigned long mpidr,
					   aff_map_node_t *cluster_node)
{
	unsigned int plat_state, rc = PSCI_E_SUCCESS;

	assert(cluster_node->level == MPIDR_AFFLVL1);

	/*
	 * Plat. management: Perform the platform specific actions
	 * as per the old state of the cluster e.g. enabling
	 * coherency at the interconnect depends upon the state with
	 * which this cluster was powered up. If anything goes wrong
	 * then assert as there is no way to recover from this
	 * situation.
	 */
	if (psci_plat_pm_ops->affinst_on_finish) {

		/* Get the physical state of this cluster */
		plat_state = psci_get_phys_state(cluster_node);
		rc = psci_plat_pm_ops->affinst_on_finish(mpidr,
							 cluster_node->level,
							 plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	/* State management: Increment the cluster reference count */
	psci_set_state(cluster_node, PSCI_STATE_ON);

	return rc;
}


static unsigned int psci_afflvl2_on_finish(unsigned long mpidr,
					   aff_map_node_t *system_node)
{
	unsigned int plat_state, rc = PSCI_E_SUCCESS;

	/* Cannot go beyond this affinity level */
	assert(system_node->level == MPIDR_AFFLVL2);

	/*
	 * Currently, there are no architectural actions to perform
	 * at the system level.
	 */

	/*
	 * Plat. management: Perform the platform specific actions
	 * as per the old state of the cluster e.g. enabling
	 * coherency at the interconnect depends upon the state with
	 * which this cluster was powered up. If anything goes wrong
	 * then assert as there is no way to recover from this
	 * situation.
	 */
	if (psci_plat_pm_ops->affinst_on_finish) {

		/* Get the physical state of the system */
		plat_state = psci_get_phys_state(system_node);
		rc = psci_plat_pm_ops->affinst_on_finish(mpidr,
							 system_node->level,
							 plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	/* State management: Increment the system reference count */
	psci_set_state(system_node, PSCI_STATE_ON);

	return rc;
}

const afflvl_power_on_finisher_t psci_afflvl_on_finishers[] = {
	psci_afflvl0_on_finish,
	psci_afflvl1_on_finish,
	psci_afflvl2_on_finish,
};

