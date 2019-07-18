/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A65_H
#define CORTEX_A65_H

#include <lib/utils_def.h>

#define CORTEX_A65_MIDR			U(0x410FD060)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A65_ECTLR_EL1		S3_0_C15_C1_4

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions
 ******************************************************************************/
#define CORTEX_A65_CPUACTLR_EL1		S3_0_C15_C1_0

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/

#define CORTEX_A65_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_A65_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	(U(1) << 0)

#endif /* CORTEX_A65_H */
