/*
 * Copyright (c) 2020-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <sgi_soc_platform_def.h>

#define PLAT_ARM_CLUSTER_COUNT		U(4)
#define CSS_SGI_MAX_CPUS_PER_CLUSTER	U(1)
#define CSS_SGI_MAX_PE_PER_CPU		U(1)

#define PLAT_CSS_MHU_BASE		UL(0x45400000)
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

/* TZC Related Constants */
#define PLAT_ARM_TZC_BASE		UL(0x21830000)
#define TZC400_BASE(n)			(PLAT_ARM_TZC_BASE + \
					 (n * TZC400_OFFSET))
#define TZC400_OFFSET			UL(0x1000000)
#define TZC400_COUNT			U(8)
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT(0)

#define TZC_NSAID_ALL_AP		U(0)
#define TZC_NSAID_PCI			U(1)
#define TZC_NSAID_HDLCD0		U(2)
#define TZC_NSAID_CLCD			U(7)
#define TZC_NSAID_AP			U(9)
#define TZC_NSAID_VIRTIO		U(15)

#define PLAT_ARM_TZC_NS_DEV_ACCESS	\
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_ALL_AP)) | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_HDLCD0)) | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_PCI))    | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_AP))     | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_CLCD))   | \
		(TZC_REGION_ACCESS_RDWR(TZC_NSAID_VIRTIO))

/* Virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xC0000000)

/* Remote chip address offset (4TB per chip) */
#define CSS_SGI_ADDR_BITS_PER_CHIP	U(42)

/* Physical and virtual address space limits for MMU in AARCH64 mode */
#define PLAT_PHY_ADDR_SPACE_SIZE	CSS_SGI_REMOTE_CHIP_MEM_OFFSET( \
						CSS_SGI_CHIP_COUNT)
#define PLAT_VIRT_ADDR_SPACE_SIZE	CSS_SGI_REMOTE_CHIP_MEM_OFFSET( \
						CSS_SGI_CHIP_COUNT)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		UL(0x30000000)
#define PLAT_ARM_GICC_BASE		UL(0x2C000000)
#define PLAT_ARM_GICR_BASE		UL(0x30140000)

#endif /* PLATFORM_DEF_H */
