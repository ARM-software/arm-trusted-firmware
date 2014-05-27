/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <gic_v2.h>
#include <tsp.h>
#include <platform.h>
#include <platform_def.h>

/*******************************************************************************
 * This function updates the TSP statistics for FIQs handled synchronously i.e
 * the ones that have been handed over by the TSPD. It also keeps count of the
 * number of times control was passed back to the TSPD after handling an FIQ.
 * In the future it will be possible that the TSPD hands over an FIQ to the TSP
 * but does not expect it to return execution. This statistic will be useful to
 * distinguish between these two models of synchronous FIQ handling.
 * The 'elr_el3' parameter contains the address of the instruction in normal
 * world where this FIQ was generated.
 ******************************************************************************/
void tsp_update_sync_fiq_stats(uint32_t type, uint64_t elr_el3)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

	tsp_stats[linear_id].sync_fiq_count++;
	if (type == TSP_HANDLE_FIQ_AND_RETURN)
		tsp_stats[linear_id].sync_fiq_ret_count++;

	spin_lock(&console_lock);
	printf("TSP: cpu 0x%x sync fiq request from 0x%llx \n\r",
	       mpidr, elr_el3);
	INFO("cpu 0x%x: %d sync fiq requests, %d sync fiq returns\n",
	     mpidr,
	     tsp_stats[linear_id].sync_fiq_count,
	     tsp_stats[linear_id].sync_fiq_ret_count);
	spin_unlock(&console_lock);
}

/*******************************************************************************
 * TSP FIQ handler called as a part of both synchronous and asynchronous
 * handling of FIQ interrupts. It returns 0 upon successfully handling a S-EL1
 * FIQ and treats all other FIQs as EL3 interrupts. It assumes that the GIC
 * architecture version in v2.0 and the secure physical timer interrupt is the
 * only S-EL1 interrupt that it needs to handle.
 ******************************************************************************/
int32_t tsp_fiq_handler()
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr), id;

	/*
	 * Get the highest priority pending interrupt id and see if it is the
	 * secure physical generic timer interrupt in which case, handle it.
	 * Otherwise throw this interrupt at the EL3 firmware.
	 */
	id = plat_ic_get_pending_interrupt_id();

	/* TSP can only handle the secure physical timer interrupt */
	if (id != IRQ_SEC_PHY_TIMER)
		return TSP_EL3_FIQ;

	/*
	 * Handle the interrupt. Also sanity check if it has been preempted by
	 * another secure interrupt through an assertion.
	 */
	id = plat_ic_acknowledge_interrupt();
	assert(id == IRQ_SEC_PHY_TIMER);
	tsp_generic_timer_handler();
	plat_ic_end_of_interrupt(id);

	/* Update the statistics and print some messages */
	tsp_stats[linear_id].fiq_count++;
	spin_lock(&console_lock);
	printf("TSP: cpu 0x%x handled fiq %d \n\r",
	       mpidr, id);
	INFO("cpu 0x%x: %d fiq requests \n",
	     mpidr, tsp_stats[linear_id].fiq_count);
	spin_unlock(&console_lock);

	return 0;
}

int32_t tsp_irq_received()
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);

	tsp_stats[linear_id].irq_count++;
	spin_lock(&console_lock);
	printf("TSP: cpu 0x%x received irq\n\r", mpidr);
	INFO("cpu 0x%x: %d irq requests \n",
	     mpidr, tsp_stats[linear_id].irq_count);
	spin_unlock(&console_lock);

	return TSP_PREEMPTED;
}
