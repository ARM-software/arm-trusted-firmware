/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_E1_H
#define NEOVERSE_E1_H

#include <lib/utils_def.h>

#define NEOVERSE_E1_MIDR		U(0x410FD4A0)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_E1_ECTLR_EL1		S3_0_C15_C1_4

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_E1_CPUACTLR_EL1	S3_0_C15_C1_0

/*******************************************************************************
 * CPU Power Control register specific definitions.
 ******************************************************************************/

#define NEOVERSE_E1_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define NEOVERSE_E1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		(U(1) << 0)

#endif /* NEOVERSE_E1_H */
