/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef C2_ULTRA_H
#define C2_ULTRA_H

#define C2_ULTRA_MIDR					U(0x410FD960)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define C2_ULTRA_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define C2_ULTRA_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define C2_ULTRA_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* C2_ULTRA_H */
