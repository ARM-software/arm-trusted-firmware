/*
 * Copyright (c) 2018-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A77_H
#define CORTEX_A77_H

#include <lib/utils_def.h>

/* Cortex-A77 MIDR */
#define CORTEX_A77_MIDR					U(0x410FD0D0)

/* Cortex-A77 loop count for CVE-2022-23960 mitigation */
#define CORTEX_A77_BHB_LOOP_COUNT			U(24)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A77_CPUECTLR_EL1				S3_0_C15_C1_4
#define CORTEX_A77_CPUECTLR_EL1_BIT_8			(ULL(1) << 8)

/*******************************************************************************
 * CPU Power Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A77_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_A77_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	(U(1) << 0)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A77_ACTLR2_EL1				S3_0_C15_C1_1
#define CORTEX_A77_ACTLR2_EL1_BIT_2			(ULL(1) << 2)

#define CORTEX_A77_CPUPSELR_EL3				S3_6_C15_C8_0
#define CORTEX_A77_CPUPCR_EL3				S3_6_C15_C8_1
#define CORTEX_A77_CPUPOR_EL3				S3_6_C15_C8_2
#define CORTEX_A77_CPUPMR_EL3				S3_6_C15_C8_3
#define CORTEX_A77_CPUPOR2_EL3				S3_6_C15_C8_4
#define CORTEX_A77_CPUPMR2_EL3				S3_6_C15_C8_5

#endif /* CORTEX_A77_H */
