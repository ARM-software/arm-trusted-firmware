/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#define SOCFPGA_GLOBAL_TIMER		0xffd01000
#define SOCFPGA_GLOBAL_TIMER_EN		0x3

/********************************************************************
 * The timer delay function
 ********************************************************************/
static uint32_t socfpga_get_timer_value(void)
{
	/*
	 * Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter. The value is
	 * clipped from 64 to 32 bits.
	 */
	return (uint32_t)(~read_cntpct_el0());
}

static const timer_ops_t plat_timer_ops = {
	.get_timer_value    = socfpga_get_timer_value,
	.clk_mult           = 1,
	.clk_div	    = PLAT_SYS_COUNTER_FREQ_IN_MHZ,
};

void socfpga_delay_timer_init(void)
{
	timer_init(&plat_timer_ops);
	mmio_write_32(SOCFPGA_GLOBAL_TIMER, SOCFPGA_GLOBAL_TIMER_EN);

	asm volatile("msr cntp_ctl_el0, %0" : : "r" (SOCFPGA_GLOBAL_TIMER_EN));
	asm volatile("msr cntp_tval_el0, %0" : : "r" (~0));

}
