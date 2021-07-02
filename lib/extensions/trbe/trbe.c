/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <lib/el3_runtime/pubsub.h>
#include <lib/extensions/trbe.h>

static void tsb_csync(void)
{
	/*
	 * The assembler does not yet understand the tsb csync mnemonic
	 * so use the equivalent hint instruction.
	 */
	__asm__ volatile("hint #18");
}

static bool trbe_supported(void)
{
	uint64_t features;

	features = read_id_aa64dfr0_el1() >> ID_AA64DFR0_TRACEBUFFER_SHIFT;
	return ((features & ID_AA64DFR0_TRACEBUFFER_MASK) ==
		ID_AA64DFR0_TRACEBUFFER_SUPPORTED);
}

void trbe_enable(void)
{
	uint64_t val;

	if (trbe_supported()) {
		/*
		 * MDCR_EL3.NSTB = 0b11
		 * Allow access of trace buffer control registers from NS-EL1
		 * and NS-EL2, tracing is prohibited in Secure and Realm state
		 * (if implemented).
		 */
		val = read_mdcr_el3();
		val |= MDCR_NSTB(MDCR_NSTB_EL1);
		write_mdcr_el3(val);
	}
}

static void *trbe_drain_trace_buffers_hook(const void *arg __unused)
{
	if (trbe_supported()) {
		/*
		 * Before switching from normal world to secure world
		 * the trace buffers need to be drained out to memory. This is
		 * required to avoid an invalid memory access when TTBR is switched
		 * for entry to S-EL1.
		 */
		tsb_csync();
		dsbnsh();
	}

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(cm_entering_secure_world, trbe_drain_trace_buffers_hook);
