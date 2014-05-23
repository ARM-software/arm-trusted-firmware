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
#include <platform.h>
#include <tsp.h>

/*******************************************************************************
 * Data structure to keep track of per-cpu secure generic timer context across
 * power management operations.
 ******************************************************************************/
typedef struct timer_context {
	uint64_t cval;
	uint32_t ctl;
} timer_context_t;

static timer_context_t pcpu_timer_context[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * This function initializes the generic timer to fire every 0.5 second
 ******************************************************************************/
void tsp_generic_timer_start()
{
	uint64_t cval;
	uint32_t ctl = 0;

	/* The timer will fire every 0.5 second */
	cval = read_cntpct_el0() + (read_cntfrq_el0() >> 1);
	write_cntps_cval_el1(cval);

	/* Enable the secure physical timer */
	set_cntp_ctl_enable(ctl);
	write_cntps_ctl_el1(ctl);
}

/*******************************************************************************
 * This function deasserts the timer interrupt and sets it up again
 ******************************************************************************/
void tsp_generic_timer_handler()
{
	/* Ensure that the timer did assert the interrupt */
	assert(get_cntp_ctl_istatus(read_cntps_ctl_el1()));

	/* Disable the timer and reprogram it */
	write_cntps_ctl_el1(0);
	tsp_generic_timer_start();
}

/*******************************************************************************
 * This function deasserts the timer interrupt prior to cpu power down
 ******************************************************************************/
void tsp_generic_timer_stop()
{
	/* Disable the timer */
	write_cntps_ctl_el1(0);
}

/*******************************************************************************
 * This function saves the timer context prior to cpu suspension
 ******************************************************************************/
void tsp_generic_timer_save()
{
	uint32_t linear_id = platform_get_core_pos(read_mpidr());

	pcpu_timer_context[linear_id].cval = read_cntps_cval_el1();
	pcpu_timer_context[linear_id].ctl = read_cntps_ctl_el1();
	flush_dcache_range((uint64_t) &pcpu_timer_context[linear_id],
			   sizeof(pcpu_timer_context[linear_id]));
}

/*******************************************************************************
 * This function restores the timer context post cpu resummption
 ******************************************************************************/
void tsp_generic_timer_restore()
{
	uint32_t linear_id = platform_get_core_pos(read_mpidr());

	write_cntps_cval_el1(pcpu_timer_context[linear_id].cval);
	write_cntps_ctl_el1(pcpu_timer_context[linear_id].ctl);
}
