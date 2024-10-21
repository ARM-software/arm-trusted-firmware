/*
 * Copyright (c) 2024, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_DEF_H
#define ARCH_DEF_H

#include <arch.h>

/* Topology constants */
#ifndef PLAT_MAX_PWR_LVL
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#endif
#define PLAT_MAX_RET_STATE		MPIDR_AFFLVL1

#ifndef PLAT_MAX_OFF_STATE
#define PLAT_MAX_OFF_STATE		MPIDR_AFFLVL2
#endif

#define PLATFORM_SYSTEM_COUNT         1
#define PLATFORM_CLUSTER_COUNT        1
#define PLATFORM_CLUSTER0_CORE_COUNT  8
#define PLATFORM_CORE_COUNT	(PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	8
#define PLATFORM_NUM_AFFS		(PLATFORM_SYSTEM_COUNT +	\
					 PLATFORM_CLUSTER_COUNT +	\
					 PLATFORM_CORE_COUNT)

/* Cachline size */
#define CACHE_WRITEBACK_SHIFT	6
#define CACHE_WRITEBACK_GRANULE	(1 << CACHE_WRITEBACK_SHIFT)
#endif /* ARCH_DEF_H */
