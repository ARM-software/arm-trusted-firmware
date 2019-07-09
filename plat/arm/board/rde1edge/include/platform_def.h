/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>

#include <sgi_base_platform_def.h>

#define PLAT_ARM_CLUSTER_COUNT		2
#define CSS_SGI_MAX_CPUS_PER_CLUSTER	8
#define CSS_SGI_MAX_PE_PER_CPU		2

#define PLAT_CSS_MHU_BASE		UL(0x45400000)
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

/* Base address of DMC-620 instances */
#define RDE1EDGE_DMC620_BASE0		UL(0x4e000000)
#define RDE1EDGE_DMC620_BASE1		UL(0x4e100000)

#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL2

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL3

/*
 * Physical and virtual address space limits for MMU in AARCH64 & AARCH32 modes
 */
#ifdef __aarch64__
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 36)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 36)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#endif

#endif /* PLATFORM_DEF_H */
