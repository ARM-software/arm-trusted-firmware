/*
 * Copyright (c) 2026, BayLibre SAS
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <plat/common/plat_hold_pen.h>

/*
 * Initialise the hold pen by writing HOLD_STATE_WAIT and magic tags
 * to every slot and flushing to main memory. The entry field is
 * written first so it is safe before the magic tags make the slot
 * visible to polling secondaries. This must be called once during
 * boot (e.g. from plat_setup_psci_ops) before any secondary CPU
 * is released.
 */
void plat_hold_pen_init(struct hold_slot *hold_pen, unsigned int core_count)
{
	for (unsigned int i = 0U; i < core_count; i++) {
		hold_pen[i].entry  = HOLD_STATE_WAIT;
		/*
		 * Ensure the entry value is committed before the magic
		 * tags that make this slot visible to polling secondaries.
		 * Not strictly necessary since the subsequent flush
		 * pushes the whole cache line at once, but provides
		 * defence in depth against reordering.
		 */
		dmbish();
		hold_pen[i].magic1 = HOLD_BOOT_MAGIC1;
		hold_pen[i].magic2 = HOLD_BOOT_MAGIC2;
	}

	flush_dcache_range((uintptr_t)hold_pen,
			   core_count * sizeof(struct hold_slot));
}

/*
 * Signal a secondary core to branch to the given entrypoint.
 * Only the target slot is written and flushed.
 */
void plat_hold_pen_signal(struct hold_slot *hold_pen, unsigned int core_idx,
			  uintptr_t entry_point)
{
	hold_pen[core_idx].entry = entry_point;

	flush_dcache_range((uintptr_t)&hold_pen[core_idx],
			   sizeof(struct hold_slot));

	/* Wake secondaries out of their WFE polling loop. */
	sev();
}
