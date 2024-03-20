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

/* Remote chip address offset */
#define NRD_REMOTE_CHIP_MEM_OFFSET(n)	\
		((ULL(1) << NRD_ADDR_BITS_PER_CHIP) * (n))

#define PLAT_ARM_CLUSTER_COUNT		U(2)
#define NRD_MAX_CPUS_PER_CLUSTER	U(4)
#define NRD_MAX_PE_PER_CPU		U(1)

#define PLAT_CSS_MHU_BASE		UL(0x45400000)

/* Base address of DMC-620 instances */
#define RDN1EDGE_DMC620_BASE0		UL(0x4e000000)
#define RDN1EDGE_DMC620_BASE1		UL(0x4e100000)

/* Virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xc0000000)

/* Maximum number of address bits used per chip */
#define NRD_ADDR_BITS_PER_CHIP	U(42)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		UL(0x30000000)
#define PLAT_ARM_GICC_BASE		UL(0x2C000000)
#define PLAT_ARM_GICR_BASE		UL(0x300C0000)

#define RDN1E1_CHIP0_SPI_START		U(32)
#define RDN1E1_CHIP0_SPI_END		U(991)

#endif /* PLATFORM_DEF_H */
