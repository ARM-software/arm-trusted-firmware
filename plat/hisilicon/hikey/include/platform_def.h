/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#include <hikey_def.h>
#include <hikey_layout.h>		/* BL memory region sizes, etc */

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define HIKEY_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

/*
 * Generic platform constants
 */

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE		0x1000

#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

#define PLATFORM_CACHE_LINE_SIZE	64
#define PLATFORM_CLUSTER_COUNT		2
#define PLATFORM_CORE_COUNT_PER_CLUSTER	4
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT *	\
					 PLATFORM_CORE_COUNT_PER_CLUSTER)
#define PLAT_MAX_PWR_LVL		(MPIDR_AFFLVL2)
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CORE_COUNT + \
					 PLATFORM_CLUSTER_COUNT + 1)

#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4
/* eMMC RPMB and eMMC User Data */
#define MAX_IO_BLOCK_DEVICES		U(2)

/* GIC related constants (no GICR in GIC-400) */
#define PLAT_ARM_GICD_BASE		0xF6801000
#define PLAT_ARM_GICC_BASE		0xF6802000
#define PLAT_ARM_GICH_BASE		0xF6804000
#define PLAT_ARM_GICV_BASE		0xF6806000

/*
 * Platform specific page table and MMU setup constants
 */
#define PLAT_VIRT_ADDR_SPACE_SIZE   (1ULL << 32)
#define PLAT_PHY_ADDR_SPACE_SIZE    (1ULL << 32)

#if defined(IMAGE_BL1) || defined(IMAGE_BL32)
#define MAX_XLAT_TABLES			3
#endif

#ifdef IMAGE_BL31
#define MAX_XLAT_TABLES			4
#endif

#ifdef IMAGE_BL2
#define MAX_XLAT_TABLES			4
#endif

#define MAX_MMAP_REGIONS		16

/*
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 */
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#endif /* PLATFORM_DEF_H */
