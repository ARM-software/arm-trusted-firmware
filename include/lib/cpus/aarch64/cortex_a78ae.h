/*
 * Copyright (c) 2019-2026, Arm Limited. All rights reserved.
 * Copyright (c) 2021-2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A78AE_H
#define CORTEX_A78AE_H

#define CORTEX_A78AE_MIDR 				U(0x410FD420)

/* Cortex-A78AE loop count for CVE-2022-23960 mitigation */
#define CORTEX_A78AE_BHB_LOOP_COUNT			U(32)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A78AE_CPUECTLR_EL1			S3_0_C15_C1_4
#define CORTEX_A78AE_CPUECTLR_EL1_BIT_8			BIT(8)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A78AE_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_A78AE_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	BIT(0)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A78AE_ACTLR_TAM_BIT			BIT(30)

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 ******************************************************************************/
#define CORTEX_A78AE_CPUACTLR2_EL1			S3_0_C15_C1_1
#define CORTEX_A78AE_CPUACTLR2_EL1_BIT_0		BIT(0)
#define CORTEX_A78AE_CPUACTLR2_EL1_BIT_40		BIT(40)
#define CORTEX_A78AE_CPUACTLR3_EL1			S3_0_C15_C1_2
#define CORTEX_A78AE_CPUACTLR5_EL1			S3_0_C15_C9_0

/*******************************************************************************
 * CPU Activity Monitor Unit register specific definitions.
 ******************************************************************************/
#define CPUAMCNTENCLR0_EL0				S3_3_C15_C2_4
#define CPUAMCNTENSET0_EL0				S3_3_C15_C2_5
#define CPUAMCNTENCLR1_EL0				S3_3_C15_C3_0
#define CPUAMCNTENSET1_EL0				S3_3_C15_C3_1

#define CORTEX_A78AE_AMU_GROUP0_MASK			U(0xF)
#define CORTEX_A78AE_AMU_GROUP1_MASK			U(0x7)

#endif /* CORTEX_A78AE_H */
