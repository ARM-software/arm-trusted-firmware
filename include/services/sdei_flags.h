/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SDEI_FLAGS_H
#define SDEI_FLAGS_H

#include <lib/utils_def.h>

/* Internal: SDEI flag bit positions */
#define SDEI_MAPF_DYNAMIC_SHIFT_	1U
#define SDEI_MAPF_BOUND_SHIFT_		2U
#define SDEI_MAPF_SIGNALABLE_SHIFT_	3U
#define SDEI_MAPF_PRIVATE_SHIFT_	4U
#define SDEI_MAPF_CRITICAL_SHIFT_	5U
#define SDEI_MAPF_EXPLICIT_SHIFT_	6U

/* SDEI event 0 */
#define SDEI_EVENT_0	0

/* Placeholder interrupt for dynamic mapping */
#define SDEI_DYN_IRQ	0U

/* SDEI flags */

/*
 * These flags determine whether or not an event can be associated with an
 * interrupt. Static events are permanently associated with an interrupt, and
 * can't be changed at runtime.  Association of dynamic events with interrupts
 * can be changed at run time using the SDEI_INTERRUPT_BIND and
 * SDEI_INTERRUPT_RELEASE calls.
 *
 * SDEI_MAPF_DYNAMIC only indicates run time configurability, where as
 * SDEI_MAPF_BOUND indicates interrupt association. For example:
 *
 *  - Calling SDEI_INTERRUPT_BIND on a dynamic event will have both
 *    SDEI_MAPF_DYNAMIC and SDEI_MAPF_BOUND set.
 *
 *  - Statically-bound events will always have SDEI_MAPF_BOUND set, and neither
 *    SDEI_INTERRUPT_BIND nor SDEI_INTERRUPT_RELEASE can be called on them.
 *
 * See also the is_map_bound() macro.
 */
#define SDEI_MAPF_DYNAMIC	BIT(SDEI_MAPF_DYNAMIC_SHIFT_)
#define SDEI_MAPF_BOUND		BIT(SDEI_MAPF_BOUND_SHIFT_)
#define SDEI_MAPF_EXPLICIT	BIT(SDEI_MAPF_EXPLICIT_SHIFT_)

#define SDEI_MAPF_SIGNALABLE	BIT(SDEI_MAPF_SIGNALABLE_SHIFT_)
#define SDEI_MAPF_PRIVATE	BIT(SDEI_MAPF_PRIVATE_SHIFT_)

#define SDEI_MAPF_NORMAL	0
#define SDEI_MAPF_CRITICAL	BIT(SDEI_MAPF_CRITICAL_SHIFT_)

#endif /* SDEI_FLAGS_H */
