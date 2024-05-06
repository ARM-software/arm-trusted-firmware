/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_N3_H
#define NEOVERSE_N3_H

#define NEOVERSE_N3_MIDR				U(0x410FD8E0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_N3_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_N3_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEOVERSE_N3_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)

#endif /* NEOVERSE_N3_H */
