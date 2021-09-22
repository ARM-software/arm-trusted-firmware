/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A710_H
#define CORTEX_A710_H

#define CORTEX_A710_MIDR					U(0x410FD470)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A710_CPUECTLR_EL1				S3_0_C15_C1_4
#define CORTEX_A710_CPUECTLR_EL1_PFSTIDIS_BIT			(ULL(1) << 8)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A710_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A710_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A710_CPUACTLR_EL1 				S3_0_C15_C1_0
#define CORTEX_A710_CPUACTLR_EL1_BIT_46				(ULL(1) << 46)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A710_CPUACTLR5_EL1				S3_0_C15_C8_0
#define CORTEX_A710_CPUACTLR5_EL1_BIT_13			(ULL(1) << 13)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A710_CPUECTLR2_EL1				S3_0_C15_C1_5
#define CORTEX_A710_CPUECTLR2_EL1_PF_MODE_CNSRV			ULL(9)
#define CPUECTLR2_EL1_PF_MODE_LSB				U(11)
#define CPUECTLR2_EL1_PF_MODE_WIDTH				U(4)

#endif /* CORTEX_A710_H */
