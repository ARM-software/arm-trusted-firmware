/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <pubsub.h>

/*
 * The assembler does not yet understand the psb csync mnemonic
 * so use the equivalent hint instruction.
 */
#define psb_csync()	asm volatile("hint #17")

void spe_enable(int el2_unused)
{
	uint64_t features;

	features = read_id_aa64dfr0_el1() >> ID_AA64DFR0_PMS_SHIFT;
	if ((features & ID_AA64DFR0_PMS_MASK) == 1) {
		uint64_t v;

		if (el2_unused) {
			/*
			 * MDCR_EL2.TPMS (ARM v8.2): Do not trap statistical
			 * profiling controls to EL2.
			 *
			 * MDCR_EL2.E2PB (ARM v8.2): SPE enabled in Non-secure
			 * state. Accesses to profiling buffer controls at
			 * Non-secure EL1 are not trapped to EL2.
			 */
			v = read_mdcr_el2();
			v &= ~MDCR_EL2_TPMS;
			v |= MDCR_EL2_E2PB(MDCR_EL2_E2PB_EL1);
			write_mdcr_el2(v);
		}

		/*
		 * MDCR_EL2.NSPB (ARM v8.2): SPE enabled in Non-secure state
		 * and disabled in secure state. Accesses to SPE registers at
		 * S-EL1 generate trap exceptions to EL3.
		 */
		v = read_mdcr_el3();
		v |= MDCR_NSPB(MDCR_NSPB_EL1);
		write_mdcr_el3(v);
	}
}

void spe_disable(void)
{
	uint64_t features;

	features = read_id_aa64dfr0_el1() >> ID_AA64DFR0_PMS_SHIFT;
	if ((features & ID_AA64DFR0_PMS_MASK) == 1) {
		uint64_t v;

		/* Drain buffered data */
		psb_csync();
		dsbnsh();

		/* Disable profiling buffer */
		v = read_pmblimitr_el1();
		v &= ~(1ULL << 0);
		write_pmblimitr_el1(v);
		isb();
	}
}

static void *spe_drain_buffers_hook(const void *arg)
{
	uint64_t features;

	features = read_id_aa64dfr0_el1() >> ID_AA64DFR0_PMS_SHIFT;
	if ((features & ID_AA64DFR0_PMS_MASK) == 1) {
		/* Drain buffered data */
		psb_csync();
		dsbnsh();
	}

	return 0;
}

SUBSCRIBE_TO_EVENT(cm_entering_secure_world, spe_drain_buffers_hook);
