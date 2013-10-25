/*
 * Copyright (c) 2013, ARM Limited. All rights reserved.
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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <psci.h>
#include <psci_private.h>

typedef int (*afflvl_on_handler)(unsigned long,
				 aff_map_node *,
				 unsigned long,
				 unsigned long);

/*******************************************************************************
 * This function checks whether a cpu which has been requested to be turned on
 * is OFF to begin with.
 ******************************************************************************/
static int cpu_on_validate_state(unsigned int state)
{
	unsigned int psci_state;

	/* Get the raw psci state */
	psci_state = psci_get_state(state);

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
			   aff_map_node *cpu_node,
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
	rc = cpu_on_validate_state(cpu_node->state);
	if (rc != PSCI_E_SUCCESS)
		return rc;

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

	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	if (psci_plat_pm_ops->affinst_on) {

		/* Get the current physical state of this cpu */
		plat_state = psci_get_aff_phys_state(cpu_node);
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
			   aff_map_node *cluster_node,
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

	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	if (psci_plat_pm_ops->affinst_on) {
		plat_state = psci_get_aff_phys_state(cluster_node);
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
			   aff_map_node *system_node,
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

	/*
	 * Plat. management: Give the platform the current state
	 * of the target cpu to allow it to perform the necessary
	 * steps to power on.
	 */
	if (psci_plat_pm_ops->affinst_on) {
		plat_state = psci_get_aff_phys_state(system_node);
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
static const afflvl_on_handler psci_afflvl_on_handlers[] = {
	psci_afflvl0_on,
	psci_afflvl1_on,
	psci_afflvl2_on,
};

/*******************************************************************************
 * This function implements the core of the processing required to turn a cpu
 * on. It avoids recursion to traverse from the lowest to the highest affinity
 * level unlike the off/suspend/pon_finisher functions. It does ensure that the
 * locks are picked in the same order as the order routines to avoid deadlocks.
 * The flow is: Take all the locks until the highest affinity level, Call the
 * handlers for turning an affinity level on & finally change the state of the
 * affinity level.
 ******************************************************************************/
int psci_afflvl_on(unsigned long target_cpu,
		   unsigned long entrypoint,
		   unsigned long context_id,
		   int current_afflvl,
		   int target_afflvl)
{
	unsigned int prev_state, next_state;
	int rc = PSCI_E_SUCCESS, level;
	aff_map_node *aff_node;
	unsigned long mpidr = read_mpidr() & MPIDR_AFFINITY_MASK;

	/*
	 * This loop acquires the lock corresponding to each
	 * affinity level so that by the time we hit the lowest
	 * affinity level, the system topology is snapshot and
	 * state management can be done safely.
	 */
	for (level = current_afflvl; level >= target_afflvl; level--) {
		aff_node = psci_get_aff_map_node(target_cpu, level);
		if (aff_node)
			bakery_lock_get(mpidr, &aff_node->lock);
	}

	/*
	 * Perform generic, architecture and platform specific
	 * handling
	 */
	for (level = current_afflvl; level >= target_afflvl; level--) {

		/* Grab the node for each affinity level once again */
		aff_node = psci_get_aff_map_node(target_cpu, level);
		if (aff_node) {

			/* Keep the old state and the next one handy */
			prev_state = psci_get_state(aff_node->state);
			rc = psci_afflvl_on_handlers[level](target_cpu,
							    aff_node,
							    entrypoint,
							    context_id);
			if (rc != PSCI_E_SUCCESS) {
				psci_set_state(aff_node->state, prev_state);
				goto exit;
			}
		}
	}

	/*
	 * State management: Update the states since this is the
	 * target affinity level requested.
	 */
	psci_change_state(target_cpu,
			  target_afflvl,
			  get_max_afflvl(),
			  PSCI_STATE_ON_PENDING);

exit:
	/*
	 * This loop releases the lock corresponding to each affinity level
	 * in the reverse order. It also checks the final state of the cpu.
	 */
	for (level = target_afflvl; level <= current_afflvl; level++) {
		aff_node = psci_get_aff_map_node(target_cpu, level);
		if (aff_node) {
			if (level == MPIDR_AFFLVL0) {
				next_state = psci_get_state(aff_node->state);
				assert(next_state == PSCI_STATE_ON_PENDING);
			}
			bakery_lock_release(mpidr, &aff_node->lock);
		}
	}

	return rc;
}

/*******************************************************************************
 * The following functions finish an earlier affinity power on request. They
 * are called by the common finisher routine in psci_common.c.
 ******************************************************************************/
static unsigned int psci_afflvl0_on_finish(unsigned long mpidr,
					   aff_map_node *cpu_node,
					   unsigned int prev_state)
{
	unsigned int index, plat_state, rc = PSCI_E_SUCCESS;

	assert(cpu_node->level == MPIDR_AFFLVL0);

	/*
	 * Plat. management: Perform the platform specific actions
	 * for this cpu e.g. enabling the gic or zeroing the mailbox
	 * register. The actual state of this cpu has already been
	 * changed.
	 */
	if (psci_plat_pm_ops->affinst_on_finish) {

		/* Get the previous physical state of this cpu */
		plat_state = psci_get_phys_state(prev_state);
		rc = psci_plat_pm_ops->affinst_on_finish(mpidr,
							 cpu_node->level,
							 plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	/*
	 * Arch. management: Turn on mmu & restore architectural state
	 */
	write_vbar((unsigned long) runtime_exceptions);
	enable_mmu();

	/*
	 * All the platform specific actions for turning this cpu
	 * on have completed. Perform enough arch.initialization
	 * to run in the non-secure address space.
	 */
	bl31_arch_setup();

	/*
	 * Generic management: Now we just need to retrieve the
	 * information that we had stashed away during the cpu_on
	 * call to set this cpu on it's way. First get the index
	 * for restoring the re-entry info
	 */
	index = cpu_node->data;
	rc = psci_get_ns_entry_info(index);

	/* Clean caches before re-entering normal world */
	dcsw_op_louis(DCCSW);

	return rc;
}

static unsigned int psci_afflvl1_on_finish(unsigned long mpidr,
					   aff_map_node *cluster_node,
					   unsigned int prev_state)
{
	unsigned int rc = PSCI_E_SUCCESS;
	unsigned int plat_state;

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
		plat_state = psci_get_phys_state(prev_state);
		rc = psci_plat_pm_ops->affinst_on_finish(mpidr,
							 cluster_node->level,
							 plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	return rc;
}


static unsigned int psci_afflvl2_on_finish(unsigned long mpidr,
					   aff_map_node *system_node,
					   unsigned int prev_state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int plat_state;

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
		plat_state = psci_get_phys_state(system_node->state);
		rc = psci_plat_pm_ops->affinst_on_finish(mpidr,
							 system_node->level,
							 plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	return rc;
}

const afflvl_power_on_finisher psci_afflvl_on_finishers[] = {
	psci_afflvl0_on_finish,
	psci_afflvl1_on_finish,
	psci_afflvl2_on_finish,
};

