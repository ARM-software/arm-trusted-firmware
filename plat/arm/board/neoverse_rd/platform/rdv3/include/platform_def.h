/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
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
#if (NRD_PLATFORM_VARIANT == 1)
#define PLAT_ARM_CLUSTER_COUNT		U(8)
#elif (NRD_PLATFORM_VARIANT == 2)
#define PLAT_ARM_CLUSTER_COUNT		U(4)
#else
#define PLAT_ARM_CLUSTER_COUNT		U(16)
#endif
#define NRD_MAX_CPUS_PER_CLUSTER	U(1)
#define NRD_MAX_PE_PER_CPU		U(1)

/* Shared RAM*/
#define NRD_CSS_SHARED_SRAM_SIZE	UL(0x000100000)

/* DRAM1 */
#define NRD_CSS_DRAM1_SIZE		ULL(0x80000000)

/* DRAM2 */
#define NRD_CSS_DRAM2_SIZE		ULL(0x180000000)

/* Address bits */
#define NRD_ADDR_BITS_PER_CHIP		U(36)  /* 64GB */

/*
 * In the current implementation, the RoT Service request that requires the
 * biggest message buffer is the RSE_DELEGATED_ATTEST_GET_PLATFORM_TOKEN. The
 * maximum required buffer size is calculated based on the platform-specific
 * needs of this request.
 */
#define PLAT_RSE_COMMS_PAYLOAD_MAX_SIZE	UL(0x1000)

/* Protected physical address size */
#define PLAT_ARM_PPS			(256 * SZ_1T)

#endif /* PLATFORM_DEF_H */
