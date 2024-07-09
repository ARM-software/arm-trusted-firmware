/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>

#include <plat/common/platform.h>
#include <services/rmmd_svc.h>

#define MEMRESERVE_FLAG_LOCALCPU	(1U << 0)
#define MEMRESERVE_NR_FLAGS		1U
#define MEMRESERVE_FLAGS_MASK		GENMASK(MEMRESERVE_NR_FLAGS - 1, 0)

#define MEMRESERVE_ALIGNMENT_SHIFT	56U
#define MEMRESERVE_ALIGNMENT(x)		(1ULL << ((x) >> MEMRESERVE_ALIGNMENT_SHIFT))
#define MEMRESERVE_FLAGS(x)		((x) & GENMASK(MEMRESERVE_ALIGNMENT_SHIFT - 1, 0))

#pragma weak plat_rmmd_reserve_memory
uintptr_t plat_rmmd_reserve_memory(size_t size, unsigned long alignment)
{
	return 0;
}

int rmmd_reserve_memory(size_t size, uint64_t *arg)
{
	unsigned long alignment = MEMRESERVE_ALIGNMENT(*arg);
	uint64_t flags = MEMRESERVE_FLAGS(*arg);
	uintptr_t addr;

	VERBOSE("%s(%ld, 0x%lx);\n", __func__, size, *arg);

	/* Any unknown flag forces an error return. */
	if ((flags & ~MEMRESERVE_FLAGS_MASK) != 0U) {
		return E_RMM_INVAL;
	}

	addr = plat_rmmd_reserve_memory(size, alignment);
	if (addr == 0UL) {
		return E_RMM_NOMEM;
	}

	*arg = addr;

	return E_RMM_OK;
}
