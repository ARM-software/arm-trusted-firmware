/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <sgi_base_platform_def.h>

#define PLAT_ARM_CLUSTER_COUNT		U(4)
#define CSS_SGI_MAX_CPUS_PER_CLUSTER	U(1)
#define CSS_SGI_MAX_PE_PER_CPU		U(1)

#define PLAT_CSS_MHU_BASE		UL(0x45400000)
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

/* Virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xC0000000)

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
