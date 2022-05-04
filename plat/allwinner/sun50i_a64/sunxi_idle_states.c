/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sunxi_private.h>

const struct psci_cpu_idle_state sunxi_idle_states[] = {
	{
		.name			= "cpu-sleep",
		.power_state		= 0x00010003,
		.local_timer_stop	= true,
		.entry_latency_us	= 800,
		.exit_latency_us	= 1500,
		.min_residency_us	= 25000
	},
	{
		.name			= "cluster-sleep",
		.power_state		= 0x01010013,
		.local_timer_stop	= true,
		.entry_latency_us	= 850,
		.exit_latency_us	= 1500,
		.min_residency_us	= 50000
	},
	{}
};
