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

typedef void (*afflvl_suspend_handler_t)(aff_map_node_t *node);

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
 * This function gets the affinity level till which the current cpu could be
 * powered down during a cpu_suspend call. Returns PSCI_INVALID_DATA if the
 * power state is invalid.
 ******************************************************************************/
int psci_get_suspend_afflvl()
{
	unsigned int power_state;

	power_state = get_cpu_data(psci_svc_cpu_data.power_state);

	return ((power_state == PSCI_INVALID_DATA) ?
		power_state : psci_get_pstate_afflvl(power_state));
}

/*******************************************************************************
 * This function gets the state id of the current cpu from the power state
 * parameter saved in the per-cpu data array. Returns PSCI_INVALID_DATA if the
 * power state saved is invalid.
 ******************************************************************************/
int psci_get_suspend_stateid()
{
	unsigned int power_state;

	power_state = get_cpu_data(psci_svc_cpu_data.power_state);

	return ((power_state == PSCI_INVALID_DATA) ?
		power_state : psci_get_pstate_id(power_state));
}

/*******************************************************************************
 * This function gets the state id of the cpu specified by the 'mpidr' parameter
 * from the power state parameter saved in the per-cpu data array. Returns
 * PSCI_INVALID_DATA if the power state saved is invalid.
 ******************************************************************************/
int psci_get_suspend_stateid_by_mpidr(unsigned long mpidr)
{
	unsigned int power_state;

	power_state = get_cpu_data_by_mpidr(mpidr,
					    psci_svc_cpu_data.power_state);

	return ((power_state == PSCI_INVALID_DATA) ?
		power_state : psci_get_pstate_id(power_state));
}

/*******************************************************************************
 * The next three functions implement a handler for each supported affinity
 * level which is called when that affinity level is about to be suspended.
 ******************************************************************************/
static void psci_afflvl0_suspend(aff_map_node_t *cpu_node)
{
	unsigned long psci_entrypoint;

	/* Sanity check to safeguard against data corruption */
	assert(cpu_node->level == MPIDR_AFFLVL0);

	/* Set the secure world (EL3) re-entry point after BL1 */
	psci_entrypoint = (unsigned long) psci_aff_suspend_finish_entry;

	/*
	 * Arch. management. Perform the necessary steps to flush all
	 * cpu caches.
	 */
	psci_do_pwrdown_cache_maintenance(MPIDR_AFFLVL0);

	/*
	 * Plat. management: Allow the platform to perform the
	 * necessary actions to turn off this cpu e.g. set the
	 * platform defined mailbox with the psci entrypoint,
	 * program the power controller etc.
	 */
	psci_plat_pm_ops->affinst_suspend(psci_entrypoint,
						 cpu_node->level,
						 psci_get_phys_state(cpu_node));
}

static void psci_afflvl1_suspend(aff_map_node_t *cluster_node)
{
	unsigned int plat_state;
	unsigned long psci_entrypoint;

	/* Sanity check the cluster level */
	assert(cluster_node->level == MPIDR_AFFLVL1);

	/*
	 * Arch. management: Flush all levels of caches to PoC if the
	 * cluster is to be shutdown.
	 */
	psci_do_pwrdown_cache_maintenance(MPIDR_AFFLVL1);

	/*
	 * Plat. Management. Allow the platform to do its cluster specific
	 * bookeeping e.g. turn off interconnect coherency, program the power
	 * controller etc. Sending the psci entrypoint is currently redundant
	 * beyond affinity level 0 but one never knows what a platform might
	 * do. Also it allows us to keep the platform handler prototype the
	 * same.
	 */
	plat_state = psci_get_phys_state(cluster_node);
	psci_entrypoint = (unsigned long) psci_aff_suspend_finish_entry;
	psci_plat_pm_ops->affinst_suspend(psci_entrypoint,
						 cluster_node->level,
						 plat_state);
}


static void psci_afflvl2_suspend(aff_map_node_t *system_node)
{
	unsigned int plat_state;
	unsigned long psci_entrypoint;

	/* Cannot go beyond this */
	assert(system_node->level == MPIDR_AFFLVL2);

	/*
	 * Keep the physical state of the system handy to decide what
	 * action needs to be taken
	 */
	plat_state = psci_get_phys_state(system_node);

	/*
	 * Arch. management: Flush all levels of caches to PoC if the
	 * system is to be shutdown.
	 */
	psci_do_pwrdown_cache_maintenance(MPIDR_AFFLVL2);

	/*
	 * Plat. Management : Allow the platform to do its bookeeping
	 * at this affinity level
	 */

	/*
	 * Sending the psci entrypoint is currently redundant
	 * beyond affinity level 0 but one never knows what a
	 * platform might do. Also it allows us to keep the
	 * platform handler prototype the same.
	 */
	plat_state = psci_get_phys_state(system_node);
	psci_entrypoint = (unsigned long) psci_aff_suspend_finish_entry;
	psci_plat_pm_ops->affinst_suspend(psci_entrypoint,
						 system_node->level,
						 plat_state);
}

static const afflvl_suspend_handler_t psci_afflvl_suspend_handlers[] = {
	psci_afflvl0_suspend,
	psci_afflvl1_suspend,
	psci_afflvl2_suspend,
};

/*******************************************************************************
 * This function takes an array of pointers to affinity instance nodes in the
 * topology tree and calls the suspend handler for the corresponding affinity
 * levels
 ******************************************************************************/
static void psci_call_suspend_handlers(aff_map_node_t *mpidr_nodes[],
				      int start_afflvl,
				      int end_afflvl)
{
	int level;
	aff_map_node_t *node;

	for (level = start_afflvl; level <= end_afflvl; level++) {
		node = mpidr_nodes[level];
		if (node == NULL)
			continue;

		psci_afflvl_suspend_handlers[level](node);
	}
}

/*******************************************************************************
 * Top level handler which is called when a cpu wants to suspend its execution.
 * It is assumed that along with turning the cpu off, higher affinity levels
 * until the target affinity level will be turned off as well. It traverses
 * through all the affinity levels performing generic, architectural, platform
 * setup and state management e.g. for a cluster that's to be suspended, it will
 * call the platform specific code which will disable coherency at the
 * interconnect level if the cpu is the last in the cluster. For a cpu it could
 * mean programming the power controller etc.
 *
 * The state of all the relevant affinity levels is changed prior to calling the
 * affinity level specific handlers as their actions would depend upon the state
 * the affinity level is about to enter.
 *
 * The affinity level specific handlers are called in ascending order i.e. from
 * the lowest to the highest affinity level implemented by the platform because
 * to turn off affinity level X it is neccesary to turn off affinity level X - 1
 * first.
 *
 * All the required parameter checks are performed at the beginning and after
 * the state transition has been done, no further error is expected and it
 * is not possible to undo any of the actions taken beyond that point.
 ******************************************************************************/
void psci_afflvl_suspend(entry_point_info_t *ep,
			int start_afflvl,
			int end_afflvl)
{
	int skip_wfi = 0;
	mpidr_aff_map_nodes_t mpidr_nodes;
	unsigned int max_phys_off_afflvl;

	/*
	 * This function must only be called on platforms where the
	 * CPU_SUSPEND platform hooks have been implemented.
	 */
	assert(psci_plat_pm_ops->affinst_suspend &&
			psci_plat_pm_ops->affinst_suspend_finish);

	/*
	 * Collect the pointers to the nodes in the topology tree for
	 * each affinity instance in the mpidr. If this function does
	 * not return successfully then either the mpidr or the affinity
	 * levels are incorrect. Either way, this an internal TF error
	 * therefore assert.
	 */
	if (psci_get_aff_map_nodes(read_mpidr_el1() & MPIDR_AFFINITY_MASK,
		   start_afflvl, end_afflvl, mpidr_nodes) != PSCI_E_SUCCESS)
		assert(0);

	/*
	 * This function acquires the lock corresponding to each affinity
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_afflvl_locks(start_afflvl,
				  end_afflvl,
				  mpidr_nodes);

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
	 * This function updates the state of each affinity instance
	 * corresponding to the mpidr in the range of affinity levels
	 * specified.
	 */
	psci_do_afflvl_state_mgmt(start_afflvl,
				  end_afflvl,
				  mpidr_nodes,
				  PSCI_STATE_SUSPEND);

	max_phys_off_afflvl = psci_find_max_phys_off_afflvl(start_afflvl,
							    end_afflvl,
							    mpidr_nodes);
	assert(max_phys_off_afflvl != PSCI_INVALID_DATA);

	/* Stash the highest affinity level that will be turned off */
	psci_set_max_phys_off_afflvl(max_phys_off_afflvl);

	/*
	 * Store the re-entry information for the non-secure world.
	 */
	cm_init_context(read_mpidr_el1(), ep);

	/* Perform generic, architecture and platform specific handling */
	psci_call_suspend_handlers(mpidr_nodes,
					start_afflvl,
					end_afflvl);

	/*
	 * Invalidate the entry for the highest affinity level stashed earlier.
	 * This ensures that any reads of this variable outside the power
	 * up/down sequences return PSCI_INVALID_DATA.
	 */
	psci_set_max_phys_off_afflvl(PSCI_INVALID_DATA);

exit:
	/*
	 * Release the locks corresponding to each affinity level in the
	 * reverse order to which they were acquired.
	 */
	psci_release_afflvl_locks(start_afflvl,
				  end_afflvl,
				  mpidr_nodes);
	if (!skip_wfi)
		psci_power_down_wfi();
}

/*******************************************************************************
 * The following functions finish an earlier affinity suspend request. They
 * are called by the common finisher routine in psci_common.c.
 ******************************************************************************/
static void psci_afflvl0_suspend_finish(aff_map_node_t *cpu_node)
{
	unsigned int plat_state, state;
	int32_t suspend_level;
	uint64_t counter_freq;

	assert(cpu_node->level == MPIDR_AFFLVL0);

	/* Ensure we have been woken up from a suspended state */
	state = psci_get_state(cpu_node);
	assert(state == PSCI_STATE_SUSPEND);

	/*
	 * Plat. management: Perform the platform specific actions
	 * before we change the state of the cpu e.g. enabling the
	 * gic or zeroing the mailbox register. If anything goes
	 * wrong then assert as there is no way to recover from this
	 * situation.
	 */

	/* Get the physical state of this cpu */
	plat_state = get_phys_state(state);
	psci_plat_pm_ops->affinst_suspend_finish(cpu_node->level,
							      plat_state);

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
		suspend_level = psci_get_suspend_afflvl();
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

static void psci_afflvl1_suspend_finish(aff_map_node_t *cluster_node)
{
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

	/* Get the physical state of this cpu */
	plat_state = psci_get_phys_state(cluster_node);
	psci_plat_pm_ops->affinst_suspend_finish(cluster_node->level,
						      plat_state);
}


static void psci_afflvl2_suspend_finish(aff_map_node_t *system_node)
{
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

	/* Get the physical state of the system */
	plat_state = psci_get_phys_state(system_node);
	psci_plat_pm_ops->affinst_suspend_finish(system_node->level,
						      plat_state);
}

const afflvl_power_on_finisher_t psci_afflvl_suspend_finishers[] = {
	psci_afflvl0_suspend_finish,
	psci_afflvl1_suspend_finish,
	psci_afflvl2_suspend_finish,
};
