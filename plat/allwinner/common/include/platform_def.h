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

#include <sunxi_mmap.h>

#ifdef SUNXI_BL31_IN_DRAM

#define BL31_BASE			SUNXI_DRAM_BASE
#define BL31_LIMIT			(SUNXI_DRAM_BASE + 0x40000)

#define MAX_XLAT_TABLES			4
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)

#define SUNXI_BL33_VIRT_BASE		PRELOADED_BL33_BASE

#else	/* !SUNXI_BL31_IN_DRAM */

#define BL31_BASE			(SUNXI_SRAM_A2_BASE + \
					 SUNXI_SRAM_A2_BL31_OFFSET)
#define BL31_LIMIT			(SUNXI_SRAM_A2_BASE + \
					 SUNXI_SRAM_A2_SIZE - SUNXI_SCP_SIZE)

/* Overwrite U-Boot SPL, but reserve the first page for the SPL header. */
#define BL31_NOBITS_BASE		(SUNXI_SRAM_A1_BASE + 0x1000)
#define BL31_NOBITS_LIMIT		(SUNXI_SRAM_A1_BASE + SUNXI_SRAM_A1_SIZE)

#define MAX_XLAT_TABLES			1
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 28)

#define SUNXI_BL33_VIRT_BASE		SUNXI_DRAM_VIRT_BASE

/* The SCP firmware is allocated the last 16KiB of SRAM A2. */
#define SUNXI_SCP_BASE			BL31_LIMIT
#define SUNXI_SCP_SIZE			0x4000

#endif /* SUNXI_BL31_IN_DRAM */

/* How much DRAM to map (to map BL33, for fetching the DTB from U-Boot) */
#define SUNXI_DRAM_MAP_SIZE		(64U << 20)

#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#define MAX_STATIC_MMAP_REGIONS		3
#define MAX_MMAP_REGIONS		(5 + MAX_STATIC_MMAP_REGIONS)

#define PLAT_CSS_SCP_COM_SHARED_MEM_BASE \
	(SUNXI_SRAM_A2_BASE + SUNXI_SRAM_A2_SIZE - 0x200)

/* These states are used directly for SCPI communication. */
#define PLAT_MAX_PWR_LVL_STATES		U(3)
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(3)

#define PLAT_MAX_PWR_LVL		U(2)
#define PLAT_NUM_PWR_DOMAINS		(U(1) + \
					 PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)

#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
					 PLATFORM_MAX_CPUS_PER_CLUSTER)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(4)
#define PLATFORM_STACK_SIZE		(0x1000 / PLATFORM_CORE_COUNT)

#ifndef SPD_none
#ifndef BL32_BASE
#define BL32_BASE			SUNXI_DRAM_BASE
#endif
#endif

#endif /* PLATFORM_DEF_H */
