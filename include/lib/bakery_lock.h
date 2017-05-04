/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BAKERY_LOCK_H__
#define __BAKERY_LOCK_H__

#include <platform_def.h>

#define BAKERY_LOCK_MAX_CPUS		PLATFORM_CORE_COUNT

#ifndef __ASSEMBLY__
#include <stdint.h>

/*****************************************************************************
 * Internal helper macros used by the bakery lock implementation.
 ****************************************************************************/
/* Convert a ticket to priority */
#define PRIORITY(t, pos)	(((t) << 8) | (pos))

#define CHOOSING_TICKET		0x1
#define CHOSEN_TICKET		0x0

#define bakery_is_choosing(info)	(info & 0x1)
#define bakery_ticket_number(info)	((info >> 1) & 0x7FFF)
#define make_bakery_data(choosing, number) \
		(((choosing & 0x1) | (number << 1)) & 0xFFFF)

/*****************************************************************************
 * External bakery lock interface.
 ****************************************************************************/
#if USE_COHERENT_MEM
/*
 * Bakery locks are stored in coherent memory
 *
 * Each lock's data is contiguous and fully allocated by the compiler
 */

typedef struct bakery_lock {
	/*
	 * The lock_data is a bit-field of 2 members:
	 * Bit[0]       : choosing. This field is set when the CPU is
	 *                choosing its bakery number.
	 * Bits[1 - 15] : number. This is the bakery number allocated.
	 */
	volatile uint16_t lock_data[BAKERY_LOCK_MAX_CPUS];
} bakery_lock_t;

#else
/*
 * Bakery locks are stored in normal .bss memory
 *
 * Each lock's data is spread across multiple cache lines, one per CPU,
 * but multiple locks can share the same cache line.
 * The compiler will allocate enough memory for one CPU's bakery locks,
 * the remaining cache lines are allocated by the linker script
 */

typedef struct bakery_info {
	/*
	 * The lock_data is a bit-field of 2 members:
	 * Bit[0]       : choosing. This field is set when the CPU is
	 *                choosing its bakery number.
	 * Bits[1 - 15] : number. This is the bakery number allocated.
	 */
	volatile uint16_t lock_data;
} bakery_info_t;

typedef bakery_info_t bakery_lock_t;

#endif /* __USE_COHERENT_MEM__ */

static inline void bakery_lock_init(bakery_lock_t *bakery) {}
void bakery_lock_get(bakery_lock_t *bakery);
void bakery_lock_release(bakery_lock_t *bakery);

#define DEFINE_BAKERY_LOCK(_name) bakery_lock_t _name __section("bakery_lock")

#define DECLARE_BAKERY_LOCK(_name) extern bakery_lock_t _name


#endif /* __ASSEMBLY__ */
#endif /* __BAKERY_LOCK_H__ */
