/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SDEI_H
#define SDEI_H

#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <services/sdei_flags.h>

/* Range 0xC4000020 - 0xC400003F reserved for SDE 64bit smc calls */
#define SDEI_VERSION				0xC4000020U
#define SDEI_EVENT_REGISTER			0xC4000021U
#define SDEI_EVENT_ENABLE			0xC4000022U
#define SDEI_EVENT_DISABLE			0xC4000023U
#define SDEI_EVENT_CONTEXT			0xC4000024U
#define SDEI_EVENT_COMPLETE			0xC4000025U
#define SDEI_EVENT_COMPLETE_AND_RESUME		0xC4000026U

#define SDEI_EVENT_UNREGISTER			0xC4000027U
#define SDEI_EVENT_STATUS			0xC4000028U
#define SDEI_EVENT_GET_INFO			0xC4000029U
#define SDEI_EVENT_ROUTING_SET			0xC400002AU
#define SDEI_PE_MASK				0xC400002BU
#define SDEI_PE_UNMASK				0xC400002CU

#define SDEI_INTERRUPT_BIND			0xC400002DU
#define SDEI_INTERRUPT_RELEASE			0xC400002EU
#define SDEI_EVENT_SIGNAL			0xC400002FU
#define SDEI_FEATURES				0xC4000030U
#define SDEI_PRIVATE_RESET			0xC4000031U
#define SDEI_SHARED_RESET			0xC4000032U

/* SDEI_EVENT_REGISTER flags */
#define SDEI_REGF_RM_ANY	0ULL
#define SDEI_REGF_RM_PE		1ULL

/* SDEI_EVENT_COMPLETE status flags */
#define SDEI_EV_HANDLED		0U
#define SDEI_EV_FAILED		1U

/* Indices of private and shared mappings */
#define SDEI_MAP_IDX_PRIV_	0U
#define SDEI_MAP_IDX_SHRD_	1U
#define SDEI_MAP_IDX_MAX_	2U

/* The macros below are used to identify SDEI calls from the SMC function ID */
#define SDEI_FID_MASK		U(0xffe0)
#define SDEI_FID_VALUE		U(0x20)
#define is_sdei_fid(_fid) \
	((((_fid) & SDEI_FID_MASK) == SDEI_FID_VALUE) && \
	 (((_fid >> FUNCID_CC_SHIFT) & FUNCID_CC_MASK) == SMC_64))

#define SDEI_EVENT_MAP(_event, _intr, _flags) \
	{ \
		.ev_num = (_event), \
		.intr = (_intr), \
		.map_flags = (_flags) \
	}

#define SDEI_SHARED_EVENT(_event, _intr, _flags) \
	SDEI_EVENT_MAP(_event, _intr, _flags)

#define SDEI_PRIVATE_EVENT(_event, _intr, _flags) \
	SDEI_EVENT_MAP(_event, _intr, (_flags) | SDEI_MAPF_PRIVATE)

#define SDEI_DEFINE_EVENT_0(_intr) \
	SDEI_PRIVATE_EVENT(SDEI_EVENT_0, (_intr), SDEI_MAPF_SIGNALABLE)

#define SDEI_EXPLICIT_EVENT(_event, _pri) \
	SDEI_EVENT_MAP((_event), 0, (_pri) | SDEI_MAPF_EXPLICIT | SDEI_MAPF_PRIVATE)

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
		[SDEI_MAP_IDX_PRIV_] = { \
			.map = (_private), \
			.num_maps = ARRAY_SIZE(_private) \
		}, \
		[SDEI_MAP_IDX_SHRD_] = { \
			.map = (_shared), \
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
int sdei_dispatch_event(int ev_num);

#endif /* SDEI_H */
