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

typedef int (*afflvl_off_handler)(unsigned long, aff_map_node *);

/*******************************************************************************
 * The next three functions implement a handler for each supported affinity
 * level which is called when that affinity level is turned off.
 ******************************************************************************/
static int psci_afflvl0_off(unsigned long mpidr, aff_map_node *cpu_node)
{
	unsigned int index, plat_state;
	int rc = PSCI_E_SUCCESS;
	unsigned long sctlr = read_sctlr();

	assert(cpu_node->level == MPIDR_AFFLVL0);

	/*
	 * Generic management: Get the index for clearing any
	 * lingering re-entry information
	 */
	index = cpu_node->data;
	memset(&psci_ns_entry_info[index], 0, sizeof(psci_ns_entry_info[index]));

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
	 * Plat. management: Perform platform specific actions to turn this
	 * cpu off e.g. exit cpu coherency, program the power controller etc.
	 */
	if (psci_plat_pm_ops->affinst_off) {

		/* Get the current physical state of this cpu */
		plat_state = psci_get_aff_phys_state(cpu_node);
		rc = psci_plat_pm_ops->affinst_off(mpidr,
						   cpu_node->level,
						   plat_state);
	}

	/*
	 * The only error cpu_off can return is E_DENIED. So check if that's
	 * indeed the case. The caller will simply 'eret' in case of an error.
	 */
	if (rc != PSCI_E_SUCCESS)
		assert(rc == PSCI_E_DENIED);

	return rc;
}

static int psci_afflvl1_off(unsigned long mpidr, aff_map_node *cluster_node)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int plat_state;

	/* Sanity check the cluster level */
	assert(cluster_node->level == MPIDR_AFFLVL1);

	/*
	 * Keep the physical state of this cluster handy to decide
	 * what action needs to be taken
	 */
	plat_state = psci_get_aff_phys_state(cluster_node);

	/*
	 * Arch. Management. Flush all levels of caches to PoC if
	 * the cluster is to be shutdown
	 */
	if (plat_state == PSCI_STATE_OFF)
		dcsw_op_all(DCCISW);

	/*
	 * Plat. Management. Allow the platform to do it's cluster
	 * specific bookeeping e.g. turn off interconnect coherency,
	 * program the power controller etc.
	 */
	if (psci_plat_pm_ops->affinst_off)
		rc = psci_plat_pm_ops->affinst_off(mpidr,
						   cluster_node->level,
						   plat_state);

	return rc;
}

static int psci_afflvl2_off(unsigned long mpidr, aff_map_node *system_node)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int plat_state;

	/* Cannot go beyond this level */
	assert(system_node->level == MPIDR_AFFLVL2);

	/*
	 * Keep the physical state of the system handy to decide what
	 * action needs to be taken
	 */
	plat_state = psci_get_aff_phys_state(system_node);

	/* No arch. and generic bookeeping to do here currently */

	/*
	 * Plat. Management : Allow the platform to do it's bookeeping
	 * at this affinity level
	 */
	if (psci_plat_pm_ops->affinst_off)
		rc = psci_plat_pm_ops->affinst_off(mpidr,
						   system_node->level,
						   plat_state);
	return rc;
}

static const afflvl_off_handler psci_afflvl_off_handlers[] = {
	psci_afflvl0_off,
	psci_afflvl1_off,
	psci_afflvl2_off,
};

/*******************************************************************************
 * This function implements the core of the processing required to turn a cpu
 * off. It's assumed that along with turning the cpu off, higher affinity levels
 * will be turned off as far as possible. We first need to determine the new
 * state off all the affinity instances in the mpidr corresponding to the target
 * cpu. Action will be taken on the basis of this new state. To do the state
 * change we first need to acquire the locks for all the implemented affinity
 * level to be able to snapshot the system state. Then we need to start turning
 * affinity levels off from the lowest to the highest (e.g. a cpu needs to be
 * off before a cluster can be turned off). To achieve this flow, we start
 * acquiring the locks from the highest to the lowest affinity level. Once we
 * reach affinity level 0, we do the state change followed by the actions
 * corresponding to the new state for affinity level 0. Actions as per the
 * updated state for higher affinity levels are performed as we unwind back to
 * highest affinity level.
 ******************************************************************************/
int psci_afflvl_off(unsigned long mpidr,
		    int cur_afflvl,
		    int tgt_afflvl)
{
	int rc = PSCI_E_SUCCESS, level;
	unsigned int next_state, prev_state;
	aff_map_node *aff_node;

	mpidr &= MPIDR_AFFINITY_MASK;;

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
	next_state = PSCI_STATE_OFF;

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
		rc = psci_afflvl_off(mpidr, level - 1, tgt_afflvl);
		if (rc != PSCI_E_SUCCESS) {
			psci_set_state(aff_node->state, prev_state);
			goto exit;
		}
	}

	/*
	 * Perform generic, architecture and platform specific
	 * handling
	 */
	rc = psci_afflvl_off_handlers[level](mpidr, aff_node);
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
		assert(next_state == PSCI_STATE_OFF);
	}

exit:
	bakery_lock_release(mpidr, &aff_node->lock);
	return rc;
}
