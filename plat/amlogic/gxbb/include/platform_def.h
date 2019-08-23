/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <lib/utils_def.h>

#include "../gxbb_def.h"

#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

/* Special value used to verify platform parameters from BL2 to BL31 */
#define GXBB_BL31_PLAT_PARAM_VAL	ULL(0x0F1E2D3C4B5A6978)

#define PLATFORM_STACK_SIZE		UL(0x1000)

#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(4)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CORE_COUNT		PLATFORM_CLUSTER0_CORE_COUNT

#define GXBB_PRIMARY_CPU		U(0)

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN		U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET		U(1)
/* Local power state for power-down. Valid for CPU and cluster power domains. */
#define PLAT_LOCAL_STATE_OFF		U(2)

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define PLAT_LOCAL_PSTATE_WIDTH		U(4)
#define PLAT_LOCAL_PSTATE_MASK		((U(1) << PLAT_LOCAL_PSTATE_WIDTH) - 1)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_SHIFT		U(6)
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

/* Memory-related defines */
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 32)

#define MAX_MMAP_REGIONS		12
#define MAX_XLAT_TABLES			5

#endif /* PLATFORM_DEF_H */
