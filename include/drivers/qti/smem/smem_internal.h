/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_INTERNAL_H
#define SMEM_INTERNAL_H

#include "smem.h"
#include "smem_version.h"

enum smem_state {
	SMEM_STATE_UNINITIALIZED, /* shared memory original state */
	SMEM_STATE_INITIALIZED /* shared memory is initialized */
};

struct smem_funcs {
	void *(*get_addr)(enum smem_mem_type smem_type, uint32_t *buf_size);
};

struct smem_heap_info {
	uint32_t initialized;
	uint32_t free_offset;
	uint32_t heap_remaining;
	uint32_t reserved;
};

/*
 * Static allocations (SHOULD NOT CHANGES ORDER OF MEMBERS)
 */
struct smem_static_allocs {
	uint32_t proc_comm[16];
	uint32_t version[SMEM_VERSION_INFO_SIZE];
	struct smem_heap_info heap_info;

	/* Remaining static buffers not needed */
};

struct smem_info {
	uint8_t *smem_base_addr;
	uint32_t smem_size;
	uint32_t version;
	uint16_t max_items;
	/*
	 * This SMEM Host ID
	 */
	uint16_t this_host;
	uint8_t version_offset;
	/*
	 * Allocation and get address function pointers.
	 * Based on the SMEM version these function pointers gets updated and used.
	 */
	const struct smem_funcs *funcs;
	enum smem_state state;
};

extern struct smem_info smem_info;

#endif /* SMEM_INTERNAL_H */
