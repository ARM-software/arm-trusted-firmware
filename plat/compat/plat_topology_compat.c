/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>

/* The power domain tree descriptor */
static unsigned char power_domain_tree_desc
				[PLATFORM_NUM_AFFS - PLATFORM_CORE_COUNT + 1];

/*******************************************************************************
 * Simple routine to set the id of an affinity instance at a given level
 * in the mpidr. The assumption is that the affinity level and the power
 * domain level are the same.
 ******************************************************************************/
unsigned long mpidr_set_aff_inst(unsigned long mpidr,
				 unsigned char aff_inst,
				 int aff_lvl)
{
	unsigned long aff_shift;

	assert(aff_lvl <= MPIDR_AFFLVL3);

	/*
	 * Decide the number of bits to shift by depending upon
	 * the power level
	 */
	aff_shift = get_afflvl_shift(aff_lvl);

	/* Clear the existing power instance & set the new one*/
	mpidr &= ~((unsigned long)MPIDR_AFFLVL_MASK << aff_shift);
	mpidr |= (unsigned long)aff_inst << aff_shift;

	return mpidr;
}

/******************************************************************************
 * This function uses insertion sort to sort a given list of mpidr's in the
 * ascending order of the index returned by platform_get_core_pos.
 *****************************************************************************/
void sort_mpidr_by_cpu_idx(unsigned int aff_count, unsigned long mpidr_list[])
{
	int i, j;
	unsigned long temp_mpidr;

	for (i = 1; i < aff_count; i++) {
		temp_mpidr = mpidr_list[i];

		for (j = i;
			j > 0 &&
			platform_get_core_pos(mpidr_list[j-1]) >
			platform_get_core_pos(temp_mpidr);
			j--)
			mpidr_list[j] = mpidr_list[j-1];

		mpidr_list[j] = temp_mpidr;
	}
}

/*******************************************************************************
 * The compatibility routine to construct the power domain tree description.
 * The assumption made is that the power domains correspond to affinity
 * instances on the platform. This routine's aim is to traverse to the target
 * affinity level and populate the number of siblings at that level in
 * 'power_domain_tree_desc' array. It uses the current affinity level to keep
 * track of how many levels from the root of the tree have been traversed.
 * If the current affinity level != target affinity level, then the platform
 * is asked to return the number of children that each affinity instance has
 * at the current affinity level. Traversal is then done for each child at the
 * next lower level i.e. current affinity level - 1.
 *
 * The power domain description needs to be constructed in such a way that
 * affinity instances containing CPUs with lower cpu indices need to be
 * described first.  Hence when traversing the power domain levels, the list
 * of mpidrs at that power domain level is sorted in the ascending order of CPU
 * indices before the lower levels are recursively described.
 *
 * CAUTION: This routine assumes that affinity instance ids are allocated in a
 * monotonically increasing manner at each affinity level in a mpidr starting
 * from 0. If the platform breaks this assumption then this code will have to
 * be reworked accordingly.
 ******************************************************************************/
static unsigned int init_pwr_domain_tree_desc(unsigned long mpidr,
					unsigned int affmap_idx,
					unsigned int cur_afflvl,
					unsigned int tgt_afflvl)
{
	unsigned int ctr, aff_count;

	/*
	 * Temporary list to hold the MPIDR list at a particular power domain
	 * level so as to sort them.
	 */
	unsigned long mpidr_list[PLATFORM_CORE_COUNT];

	assert(cur_afflvl >= tgt_afflvl);

	/*
	 * Find the number of siblings at the current power level &
	 * assert if there are none 'cause then we have been invoked with
	 * an invalid mpidr.
	 */
	aff_count = plat_get_aff_count(cur_afflvl, mpidr);
	assert(aff_count);

	if (tgt_afflvl < cur_afflvl) {
		for (ctr = 0; ctr < aff_count; ctr++) {
			mpidr_list[ctr] = mpidr_set_aff_inst(mpidr, ctr,
						cur_afflvl);
		}

		/* Need to sort mpidr list according to CPU index */
		sort_mpidr_by_cpu_idx(aff_count, mpidr_list);
		for (ctr = 0; ctr < aff_count; ctr++) {
			affmap_idx = init_pwr_domain_tree_desc(mpidr_list[ctr],
						       affmap_idx,
						       cur_afflvl - 1,
						       tgt_afflvl);
		}
	} else {
		power_domain_tree_desc[affmap_idx++] = aff_count;
	}
	return affmap_idx;
}


/*******************************************************************************
 * This function constructs the topology tree description at runtime
 * and returns it. The assumption made is that the power domains correspond
 * to affinity instances on the platform.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	int afflvl;
	unsigned int affmap_idx;

	/*
	 * We assume that the platform allocates affinity instance ids from
	 * 0 onwards at each affinity level in the mpidr. FIRST_MPIDR = 0.0.0.0
	 */
	affmap_idx = 0;
	for (afflvl = (int) PLATFORM_MAX_AFFLVL;
			afflvl >= (int) MPIDR_AFFLVL0; afflvl--) {
		affmap_idx = init_pwr_domain_tree_desc(FIRST_MPIDR,
					       affmap_idx,
					       PLATFORM_MAX_AFFLVL,
					       (unsigned int) afflvl);
	}

	assert(affmap_idx == (PLATFORM_NUM_AFFS - PLATFORM_CORE_COUNT + 1));

	return power_domain_tree_desc;
}

/******************************************************************************
 * The compatibility helper function for plat_core_pos_by_mpidr(). It
 * validates the 'mpidr' by making sure that it is within acceptable bounds
 * for the platform and queries the platform layer whether the CPU specified
 * by the mpidr is present or not. If present, it returns the index of the
 * core corresponding to the 'mpidr'. Else it returns -1.
 *****************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned long shift, aff_inst;
	int i;

	/* Ignore the Reserved bits and U bit in MPIDR */
	mpidr &= MPIDR_AFFINITY_MASK;

	/*
	 * Check if any affinity field higher than
	 * the PLATFORM_MAX_AFFLVL is set.
	 */
	shift = get_afflvl_shift(PLATFORM_MAX_AFFLVL + 1);
	if (mpidr >> shift)
		return -1;

	for (i = PLATFORM_MAX_AFFLVL; i >= 0; i--) {
		shift = get_afflvl_shift(i);
		aff_inst = ((mpidr &
			((unsigned long)MPIDR_AFFLVL_MASK << shift)) >> shift);
		if (aff_inst >= plat_get_aff_count(i, mpidr))
			return -1;
	}

	if (plat_get_aff_state(0, mpidr) == PSCI_AFF_ABSENT)
		return -1;

	return platform_get_core_pos(mpidr);
}
