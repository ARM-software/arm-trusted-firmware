/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <plat/arm/css/common/css_def.h>
#include <nrd_css_fw_def3.h>
#include <nrd_pas_def3.h>
#include <nrd_plat_arm_def3.h>
#include <nrd_ros_fw_def3.h>

/* Remote chip address offset */
#define NRD_REMOTE_CHIP_MEM_OFFSET(n)	\
		((ULL(1) << NRD_ADDR_BITS_PER_CHIP) * (n))

/* PE-Cluster count */
#define PLAT_ARM_CLUSTER_COUNT			U(16)
#define NRD_MAX_CPUS_PER_CLUSTER		U(1)
#define NRD_MAX_PE_PER_CPU			U(1)

/* Shared RAM*/
#define NRD_CSS_SHARED_SRAM_SIZE	UL(0x000100000)

/* DRAM1 */
#define NRD_CSS_DRAM1_SIZE		ULL(0x80000000)

/* DRAM2 */
#define NRD_CSS_DRAM2_SIZE		ULL(0x180000000)

/* Address bits */
#define NRD_ADDR_BITS_PER_CHIP		U(36)  /* 64GB */

#endif /* PLATFORM_DEF_H */
