/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cpu_config.h>

#define KODIAK_SILVER_CORES_CPUMASK	0x0fU
#define KODIAK_GOLD_CORES_CPUMASK	0x70U
#define KODIAK_GOLDPLUS_CORES_CPUMASK	0x80U

/* Fuse-disabled cores are removed by the CPUCP start sequence. */
struct clkdom_cpumask clkdom_cpumasks[CD_MAX] = {
	{ CD_L3,       0x00U                         },
	{ CD_SILVER,   KODIAK_SILVER_CORES_CPUMASK   },
	{ CD_GOLD,     KODIAK_GOLD_CORES_CPUMASK     },
	{ CD_GOLDPLUS, KODIAK_GOLDPLUS_CORES_CPUMASK },
};

/*
 * The boot firmware brings up the L3 and Cortex-A55 domains. The gold and
 * gold+ domains are enabled through CPUCP when their first core powers on.
 */
unsigned int clkdom_init_status[CD_MAX] = {
	1U, 1U, 0U, 0U
};
