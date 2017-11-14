/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SDEI_H__
#define __SDEI_H__

#include <spinlock.h>
#include <utils_def.h>

/* Range 0xC4000020 - 0xC400003F reserved for SDE 64bit smc calls */
#define SDEI_VERSION				0xC4000020
#define SDEI_EVENT_REGISTER			0xC4000021
#define SDEI_EVENT_ENABLE			0xC4000022
#define SDEI_EVENT_DISABLE			0xC4000023
#define SDEI_EVENT_CONTEXT			0xC4000024
#define SDEI_EVENT_COMPLETE			0xC4000025
#define SDEI_EVENT_COMPLETE_AND_RESUME		0xC4000026

#define SDEI_EVENT_UNREGISTER			0xC4000027
#define SDEI_EVENT_STATUS			0xC4000028
#define SDEI_EVENT_GET_INFO			0xC4000029
#define SDEI_EVENT_ROUTING_SET			0xC400002A
#define SDEI_PE_MASK				0xC400002B
#define SDEI_PE_UNMASK				0xC400002C

#define SDEI_INTERRUPT_BIND			0xC400002D
#define SDEI_INTERRUPT_RELEASE			0xC400002E
#define SDEI_EVENT_SIGNAL			0xC400002F
#define SDEI_FEATURES				0xC4000030
#define SDEI_PRIVATE_RESET			0xC4000031
#define SDEI_SHARED_RESET			0xC4000032

/* SDEI_EVENT_REGISTER flags */
#define SDEI_REGF_RM_ANY	0
#define SDEI_REGF_RM_PE		1

/* SDEI_EVENT_COMPLETE status flags */
#define SDEI_EV_HANDLED		0
#define SDEI_EV_FAILED		1

/* SDE event status values in bit position */
#define SDEI_STATF_REGISTERED		0
#define SDEI_STATF_ENABLED		1
#define SDEI_STATF_RUNNING		2

/* Internal: SDEI flag bit positions */
#define _SDEI_MAPF_DYNAMIC_SHIFT	1
#define _SDEI_MAPF_BOUND_SHIFT		2
#define _SDEI_MAPF_SIGNALABLE_SHIFT	3
#define _SDEI_MAPF_PRIVATE_SHIFT	4
#define _SDEI_MAPF_CRITICAL_SHIFT	5

/* SDEI event 0 */
#define SDEI_EVENT_0	0

/* Placeholder interrupt for dynamic mapping */
#define SDEI_DYN_IRQ	0

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
#define SDEI_MAPF_DYNAMIC	BIT(_SDEI_MAPF_DYNAMIC_SHIFT)
#define SDEI_MAPF_BOUND		BIT(_SDEI_MAPF_BOUND_SHIFT)

#define SDEI_MAPF_SIGNALABLE	BIT(_SDEI_MAPF_SIGNALABLE_SHIFT)
#define SDEI_MAPF_PRIVATE	BIT(_SDEI_MAPF_PRIVATE_SHIFT)
#define SDEI_MAPF_CRITICAL	BIT(_SDEI_MAPF_CRITICAL_SHIFT)

/* Indices of private and shared mappings */
#define _SDEI_MAP_IDX_PRIV	0
#define _SDEI_MAP_IDX_SHRD	1
#define _SDEI_MAP_IDX_MAX	2

/* The macros below are used to identify SDEI calls from the SMC function ID */
#define SDEI_FID_MASK		U(0xffe0)
#define SDEI_FID_VALUE		U(0x20)
#define is_sdei_fid(_fid) \
	((((_fid) & SDEI_FID_MASK) == SDEI_FID_VALUE) && \
	 (((_fid >> FUNCID_CC_SHIFT) & FUNCID_CC_MASK) == SMC_64))

#define SDEI_EVENT_MAP(_event, _intr, _flags) \
	{ \
		.ev_num = _event, \
		.intr = _intr, \
		.map_flags = _flags \
	}

#define SDEI_SHARED_EVENT(_event, _intr, _flags) \
	SDEI_EVENT_MAP(_event, _intr, _flags)

#define SDEI_PRIVATE_EVENT(_event, _intr, _flags) \
	SDEI_EVENT_MAP(_event, _intr, _flags | SDEI_MAPF_PRIVATE)

#define SDEI_DEFINE_EVENT_0(_intr) \
	SDEI_PRIVATE_EVENT(SDEI_EVENT_0, _intr, SDEI_MAPF_SIGNALABLE)

/*
 * Declare shared and private entries for each core. Also declare a global
 * structure containing private and share entries.
 *
 * This macro must be used in the same file as the platform SDEI mappings are
 * declared. Only then would ARRAY_SIZE() yield a meaningful value.
 */
#define REGISTER_SDEI_MAP(_private, _shared) \
	sdei_entry_t sdei_private_event_table \
		[PLATFORM_CORE_COUNT * ARRAY_SIZE(_private)]; \
	sdei_entry_t sdei_shared_event_table[ARRAY_SIZE(_shared)]; \
	const sdei_mapping_t sdei_global_mappings[] = { \
		[_SDEI_MAP_IDX_PRIV] = { \
			.map = _private, \
			.num_maps = ARRAY_SIZE(_private) \
		}, \
		[_SDEI_MAP_IDX_SHRD] = { \
			.map = _shared, \
			.num_maps = ARRAY_SIZE(_shared) \
		}, \
	}

typedef uint8_t sdei_state_t;

/* Runtime data of SDEI event */
typedef struct sdei_entry {
	uint64_t ep;		/* Entry point */
	uint64_t arg;		/* Entry point argument */
	uint64_t affinity;	/* Affinity of shared event */
	unsigned int reg_flags;	/* Registration flags */

	/* Event handler states: registered, enabled, running */
	sdei_state_t state;
} sdei_entry_t;

/* Mapping of SDEI events to interrupts, and associated data */
typedef struct sdei_ev_map {
	int32_t ev_num;		/* Event number */
	unsigned int intr;	/* Physical interrupt number for a bound map */
	unsigned int map_flags;	/* Mapping flags, see SDEI_MAPF_* */
	int reg_count;		/* Registration count */
	spinlock_t lock;	/* Per-event lock */
} sdei_ev_map_t;

typedef struct sdei_mapping {
	sdei_ev_map_t *map;
	size_t num_maps;
} sdei_mapping_t;

/* Handler to be called to handle SDEI smc calls */
uint64_t sdei_smc_handler(uint32_t smc_fid,
		uint64_t x1,
		uint64_t x2,
		uint64_t x3,
		uint64_t x4,
		void *cookie,
		void *handle,
		uint64_t flags);

void sdei_init(void);

/* Public API to dispatch an event to Normal world */
int sdei_dispatch_event(int ev_num, unsigned int preempted_sec_state);

#endif /* __SDEI_H__ */
