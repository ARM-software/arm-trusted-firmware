/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A78C_H
#define CORTEX_A78C_H


#define CORTEX_A78C_MIDR			        U(0x410FD4B1)

/* Cortex-A76 loop count for CVE-2022-23960 mitigation */
#define CORTEX_A78C_BHB_LOOP_COUNT			U(32)

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 * ****************************************************************************/
#define CORTEX_A78C_CPUACTLR2_EL1			S3_0_C15_C1_1
#define CORTEX_A78C_CPUACTLR2_EL1_BIT_0			(ULL(1) << 0)
#define CORTEX_A78C_CPUACTLR2_EL1_BIT_40 		(ULL(1) << 40)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A78C_CPUECTLR_EL1		        S3_0_C15_C1_4
#define CORTEX_A78C_CPUECTLR_EL1_BIT_6		        (ULL(1) << 6)
#define CORTEX_A78C_CPUECTLR_EL1_BIT_7		        (ULL(1) << 7)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A78C_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_A78C_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

/*******************************************************************************
 * CPU Implementation Specific Selected Instruction registers
 ******************************************************************************/
#define CORTEX_A78C_IMP_CPUPSELR_EL3			S3_6_C15_C8_0
#define CORTEX_A78C_IMP_CPUPCR_EL3			S3_6_C15_C8_1
#define CORTEX_A78C_IMP_CPUPOR_EL3			S3_6_C15_C8_2
#define CORTEX_A78C_IMP_CPUPMR_EL3			S3_6_C15_C8_3

#endif /* CORTEX_A78C_H */
