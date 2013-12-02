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
#include <runtime_svc.h>

/*******************************************************************************
 * Arrays that contains information needs to resume a cpu's execution when woken
 * out of suspend or off states. 'psci_ns_einfo_idx' keeps track of the next
 * free index in the 'psci_ns_entry_info' & 'psci_secure_context' arrays. Each
 * cpu is allocated a single entry in each array during startup.
 ******************************************************************************/
secure_context psci_secure_context[PSCI_NUM_AFFS];
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
 * Simple routine to determine the first affinity level instance that is present
 * between the start and end affinity levels. This helps to skip handling of
 * absent affinity levels while performing psci operations.
 * The start level can be > or <= to the end level depending upon whether this
 * routine is expected to search top down or bottom up.
 ******************************************************************************/
int psci_get_first_present_afflvl(unsigned long mpidr,
				  int start_afflvl,
				  int end_afflvl,
				  aff_map_node **node)
{
	int level;

	/* Check whether we have to search up or down */
	if (start_afflvl <= end_afflvl) {
		for (level = start_afflvl; level <= end_afflvl; level++) {
			*node = psci_get_aff_map_node(mpidr, level);
			if (*node && ((*node)->state & PSCI_AFF_PRESENT))
				break;
		}
	} else {
		for (level = start_afflvl; level >= end_afflvl; level--) {
			*node = psci_get_aff_map_node(mpidr, level);
			if (*node && ((*node)->state & PSCI_AFF_PRESENT))
				break;
		}
	}

	return level;
}

/*******************************************************************************
 * Recursively change the affinity state between the current and target affinity
 * levels. The target state matters only if we are starting from affinity level
 * 0 i.e. a cpu otherwise the state depends upon the state of the lower affinity
 * levels.
 ******************************************************************************/
int psci_change_state(unsigned long mpidr,
		      int cur_afflvl,
		      int tgt_afflvl,
		      unsigned int tgt_state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int state;
	aff_map_node *aff_node;

	/* Sanity check the affinity levels */
	assert(tgt_afflvl >= cur_afflvl);

	aff_node = psci_get_aff_map_node(mpidr, cur_afflvl);
	assert(aff_node);

	/* TODO: Check whether the affinity level is present or absent*/

	if (cur_afflvl == MPIDR_AFFLVL0) {
		psci_set_state(aff_node->state, tgt_state);
	} else {
		state = psci_calculate_affinity_state(aff_node);
		psci_set_state(aff_node->state, state);
	}

	if (cur_afflvl != tgt_afflvl)
		psci_change_state(mpidr, cur_afflvl + 1, tgt_afflvl, tgt_state);

	return rc;
}

/*******************************************************************************
 * This routine does the heavy lifting for psci_change_state(). It examines the
 * state of each affinity instance at the next lower affinity level and decides
 * its final state accordingly. If a lower affinity instance is ON then the
 * higher affinity instance is ON. If all the lower affinity instances are OFF
 * then the higher affinity instance is OFF. If atleast one lower affinity
 * instance is SUSPENDED then the higher affinity instance is SUSPENDED. If only
 * a single lower affinity instance is ON_PENDING then the higher affinity
 * instance in ON_PENDING as well.
 ******************************************************************************/
unsigned int psci_calculate_affinity_state(aff_map_node *aff_node)
{
	int ctr;
	unsigned int aff_count, hi_aff_state;
	unsigned long tempidr;
	aff_map_node *lo_aff_node;

	/* Cannot calculate lowest affinity state. It is simply assigned */
	assert(aff_node->level > MPIDR_AFFLVL0);

	/*
	 * Find the number of affinity instances at level X-1 e.g. number of
	 * cpus in a cluster. The level X state depends upon the state of each
	 * instance at level X-1
	 */
	hi_aff_state = PSCI_STATE_OFF;
	aff_count = plat_get_aff_count(aff_node->level - 1, aff_node->mpidr);
	for (ctr = 0; ctr < aff_count; ctr++) {

		/*
		 * Create a mpidr for each lower affinity level (X-1). Use their
		 * states to influence the higher affinity state (X).
		 */
		tempidr = mpidr_set_aff_inst(aff_node->mpidr,
					     ctr,
					     aff_node->level - 1);
		lo_aff_node = psci_get_aff_map_node(tempidr,
						    aff_node->level - 1);
		assert(lo_aff_node);

		/* Continue only if the cpu exists within the cluster */
		if (!(lo_aff_node->state & PSCI_AFF_PRESENT))
			continue;

		switch (psci_get_state(lo_aff_node->state)) {

		/*
		 * If any lower affinity is on within the cluster, then
		 * the higher affinity is on.
		 */
		case PSCI_STATE_ON:
			return PSCI_STATE_ON;

		/*
		 * At least one X-1 needs to be suspended for X to be suspended
		 * but it is effectively on for the affinity_info call.
		 * SUSPEND > ON_PENDING > OFF.
		 */
		case PSCI_STATE_SUSPEND:
			hi_aff_state = PSCI_STATE_SUSPEND;
			continue;

		/*
		 * Atleast one X-1 needs to be on_pending & the rest off for X
		 * to be on_pending. ON_PENDING > OFF.
		 */
		case PSCI_STATE_ON_PENDING:
			if (hi_aff_state != PSCI_STATE_SUSPEND)
				hi_aff_state = PSCI_STATE_ON_PENDING;
			continue;

		/* Higher affinity is off if all lower affinities are off. */
		case PSCI_STATE_OFF:
			continue;

		default:
			assert(0);
		}
	}

	return hi_aff_state;
}

/*******************************************************************************
 * This function retrieves all the stashed information needed to correctly
 * resume a cpu's execution in the non-secure state after it has been physically
 * powered on i.e. turned ON or resumed from SUSPEND
 ******************************************************************************/
void psci_get_ns_entry_info(unsigned int index)
{
	unsigned long sctlr = 0, scr, el_status, id_aa64pfr0;
	gp_regs *ns_gp_regs;

	scr = read_scr();

	/* Switch to the non-secure view of the registers */
	write_scr(scr | SCR_NS_BIT);

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
	write_scr(scr);
	write_elr(psci_ns_entry_info[index].eret_info.entrypoint);

	/*
	 * Set the general purpose registers to ~0 upon entry into the
	 * non-secure world except for x0 which should contain the
	 * context id & spsr. This is done directly on the "would be"
	 * stack pointer. Prior to entry into the non-secure world, an
	 * offset equivalent to the size of the 'gp_regs' structure is
	 * added to the sp. This general purpose register context is
	 * retrieved then.
	 */
	ns_gp_regs = (gp_regs *) platform_get_stack(read_mpidr());
	ns_gp_regs--;
	memset(ns_gp_regs, ~0, sizeof(*ns_gp_regs));
	ns_gp_regs->x0 = psci_ns_entry_info[index].context_id;
	ns_gp_regs->spsr = psci_ns_entry_info[index].eret_info.spsr;
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
		if(ee)
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
 * An affinity level could be on, on_pending, suspended or off. These are the
 * logical states it can be in. Physically either it is off or on. When it is in
 * the state on_pending then it is about to be turned on. It is not possible to
 * tell whether that's actually happenned or not. So we err on the side of
 * caution & treat the affinity level as being turned off.
 ******************************************************************************/
inline unsigned int psci_get_phys_state(unsigned int aff_state)
{
	return (aff_state != PSCI_STATE_ON ? PSCI_STATE_OFF : PSCI_STATE_ON);
}

unsigned int psci_get_aff_phys_state(aff_map_node *aff_node)
{
	unsigned int aff_state;

	aff_state = psci_get_state(aff_node->state);
	return psci_get_phys_state(aff_state);
}

/*******************************************************************************
 * Generic handler which is called when a cpu is physically powered on. It
 * recurses through all the affinity levels performing generic, architectural,
 * platform setup and state management e.g. for a cluster that's been powered
 * on, it will call the platform specific code which will enable coherency at
 * the interconnect level. For a cpu it could mean turning on the MMU etc.
 *
 * This function traverses from the lowest to the highest affinity level
 * implemented by the platform. Since it's recursive, for each call the
 * 'cur_afflvl' & 'tgt_afflvl' parameters keep track of which level we are at
 * and which level we need to get to respectively. Locks are picked up along the
 * way so that when the lowest affinity level is hit, state management can be
 * safely done. Prior to this, each affinity level does it's bookeeping as per
 * the state out of reset.
 *
 * CAUTION: This function is called with coherent stacks so that coherency and
 * the mmu can be turned on safely.
 ******************************************************************************/
unsigned int psci_afflvl_power_on_finish(unsigned long mpidr,
					 int cur_afflvl,
					 int tgt_afflvl,
					 afflvl_power_on_finisher *pon_handlers)
{
	unsigned int prev_state, next_state, rc = PSCI_E_SUCCESS;
	aff_map_node *aff_node;
	int level;

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
	 * affinity level so that by the time we hit the highest
	 * affinity level, the system topology is snapshot and state
	 * management can be done safely.
	 */
	bakery_lock_get(mpidr, &aff_node->lock);

	/* Keep the old and new state handy */
	prev_state = psci_get_state(aff_node->state);
	next_state = PSCI_STATE_ON;

	/* Perform generic, architecture and platform specific handling */
	rc = pon_handlers[level](mpidr, aff_node, prev_state);
	if (rc != PSCI_E_SUCCESS) {
		psci_set_state(aff_node->state, prev_state);
		goto exit;
	}

	/*
	 * State management: Update the states if this is the highest
	 * affinity level requested else pass the job to the next level.
	 */
	if (aff_node->level != tgt_afflvl) {
		rc = psci_afflvl_power_on_finish(mpidr,
						 level + 1,
						 tgt_afflvl,
						 pon_handlers);
	} else {
		psci_change_state(mpidr, MPIDR_AFFLVL0, tgt_afflvl, next_state);
	}

	/* If all has gone as per plan then this cpu should be marked as ON */
	if (level == MPIDR_AFFLVL0) {
		next_state = psci_get_state(aff_node->state);
		assert(next_state == PSCI_STATE_ON);
	}

exit:
	bakery_lock_release(mpidr, &aff_node->lock);
	return rc;
}
