/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <plat/common/platform.h>

#include "tsp_private.h"

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
void tsp_generic_timer_start(void)
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
void tsp_generic_timer_handler(void)
{
	/* Ensure that the timer did assert the interrupt */
	assert(get_cntp_ctl_istatus(read_cntps_ctl_el1()));

	/*
	 * Disable the timer and reprogram it. The barriers ensure that there is
	 * no reordering of instructions around the reprogramming code.
	 */
	isb();
	write_cntps_ctl_el1(0);
	tsp_generic_timer_start();
	isb();
}

/*******************************************************************************
 * This function deasserts the timer interrupt prior to cpu power down
 ******************************************************************************/
void tsp_generic_timer_stop(void)
{
	/* Disable the timer */
	write_cntps_ctl_el1(0);
}

/*******************************************************************************
 * This function saves the timer context prior to cpu suspension
 ******************************************************************************/
void tsp_generic_timer_save(void)
{
	uint32_t linear_id = plat_my_core_pos();

	pcpu_timer_context[linear_id].cval = read_cntps_cval_el1();
	pcpu_timer_context[linear_id].ctl = read_cntps_ctl_el1();
	flush_dcache_range((uint64_t) &pcpu_timer_context[linear_id],
			   sizeof(pcpu_timer_context[linear_id]));
}

/*******************************************************************************
 * This function restores the timer context post cpu resumption
 ******************************************************************************/
void tsp_generic_timer_restore(void)
{
	uint32_t linear_id = plat_my_core_pos();

	write_cntps_cval_el1(pcpu_timer_context[linear_id].cval);
	write_cntps_ctl_el1(pcpu_timer_context[linear_id].ctl);
}
