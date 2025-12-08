/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef C1_ULTRA_H
#define C1_ULTRA_H

#define C1_ULTRA_MIDR					        U(0x410FD8C0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define C1_ULTRA_IMP_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define C1_ULTRA_IMP_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define C1_ULTRA_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT		U(1)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions
 ******************************************************************************/
#define C1_ULTRA_IMP_CPUACTLR_EL1				S3_0_C15_C1_0
#define C1_ULTRA_IMP_CPUACTLR_EL1_LOAD_BIT			U(1)
#define C1_ULTRA_IMP_CPUACTLR_EL1_LOAD_SHIFT			U(58)
#define C1_ULTRA_IMP_CPUACTLR_EL1_LOAD_WIDTH			U(3)
#define C1_ULTRA_IMP_CPUACTLR4_EL1				S3_0_C15_C1_3
#define C1_ULTRA_IMP_CPUACTLR6_EL1				S3_0_C15_C8_1

#endif /* C1_ULTRA_H */
