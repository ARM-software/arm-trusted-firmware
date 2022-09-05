/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_DEF_H
#define ARCH_DEF_H

/* Topology constants */
#define PLAT_MAX_PWR_LVL		(2)
#define PLAT_MAX_RET_STATE		(1)
#define PLAT_MAX_OFF_STATE		(2)

#define PLATFORM_SYSTEM_COUNT		(1)
#define PLATFORM_CLUSTER_COUNT		(1)
#define PLATFORM_CLUSTER0_CORE_COUNT	(8)
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	(8)
#define PLATFORM_NUM_AFFS		(PLATFORM_SYSTEM_COUNT + \
					 PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
/* Cachline size */
#define CACHE_WRITEBACK_SHIFT		(6)
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#endif /* ARCH_DEF_H */

