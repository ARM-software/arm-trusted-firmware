/*
 * Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <nrd_css_fw_def1.h>
#include <nrd_plat_arm_def1.h>
#include <nrd_ros_fw_def1.h>
#include <nrd_sdei.h>
#include <nrd_soc_platform_def.h>

/* Remote chip address offset */
#define NRD_REMOTE_CHIP_MEM_OFFSET(n)	\
		((ULL(1) << NRD_ADDR_BITS_PER_CHIP) * (n))

#define PLAT_ARM_CLUSTER_COUNT		U(2)
#define NRD_MAX_CPUS_PER_CLUSTER	U(4)
#define NRD_MAX_PE_PER_CPU		U(1)

#define PLAT_CSS_MHU_BASE		UL(0x45000000)

/* Base address of DMC-620 instances */
#define SGI575_DMC620_BASE0		UL(0x4e000000)
#define SGI575_DMC620_BASE1		UL(0x4e100000)

/* System power domain level */
#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2

#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

/* Maximum number of address bits used per chip */
#define NRD_ADDR_BITS_PER_CHIP	U(36)

/*
 * Physical and virtual address space limits for MMU in AARCH64 & AARCH32 modes
 */
#ifdef __aarch64__
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << NRD_ADDR_BITS_PER_CHIP)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << NRD_ADDR_BITS_PER_CHIP)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#endif

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		UL(0x30000000)
#define PLAT_ARM_GICC_BASE		UL(0x2C000000)
#define PLAT_ARM_GICR_BASE		UL(0x300C0000)

#endif /* PLATFORM_DEF_H */
