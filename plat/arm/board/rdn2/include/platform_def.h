/*
 * Copyright (c) 2020-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>

#include <sgi_soc_platform_def_v2.h>

#if (CSS_SGI_PLATFORM_VARIANT == 1)
#define PLAT_ARM_CLUSTER_COUNT		U(8)
#elif (CSS_SGI_PLATFORM_VARIANT == 2)
#define PLAT_ARM_CLUSTER_COUNT		U(4)
#else
#define PLAT_ARM_CLUSTER_COUNT		U(16)
#endif

#define CSS_SGI_MAX_CPUS_PER_CLUSTER	U(1)
#define CSS_SGI_MAX_PE_PER_CPU		U(1)

#define PLAT_CSS_MHU_BASE		UL(0x2A920000)
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

/* TZC Related Constants */
#define PLAT_ARM_TZC_BASE		UL(0x10720000)
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT(0)

#define TZC400_OFFSET			UL(0x1000000)

#if (CSS_SGI_PLATFORM_VARIANT == 1)
#define TZC400_COUNT			U(2)
#elif (CSS_SGI_PLATFORM_VARIANT == 2)
#define TZC400_COUNT			U(4)
#else
#define TZC400_COUNT			U(8)
#endif

#define TZC400_BASE(n)			(PLAT_ARM_TZC_BASE + \
						(n * TZC400_OFFSET))

#define TZC_NSAID_ALL_AP		U(0)
#define TZC_NSAID_PCI			U(1)
#define TZC_NSAID_HDLCD0		U(2)
#define TZC_NSAID_DMA			U(5)
#define TZC_NSAID_DMA2			U(8)
#define TZC_NSAID_CLCD			U(7)
#define TZC_NSAID_AP			U(9)
#define TZC_NSAID_VIRTIO		U(15)

#define PLAT_ARM_TZC_NS_DEV_ACCESS	\
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_ALL_AP)) | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_HDLCD0)) | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_PCI))    | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_DMA))    | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_DMA2))   | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_AP))     | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_CLCD))   | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_VIRTIO))

/*
 * Physical and virtual address space limits for MMU in AARCH64 & AARCH32 modes
 */
#ifdef __aarch64__
#if (CSS_SGI_PLATFORM_VARIANT == 2)
#define PLAT_PHY_ADDR_SPACE_SIZE	CSS_SGI_REMOTE_CHIP_MEM_OFFSET( \
						CSS_SGI_CHIP_COUNT)
#define PLAT_VIRT_ADDR_SPACE_SIZE	CSS_SGI_REMOTE_CHIP_MEM_OFFSET( \
						CSS_SGI_CHIP_COUNT)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 42)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 42)
#endif
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#endif

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		UL(0x30000000)
#define PLAT_ARM_GICC_BASE		UL(0x2C000000)

/* Virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xC0000000)

#if (CSS_SGI_PLATFORM_VARIANT == 1)
#define PLAT_ARM_GICR_BASE		UL(0x30100000)
#else
#define PLAT_ARM_GICR_BASE		UL(0x301C0000)
#endif

#endif /* PLATFORM_DEF_H */
