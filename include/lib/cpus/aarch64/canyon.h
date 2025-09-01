/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CANYON_H
#define CANYON_H

#define CANYON_MIDR					U(0x410FD960)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CANYON_IMP_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CANYON_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CANYON_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* CANYON_H */

