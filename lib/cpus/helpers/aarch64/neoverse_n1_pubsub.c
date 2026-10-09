/*
 * Copyright (c) 2018-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cpuamu.h>
#include <lib/cpus/cpu_ops.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <neoverse_n1.h>

static void *neoverse_n1_context_save(const void *arg)
{
	if (midr_match(NEOVERSE_N1_MIDR) != 0)
		cpuamu_context_save(NEOVERSE_N1_AMU_NR_COUNTERS);

	return (void *)0;
}

static void *neoverse_n1_context_restore(const void *arg)
{
	if (midr_match(NEOVERSE_N1_MIDR) != 0)
		cpuamu_context_restore(NEOVERSE_N1_AMU_NR_COUNTERS);

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, neoverse_n1_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, neoverse_n1_context_restore);
