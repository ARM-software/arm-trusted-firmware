/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/spinlock.h>
#include <plat/common/platform.h>

static spinlock_t mem_reserve_lock;
static uintptr_t top_mem = RMM_LIMIT;

uintptr_t plat_rmmd_reserve_memory(size_t size, unsigned long alignment)
{
	uint64_t align_mask = alignment - 1;
	uintptr_t addr;

	spin_lock(&mem_reserve_lock);
	addr = (top_mem - size) & ~align_mask;
	if (addr >= RMM_PAYLOAD_LIMIT) {
		top_mem = addr;
	} else {
		addr = 0;
	}
	spin_unlock(&mem_reserve_lock);

	return addr;
}
