/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <sgm_base_platform_def.h>

#define PLAT_MAX_CPUS_PER_CLUSTER	U(8)
#define PLAT_MAX_PE_PER_CPU		U(1)

#define PLAT_ARM_DRAM2_BASE		ULL(0x880000000)
#define PLAT_ARM_DRAM2_SIZE		ULL(0x180000000)

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
