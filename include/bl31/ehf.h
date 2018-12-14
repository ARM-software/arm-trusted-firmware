/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EHF_H
#define EHF_H

#ifndef __ASSEMBLY__

#include <cdefs.h>
#include <stdint.h>

#include <lib/utils_def.h>

/* Valid priorities set bit 0 of the priority handler. */
#define EHF_PRI_VALID_	BIT(0)

/* Marker for no handler registered for a valid priority */
#define EHF_NO_HANDLER_	(0U | EHF_PRI_VALID_)

/* Extract the specified number of top bits from 7 lower bits of priority */
#define EHF_PRI_TO_IDX(pri, plat_bits) \
	((((unsigned) (pri)) & 0x7fu) >> (7u - (plat_bits)))

/* Install exception priority descriptor at a suitable index */
#define EHF_PRI_DESC(plat_bits, priority) \
	[EHF_PRI_TO_IDX(priority, plat_bits)] = { \
		.ehf_handler = EHF_NO_HANDLER_, \
	}

/* Macro for platforms to regiter its exception priorities */
#define EHF_REGISTER_PRIORITIES(priorities, num, bits) \
	const ehf_priorities_t exception_data = { \
		.num_priorities = (num), \
		.ehf_priorities = (priorities), \
		.pri_bits = (bits), \
	}

/*
 * Priority stack, managed as a bitmap.
 *
 * Currently only supports 32 priority levels, allowing platforms to use up to 5
 * top bits of priority. But the type can be changed to uint64_t should need
 * arise to support 64 priority levels, allowing platforms to use up to 6 top
 * bits of priority.
 */
typedef uint32_t ehf_pri_bits_t;

/*
 * Per-PE exception data. The data for each PE is kept as a per-CPU data field.
 * See cpu_data.h.
 */
typedef struct {
	ehf_pri_bits_t active_pri_bits;

	/* Priority mask value before any priority levels were active */
	uint8_t init_pri_mask;

	/* Non-secure priority mask value stashed during Secure execution */
	uint8_t ns_pri_mask;
} __aligned(sizeof(uint64_t)) pe_exc_data_t;

typedef int (*ehf_handler_t)(uint32_t intr_raw, uint32_t flags, void *handle,
		void *cookie);

typedef struct ehf_pri_desc {
	/*
	 * 4-byte-aligned exception handler. Bit 0 indicates the corresponding
	 * priority level is valid. This is effectively of ehf_handler_t type,
	 * but left as uintptr_t in order to make pointer arithmetic convenient.
	 */
	uintptr_t ehf_handler;
} ehf_pri_desc_t;

typedef struct ehf_priority_type {
	ehf_pri_desc_t *ehf_priorities;
	unsigned int num_priorities;
	unsigned int pri_bits;
} ehf_priorities_t;

void ehf_init(void);
void ehf_activate_priority(unsigned int priority);
void ehf_deactivate_priority(unsigned int priority);
void ehf_register_priority_handler(unsigned int pri, ehf_handler_t handler);
void ehf_allow_ns_preemption(uint64_t preempt_ret_code);
unsigned int ehf_is_ns_preemption_allowed(void);

#endif /* __ASSEMBLY__ */

#endif /* EHF_H */
