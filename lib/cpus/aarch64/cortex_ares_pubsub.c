/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cortex_ares.h>
#include <cpuamu.h>
#include <lib/el3_runtime/pubsub_events.h>

static void *cortex_ares_context_save(const void *arg)
{
	if (midr_match(CORTEX_ARES_MIDR) != 0)
		cpuamu_context_save(CORTEX_ARES_AMU_NR_COUNTERS);

	return (void *)0;
}

static void *cortex_ares_context_restore(const void *arg)
{
	if (midr_match(CORTEX_ARES_MIDR) != 0)
		cpuamu_context_restore(CORTEX_ARES_AMU_NR_COUNTERS);

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, cortex_ares_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, cortex_ares_context_restore);
