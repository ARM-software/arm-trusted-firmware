/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef C1_NANO_H
#define C1_NANO_H

#define C1_NANO_MIDR					U(0x410FD8A0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define C1_NANO_IMP_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define C1_NANO_IMP_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define C1_NANO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)
#define C1_NANO_IMP_CPUPWRCTLR_EL1_WFE_RET_CTRL_SHIFT	U(7)
#define C1_NANO_IMP_CPUPWRCTLR_EL1_WFE_RET_CTRL_WIDTH	U(3)
#define C1_NANO_IMP_CPUPWRCTLR_EL1_WFE_RET_CTRL_BIT	U(0)
#define C1_NANO_IMP_CPUPWRCTLR_EL1_WFI_RET_CTRL_SHIFT	U(4)
#define C1_NANO_IMP_CPUPWRCTLR_EL1_WFI_RET_CTRL_WIDTH	U(3)
#define C1_NANO_IMP_CPUPWRCTLR_EL1_WFI_RET_CTRL_BIT	U(0)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions
 ******************************************************************************/
#define C1_NANO_IMP_CPUACTLR_EL1			S3_0_C15_C1_0
#define C1_NANO_IMP_CPUACTLR_EL1_MTE_DIS_BIT		U(1)
#define C1_NANO_IMP_CPUACTLR_EL1_MTE_DIS_SHIFT		U(27)
#define C1_NANO_IMP_CPUACTLR_EL1_MTE_DIS_WIDTH		U(1)
#define C1_NANO_IMP_CPUACTLR3_EL1			S3_0_C15_C1_2

/*******************************************************************************
 * CPU Instruction Private register specific definitions
 ******************************************************************************/
#define C1_NANO_IMP_CPUPSELR_EL3			S3_6_C15_C8_0
#define C1_NANO_IMP_CPUPCR_EL3				S3_6_C15_C8_1
#define C1_NANO_IMP_CPUPOR_EL3				S3_6_C15_C8_2
#define C1_NANO_IMP_CPUPMR_EL3				S3_6_C15_C8_3

#endif /* C1_NANO_H */
