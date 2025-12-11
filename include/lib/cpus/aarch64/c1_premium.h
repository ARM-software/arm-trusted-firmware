/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef C1_PREMIUM_H
#define C1_PREMIUM_H

#define C1_PREMIUM_MIDR					        U(0x411FD900)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define C1_PREMIUM_IMP_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define C1_PREMIUM_IMP_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define C1_PREMIUM_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

/*******************************************************************************
 * CPU Auxiliary control register specific definitions
 ******************************************************************************/
#define C1_PREMIUM_IMP_CPUACTLR_EL1				S3_0_C15_C1_0
#define C1_PREMIUM_IMP_CPUACTLR_EL1_LOAD_BIT			U(1)
#define C1_PREMIUM_IMP_CPUACTLR_EL1_LOAD_SHIFT			U(58)
#define C1_PREMIUM_IMP_CPUACTLR_EL1_LOAD_WIDTH			U(3)
#define C1_PREMIUM_IMP_CPUACTLR2_EL1				S3_0_C15_C1_1
#define C1_PREMIUM_IMP_CPUACTLR4_EL1				S3_0_C15_C1_3
#define C1_PREMIUM_IMP_CPUACTLR5_EL1				S3_0_C15_C8_0
#define C1_PREMIUM_CPUACTLR6_EL1				S3_0_C15_C8_1

/*******************************************************************************
 * Selected Instruction Private Select register specific definitions
 ******************************************************************************/
#define C1_PREMIUM_IMP_CPUPSELR_EL3				S3_6_C15_C8_0
#define C1_PREMIUM_IMP_CPUPCR_EL3				S3_6_C15_C8_1
#define C1_PREMIUM_IMP_CPUPOR_EL3				S3_6_C15_C8_2
#define C1_PREMIUM_IMP_CPUPMR_EL3				S3_6_C15_C8_3

#endif /* C1_PREMIUM_H */
