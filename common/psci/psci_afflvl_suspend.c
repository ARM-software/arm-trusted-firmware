/*
 * Copyright (c) 2013, ARM Limited and Contributors. All rights reserved.
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

typedef int (*afflvl_suspend_handler)(unsigned long,
				      aff_map_node *,
				      unsigned long,
				      unsigned long,
				      unsigned int);

/*******************************************************************************
 * The next three functions implement a handler for each supported affinity
 * level which is called when that affinity level is about to be suspended.
 ******************************************************************************/
static int psci_afflvl0_suspend(unsigned long mpidr,
				aff_map_node *cpu_node,
				unsigned long ns_entrypoint,
				unsigned long context_id,
				unsigned int power_state)
{
	unsigned int index, plat_state;
	unsigned long psci_entrypoint, sctlr = read_sctlr();
	int rc = PSCI_E_SUCCESS;

	/* Sanity check to safeguard against data corruption */
	assert(cpu_node->level == MPIDR_AFFLVL0);

	/*
	 * Generic management: Store the re-entry information for the
	 * non-secure world
	 */
	index = cpu_node->data;
	rc = psci_set_ns_entry_info(index, ns_entrypoint, context_id);
	if (rc != PSCI_E_SUCCESS)
		return rc;

	/*
	 * Arch. management: Save the secure context, flush the
	 * L1 caches and exit intra-cluster coherency et al
	 */
	psci_secure_context[index].sctlr = read_sctlr();
	psci_secure_context[index].scr = read_scr();
	psci_secure_context[index].cptr = read_cptr();
	psci_secure_context[index].cpacr = read_cpacr();
	psci_secure_context[index].cntfrq = read_cntfrq_el0();
	psci_secure_context[index].mair = read_mair();
	psci_secure_context[index].tcr = read_tcr();
	psci_secure_context[index].ttbr = read_ttbr0();
	psci_secure_context[index].vbar = read_vbar();
	psci_secure_context[index].pstate =
		read_daif() & (DAIF_ABT_BIT | DAIF_DBG_BIT);

	/* Set the secure world (EL3) re-entry point after BL1 */
	psci_entrypoint = (unsigned long) psci_aff_suspend_finish_entry;

	/*
	 * Arch. management. Perform the necessary steps to flush all
	 * cpu caches.
	 *
	 * TODO: This power down sequence varies across cpus so it needs to be
	 * abstracted out on the basis of the MIDR like in cpu_reset_handler().
	 * Do the bare minimal for the time being. Fix this before porting to
	 * Cortex models.
	 */
	sctlr &= ~SCTLR_C_BIT;
	write_sctlr(sctlr);

	/*
	 * CAUTION: This flush to the level of unification makes an assumption
	 * about the cache hierarchy at affinity level 0 (cpu) in the platform.
	 * Ideally the platform should tell psci which levels to flush to exit
	 * coherency.
	 */
	dcsw_op_louis(DCCISW);

	/*
	 * Plat. management: Allow the platform to perform the
	 * necessary actions to turn off this cpu e.g. set the
	 * platform defined mailbox with the psci entrypoint,
	 * program the power controller etc.
	 */
	if (psci_plat_pm_ops->affinst_suspend) {
		plat_state = psci_get_aff_phys_state(cpu_node);
		rc = psci_plat_pm_ops->affinst_suspend(mpidr,
						       psci_entrypoint,
						       ns_entrypoint,
						       cpu_node->level,
						       plat_state);
	}

	return rc;
}

static int psci_afflvl1_suspend(unsigned long mpidr,
				aff_map_node *cluster_node,
				unsigned long ns_entrypoint,
				unsigned long context_id,
				unsigned int power_state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int plat_state;
	unsigned long psci_entrypoint;

	/* Sanity check the cluster level */
	assert(cluster_node->level == MPIDR_AFFLVL1);

	/*
	 * Keep the physical state of this cluster handy to decide
	 * what action needs to be taken
	 */
	plat_state = psci_get_aff_phys_state(cluster_node);

	/*
	 * Arch. management: Flush all levels of caches to PoC if the
	 * cluster is to be shutdown
	 */
	if (plat_state == PSCI_STATE_OFF)
		dcsw_op_all(DCCISW);

	/*
	 * Plat. Management. Allow the platform to do it's cluster
	 * specific bookeeping e.g. turn off interconnect coherency,
	 * program the power controller etc.
	 */
	if (psci_plat_pm_ops->affinst_suspend) {

		/*
		 * Sending the psci entrypoint is currently redundant
		 * beyond affinity level 0 but one never knows what a
		 * platform might do. Also it allows us to keep the
		 * platform handler prototype the same.
		 */
		psci_entrypoint = (unsigned long) psci_aff_suspend_finish_entry;

		rc = psci_plat_pm_ops->affinst_suspend(mpidr,
						       psci_entrypoint,
						       ns_entrypoint,
						       cluster_node->level,
						       plat_state);
	}

	return rc;
}


static int psci_afflvl2_suspend(unsigned long mpidr,
				aff_map_node *system_node,
				unsigned long ns_entrypoint,
				unsigned long context_id,
				unsigned int power_state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int plat_state;
	unsigned long psci_entrypoint;

	/* Cannot go beyond this */
	assert(system_node->level == MPIDR_AFFLVL2);

	/*
	 * Keep the physical state of the system handy to decide what
	 * action needs to be taken
	 */
	plat_state = psci_get_aff_phys_state(system_node);

	/*
	 * Plat. Management : Allow the platform to do it's bookeeping
	 * at this affinity level
	 */
	if (psci_plat_pm_ops->affinst_suspend) {

		/*
		 * Sending the psci entrypoint is currently redundant
		 * beyond affinity level 0 but one never knows what a
		 * platform might do. Also it allows us to keep the
		 * platform handler prototype the same.
		 */
		psci_entrypoint = (unsigned long) psci_aff_suspend_finish_entry;

		rc = psci_plat_pm_ops->affinst_suspend(mpidr,
						       psci_entrypoint,
						       ns_entrypoint,
						       system_node->level,
						       plat_state);
	}

	return rc;
}

static const afflvl_suspend_handler psci_afflvl_suspend_handlers[] = {
	psci_afflvl0_suspend,
	psci_afflvl1_suspend,
	psci_afflvl2_suspend,
};

/*******************************************************************************
 * This function implements the core of the processing required to suspend a cpu
 * It'S assumed that along with suspending the cpu, higher affinity levels will
 * be suspended as far as possible. Suspending a cpu is equivalent to physically
 * powering it down, but the cpu is still available to the OS for scheduling.
 * We first need to determine the new state off all the affinity instances in
 * the mpidr corresponding to the target cpu. Action will be taken on the basis
 * of this new state. To do the state change we first need to acquire the locks
 * for all the implemented affinity level to be able to snapshot the system
 * state. Then we need to start suspending affinity levels from the lowest to
 * the highest (e.g. a cpu needs to be suspended before a cluster can be). To
 * achieve this flow, we start acquiring the locks from the highest to the
 * lowest affinity level. Once we reach affinity level 0, we do the state change
 * followed by the actions corresponding to the new state for affinity level 0.
 * Actions as per the updated state for higher affinity levels are performed as
 * we unwind back to highest affinity level.
 ******************************************************************************/
int psci_afflvl_suspend(unsigned long mpidr,
			unsigned long entrypoint,
			unsigned long context_id,
			unsigned int power_state,
			int cur_afflvl,
			int tgt_afflvl)
{
	int rc = PSCI_E_SUCCESS, level;
	unsigned int prev_state, next_state;
	aff_map_node *aff_node;

	mpidr &= MPIDR_AFFINITY_MASK;

	/*
	 * Some affinity instances at levels between the current and
	 * target levels could be absent in the mpidr. Skip them and
	 * start from the first present instance.
	 */
	level = psci_get_first_present_afflvl(mpidr,
					      cur_afflvl,
					      tgt_afflvl,
					      &aff_node);

	/*
	 * Return if there are no more affinity instances beyond this
	 * level to process. Else ensure that the returned affinity
	 * node makes sense.
	 */
	if (aff_node == NULL)
		return rc;

	assert(level == aff_node->level);

	/*
	 * This function acquires the lock corresponding to each
	 * affinity level so that state management can be done safely.
	 */
	bakery_lock_get(mpidr, &aff_node->lock);

	/* Keep the old state and the next one handy */
	prev_state = psci_get_state(aff_node->state);
	next_state = PSCI_STATE_SUSPEND;

	/*
	 * We start from the highest affinity level and work our way
	 * downwards to the lowest i.e. MPIDR_AFFLVL0.
	 */
	if (aff_node->level == tgt_afflvl) {
		psci_change_state(mpidr,
				  tgt_afflvl,
				  get_max_afflvl(),
				  next_state);
	} else {
		rc = psci_afflvl_suspend(mpidr,
					 entrypoint,
					 context_id,
					 power_state,
					 level - 1,
					 tgt_afflvl);
		if (rc != PSCI_E_SUCCESS) {
			psci_set_state(aff_node->state, prev_state);
			goto exit;
		}
	}

	/*
	 * Perform generic, architecture and platform specific
	 * handling
	 */
	rc = psci_afflvl_suspend_handlers[level](mpidr,
						 aff_node,
						 entrypoint,
						 context_id,
						 power_state);
	if (rc != PSCI_E_SUCCESS) {
		psci_set_state(aff_node->state, prev_state);
		goto exit;
	}

	/*
	 * If all has gone as per plan then this cpu should be
	 * marked as OFF
	 */
	if (level == MPIDR_AFFLVL0) {
		next_state = psci_get_state(aff_node->state);
		assert(next_state == PSCI_STATE_SUSPEND);
	}

exit:
	bakery_lock_release(mpidr, &aff_node->lock);
	return rc;
}

/*******************************************************************************
 * The following functions finish an earlier affinity suspend request. They
 * are called by the common finisher routine in psci_common.c.
 ******************************************************************************/
static unsigned int psci_afflvl0_suspend_finish(unsigned long mpidr,
						aff_map_node *cpu_node,
						unsigned int prev_state)
{
	unsigned int index, plat_state, rc = 0;

	assert(cpu_node->level == MPIDR_AFFLVL0);

	/*
	 * Plat. management: Perform the platform specific actions
	 * before we change the state of the cpu e.g. enabling the
	 * gic or zeroing the mailbox register. If anything goes
	 * wrong then assert as there is no way to recover from this
	 * situation.
	 */
	if (psci_plat_pm_ops->affinst_suspend_finish) {
		plat_state = psci_get_phys_state(prev_state);
		rc = psci_plat_pm_ops->affinst_suspend_finish(mpidr,
							      cpu_node->level,
							      plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	/* Get the index for restoring the re-entry information */
	index = cpu_node->data;

	/*
	 * Arch. management: Restore the stashed secure architectural
	 * context in the right order.
	 */
	write_vbar(psci_secure_context[index].vbar);
	write_daif(read_daif() | psci_secure_context[index].pstate);
	write_mair(psci_secure_context[index].mair);
	write_tcr(psci_secure_context[index].tcr);
	write_ttbr0(psci_secure_context[index].ttbr);
	write_sctlr(psci_secure_context[index].sctlr);

	/* MMU and coherency should be enabled by now */
	write_scr(psci_secure_context[index].scr);
	write_cptr(psci_secure_context[index].cptr);
	write_cpacr(psci_secure_context[index].cpacr);
	write_cntfrq_el0(psci_secure_context[index].cntfrq);

	/*
	 * Generic management: Now we just need to retrieve the
	 * information that we had stashed away during the suspend
	 * call to set this cpu on it's way.
	 */
	psci_get_ns_entry_info(index);

	/* Clean caches before re-entering normal world */
	dcsw_op_louis(DCCSW);

	return rc;
}

static unsigned int psci_afflvl1_suspend_finish(unsigned long mpidr,
						aff_map_node *cluster_node,
						unsigned int prev_state)
{
	unsigned int rc = 0;
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
	if (psci_plat_pm_ops->affinst_suspend_finish) {
		plat_state = psci_get_phys_state(prev_state);
		rc = psci_plat_pm_ops->affinst_suspend_finish(mpidr,
							      cluster_node->level,
							      plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	return rc;
}


static unsigned int psci_afflvl2_suspend_finish(unsigned long mpidr,
						aff_map_node *system_node,
						unsigned int target_afflvl)
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
	if (psci_plat_pm_ops->affinst_suspend_finish) {
		plat_state = psci_get_phys_state(system_node->state);
		rc = psci_plat_pm_ops->affinst_suspend_finish(mpidr,
							      system_node->level,
							      plat_state);
		assert(rc == PSCI_E_SUCCESS);
	}

	return rc;
}

const afflvl_power_on_finisher psci_afflvl_suspend_finishers[] = {
	psci_afflvl0_suspend_finish,
	psci_afflvl1_suspend_finish,
	psci_afflvl2_suspend_finish,
};

