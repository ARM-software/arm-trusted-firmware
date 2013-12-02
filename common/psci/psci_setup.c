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
#include <psci_private.h>

/*******************************************************************************
 * Routines for retrieving the node corresponding to an affinity level instance
 * in the mpidr. The first one uses binary search to find the node corresponding
 * to the mpidr (key) at a particular affinity level. The second routine decides
 * extents of the binary search at each affinity level.
 ******************************************************************************/
static int psci_aff_map_get_idx(unsigned long key,
				int min_idx,
				int max_idx)
{
	int mid;

	/*
	 * Terminating condition: If the max and min indices have crossed paths
	 * during the binary search then the key has not been found.
	 */
	if (max_idx < min_idx)
		return PSCI_E_INVALID_PARAMS;

	/*
	 * Bisect the array around 'mid' and then recurse into the array chunk
	 * where the key is likely to be found. The mpidrs in each node in the
	 * 'psci_aff_map' for a given affinity level are stored in an ascending
	 * order which makes the binary search possible.
	 */
	mid = min_idx + ((max_idx - min_idx) >> 1);	/* Divide by 2 */
	if (psci_aff_map[mid].mpidr > key)
		return psci_aff_map_get_idx(key, min_idx, mid - 1);
	else if (psci_aff_map[mid].mpidr < key)
		return psci_aff_map_get_idx(key, mid + 1, max_idx);
	else
		return mid;
}

aff_map_node *psci_get_aff_map_node(unsigned long mpidr, int aff_lvl)
{
	int rc;

	/* Right shift the mpidr to the required affinity level */
	mpidr = mpidr_mask_lower_afflvls(mpidr, aff_lvl);

	rc = psci_aff_map_get_idx(mpidr,
				  psci_aff_limits[aff_lvl].min,
				  psci_aff_limits[aff_lvl].max);
	if (rc >= 0)
		return &psci_aff_map[rc];
	else
		return NULL;
}

/*******************************************************************************
 * Function which initializes the 'aff_map_node' corresponding to an affinity
 * level instance. Each node has a unique mpidr, level and bakery lock. The data
 * field is opaque and holds affinity level specific data e.g. for affinity
 * level 0 it contains the index into arrays that hold the secure/non-secure
 * state for a cpu that's been turned on/off
 ******************************************************************************/
static void psci_init_aff_map_node(unsigned long mpidr,
				   int level,
				   unsigned int idx)
{
	unsigned char state;
	psci_aff_map[idx].mpidr = mpidr;
	psci_aff_map[idx].level = level;
	bakery_lock_init(&psci_aff_map[idx].lock);

	/*
	 * If an affinity instance is present then mark it as OFF to begin with.
	 */
	state = plat_get_aff_state(level, mpidr);
	psci_aff_map[idx].state = state;
	if (state & PSCI_AFF_PRESENT) {
		psci_set_state(psci_aff_map[idx].state, PSCI_STATE_OFF);
	}

	if (level == MPIDR_AFFLVL0) {
		/* Ensure that we have not overflowed the psci_ns_einfo array */
		assert(psci_ns_einfo_idx < PSCI_NUM_AFFS);

		psci_aff_map[idx].data = psci_ns_einfo_idx;
		psci_ns_einfo_idx++;
	}

	return;
}

/*******************************************************************************
 * Core routine used by the Breadth-First-Search algorithm to populate the
 * affinity tree. Each level in the tree corresponds to an affinity level. This
 * routine's aim is to traverse to the target affinity level and populate nodes
 * in the 'psci_aff_map' for all the siblings at that level. It uses the current
 * affinity level to keep track of how many levels from the root of the tree
 * have been traversed. If the current affinity level != target affinity level,
 * then the platform is asked to return the number of children that each
 * affinity instance has at the current affinity level. Traversal is then done
 * for each child at the next lower level i.e. current affinity level - 1.
 *
 * CAUTION: This routine assumes that affinity instance ids are allocated in a
 * monotonically increasing manner at each affinity level in a mpidr starting
 * from 0. If the platform breaks this assumption then this code will have to
 * be reworked accordingly.
 ******************************************************************************/
static unsigned int psci_init_aff_map(unsigned long mpidr,
				      unsigned int affmap_idx,
				      int cur_afflvl,
				      int tgt_afflvl)
{
	unsigned int ctr, aff_count;

	assert(cur_afflvl >= tgt_afflvl);

	/*
	 * Find the number of siblings at the current affinity level &
	 * assert if there are none 'cause then we have been invoked with
	 * an invalid mpidr.
	 */
	aff_count = plat_get_aff_count(cur_afflvl, mpidr);
	assert(aff_count);

	if (tgt_afflvl < cur_afflvl) {
		for (ctr = 0; ctr < aff_count; ctr++) {
			mpidr = mpidr_set_aff_inst(mpidr, ctr, cur_afflvl);
			affmap_idx = psci_init_aff_map(mpidr,
						       affmap_idx,
						       cur_afflvl - 1,
						       tgt_afflvl);
		}
	} else {
		for (ctr = 0; ctr < aff_count; ctr++, affmap_idx++) {
			mpidr = mpidr_set_aff_inst(mpidr, ctr, cur_afflvl);
			psci_init_aff_map_node(mpidr, cur_afflvl, affmap_idx);
		}

		/* affmap_idx is 1 greater than the max index of cur_afflvl */
		psci_aff_limits[cur_afflvl].max = affmap_idx - 1;
	}

	return affmap_idx;
}

/*******************************************************************************
 * This function initializes the topology tree by querying the platform. To do
 * so, it's helper routines implement a Breadth-First-Search. At each affinity
 * level the platform conveys the number of affinity instances that exist i.e.
 * the affinity count. The algorithm populates the psci_aff_map recursively
 * using this information. On a platform that implements two clusters of 4 cpus
 * each, the populated aff_map_array would look like this:
 *
 *            <- cpus cluster0 -><- cpus cluster1 ->
 * ---------------------------------------------------
 * | 0  | 1  | 0  | 1  | 2  | 3  | 0  | 1  | 2  | 3  |
 * ---------------------------------------------------
 *           ^                                       ^
 * cluster __|                                 cpu __|
 * limit                                      limit
 *
 * The first 2 entries are of the cluster nodes. The next 4 entries are of cpus
 * within cluster 0. The last 4 entries are of cpus within cluster 1.
 * The 'psci_aff_limits' array contains the max & min index of each affinity
 * level within the 'psci_aff_map' array. This allows restricting search of a
 * node at an affinity level between the indices in the limits array.
 ******************************************************************************/
void psci_setup(unsigned long mpidr)
{
	int afflvl, affmap_idx, rc, max_afflvl;
	aff_map_node *node;

	/* Initialize psci's internal state */
	memset(psci_aff_map, 0, sizeof(psci_aff_map));
	memset(psci_aff_limits, 0, sizeof(psci_aff_limits));
	memset(psci_ns_entry_info, 0, sizeof(psci_ns_entry_info));
	psci_ns_einfo_idx = 0;
	psci_plat_pm_ops = NULL;

	/* Find out the maximum affinity level that the platform implements */
	max_afflvl = get_max_afflvl();
	assert(max_afflvl <= MPIDR_MAX_AFFLVL);

	/*
	 * This call traverses the topology tree with help from the platform and
	 * populates the affinity map using a breadth-first-search recursively.
	 * We assume that the platform allocates affinity instance ids from 0
	 * onwards at each affinity level in the mpidr. FIRST_MPIDR = 0.0.0.0
	 */
	affmap_idx = 0;
	for (afflvl = max_afflvl; afflvl >= MPIDR_AFFLVL0; afflvl--) {
		affmap_idx = psci_init_aff_map(FIRST_MPIDR,
					       affmap_idx,
					       max_afflvl,
					       afflvl);
	}

	/*
	 * Set the bounds for the affinity counts of each level in the map. Also
	 * flush out the entire array so that it's visible to subsequent power
	 * management operations. The 'psci_aff_map' array is allocated in
	 * coherent memory so does not need flushing. The 'psci_aff_limits'
	 * array is allocated in normal memory. It will be accessed when the mmu
	 * is off e.g. after reset. Hence it needs to be flushed.
	 */
	for (afflvl = MPIDR_AFFLVL0; afflvl < max_afflvl; afflvl++) {
		psci_aff_limits[afflvl].min =
			psci_aff_limits[afflvl + 1].max + 1;
	}

	flush_dcache_range((unsigned long) psci_aff_limits,
			   sizeof(psci_aff_limits));

	/*
	 * Mark the affinity instances in our mpidr as ON. No need to lock as
	 * this is the primary cpu.
	 */
	mpidr &= MPIDR_AFFINITY_MASK;
	for (afflvl = max_afflvl; afflvl >= MPIDR_AFFLVL0; afflvl--) {

		node = psci_get_aff_map_node(mpidr, afflvl);
		assert(node);

		/* Mark each present node as ON. */
		if (node->state & PSCI_AFF_PRESENT) {
			psci_set_state(node->state, PSCI_STATE_ON);
		}
	}

	rc = platform_setup_pm(&psci_plat_pm_ops);
	assert(rc == 0);
	assert(psci_plat_pm_ops);

	return;
}
