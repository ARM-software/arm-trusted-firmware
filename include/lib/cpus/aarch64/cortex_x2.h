/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_X2_H
#define CORTEX_X2_H

#define CORTEX_X2_MIDR						U(0x410FD480)

/* Cortex-X2 loop count for CVE-2022-23960 mitigation */
#define CORTEX_X2_BHB_LOOP_COUNT       				U(32)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_X2_CPUECTLR_EL1					S3_0_C15_C1_4
#define CORTEX_X2_CPUECTLR_EL1_PFSTIDIS_BIT			(ULL(1) << 8)

/*******************************************************************************
 * CPU Extended Control register 2 specific definitions
 ******************************************************************************/
#define CORTEX_X2_CPUECTLR2_EL1					S3_0_C15_C1_5

#define CORTEX_X2_CPUECTLR2_EL1_PF_MODE_SHIFT			U(11)
#define CORTEX_X2_CPUECTLR2_EL1_PF_MODE_WIDTH			U(4)
#define CORTEX_X2_CPUECTLR2_EL1_PF_MODE_CNSRV			ULL(0x9)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_X2_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_X2_CPUPWRCTLR_EL1_CORE_PWRDN_BIT			U(1)

/*******************************************************************************
 * CPU Auxiliary Control Register definitions
 ******************************************************************************/
#define CORTEX_X2_CPUACTLR_EL1					S3_0_C15_C1_0
#define CORTEX_X2_CPUACTLR_EL1_BIT_22				(ULL(1) << 22)

/*******************************************************************************
 * CPU Auxiliary Control Register 2 definitions
 ******************************************************************************/
#define CORTEX_X2_CPUACTLR2_EL1					S3_0_C15_C1_1
#define CORTEX_X2_CPUACTLR2_EL1_BIT_40				(ULL(1) << 40)

/*******************************************************************************
 * CPU Auxiliary Control Register 5 definitions
 ******************************************************************************/
#define CORTEX_X2_CPUACTLR5_EL1					S3_0_C15_C8_0
#define CORTEX_X2_CPUACTLR5_EL1_BIT_17				(ULL(1) << 17)

/*******************************************************************************
 * CPU Implementation Specific Selected Instruction registers
 ******************************************************************************/
#define CORTEX_X2_IMP_CPUPSELR_EL3				S3_6_C15_C8_0
#define CORTEX_X2_IMP_CPUPCR_EL3				S3_6_C15_C8_1
#define CORTEX_X2_IMP_CPUPOR_EL3				S3_6_C15_C8_2
#define CORTEX_X2_IMP_CPUPMR_EL3				S3_6_C15_C8_3

#endif /* CORTEX_X2_H */
