/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#define PLATFORM_STACK_SIZE		0x1000

#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << (CACHE_WRITEBACK_SHIFT))

/* topology */
#define UNIPHIER_MAX_CPUS_PER_CLUSTER	U(4)
#define UNIPHIER_CLUSTER_COUNT		U(2)

#define PLATFORM_CORE_COUNT		\
	((UNIPHIER_MAX_CPUS_PER_CLUSTER) * (UNIPHIER_CLUSTER_COUNT))

#define PLAT_MAX_PWR_LVL		U(1)

#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)

#define UNIPHIER_BL2_OFFSET		UL(0x00000000)
#define UNIPHIER_BL2_MAX_SIZE		UL(0x00080000)

/* 0x00080000-0x01000000: reserved for DSP */

#define UNIPHIER_BL31_OFFSET		UL(0x01000000)
#define UNIPHIER_BL31_MAX_SIZE		UL(0x00080000)

#define UNIPHIER_BL32_OFFSET		UL(0x01080000)
#define UNIPHIER_BL32_MAX_SIZE		UL(0x00100000)

/*
 * The link addresses are determined by UNIPHIER_MEM_BASE + offset.
 * When ENABLE_PIE is set, all the TF images can be loaded anywhere, so
 * UNIPHIER_MEM_BASE is arbitrary.
 *
 * When ENABLE_PIE is unset, UNIPHIER_MEM_BASE should be chosen so that
 * BL2_BASE matches to the physical address where BL2 is loaded, that is,
 * UNIPHIER_MEM_BASE should be the base address of the DRAM region.
 */
#define UNIPHIER_MEM_BASE		UL(0x00000000)

#define BL2_BASE		(UNIPHIER_MEM_BASE + UNIPHIER_BL2_OFFSET)
#define BL2_LIMIT		(BL2_BASE + UNIPHIER_BL2_MAX_SIZE)

#define BL31_BASE		(UNIPHIER_MEM_BASE + UNIPHIER_BL31_OFFSET)
#define BL31_LIMIT		(BL31_BASE + UNIPHIER_BL31_MAX_SIZE)

#define BL32_BASE		(UNIPHIER_MEM_BASE + UNIPHIER_BL32_OFFSET)
#define BL32_LIMIT		(BL32_BASE + UNIPHIER_BL32_MAX_SIZE)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)

#define MAX_XLAT_TABLES			9
#define MAX_MMAP_REGIONS		13

#define MAX_IO_HANDLES			2
#define MAX_IO_DEVICES			2
#define MAX_IO_BLOCK_DEVICES		U(1)

#define TSP_SEC_MEM_BASE		(BL32_BASE)
#define TSP_SEC_MEM_SIZE		((BL32_LIMIT) - (BL32_BASE))
#define TSP_IRQ_SEC_PHY_TIMER		29

#endif /* PLATFORM_DEF_H */
