/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>

#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <tegra_def.h>
#include <tegra_private.h>

static uint32_t tegra_timer_get_value(void)
{
	/* enable cntps_tval_el1 timer, mask interrupt */
	write_cntps_ctl_el1(CNTP_CTL_IMASK_BIT | CNTP_CTL_ENABLE_BIT);

	/*
	 * Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntps_tval_el1() to simulate the down counter. The value is
	 * clipped from 64 to 32 bits.
	 */
	return (uint32_t)(~read_cntps_tval_el1());
}

/*
 * Initialise the architecture provided counter as the delay timer.
 */
void tegra_delay_timer_init(void)
{
	static timer_ops_t tegra_timer_ops;

	/* Value in ticks */
	uint32_t multiplier = MHZ_TICKS_PER_SEC;

	/* Value in ticks per second (Hz) */
	uint32_t divider  = plat_get_syscnt_freq2();

	/* Reduce multiplier and divider by dividing them repeatedly by 10 */
	while (((multiplier % 10U) == 0U) && ((divider % 10U) == 0U)) {
		multiplier /= 10U;
		divider /= 10U;
	}

	/* enable cntps_tval_el1 timer, mask interrupt */
	write_cntps_ctl_el1(CNTP_CTL_IMASK_BIT | CNTP_CTL_ENABLE_BIT);

	/* register the timer */
	tegra_timer_ops.get_timer_value = tegra_timer_get_value;
	tegra_timer_ops.clk_mult = multiplier;
	tegra_timer_ops.clk_div = divider;
	timer_init(&tegra_timer_ops);
}
