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
 * management of power domain instances. Each node (pwr_map_node) in the array
 * corresponds to a power domain instance e.g. cluster, cpu within an mpidr
 ******************************************************************************/
pwr_map_node_t psci_pwr_domain_map[PSCI_NUM_PWR_DOMAINS]
#if USE_COHERENT_MEM
__attribute__ ((section("tzfw_coherent_mem")))
#endif
;

/*******************************************************************************
 * Pointer to functions exported by the platform to complete power mgmt. ops
 ******************************************************************************/
const plat_pm_ops_t *psci_plat_pm_ops;

/*******************************************************************************
 * Check that the maximum power level supported by the platform makes sense
 * ****************************************************************************/
CASSERT(PLAT_MAX_PWR_LVL <= MPIDR_MAX_AFFLVL && \
		PLAT_MAX_PWR_LVL >= MPIDR_AFFLVL0, \
		assert_platform_max_pwrlvl_check);

/*******************************************************************************
 * This function is passed an array of pointers to power domain nodes in the
 * topology tree for an mpidr. It iterates through the nodes to find the
 * highest power level where the power domain is marked as physically powered
 * off.
 ******************************************************************************/
uint32_t psci_find_max_phys_off_pwrlvl(uint32_t start_pwrlvl,
				       uint32_t end_pwrlvl,
				       pwr_map_node_t *mpidr_nodes[])
{
	uint32_t max_pwrlvl = PSCI_INVALID_DATA;

	for (; start_pwrlvl <= end_pwrlvl; start_pwrlvl++) {
		if (mpidr_nodes[start_pwrlvl] == NULL)
			continue;

		if (psci_get_phys_state(mpidr_nodes[start_pwrlvl]) ==
		    PSCI_STATE_OFF)
			max_pwrlvl = start_pwrlvl;
	}

	return max_pwrlvl;
}

/*******************************************************************************
 * This function verifies that the all the other cores in the system have been
 * turned OFF and the current CPU is the last running CPU in the system.
 * Returns 1 (true) if the current CPU is the last ON CPU or 0 (false)
 * otherwise.
 ******************************************************************************/
unsigned int psci_is_last_on_cpu(void)
{
	unsigned long mpidr = read_mpidr_el1() & MPIDR_AFFINITY_MASK;
	unsigned int i;

	for (i = psci_pwr_lvl_limits[MPIDR_AFFLVL0].min;
			i <= psci_pwr_lvl_limits[MPIDR_AFFLVL0].max; i++) {

		assert(psci_pwr_domain_map[i].level == MPIDR_AFFLVL0);

		if (!(psci_pwr_domain_map[i].state & PSCI_AFF_PRESENT))
			continue;

		if (psci_pwr_domain_map[i].mpidr == mpidr) {
			assert(psci_get_state(&psci_pwr_domain_map[i])
					== PSCI_STATE_ON);
			continue;
		}

		if (psci_get_state(&psci_pwr_domain_map[i]) != PSCI_STATE_OFF)
			return 0;
	}

	return 1;
}

/*******************************************************************************
 * Routine to return the maximum power level to traverse to after a cpu has
 * been physically powered up. It is expected to be called immediately after
 * reset from assembler code.
 ******************************************************************************/
int get_power_on_target_pwrlvl(void)
{
	int pwrlvl;

#if DEBUG
	unsigned int state;
	pwr_map_node_t *node;

	/* Retrieve our node from the topology tree */
	node = psci_get_pwr_map_node(read_mpidr_el1() & MPIDR_AFFINITY_MASK,
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
	 * Assume that this cpu was suspended and retrieve its target power
	 * level. If it is invalid then it could only have been turned off
	 * earlier. PLAT_MAX_PWR_LVL will be the highest power level a
	 * cpu can be turned off to.
	 */
	pwrlvl = psci_get_suspend_pwrlvl();
	if (pwrlvl == PSCI_INVALID_DATA)
		pwrlvl = PLAT_MAX_PWR_LVL;
	return pwrlvl;
}

/*******************************************************************************
 * Simple routine to set the id of a power domain instance at a given level
 * in the mpidr. The assumption is that the affinity level and the power
 * level are the same.
 ******************************************************************************/
unsigned long mpidr_set_pwr_domain_inst(unsigned long mpidr,
				 unsigned char pwr_inst,
				 int pwr_lvl)
{
	unsigned long aff_shift;

	assert(pwr_lvl <= MPIDR_AFFLVL3);

	/*
	 * Decide the number of bits to shift by depending upon
	 * the power level
	 */
	aff_shift = get_afflvl_shift(pwr_lvl);

	/* Clear the existing affinity instance & set the new one*/
	mpidr &= ~(((unsigned long)MPIDR_AFFLVL_MASK) << aff_shift);
	mpidr |= ((unsigned long)pwr_inst) << aff_shift;

	return mpidr;
}

/*******************************************************************************
 * This function sanity checks a range of power levels.
 ******************************************************************************/
int psci_check_pwrlvl_range(int start_pwrlvl, int end_pwrlvl)
{
	/* Sanity check the parameters passed */
	if (end_pwrlvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	if (start_pwrlvl < MPIDR_AFFLVL0)
		return PSCI_E_INVALID_PARAMS;

	if (end_pwrlvl < start_pwrlvl)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * This function is passed an array of pointers to power domain nodes in the
 * topology tree for an mpidr and the state which each node should transition
 * to. It updates the state of each node between the specified power levels.
 ******************************************************************************/
void psci_do_state_coordination(uint32_t start_pwrlvl,
			       uint32_t end_pwrlvl,
			       pwr_map_node_t *mpidr_nodes[],
			       uint32_t state)
{
	uint32_t level;

	for (level = start_pwrlvl; level <= end_pwrlvl; level++) {
		if (mpidr_nodes[level] == NULL)
			continue;
		psci_set_state(mpidr_nodes[level], state);
	}
}

/*******************************************************************************
 * This function is passed an array of pointers to power domain nodes in the
 * topology tree for an mpidr. It picks up locks for each power level bottom
 * up in the range specified.
 ******************************************************************************/
void psci_acquire_pwr_domain_locks(int start_pwrlvl,
			       int end_pwrlvl,
			       pwr_map_node_t *mpidr_nodes[])
{
	int level;

	for (level = start_pwrlvl; level <= end_pwrlvl; level++) {
		if (mpidr_nodes[level] == NULL)
			continue;

		psci_lock_get(mpidr_nodes[level]);
	}
}

/*******************************************************************************
 * This function is passed an array of pointers to power domain nodes in the
 * topology tree for an mpidr. It releases the lock for each power level top
 * down in the range specified.
 ******************************************************************************/
void psci_release_pwr_domain_locks(int start_pwrlvl,
			       int end_pwrlvl,
			       pwr_map_node_t *mpidr_nodes[])
{
	int level;

	for (level = end_pwrlvl; level >= start_pwrlvl; level--) {
		if (mpidr_nodes[level] == NULL)
			continue;

		psci_lock_release(mpidr_nodes[level]);
	}
}

/*******************************************************************************
 * Simple routine to determine whether a mpidr is valid or not.
 ******************************************************************************/
int psci_validate_mpidr(unsigned long mpidr)
{
	if (plat_core_pos_by_mpidr(mpidr) < 0)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
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
 * This function takes a pointer to a power domain node in the topology tree
 * and returns its state. State of a non-leaf node needs to be calculated.
 ******************************************************************************/
unsigned short psci_get_state(pwr_map_node_t *node)
{
#if !USE_COHERENT_MEM
	flush_dcache_range((uint64_t) node, sizeof(*node));
#endif

	assert(node->level >= MPIDR_AFFLVL0 && node->level <= MPIDR_MAX_AFFLVL);

	/* A cpu node just contains the state which can be directly returned */
	if (node->level == MPIDR_AFFLVL0)
		return (node->state >> PSCI_STATE_SHIFT) & PSCI_STATE_MASK;

	/*
	 * For a power level higher than a cpu, the state has to be
	 * calculated. It depends upon the value of the reference count
	 * which is managed by each node at the next lower power level
	 * e.g. for a cluster, each cpu increments/decrements the reference
	 * count. If the reference count is 0 then the power level is
	 * OFF else ON.
	 */
	if (node->ref_count)
		return PSCI_STATE_ON;
	else
		return PSCI_STATE_OFF;
}

/*******************************************************************************
 * This function takes a pointer to a power domain node in the topology
 * tree and a target state. State of a non-leaf node needs to be converted
 * to a reference count. State of a leaf node can be set directly.
 ******************************************************************************/
void psci_set_state(pwr_map_node_t *node, unsigned short state)
{
	assert(node->level >= MPIDR_AFFLVL0 && node->level <= MPIDR_MAX_AFFLVL);

	/*
	 * For a power level higher than a cpu, the state is used
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
			 * A power level higher than a cpu will not undergo
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
 * A power domain could be on, on_pending, suspended or off. These are the
 * logical states it can be in. Physically either it is off or on. When it is in
 * the state on_pending then it is about to be turned on. It is not possible to
 * tell whether that's actually happened or not. So we err on the side of
 * caution & treat the power domain as being turned off.
 ******************************************************************************/
unsigned short psci_get_phys_state(pwr_map_node_t *node)
{
	unsigned int state;

	state = psci_get_state(node);
	return get_phys_state(state);
}

/*******************************************************************************
 * Generic handler which is called when a cpu is physically powered on. It
 * traverses the node information and finds the highest power level powered
 * off and performs generic, architectural, platform setup and state management
 * to power on that power level and power levels below it.
 * e.g. For a cpu that's been powered on, it will call the platform specific
 * code to enable the gic cpu interface and for a cluster it will enable
 * coherency at the interconnect level in addition to gic cpu interface.
 ******************************************************************************/
void psci_power_up_finish(int end_pwrlvl,
				 pwrlvl_power_on_finisher_t pon_handler)
{
	mpidr_pwr_map_nodes_t mpidr_nodes;
	int rc;
	unsigned int max_phys_off_pwrlvl;


	/*
	 * Collect the pointers to the nodes in the topology tree for
	 * each power domain instances in the mpidr. If this function does
	 * not return successfully then either the mpidr or the power
	 * levels are incorrect. Either case is an irrecoverable error.
	 */
	rc = psci_get_pwr_map_nodes(read_mpidr_el1() & MPIDR_AFFINITY_MASK,
				    MPIDR_AFFLVL0,
				    end_pwrlvl,
				    mpidr_nodes);
	if (rc != PSCI_E_SUCCESS)
		panic();

	/*
	 * This function acquires the lock corresponding to each power
	 * level so that by the time all locks are taken, the system topology
	 * is snapshot and state management can be done safely.
	 */
	psci_acquire_pwr_domain_locks(MPIDR_AFFLVL0,
				  end_pwrlvl,
				  mpidr_nodes);

	max_phys_off_pwrlvl = psci_find_max_phys_off_pwrlvl(MPIDR_AFFLVL0,
							    end_pwrlvl,
							    mpidr_nodes);
	assert(max_phys_off_pwrlvl != PSCI_INVALID_DATA);

	/* Perform generic, architecture and platform specific handling */
	pon_handler(mpidr_nodes, max_phys_off_pwrlvl);

	/*
	 * This function updates the state of each power instance
	 * corresponding to the mpidr in the range of power levels
	 * specified.
	 */
	psci_do_state_coordination(MPIDR_AFFLVL0,
				  end_pwrlvl,
				  mpidr_nodes,
				  PSCI_STATE_ON);

	/*
	 * This loop releases the lock corresponding to each power level
	 * in the reverse order to which they were acquired.
	 */
	psci_release_pwr_domain_locks(MPIDR_AFFLVL0,
				  end_pwrlvl,
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
 * This function prints the state of all power domains present in the
 * system
 ******************************************************************************/
void psci_print_power_domain_map(void)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO
	pwr_map_node_t *node;
	unsigned int idx;
	/* This array maps to the PSCI_STATE_X definitions in psci.h */
	static const char *psci_state_str[] = {
		"ON",
		"OFF",
		"ON_PENDING",
		"SUSPEND"
	};

	INFO("PSCI Power Domain Map:\n");
	for (idx = 0; idx < PSCI_NUM_PWR_DOMAINS; idx++) {
		node = &psci_pwr_domain_map[idx];
		if (!(node->state & PSCI_PWR_DOMAIN_PRESENT)) {
			continue;
		}
		INFO("  pwrInst: Level %u, MPID 0x%lx, State %s\n",
				node->level, node->mpidr,
				psci_state_str[psci_get_state(node)]);
	}
#endif
}
