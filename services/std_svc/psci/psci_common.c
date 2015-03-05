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
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <platform.h>
#include <string.h>
#include "psci_private.h"

/*
 * SPD power management operations, expected to be supplied by the registered
 * SPD on successful SP initialization
 */
const spd_pm_ops_t *psci_spd_pm;

/*******************************************************************************
 * Grand array that holds the platform's topology information for state
 * management of affinity instances. Each node (aff_map_node) in the array
 * corresponds to an affinity instance e.g. cluster, cpu within an mpidr
 ******************************************************************************/
aff_map_node_t psci_aff_map[PSCI_NUM_AFFS]
#if USE_COHERENT_MEM
__attribute__ ((section("tzfw_coherent_mem")))
#endif
;

/*******************************************************************************
 * Pointer to functions exported by the platform to complete power mgmt. ops
 ******************************************************************************/
const plat_pm_ops_t *psci_plat_pm_ops;

/*******************************************************************************
 * Check that the maximum affinity level supported by the platform makes sense
 * ****************************************************************************/
CASSERT(PLATFORM_MAX_AFFLVL <= MPIDR_MAX_AFFLVL && \
		PLATFORM_MAX_AFFLVL >= MPIDR_AFFLVL0, \
		assert_platform_max_afflvl_check);

/*******************************************************************************
 * This function is passed an array of pointers to affinity level nodes in the
 * topology tree for an mpidr. It iterates through the nodes to find the highest
 * affinity level which is marked as physically powered off.
 ******************************************************************************/
uint32_t psci_find_max_phys_off_afflvl(uint32_t start_afflvl,
				       uint32_t end_afflvl,
				       aff_map_node_t *mpidr_nodes[])
{
	uint32_t max_afflvl = PSCI_INVALID_DATA;

	for (; start_afflvl <= end_afflvl; start_afflvl++) {
		if (mpidr_nodes[start_afflvl] == NULL)
			continue;

		if (psci_get_phys_state(mpidr_nodes[start_afflvl]) ==
		    PSCI_STATE_OFF)
			max_afflvl = start_afflvl;
	}

	return max_afflvl;
}

/*******************************************************************************
 * This function saves the highest affinity level which is in OFF state. The
 * affinity instance with which the level is associated is determined by the
 * caller.
 ******************************************************************************/
void psci_set_max_phys_off_afflvl(uint32_t afflvl)
{
	set_cpu_data(psci_svc_cpu_data.max_phys_off_afflvl, afflvl);

	/*
	 * Ensure that the saved value is flushed to main memory and any
	 * speculatively pre-fetched stale copies are invalidated from the
	 * caches of other cpus in the same coherency domain. This ensures that
	 * the value can be safely read irrespective of the state of the data
	 * cache.
	 */
	flush_cpu_data(psci_svc_cpu_data.max_phys_off_afflvl);
}

/*******************************************************************************
 * This function reads the saved highest affinity level which is in OFF
 * state. The affinity instance with which the level is associated is determined
 * by the caller.
 ******************************************************************************/
uint32_t psci_get_max_phys_off_afflvl(void)
{
	/*
	 * Ensure that the last update of this value in this cpu's cache is
	 * flushed to main memory and any speculatively pre-fetched stale copies
	 * are invalidated from the caches of other cpus in the same coherency
	 * domain. This ensures that the value is always read from the main
	 * memory when it was written before the data cache was enabled.
	 */
	flush_cpu_data(psci_svc_cpu_data.max_phys_off_afflvl);
	return get_cpu_data(psci_svc_cpu_data.max_phys_off_afflvl);
}

/*******************************************************************************
 * Routine to return the maximum affinity level to traverse to after a cpu has
 * been physically powered up. It is expected to be called immediately after
 * reset from assembler code.
 ******************************************************************************/
int get_power_on_target_afflvl(void)
{
	int afflvl;

#if DEBUG
	unsigned int state;
	aff_map_node_t *node;

	/* Retrieve our node from the topology tree */
	node = psci_get_aff_map_node(read_mpidr_el1() & MPIDR_AFFINITY_MASK,
				     MPIDR_AFFLVL0);
	assert(node);

	/*
	 * Sanity check the state of the cpu. It should be either suspend or "on
	 * pending"
	 */
	state = psci_get_state(node);
	assert(state == PSCI_STATE_SUSPEND || state == PSCI_STATE_ON_PENDING);
#endif

	/*
	 * Assume that this cpu was suspended and retrieve its target affinity
	 * level. If it is invalid then it could only have been turned off
	 * earlier. PLATFORM_MAX_AFFLVL will be the highest affinity level a
	 * cpu can be turned off to.
	 */
	afflvl = psci_get_suspend_afflvl();
	if (afflvl == PSCI_INVALID_DATA)
		afflvl = PLATFORM_MAX_AFFLVL;
	return afflvl;
}

/*******************************************************************************
 * Simple routine to set the id of an affinity instance at a given level in the
 * mpidr.
 ******************************************************************************/
unsigned long mpidr_set_aff_inst(unsigned long mpidr,
				 unsigned char aff_inst,
				 int aff_lvl)
{
	unsigned long aff_shift;

	assert(aff_lvl <= MPIDR_AFFLVL3);

	/*
	 * Decide the number of bits to shift by depending upon
	 * the affinity level
	 */
	aff_shift = get_afflvl_shift(aff_lvl);

	/* Clear the existing affinity instance & set the new one*/
	mpidr &= ~(MPIDR_AFFLVL_MASK << aff_shift);
	mpidr |= aff_inst << aff_shift;

	return mpidr;
}

/*******************************************************************************
 * This function sanity checks a range of affinity levels.
 ******************************************************************************/
int psci_check_afflvl_range(int start_afflvl, int end_afflvl)
{
	/* Sanity check the parameters passed */
	if (end_afflvl > PLATFORM_MAX_AFFLVL)
		return PSCI_E_INVALID_PARAMS;

	if (start_afflvl < MPIDR_AFFLVL0)
		return PSCI_E_INVALID_PARAMS;

	if (end_afflvl < start_afflvl)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * This function is passed an array of pointers to affinity level nodes in the
 * topology tree for an mpidr and the state which each node should transition
 * to. It updates the state of each node between the specified affinity levels.
 ******************************************************************************/
void psci_do_afflvl_state_mgmt(uint32_t start_afflvl,
			       uint32_t end_afflvl,
			       aff_map_node_t *mpidr_nodes[],
			       uint32_t state)
{
	uint32_t level;

	for (level = start_afflvl; level <= end_afflvl; level++) {
		if (mpidr_nodes[level] == NULL)
			continue;
		psci_set_state(mpidr_nodes[level], state);
	}
}

/*******************************************************************************
 * This function is passed an array of pointers to affinity level nodes in the
 * topology tree for an mpidr. It picks up locks for each affinity level bottom
 * up in the range specified.
 ******************************************************************************/
void psci_acquire_afflvl_locks(int start_afflvl,
			       int end_afflvl,
			       aff_map_node_t *mpidr_nodes[])
{
	int level;

	for (level = start_afflvl; level <= end_afflvl; level++) {
		if (mpidr_nodes[level] == NULL)
			continue;

		psci_lock_get(mpidr_nodes[level]);
	}
}

/*******************************************************************************
 * This function is passed an array of pointers to affinity level nodes in the
 * topology tree for an mpidr. It releases the lock for each affinity level top
 * down in the range specified.
 ******************************************************************************/
void psci_release_afflvl_locks(int start_afflvl,
			       int end_afflvl,
			       aff_map_node_t *mpidr_nodes[])
{
	int level;

	for (level = end_afflvl; level >= start_afflvl; level--) {
		if (mpidr_nodes[level] == NULL)
			continue;

		psci_lock_release(mpidr_nodes[level]);
	}
}

/*******************************************************************************
 * Simple routine to determine whether an affinity instance at a given level
 * in an mpidr exists or not.
 ******************************************************************************/
int psci_validate_mpidr(unsigned long mpidr, int level)
{
	aff_map_node_t *node;

	node = psci_get_aff_map_node(mpidr, level);
	if (node && (node->state & PSCI_AFF_PRESENT))
		return PSCI_E_SUCCESS;
	else
		return PSCI_E_INVALID_PARAMS;
}

/*******************************************************************************
 * This function determines the full entrypoint information for the requested
 * PSCI entrypoint on power on/resume and returns it.
 ******************************************************************************/
int psci_get_ns_ep_info(entry_point_info_t *ep,
		       uint64_t entrypoint, uint64_t context_id)
{
	uint32_t ep_attr, mode, sctlr, daif, ee;
	uint32_t ns_scr_el3 = read_scr_el3();
	uint32_t ns_sctlr_el1 = read_sctlr_el1();

	sctlr = ns_scr_el3 & SCR_HCE_BIT ? read_sctlr_el2() : ns_sctlr_el1;
	ee = 0;

	ep_attr = NON_SECURE | EP_ST_DISABLE;
	if (sctlr & SCTLR_EE_BIT) {
		ep_attr |= EP_EE_BIG;
		ee = 1;
	}
	SET_PARAM_HEAD(ep, PARAM_EP, VERSION_1, ep_attr);

	ep->pc = entrypoint;
	memset(&ep->args, 0, sizeof(ep->args));
	ep->args.arg0 = context_id;

	/*
	 * Figure out whether the cpu enters the non-secure address space
	 * in aarch32 or aarch64
	 */
	if (ns_scr_el3 & SCR_RW_BIT) {

		/*
		 * Check whether a Thumb entry point has been provided for an
		 * aarch64 EL
		 */
		if (entrypoint & 0x1)
			return PSCI_E_INVALID_PARAMS;

		mode = ns_scr_el3 & SCR_HCE_BIT ? MODE_EL2 : MODE_EL1;

		ep->spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	} else {

		mode = ns_scr_el3 & SCR_HCE_BIT ? MODE32_hyp : MODE32_svc;

		/*
		 * TODO: Choose async. exception bits if HYP mode is not
		 * implemented according to the values of SCR.{AW, FW} bits
		 */
		daif = DAIF_ABT_BIT | DAIF_IRQ_BIT | DAIF_FIQ_BIT;

		ep->spsr = SPSR_MODE32(mode, entrypoint & 0x1, ee, daif);
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * This function takes a pointer to an affinity node in the topology tree and
 * returns its state. State of a non-leaf node needs to be calculated.
 ******************************************************************************/
unsigned short psci_get_state(aff_map_node_t *node)
{
#if !USE_COHERENT_MEM
	flush_dcache_range((uint64_t) node, sizeof(*node));
#endif

	assert(node->level >= MPIDR_AFFLVL0 && node->level <= MPIDR_MAX_AFFLVL);

	/* A cpu node just contains the state which can be directly returned */
	if (node->level == MPIDR_AFFLVL0)
		return (node->state >> PSCI_STATE_SHIFT) & PSCI_STATE_MASK;

	/*
	 * For an affinity level higher than a cpu, the state has to be
	 * calculated. It depends upon the value of the reference count
	 * which is managed by each node at the next lower affinity level
	 * e.g. for a cluster, each cpu increments/decrements the reference
	 * count. If the reference count is 0 then the affinity level is
	 * OFF else ON.
	 */
	if (node->ref_count)
		return PSCI_STATE_ON;
	else
		return PSCI_STATE_OFF;
}

/*******************************************************************************
 * This function takes a pointer to an affinity node in the topology tree and
 * a target state. State of a non-leaf node needs to be converted to a reference
 * count. State of a leaf node can be set directly.
 ******************************************************************************/
void psci_set_state(aff_map_node_t *node, unsigned short state)
{
	assert(node->level >= MPIDR_AFFLVL0 && node->level <= MPIDR_MAX_AFFLVL);

	/*
	 * For an affinity level higher than a cpu, the state is used
	 * to decide whether the reference count is incremented or
	 * decremented. Entry into the ON_PENDING state does not have
	 * effect.
	 */
	if (node->level > MPIDR_AFFLVL0) {
		switch (state) {
		case PSCI_STATE_ON:
			node->ref_count++;
			break;
		case PSCI_STATE_OFF:
		case PSCI_STATE_SUSPEND:
			node->ref_count--;
			break;
		case PSCI_STATE_ON_PENDING:
			/*
			 * An affinity level higher than a cpu will not undergo
			 * a state change when it is about to be turned on
			 */
			return;
		default:
			assert(0);
		}
	} else {
		node->state &= ~(PSCI_STATE_MASK << PSCI_STATE_SHIFT);
		node->state |= (state & PSCI_STATE_MASK) << PSCI_STATE_SHIFT;
	}

#if !USE_COHERENT_MEM
	flush_dcache_range((uint64_t) node, sizeof(*node));
#endif
}

/*******************************************************************************
 * An affinity level could be on, on_pending, suspended or off. These are the
 * logical states it can be in. Physically either it is off or on. When it is in
 * the state on_pending then it is about to be turned on. It is not possible to
 * tell whether that's actually happenned or not. So we err on the side of
 * caution & treat the affinity level as being turned off.
 ******************************************************************************/
unsigned short psci_get_phys_state(aff_map_node_t *node)
{
	unsigned int state;

	state = psci_get_state(node);
	return get_phys_state(state);
}

/*******************************************************************************
 * This function takes an array of pointers to affinity instance nodes in the
 * topology tree and calls the physical power on handler for the corresponding
 * affinity levels
 ******************************************************************************/
static void psci_call_power_on_handlers(aff_map_node_t *mpidr_nodes[],
				       int start_afflvl,
				       int end_afflvl,
				       afflvl_power_on_finisher_t *pon_handlers)
{
	int level;
	aff_map_node_t *node;

	for (level = end_afflvl; level >= start_afflvl; level--) {
		node = mpidr_nodes[level];
		if (node == NULL)
			continue;

		/*
		 * If we run into any trouble while powering up an
		 * affinity instance, then there is no recovery path
		 * so simply return an error and let the caller take
		 * care of the situation.
		 */
		pon_handlers[level](node);
	}
}

/*******************************************************************************
 * Generic handler which is called when a cpu is physically powered on. It
 * traverses through all the affinity levels performing generic, architectural,
 * platform setup and state management e.g. for a cluster that's been powered
 * on, it will call the platform specific code which will enable coherency at
 * the interconnect level. For a cpu it could mean turning on the MMU etc.
 *
 * The state of all the relevant affinity levels is changed after calling the
 * affinity level specific handlers as their actions would depend upon the state
 * the affinity level is exiting from.
 *
 * The affinity level specific handlers are called in descending order i.e. from
 * the highest to the lowest affinity level implemented by the platform because
 * to turn on affinity level X it is neccesary to turn on affinity level X + 1
 * first.
 ******************************************************************************/
void psci_afflvl_power_on_finish(int start_afflvl,
				 int end_afflvl,
				 afflvl_power_on_finisher_t *pon_handlers)
{
	mpidr_aff_map_nodes_t mpidr_nodes;
	int rc;
	unsigned int max_phys_off_afflvl;


	/*
	 * Collect the pointers to the nodes in the topology tree for
	 * each affinity instance in the mpidr. If this function does
	 * not return successfully then either the mpidr or the affinity
	 * levels are incorrect. Either case is an irrecoverable error.
	 */
	rc = psci_get_aff_map_nodes(read_mpidr_el1() & MPIDR_AFFINITY_MASK,
				    start_afflvl,
				    end_afflvl,
				    mpidr_nodes);
	if (rc != PSCI_E_SUCCESS)
		panic();

	/*
	 * This function acquires the lock corresponding to each affinity
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_afflvl_locks(start_afflvl,
				  end_afflvl,
				  mpidr_nodes);

	max_phys_off_afflvl = psci_find_max_phys_off_afflvl(start_afflvl,
							    end_afflvl,
							    mpidr_nodes);
	assert(max_phys_off_afflvl != PSCI_INVALID_DATA);

	/*
	 * Stash the highest affinity level that will come out of the OFF or
	 * SUSPEND states.
	 */
	psci_set_max_phys_off_afflvl(max_phys_off_afflvl);

	/* Perform generic, architecture and platform specific handling */
	psci_call_power_on_handlers(mpidr_nodes,
					 start_afflvl,
					 end_afflvl,
					 pon_handlers);

	/*
	 * This function updates the state of each affinity instance
	 * corresponding to the mpidr in the range of affinity levels
	 * specified.
	 */
	psci_do_afflvl_state_mgmt(start_afflvl,
				  end_afflvl,
				  mpidr_nodes,
				  PSCI_STATE_ON);

	/*
	 * Invalidate the entry for the highest affinity level stashed earlier.
	 * This ensures that any reads of this variable outside the power
	 * up/down sequences return PSCI_INVALID_DATA
	 */
	psci_set_max_phys_off_afflvl(PSCI_INVALID_DATA);

	/*
	 * This loop releases the lock corresponding to each affinity level
	 * in the reverse order to which they were acquired.
	 */
	psci_release_afflvl_locks(start_afflvl,
				  end_afflvl,
				  mpidr_nodes);
}

/*******************************************************************************
 * This function initializes the set of hooks that PSCI invokes as part of power
 * management operation. The power management hooks are expected to be provided
 * by the SPD, after it finishes all its initialization
 ******************************************************************************/
void psci_register_spd_pm_hook(const spd_pm_ops_t *pm)
{
	assert(pm);
	psci_spd_pm = pm;

	if (pm->svc_migrate)
		psci_caps |= define_psci_cap(PSCI_MIG_AARCH64);

	if (pm->svc_migrate_info)
		psci_caps |= define_psci_cap(PSCI_MIG_INFO_UP_CPU_AARCH64)
				| define_psci_cap(PSCI_MIG_INFO_TYPE);
}

/*******************************************************************************
 * This function invokes the migrate info hook in the spd_pm_ops. It performs
 * the necessary return value validation. If the Secure Payload is UP and
 * migrate capable, it returns the mpidr of the CPU on which the Secure payload
 * is resident through the mpidr parameter. Else the value of the parameter on
 * return is undefined.
 ******************************************************************************/
int psci_spd_migrate_info(uint64_t *mpidr)
{
	int rc;

	if (!psci_spd_pm || !psci_spd_pm->svc_migrate_info)
		return PSCI_E_NOT_SUPPORTED;

	rc = psci_spd_pm->svc_migrate_info(mpidr);

	assert(rc == PSCI_TOS_UP_MIG_CAP || rc == PSCI_TOS_NOT_UP_MIG_CAP \
		|| rc == PSCI_TOS_NOT_PRESENT_MP || rc == PSCI_E_NOT_SUPPORTED);

	return rc;
}


/*******************************************************************************
 * This function prints the state of all affinity instances present in the
 * system
 ******************************************************************************/
void psci_print_affinity_map(void)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO
	aff_map_node_t *node;
	unsigned int idx;
	/* This array maps to the PSCI_STATE_X definitions in psci.h */
	static const char *psci_state_str[] = {
		"ON",
		"OFF",
		"ON_PENDING",
		"SUSPEND"
	};

	INFO("PSCI Affinity Map:\n");
	for (idx = 0; idx < PSCI_NUM_AFFS ; idx++) {
		node = &psci_aff_map[idx];
		if (!(node->state & PSCI_AFF_PRESENT)) {
			continue;
		}
		INFO("  AffInst: Level %u, MPID 0x%lx, State %s\n",
				node->level, node->mpidr,
				psci_state_str[psci_get_state(node)]);
	}
#endif
}
