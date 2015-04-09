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
#include <context.h>
#include <context_mgmt.h>
#include <platform.h>
#include <stddef.h>
#include "psci_private.h"

/*******************************************************************************
 * Per cpu non-secure contexts used to program the architectural state prior
 * return to the normal world.
 * TODO: Use the memory allocator to set aside memory for the contexts instead
 * of relying on platform defined constants. Using PSCI_NUM_PWR_DOMAINS will be
 * an overkill.
 ******************************************************************************/
static cpu_context_t psci_ns_context[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * In a system, a certain number of power domain instances are present at a
 * power level. The cumulative number of instances across all levels are
 * stored in 'psci_pwr_domain_map'. The topology tree has been flattenned into
 * this array. To retrieve nodes, information about the extents of each power
 * level i.e. start index and end index needs to be present.
 * 'psci_pwr_lvl_limits' stores this information.
 ******************************************************************************/
pwr_lvl_limits_node_t psci_pwr_lvl_limits[MPIDR_MAX_AFFLVL + 1];

/******************************************************************************
 * Define the psci capability variable.
 *****************************************************************************/
uint32_t psci_caps;


/*******************************************************************************
 * Routines for retrieving the node corresponding to a power domain instance
 * in the mpidr. The first one uses binary search to find the node corresponding
 * to the mpidr (key) at a particular power level. The second routine decides
 * extents of the binary search at each power level.
 ******************************************************************************/
static int psci_pwr_domain_map_get_idx(unsigned long key,
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
	 * Make sure we are within array limits.
	 */
	assert(min_idx >= 0 && max_idx < PSCI_NUM_PWR_DOMAINS);

	/*
	 * Bisect the array around 'mid' and then recurse into the array chunk
	 * where the key is likely to be found. The mpidrs in each node in the
	 * 'psci_pwr_domain_map' for a given power level are stored in an
	 * ascending order which makes the binary search possible.
	 */
	mid = min_idx + ((max_idx - min_idx) >> 1);	/* Divide by 2 */

	if (psci_pwr_domain_map[mid].mpidr > key)
		return psci_pwr_domain_map_get_idx(key, min_idx, mid - 1);
	else if (psci_pwr_domain_map[mid].mpidr < key)
		return psci_pwr_domain_map_get_idx(key, mid + 1, max_idx);
	else
		return mid;
}

pwr_map_node_t *psci_get_pwr_map_node(unsigned long mpidr, int pwr_lvl)
{
	int rc;

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return NULL;

	/* Right shift the mpidr to the required power level */
	mpidr = mpidr_mask_lower_afflvls(mpidr, pwr_lvl);

	rc = psci_pwr_domain_map_get_idx(mpidr,
				  psci_pwr_lvl_limits[pwr_lvl].min,
				  psci_pwr_lvl_limits[pwr_lvl].max);
	if (rc >= 0)
		return &psci_pwr_domain_map[rc];
	else
		return NULL;
}

/*******************************************************************************
 * This function populates an array with nodes corresponding to a given range of
 * power levels in an mpidr. It returns successfully only when the power
 * levels are correct, the mpidr is valid i.e. no power level is absent from
 * the topology tree & the power domain instance at level 0 is not absent.
 ******************************************************************************/
int psci_get_pwr_map_nodes(unsigned long mpidr,
			   int start_pwrlvl,
			   int end_pwrlvl,
			   pwr_map_node_t *mpidr_nodes[])
{
	int rc = PSCI_E_INVALID_PARAMS, level;
	pwr_map_node_t *node;

	rc = psci_check_pwrlvl_range(start_pwrlvl, end_pwrlvl);
	if (rc != PSCI_E_SUCCESS)
		return rc;

	for (level = start_pwrlvl; level <= end_pwrlvl; level++) {

		/*
		 * Grab the node for each power level. No power level
		 * can be missing as that would mean that the topology tree
		 * is corrupted.
		 */
		node = psci_get_pwr_map_node(mpidr, level);
		if (node == NULL) {
			rc = PSCI_E_INVALID_PARAMS;
			break;
		}

		/*
		 * Skip absent power levels unless it's power level 0.
		 * An absent cpu means that the mpidr is invalid. Save the
		 * pointer to the node for the present power level
		 */
		if (!(node->state & PSCI_PWR_DOMAIN_PRESENT)) {
			if (level == MPIDR_AFFLVL0) {
				rc = PSCI_E_INVALID_PARAMS;
				break;
			}

			mpidr_nodes[level] = NULL;
		} else
			mpidr_nodes[level] = node;
	}

	return rc;
}

/*******************************************************************************
 * Function which initializes the 'pwr_map_node' corresponding to a power
 * domain instance. Each node has a unique mpidr, level and bakery lock.
 ******************************************************************************/
static void psci_init_pwr_map_node(unsigned long mpidr,
				   int level,
				   unsigned int idx)
{
	unsigned char state;
	uint32_t linear_id;
	psci_pwr_domain_map[idx].mpidr = mpidr;
	psci_pwr_domain_map[idx].level = level;
	psci_lock_init(psci_pwr_domain_map, idx);

	/*
	 * If an power domain instance is present then mark it as OFF
	 * to begin with.
	 */
	state = plat_get_pwr_domain_state(level, mpidr);
	psci_pwr_domain_map[idx].state = state;

	/*
	 * Check if this is a CPU node and is present in which case certain
	 * other initialisations are required.
	 */
	if (level != MPIDR_AFFLVL0)
		return;

	if (!(state & PSCI_PWR_DOMAIN_PRESENT))
		return;

	/*
	 * Mark the cpu as OFF. Higher power level reference counts
	 * have already been memset to 0
	 */
	psci_set_state(&psci_pwr_domain_map[idx], PSCI_STATE_OFF);

	/*
	 * Associate a non-secure context with this power
	 * instance through the context management library.
	 */
	linear_id = plat_core_pos_by_mpidr(mpidr);
	assert(linear_id < PLATFORM_CORE_COUNT);

	/* Invalidate the suspend context for the node */
	set_cpu_data_by_index(linear_id,
			      psci_svc_cpu_data.power_state,
			      PSCI_INVALID_DATA);

	flush_cpu_data_by_index(linear_id, psci_svc_cpu_data);

	cm_set_context_by_index(linear_id,
				(void *) &psci_ns_context[linear_id],
				NON_SECURE);
}

/*******************************************************************************
 * Core routine used by the Breadth-First-Search algorithm to populate the
 * power domain tree. Each level in the tree corresponds to a power level. This
 * routine's aim is to traverse to the target power level and populate nodes
 * in the 'psci_pwr_domain_map' for all the siblings at that level. It uses the
 * current power level to keep track of how many levels from the root of the
 * tree have been traversed. If the current power level != target power level,
 * then the platform is asked to return the number of children that each
 * power domain instance has at the current power level. Traversal is then done
 * for each child at the next lower level i.e. current power level - 1.
 *
 * CAUTION: This routine assumes that power domain instance ids are allocated
 * in a monotonically increasing manner at each power level in a mpidr starting
 * from 0. If the platform breaks this assumption then this code will have to
 * be reworked accordingly.
 ******************************************************************************/
static unsigned int psci_init_pwr_map(unsigned long mpidr,
				      unsigned int pwrmap_idx,
				      int cur_pwrlvl,
				      int tgt_pwrlvl)
{
	unsigned int ctr, pwr_inst_count;

	assert(cur_pwrlvl >= tgt_pwrlvl);

	/*
	 * Find the number of siblings at the current power level &
	 * assert if there are none 'cause then we have been invoked with
	 * an invalid mpidr.
	 */
	pwr_inst_count = plat_get_pwr_domain_count(cur_pwrlvl, mpidr);
	assert(pwr_inst_count);

	if (tgt_pwrlvl < cur_pwrlvl) {
		for (ctr = 0; ctr < pwr_inst_count; ctr++) {
			mpidr = mpidr_set_pwr_domain_inst(mpidr, ctr,
								cur_pwrlvl);
			pwrmap_idx = psci_init_pwr_map(mpidr,
						       pwrmap_idx,
						       cur_pwrlvl - 1,
						       tgt_pwrlvl);
		}
	} else {
		for (ctr = 0; ctr < pwr_inst_count; ctr++, pwrmap_idx++) {
			mpidr = mpidr_set_pwr_domain_inst(mpidr, ctr,
								cur_pwrlvl);
			psci_init_pwr_map_node(mpidr, cur_pwrlvl, pwrmap_idx);
		}

		/* pwrmap_idx is 1 greater than the max index of cur_pwrlvl */
		psci_pwr_lvl_limits[cur_pwrlvl].max = pwrmap_idx - 1;
	}

	return pwrmap_idx;
}

/*******************************************************************************
 * This function initializes the topology tree by querying the platform. To do
 * so, it's helper routines implement a Breadth-First-Search. At each power
 * level the platform conveys the number of power domain instances that exist
 * i.e. the power instance count. The algorithm populates the
 * psci_pwr_domain_map* recursively using this information. On a platform that
 * implements two clusters of 4 cpus each, the populated pwr_map_array would
 * look like this:
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
 * The 'psci_pwr_lvl_limits' array contains the max & min index of each power
 * level within the 'psci_pwr_domain_map' array. This allows restricting search
 * of a node at a power level between the indices in the limits array.
 ******************************************************************************/
int32_t psci_setup(void)
{
	unsigned long mpidr = read_mpidr();
	int pwrlvl, pwrmap_idx, max_pwrlvl;
	pwr_map_node_t *node;

	psci_plat_pm_ops = NULL;

	/* Find out the maximum power level that the platform implements */
	max_pwrlvl = PLAT_MAX_PWR_LVL;
	assert(max_pwrlvl <= MPIDR_MAX_AFFLVL);

	/*
	 * This call traverses the topology tree with help from the platform and
	 * populates the power map using a breadth-first-search recursively.
	 * We assume that the platform allocates power domain instance ids from
	 * 0 onwards at each power level in the mpidr. FIRST_MPIDR = 0.0.0.0
	 */
	pwrmap_idx = 0;
	for (pwrlvl = max_pwrlvl; pwrlvl >= MPIDR_AFFLVL0; pwrlvl--) {
		pwrmap_idx = psci_init_pwr_map(FIRST_MPIDR,
					       pwrmap_idx,
					       max_pwrlvl,
					       pwrlvl);
	}

#if !USE_COHERENT_MEM
	/*
	 * The psci_pwr_domain_map only needs flushing when it's not allocated
	 * in coherent memory.
	 */
	flush_dcache_range((uint64_t) &psci_pwr_domain_map,
					sizeof(psci_pwr_domain_map));
#endif

	/*
	 * Set the bounds for number of instances of each level in the map. Also
	 * flush out the entire array so that it's visible to subsequent power
	 * management operations. The 'psci_pwr_lvl_limits' array is allocated
	 * in normal memory. It will be accessed when the mmu is off e.g. after
	 * reset. Hence it needs to be flushed.
	 */
	for (pwrlvl = MPIDR_AFFLVL0; pwrlvl < max_pwrlvl; pwrlvl++) {
		psci_pwr_lvl_limits[pwrlvl].min =
			psci_pwr_lvl_limits[pwrlvl + 1].max + 1;
	}

	flush_dcache_range((unsigned long) psci_pwr_lvl_limits,
			   sizeof(psci_pwr_lvl_limits));

	/*
	 * Mark the power domain instances in our mpidr as ON. No need to lock
	 * as this is the primary cpu.
	 */
	mpidr &= MPIDR_AFFINITY_MASK;
	for (pwrlvl = MPIDR_AFFLVL0; pwrlvl <= max_pwrlvl; pwrlvl++) {

		node = psci_get_pwr_map_node(mpidr, pwrlvl);
		assert(node);

		/* Mark each present node as ON. */
		if (node->state & PSCI_PWR_DOMAIN_PRESENT)
			psci_set_state(node, PSCI_STATE_ON);
	}

	platform_setup_pm(&psci_plat_pm_ops);
	assert(psci_plat_pm_ops);

	/* Initialize the psci capability */
	psci_caps = PSCI_GENERIC_CAP;

	if (psci_plat_pm_ops->pwr_domain_off)
		psci_caps |=  define_psci_cap(PSCI_CPU_OFF);
	if (psci_plat_pm_ops->pwr_domain_on &&
			psci_plat_pm_ops->pwr_domain_on_finish)
		psci_caps |=  define_psci_cap(PSCI_CPU_ON_AARCH64);
	if (psci_plat_pm_ops->pwr_domain_suspend &&
			psci_plat_pm_ops->pwr_domain_suspend_finish) {
		psci_caps |=  define_psci_cap(PSCI_CPU_SUSPEND_AARCH64);
		if (psci_plat_pm_ops->get_sys_suspend_power_state)
			psci_caps |=  define_psci_cap(PSCI_SYSTEM_SUSPEND_AARCH64);
	}
	if (psci_plat_pm_ops->system_off)
		psci_caps |=  define_psci_cap(PSCI_SYSTEM_OFF);
	if (psci_plat_pm_ops->system_reset)
		psci_caps |=  define_psci_cap(PSCI_SYSTEM_RESET);

	return 0;
}
