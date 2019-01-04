/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cortex_a75.h>
#include <cpuamu.h>
#include <lib/el3_runtime/pubsub_events.h>

static void *cortex_a75_context_save(const void *arg)
{
	if (midr_match(CORTEX_A75_MIDR) != 0)
		cpuamu_context_save(CORTEX_A75_AMU_NR_COUNTERS);

	return (void *)0;
}

static void *cortex_a75_context_restore(const void *arg)
{
	if (midr_match(CORTEX_A75_MIDR) != 0)
		cpuamu_context_restore(CORTEX_A75_AMU_NR_COUNTERS);

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, cortex_a75_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, cortex_a75_context_restore);
