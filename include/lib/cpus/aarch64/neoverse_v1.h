/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_V1_H
#define NEOVERSE_V1_H

#define NEOVERSE_V1_MIDR					U(0x410FD400)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_V1_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V1_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define NEOVERSE_V1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* NEOVERSE_V1_H */
