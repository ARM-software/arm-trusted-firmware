/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_X3_H
#define CORTEX_X3_H

#define CORTEX_X3_MIDR				U(0x410FD4E0)

/* Cortex-X3 loop count for CVE-2022-23960 mitigation */
#define CORTEX_X3_BHB_LOOP_COUNT		U(132)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_X3_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_X3_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_X3_CPUPWRCTLR_EL1_CORE_PWRDN_BIT			U(1)
#define CORTEX_X3_CPUPWRCTLR_EL1_WFI_RET_CTRL_BITS_SHIFT	U(4)
#define CORTEX_X3_CPUPWRCTLR_EL1_WFE_RET_CTRL_BITS_SHIFT	U(7)

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 ******************************************************************************/
#define CORTEX_X3_CPUACTLR2_EL1			S3_0_C15_C1_1
#define CORTEX_X3_CPUACTLR2_EL1_BIT_36		(ULL(1) << 36)

/*******************************************************************************
 * CPU Auxiliary Control register 5 specific definitions.
 ******************************************************************************/
#define CORTEX_X3_CPUACTLR5_EL1			S3_0_C15_C8_0
#define CORTEX_X3_CPUACTLR5_EL1_BIT_55		(ULL(1) << 55)
#define CORTEX_X3_CPUACTLR5_EL1_BIT_56		(ULL(1) << 56)

/*******************************************************************************
 * CPU Extended Control register 2 specific definitions.
 ******************************************************************************/
#define CORTEX_X3_CPUECTLR2_EL1			S3_0_C15_C1_5

#define CORTEX_X3_CPUECTLR2_EL1_PF_MODE_LSB	U(11)
#define CORTEX_X3_CPUECTLR2_EL1_PF_MODE_WIDTH	U(4)
#define CORTEX_X3_CPUECTLR2_EL1_PF_MODE_CNSRV	ULL(0x9)

#endif /* CORTEX_X3_H */
