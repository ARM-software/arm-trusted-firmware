/*
 * Copyright (c) 2026, BayLibre SAS
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_HOLD_PEN_H
#define PLAT_HOLD_PEN_H

#ifndef __ASSEMBLER__
#include <cdefs.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/cassert.h>
#endif

#include <lib/cpus/cpu_ops.h>
#include <lib/utils_def.h>
#include <platform_def.h>

/*
 * Generic hold pen for SMP secondary CPU bring-up.
 *
 * Platforms call plat_hold_pen_init() once at boot to write
 * HOLD_STATE_WAIT followed by magic tags into every slot, then flush
 * to main memory.
 *
 * The primary CPU signals a secondary by calling plat_hold_pen_signal(),
 * which writes the warm boot entrypoint into the target slot,
 * flushes it, and issues SEV.
 *
 * Secondary CPUs may reach their polling loop before plat_hold_pen_init()
 * has run (they diverge in el3_entrypoint_common before C runtime
 * init). To guard against branching to stale SRAM content, each
 * slot carries two magic tags that must both match before the entry
 * field is considered. Random memory is unlikely to contain both
 * magic values in sequence. The entry field is written before the
 * magic tags during init so that a valid entry value is always in
 * place before the tags make the slot "live".
 *
 * Before branching, the secondary writes HOLD_STATE_WAIT back into
 * its entry field so that a subsequent warm boot re-entering the
 * polling loop does not see a stale entrypoint.
 *
 * Each slot is cache-line aligned so that flushing one core's slot
 * cannot accidentally evict or corrupt another's.
 */

#define HOLD_BOOT_MAGIC1	UL(0xCAFECAFE)
#define HOLD_BOOT_MAGIC2	UL(0xBEEFBEEF)

/*
 * Magic values for suspend/resume.
 *
 * Platforms that support suspend/resume may use these tags to mark
 * suspend-related hold-pen states and distinguish suspend-related hold-pen
 * states from the normal boot bring-up path.
 */
#define HOLD_SUSPEND_MAGIC1	UL(0xBA5EBA11)
#define HOLD_SUSPEND_MAGIC2	UL(0xFEEDF00D)

/*
 * All-ones sentinel: no valid entrypoint can live here.
 * Evaluates to 0xffffffff on AArch32 and 0xffffffffffffffff on AArch64.
 */
#define HOLD_STATE_WAIT		(~UL(0))

#define HOLD_SLOT_SIZE		CACHE_WRITEBACK_GRANULE

/*
 * Field sizes and offsets within struct hold_slot, following the
 * pattern established in include/lib/cpus/cpu_ops.h.
 */
#define HOLD_SLOT_ENTRY_SIZE	CPU_WORD_SIZE
#define HOLD_SLOT_MAGIC1_SIZE	CPU_WORD_SIZE
#define HOLD_SLOT_MAGIC2_SIZE	CPU_WORD_SIZE

#define HOLD_SLOT_ENTRY		0
#define HOLD_SLOT_MAGIC1	HOLD_SLOT_ENTRY + HOLD_SLOT_ENTRY_SIZE
#define HOLD_SLOT_MAGIC2	HOLD_SLOT_MAGIC1 + HOLD_SLOT_MAGIC1_SIZE

#ifndef __ASSEMBLER__

/*
 * Per-core hold pen slot, cache-line aligned.
 * The 'entry' field holds HOLD_STATE_WAIT while the core should keep
 * polling, or the warm boot entrypoint address when it should go.
 */
struct hold_slot {
	uintptr_t entry;
	uintptr_t magic1;
	uintptr_t magic2;
	/* Padded to cache line boundary by __aligned. */
} __aligned(HOLD_SLOT_SIZE);

CASSERT(sizeof(struct hold_slot) == HOLD_SLOT_SIZE,
	hold_slot_not_cacheline_sized);
CASSERT(offsetof(struct hold_slot, entry) == HOLD_SLOT_ENTRY,
	hold_slot_entry_off_mismatch);
CASSERT(offsetof(struct hold_slot, magic1) == HOLD_SLOT_MAGIC1,
	hold_slot_magic1_off_mismatch);
CASSERT(offsetof(struct hold_slot, magic2) == HOLD_SLOT_MAGIC2,
	hold_slot_magic2_off_mismatch);

void plat_hold_pen_init(struct hold_slot *hold_pen, unsigned int core_count);
void plat_hold_pen_signal(struct hold_slot *hold_pen, unsigned int core_idx,
			  uintptr_t entry_point);

#endif /* __ASSEMBLER__ */

#endif /* PLAT_HOLD_PEN_H */
