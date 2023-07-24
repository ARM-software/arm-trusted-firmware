/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
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

void trbe_init_el3(void)
{
	u_register_t val;

	/*
	 * MDCR_EL3.NSTBE = 0b0
	 *  Trace Buffer owning Security state is Non-secure state. If FEAT_RME
	 *  is not implemented, this field is RES0.
	 *
	 * MDCR_EL3.NSTB = 0b11
	 *  Allow access of trace buffer control registers from NS-EL1 and
	 *  NS-EL2, tracing is prohibited in Secure and Realm state (if
	 *  implemented).
	 */
	val = read_mdcr_el3();
	val |= MDCR_NSTB(MDCR_NSTB_EL1);
	val &= ~(MDCR_NSTBE_BIT);
	write_mdcr_el3(val);
}

void trbe_init_el2_unused(void)
{
	/*
	 * MDCR_EL2.E2TB: Set to zero so that the trace Buffer
	 *  owning exception level is NS-EL1 and, tracing is
	 *  prohibited at NS-EL2. These bits are RES0 when
	 *  FEAT_TRBE is not implemented.
	 */
	write_mdcr_el2(read_mdcr_el2() & ~MDCR_EL2_E2TB(MDCR_EL2_E2TB_EL1));
}

static void *trbe_drain_trace_buffers_hook(const void *arg __unused)
{
	if (is_feat_trbe_supported()) {
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
