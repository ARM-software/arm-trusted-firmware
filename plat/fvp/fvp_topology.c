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
#include <platform_def.h>
/* TODO: Reusing psci error codes & state information. Get our own! */
#include <psci.h>
#include "drivers/pwrc/fvp_pwrc.h"

/* We treat '255' as an invalid affinity instance */
#define AFFINST_INVAL	0xff

/*******************************************************************************
 * We support 3 flavours of the FVP: Foundation, Base AEM & Base Cortex. Each
 * flavour has a different topology. The common bit is that there can be a max.
 * of 2 clusters (affinity 1) and 4 cpus (affinity 0) per cluster. So we define
 * a tree like data structure which caters to these maximum bounds. It simply
 * marks the absent affinity level instances as PSCI_AFF_ABSENT e.g. there is no
 * cluster 1 on the Foundation FVP. The 'data' field is currently unused.
 ******************************************************************************/
typedef struct affinity_info {
	unsigned char sibling;
	unsigned char child;
	unsigned char state;
	unsigned int data;
} affinity_info_t;

/*******************************************************************************
 * The following two data structures store the topology tree for the fvp. There
 * is a separate array for each affinity level i.e. cpus and clusters. The child
 * and sibling references allow traversal inside and in between the two arrays.
 ******************************************************************************/
static affinity_info_t fvp_aff1_topology_map[PLATFORM_CLUSTER_COUNT];
static affinity_info_t fvp_aff0_topology_map[PLATFORM_CORE_COUNT];

/* Simple global variable to safeguard us from stupidity */
static unsigned int topology_setup_done;

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform to allow the former to detect the platform
 * topology. psci queries the platform to determine how many affinity instances
 * are present at a particular level for a given mpidr e.g. consider a dual
 * cluster platform where each cluster has 4 cpus. A call to this function with
 * (0, 0x100) will return the number of cpus implemented under cluster 1 i.e. 4.
 * Similarly a call with (1, 0x100) will return 2 i.e. the number of clusters.
 * This is 'cause we are effectively asking how many affinity level 1 instances
 * are implemented under affinity level 2 instance 0.
 ******************************************************************************/
unsigned int plat_get_aff_count(unsigned int aff_lvl,
				unsigned long mpidr)
{
	unsigned int aff_count = 1, ctr;
	unsigned char parent_aff_id;

	assert(topology_setup_done == 1);

	switch (aff_lvl) {
	case 3:
	case 2:
		/*
		 * Assert if the parent affinity instance is not 0.
		 * This also takes care of level 3 in an obfuscated way
		 */
		parent_aff_id = (mpidr >> MPIDR_AFF3_SHIFT) & MPIDR_AFFLVL_MASK;
		assert(parent_aff_id == 0);

		/*
		 * Report that we implement a single instance of
		 * affinity levels 2 & 3 which are AFF_ABSENT
		 */
		break;
	case 1:
		/* Assert if the parent affinity instance is not 0. */
		parent_aff_id = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
		assert(parent_aff_id == 0);

		/* Fetch the starting index in the aff1 array */
		for (ctr = 0;
		     fvp_aff1_topology_map[ctr].sibling != AFFINST_INVAL;
		     ctr = fvp_aff1_topology_map[ctr].sibling) {
			aff_count++;
		}

		break;
	case 0:
		/* Assert if the cluster id is anything apart from 0 or 1 */
		parent_aff_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
		assert(parent_aff_id < PLATFORM_CLUSTER_COUNT);

		/* Fetch the starting index in the aff0 array */
		for (ctr = fvp_aff1_topology_map[parent_aff_id].child;
		     fvp_aff0_topology_map[ctr].sibling != AFFINST_INVAL;
		     ctr = fvp_aff0_topology_map[ctr].sibling) {
			aff_count++;
		}

		break;
	default:
		assert(0);
	}

	return aff_count;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform to allow the former to detect the state of a
 * affinity instance in the platform topology. psci queries the platform to
 * determine whether an affinity instance is present or absent. This caters for
 * topologies where an intermediate affinity level instance is missing e.g.
 * consider a platform which implements a single cluster with 4 cpus and there
 * is another cpu sitting directly on the interconnect along with the cluster.
 * The mpidrs of the cluster would range from 0x0-0x3. The mpidr of the single
 * cpu would be 0x100 to highlight that it does not belong to cluster 0. Cluster
 * 1 is however missing but needs to be accounted to reach this single cpu in
 * the topology tree. Hence it will be marked as PSCI_AFF_ABSENT. This is not
 * applicable to the FVP but depicted as an example.
 ******************************************************************************/
unsigned int plat_get_aff_state(unsigned int aff_lvl,
				unsigned long mpidr)
{
	unsigned int aff_state = PSCI_AFF_ABSENT, idx;
	idx = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;

	assert(topology_setup_done == 1);

	switch (aff_lvl) {
	case 3:
	case 2:
		/* Report affinity levels 2 & 3 as absent */
		break;
	case 1:
		aff_state = fvp_aff1_topology_map[idx].state;
		break;
	case 0:
		/*
		 * First get start index of the aff0 in its array & then add
		 * to it the affinity id that we want the state of
		 */
		idx = fvp_aff1_topology_map[idx].child;
		idx += (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;
		aff_state = fvp_aff0_topology_map[idx].state;
		break;
	default:
		assert(0);
	}

	return aff_state;
}

/*******************************************************************************
 * Handy optimization to prevent the psci implementation from traversing through
 * affinity levels which are not present while detecting the platform topology.
 ******************************************************************************/
int plat_get_max_afflvl()
{
	return MPIDR_AFFLVL1;
}

/*******************************************************************************
 * This function populates the FVP specific topology information depending upon
 * the FVP flavour its running on. We construct all the mpidrs we can handle
 * and rely on the PWRC.PSYSR to flag absent cpus when their status is queried.
 ******************************************************************************/
int fvp_setup_topology()
{
	unsigned char aff0, aff1, aff_state, aff0_offset = 0;
	unsigned long mpidr;

	topology_setup_done = 0;

	for (aff1 = 0; aff1 < PLATFORM_CLUSTER_COUNT; aff1++) {

		fvp_aff1_topology_map[aff1].child = aff0_offset;
		fvp_aff1_topology_map[aff1].sibling = aff1 + 1;

		for (aff0 = 0; aff0 < PLATFORM_MAX_CPUS_PER_CLUSTER; aff0++) {

			mpidr = aff1 << MPIDR_AFF1_SHIFT;
			mpidr |= aff0 << MPIDR_AFF0_SHIFT;

			if (fvp_pwrc_read_psysr(mpidr) != PSYSR_INVALID) {
				/*
				 * Presence of even a single aff0 indicates
				 * presence of parent aff1 on the FVP.
				 */
				aff_state = PSCI_AFF_PRESENT;
				fvp_aff1_topology_map[aff1].state =
					PSCI_AFF_PRESENT;
			} else {
				aff_state = PSCI_AFF_ABSENT;
			}

			fvp_aff0_topology_map[aff0_offset].child = AFFINST_INVAL;
			fvp_aff0_topology_map[aff0_offset].state = aff_state;
			fvp_aff0_topology_map[aff0_offset].sibling =
				aff0_offset + 1;

			/* Increment the absolute number of aff0s traversed */
			aff0_offset++;
		}

		/* Tie-off the last aff0 sibling to -1 to avoid overflow */
		fvp_aff0_topology_map[aff0_offset - 1].sibling = AFFINST_INVAL;
	}

	/* Tie-off the last aff1 sibling to AFFINST_INVAL to avoid overflow */
	fvp_aff1_topology_map[aff1 - 1].sibling = AFFINST_INVAL;

	topology_setup_done = 1;
	return 0;
}
