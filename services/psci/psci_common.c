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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <psci.h>
#include <psci_private.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include "debug.h"

/*
 * Provide a null weak instantiation for SPD power management operations. An SPD
 * can define its own instance overriding this one
 */
const spd_pm_ops __attribute__((weak)) spd_pm = {0};

/*******************************************************************************
 * Arrays that contains information needs to resume a cpu's execution when woken
 * out of suspend or off states. 'psci_ns_einfo_idx' keeps track of the next
 * free index in the 'psci_ns_entry_info' & 'psci_suspend_context' arrays. Each
 * cpu is allocated a single entry in each array during startup.
 ******************************************************************************/
suspend_context psci_suspend_context[PSCI_NUM_AFFS];
ns_entry_info psci_ns_entry_info[PSCI_NUM_AFFS];
unsigned int psci_ns_einfo_idx;

/*******************************************************************************
 * Grand array that holds the platform's topology information for state
 * management of affinity instances. Each node (aff_map_node) in the array
 * corresponds to an affinity instance e.g. cluster, cpu within an mpidr
 ******************************************************************************/
aff_map_node psci_aff_map[PSCI_NUM_AFFS]
__attribute__ ((section("tzfw_coherent_mem")));

/*******************************************************************************
 * In a system, a certain number of affinity instances are present at an
 * affinity level. The cumulative number of instances across all levels are
 * stored in 'psci_aff_map'. The topology tree has been flattenned into this
 * array. To retrieve nodes, information about the extents of each affinity
 * level i.e. start index and end index needs to be present. 'psci_aff_limits'
 * stores this information.
 ******************************************************************************/
aff_limits_node psci_aff_limits[MPIDR_MAX_AFFLVL + 1];

/*******************************************************************************
 * Pointer to functions exported by the platform to complete power mgmt. ops
 ******************************************************************************/
plat_pm_ops *psci_plat_pm_ops;

/*******************************************************************************
 * Routine to return the maximum affinity level to traverse to after a cpu has
 * been physically powered up. It is expected to be called immediately after
 * reset from assembler code. It has to find its 'aff_map_node' instead of
 * getting it as an argument.
 * TODO: Calling psci_get_aff_map_node() with the MMU disabled is slow. Add
 * support to allow faster access to the target affinity level.
 ******************************************************************************/
int get_power_on_target_afflvl(unsigned long mpidr)
{
	aff_map_node *node;
	unsigned int state;

	/* Retrieve our node from the topology tree */
	node = psci_get_aff_map_node(mpidr & MPIDR_AFFINITY_MASK,
			MPIDR_AFFLVL0);
	assert(node);

	/*
	 * Return the maximum supported affinity level if this cpu was off.
	 * Call the handler in the suspend code if this cpu had been suspended.
	 * Any other state is invalid.
	 */
	state = psci_get_state(node);
	if (state == PSCI_STATE_ON_PENDING)
		return get_max_afflvl();

	if (state == PSCI_STATE_SUSPEND)
		return psci_get_suspend_afflvl(node);

	return PSCI_E_INVALID_PARAMS;
}

/*******************************************************************************
 * Simple routine to retrieve the maximum affinity level supported by the
 * platform and check that it makes sense.
 ******************************************************************************/
int get_max_afflvl()
{
	int aff_lvl;

	aff_lvl = plat_get_max_afflvl();
	assert(aff_lvl <= MPIDR_MAX_AFFLVL && aff_lvl >= MPIDR_AFFLVL0);

	return aff_lvl;
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
	if (end_afflvl > MPIDR_MAX_AFFLVL)
		return PSCI_E_INVALID_PARAMS;

	if (start_afflvl < MPIDR_AFFLVL0)
		return PSCI_E_INVALID_PARAMS;

	if (end_afflvl < start_afflvl)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * This function is passed an array of pointers to affinity level nodes in the
 * topology tree for an mpidr. It picks up locks for each affinity level bottom
 * up in the range specified.
 ******************************************************************************/
void psci_acquire_afflvl_locks(unsigned long mpidr,
			       int start_afflvl,
			       int end_afflvl,
			       mpidr_aff_map_nodes mpidr_nodes)
{
	int level;

	for (level = start_afflvl; level <= end_afflvl; level++) {
		if (mpidr_nodes[level] == NULL)
			continue;
		bakery_lock_get(mpidr, &mpidr_nodes[level]->lock);
	}
}

/*******************************************************************************
 * This function is passed an array of pointers to affinity level nodes in the
 * topology tree for an mpidr. It releases the lock for each affinity level top
 * down in the range specified.
 ******************************************************************************/
void psci_release_afflvl_locks(unsigned long mpidr,
			       int start_afflvl,
			       int end_afflvl,
			       mpidr_aff_map_nodes mpidr_nodes)
{
	int level;

	for (level = end_afflvl; level >= start_afflvl; level--) {
		if (mpidr_nodes[level] == NULL)
			continue;
		bakery_lock_release(mpidr, &mpidr_nodes[level]->lock);
	}
}

/*******************************************************************************
 * Simple routine to determine whether an affinity instance at a given level
 * in an mpidr exists or not.
 ******************************************************************************/
int psci_validate_mpidr(unsigned long mpidr, int level)
{
	aff_map_node *node;

	node = psci_get_aff_map_node(mpidr, level);
	if (node && (node->state & PSCI_AFF_PRESENT))
		return PSCI_E_SUCCESS;
	else
		return PSCI_E_INVALID_PARAMS;
}

/*******************************************************************************
 * This function retrieves all the stashed information needed to correctly
 * resume a cpu's execution in the non-secure state after it has been physically
 * powered on i.e. turned ON or resumed from SUSPEND
 ******************************************************************************/
void psci_get_ns_entry_info(unsigned int index)
{
	unsigned long sctlr = 0, scr, el_status, id_aa64pfr0;
	uint64_t mpidr = read_mpidr();
	cpu_context *ns_entry_context;
	gp_regs *ns_entry_gpregs;

	scr = read_scr();

	/* Find out which EL we are going to */
	id_aa64pfr0 = read_id_aa64pfr0_el1();
	el_status = (id_aa64pfr0 >> ID_AA64PFR0_EL2_SHIFT) &
		ID_AA64PFR0_ELX_MASK;

	/* Restore endianess */
	if (psci_ns_entry_info[index].sctlr & SCTLR_EE_BIT)
		sctlr |= SCTLR_EE_BIT;
	else
		sctlr &= ~SCTLR_EE_BIT;

	/* Turn off MMU and Caching */
	sctlr &= ~(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_M_BIT);

	/* Set the register width */
	if (psci_ns_entry_info[index].scr & SCR_RW_BIT)
		scr |= SCR_RW_BIT;
	else
		scr &= ~SCR_RW_BIT;

	scr |= SCR_NS_BIT;

	if (el_status)
		write_sctlr_el2(sctlr);
	else
		write_sctlr_el1(sctlr);

	/* Fulfill the cpu_on entry reqs. as per the psci spec */
	ns_entry_context = (cpu_context *) cm_get_context(mpidr, NON_SECURE);
	assert(ns_entry_context);

	/*
	 * Setup general purpose registers to return the context id and
	 * prevent leakage of secure information into the normal world.
	 */
	ns_entry_gpregs = get_gpregs_ctx(ns_entry_context);
	write_ctx_reg(ns_entry_gpregs,
		      CTX_GPREG_X0,
		      psci_ns_entry_info[index].context_id);

	/*
	 * Tell the context management library to setup EL3 system registers to
	 * be able to ERET into the ns state, and SP_EL3 points to the right
	 * context to exit from EL3 correctly.
	 */
	cm_set_el3_eret_context(NON_SECURE,
			psci_ns_entry_info[index].eret_info.entrypoint,
			psci_ns_entry_info[index].eret_info.spsr,
			scr);

	cm_set_next_eret_context(NON_SECURE);
}

/*******************************************************************************
 * This function retrieves and stashes all the information needed to correctly
 * resume a cpu's execution in the non-secure state after it has been physically
 * powered on i.e. turned ON or resumed from SUSPEND. This is done prior to
 * turning it on or before suspending it.
 ******************************************************************************/
int psci_set_ns_entry_info(unsigned int index,
			   unsigned long entrypoint,
			   unsigned long context_id)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int rw, mode, ee, spsr = 0;
	unsigned long id_aa64pfr0 = read_id_aa64pfr0_el1(), scr = read_scr();
	unsigned long el_status;

	/* Figure out what mode do we enter the non-secure world in */
	el_status = (id_aa64pfr0 >> ID_AA64PFR0_EL2_SHIFT) &
		ID_AA64PFR0_ELX_MASK;

	/*
	 * Figure out whether the cpu enters the non-secure address space
	 * in aarch32 or aarch64
	 */
	rw = scr & SCR_RW_BIT;
	if (rw) {

		/*
		 * Check whether a Thumb entry point has been provided for an
		 * aarch64 EL
		 */
		if (entrypoint & 0x1)
			return PSCI_E_INVALID_PARAMS;

		if (el_status && (scr & SCR_HCE_BIT)) {
			mode = MODE_EL2;
			ee = read_sctlr_el2() & SCTLR_EE_BIT;
		} else {
			mode = MODE_EL1;
			ee = read_sctlr_el1() & SCTLR_EE_BIT;
		}

		spsr = DAIF_DBG_BIT | DAIF_ABT_BIT;
		spsr |= DAIF_IRQ_BIT | DAIF_FIQ_BIT;
		spsr <<= PSR_DAIF_SHIFT;
		spsr |= make_spsr(mode, MODE_SP_ELX, !rw);

		psci_ns_entry_info[index].sctlr |= ee;
		psci_ns_entry_info[index].scr |= SCR_RW_BIT;
	} else {

		/* Check whether aarch32 has to be entered in Thumb mode */
		if (entrypoint & 0x1)
			spsr = SPSR32_T_BIT;

		if (el_status && (scr & SCR_HCE_BIT)) {
			mode = AARCH32_MODE_HYP;
			ee = read_sctlr_el2() & SCTLR_EE_BIT;
		} else {
			mode = AARCH32_MODE_SVC;
			ee = read_sctlr_el1() & SCTLR_EE_BIT;
		}

		/*
		 * TODO: Choose async. exception bits if HYP mode is not
		 * implemented according to the values of SCR.{AW, FW} bits
		 */
		spsr |= DAIF_ABT_BIT | DAIF_IRQ_BIT | DAIF_FIQ_BIT;
		spsr <<= PSR_DAIF_SHIFT;
		if (ee)
			spsr |= SPSR32_EE_BIT;
		spsr |= mode;

		/* Ensure that the CSPR.E and SCTLR.EE bits match */
		psci_ns_entry_info[index].sctlr |= ee;
		psci_ns_entry_info[index].scr &= ~SCR_RW_BIT;
	}

	psci_ns_entry_info[index].eret_info.entrypoint = entrypoint;
	psci_ns_entry_info[index].eret_info.spsr = spsr;
	psci_ns_entry_info[index].context_id = context_id;

	return rc;
}

/*******************************************************************************
 * This function takes a pointer to an affinity node in the topology tree and
 * returns its state. State of a non-leaf node needs to be calculated.
 ******************************************************************************/
unsigned short psci_get_state(aff_map_node *node)
{
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
void psci_set_state(aff_map_node *node, unsigned short state)
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
}

/*******************************************************************************
 * An affinity level could be on, on_pending, suspended or off. These are the
 * logical states it can be in. Physically either it is off or on. When it is in
 * the state on_pending then it is about to be turned on. It is not possible to
 * tell whether that's actually happenned or not. So we err on the side of
 * caution & treat the affinity level as being turned off.
 ******************************************************************************/
unsigned short psci_get_phys_state(aff_map_node *node)
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
static int psci_call_power_on_handlers(mpidr_aff_map_nodes mpidr_nodes,
				       int start_afflvl,
				       int end_afflvl,
				       afflvl_power_on_finisher *pon_handlers,
				       unsigned long mpidr)
{
	int rc = PSCI_E_INVALID_PARAMS, level;
	aff_map_node *node;

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
		rc = pon_handlers[level](mpidr, node);
		if (rc != PSCI_E_SUCCESS)
			break;
	}

	return rc;
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
 *
 * CAUTION: This function is called with coherent stacks so that coherency and
 * the mmu can be turned on safely.
 ******************************************************************************/
void psci_afflvl_power_on_finish(unsigned long mpidr,
				 int start_afflvl,
				 int end_afflvl,
				 afflvl_power_on_finisher *pon_handlers)
{
	mpidr_aff_map_nodes mpidr_nodes;
	int rc;

	mpidr &= MPIDR_AFFINITY_MASK;

	/*
	 * Collect the pointers to the nodes in the topology tree for
	 * each affinity instance in the mpidr. If this function does
	 * not return successfully then either the mpidr or the affinity
	 * levels are incorrect. Either case is an irrecoverable error.
	 */
	rc = psci_get_aff_map_nodes(mpidr,
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
	psci_acquire_afflvl_locks(mpidr,
				  start_afflvl,
				  end_afflvl,
				  mpidr_nodes);

	/* Perform generic, architecture and platform specific handling */
	rc = psci_call_power_on_handlers(mpidr_nodes,
					 start_afflvl,
					 end_afflvl,
					 pon_handlers,
					 mpidr);
	if (rc != PSCI_E_SUCCESS)
		panic();

	/*
	 * This loop releases the lock corresponding to each affinity level
	 * in the reverse order to which they were acquired.
	 */
	psci_release_afflvl_locks(mpidr,
				  start_afflvl,
				  end_afflvl,
				  mpidr_nodes);
}
