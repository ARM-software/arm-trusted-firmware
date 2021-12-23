/*
 * Copyright (c) 2014-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <bl32/tsp/tsp.h>
#include <common/debug.h>
#include <plat/common/platform.h>

#include "tsp_private.h"

/*******************************************************************************
 * This function updates the TSP statistics for S-EL1 interrupts handled
 * synchronously i.e the ones that have been handed over by the TSPD. It also
 * keeps count of the number of times control was passed back to the TSPD
 * after handling the interrupt. In the future it will be possible that the
 * TSPD hands over an S-EL1 interrupt to the TSP but does not expect it to
 * return execution. This statistic will be useful to distinguish between these
 * two models of synchronous S-EL1 interrupt handling. The 'elr_el3' parameter
 * contains the address of the instruction in normal world where this S-EL1
 * interrupt was generated.
 ******************************************************************************/
void tsp_update_sync_sel1_intr_stats(uint32_t type, uint64_t elr_el3)
{
	uint32_t linear_id = plat_my_core_pos();

	tsp_stats[linear_id].sync_sel1_intr_count++;
	if (type == TSP_HANDLE_SEL1_INTR_AND_RETURN)
		tsp_stats[linear_id].sync_sel1_intr_ret_count++;

	VERBOSE("TSP: cpu 0x%lx sync s-el1 interrupt request from 0x%" PRIx64 "\n",
		read_mpidr(), elr_el3);
	VERBOSE("TSP: cpu 0x%lx: %d sync s-el1 interrupt requests,"
		" %d sync s-el1 interrupt returns\n",
		read_mpidr(),
		tsp_stats[linear_id].sync_sel1_intr_count,
		tsp_stats[linear_id].sync_sel1_intr_ret_count);
}

/******************************************************************************
 * This function is invoked when a non S-EL1 interrupt is received and causes
 * the preemption of TSP. This function returns TSP_PREEMPTED and results
 * in the control being handed over to EL3 for handling the interrupt.
 *****************************************************************************/
int32_t tsp_handle_preemption(void)
{
	uint32_t linear_id = plat_my_core_pos();

	tsp_stats[linear_id].preempt_intr_count++;
	VERBOSE("TSP: cpu 0x%lx: %d preempt interrupt requests\n",
		read_mpidr(), tsp_stats[linear_id].preempt_intr_count);
	return TSP_PREEMPTED;
}

/*******************************************************************************
 * TSP interrupt handler is called as a part of both synchronous and
 * asynchronous handling of TSP interrupts. Currently the physical timer
 * interrupt is the only S-EL1 interrupt that this handler expects. It returns
 * 0 upon successfully handling the expected interrupt and all other
 * interrupts are treated as normal world or EL3 interrupts.
 ******************************************************************************/
int32_t tsp_common_int_handler(void)
{
	uint32_t linear_id = plat_my_core_pos(), id;

	/*
	 * Get the highest priority pending interrupt id and see if it is the
	 * secure physical generic timer interrupt in which case, handle it.
	 * Otherwise throw this interrupt at the EL3 firmware.
	 *
	 * There is a small time window between reading the highest priority
	 * pending interrupt and acknowledging it during which another
	 * interrupt of higher priority could become the highest pending
	 * interrupt. This is not expected to happen currently for TSP.
	 */
	id = plat_ic_get_pending_interrupt_id();

	/* TSP can only handle the secure physical timer interrupt */
	if (id != TSP_IRQ_SEC_PHY_TIMER) {
#if SPMC_AT_EL3
		/*
		 * With the EL3 FF-A SPMC we expect only Timer secure interrupt to fire in
		 * the TSP, so panic if any other interrupt does.
		 */
		ERROR("Unexpected interrupt id %u\n", id);
		panic();
#else
		return tsp_handle_preemption();
#endif
	}

	/*
	 * Acknowledge and handle the secure timer interrupt. Also sanity check
	 * if it has been preempted by another interrupt through an assertion.
	 */
	id = plat_ic_acknowledge_interrupt();
	assert(id == TSP_IRQ_SEC_PHY_TIMER);
	tsp_generic_timer_handler();
	plat_ic_end_of_interrupt(id);

	/* Update the statistics and print some messages */
	tsp_stats[linear_id].sel1_intr_count++;
	VERBOSE("TSP: cpu 0x%lx handled S-EL1 interrupt %d\n",
	       read_mpidr(), id);
	VERBOSE("TSP: cpu 0x%lx: %d S-EL1 requests\n",
	     read_mpidr(), tsp_stats[linear_id].sel1_intr_count);
	return 0;
}
