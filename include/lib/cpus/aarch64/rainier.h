/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RAINIER_H
#define RAINIER_H

#include <lib/utils_def.h>

/* RAINIER MIDR for revision 0 */
#define RAINIER_MIDR			U(0x3f0f4120)

/* Exception Syndrome register EC code for IC Trap */
#define RAINIER_EC_IC_TRAP		U(0x1f)

/*******************************************************************************
 * CPU Power Control register specific definitions.
 ******************************************************************************/
#define RAINIER_CPUPWRCTLR_EL1		S3_0_C15_C2_7

/* Definitions of register field mask in RAINIER_CPUPWRCTLR_EL1 */
#define RAINIER_CORE_PWRDN_EN_MASK	U(0x1)

#define RAINIER_ACTLR_AMEN_BIT		(U(1) << 4)

#define RAINIER_AMU_NR_COUNTERS		U(5)
#define RAINIER_AMU_GROUP0_MASK		U(0x1f)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define RAINIER_CPUECTLR_EL1			S3_0_C15_C1_4

#define RAINIER_WS_THR_L2_MASK			(ULL(3) << 24)
#define RAINIER_CPUECTLR_EL1_MM_TLBPF_DIS_BIT	(ULL(1) << 51)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define RAINIER_CPUACTLR_EL1		S3_0_C15_C1_0

#define RAINIER_CPUACTLR_EL1_BIT_6	(ULL(1) << 6)
#define RAINIER_CPUACTLR_EL1_BIT_13	(ULL(1) << 13)

#define RAINIER_CPUACTLR2_EL1		S3_0_C15_C1_1

#define RAINIER_CPUACTLR2_EL1_BIT_0	(ULL(1) << 0)
#define RAINIER_CPUACTLR2_EL1_BIT_2	(ULL(1) << 2)
#define RAINIER_CPUACTLR2_EL1_BIT_11	(ULL(1) << 11)
#define RAINIER_CPUACTLR2_EL1_BIT_15	(ULL(1) << 15)
#define RAINIER_CPUACTLR2_EL1_BIT_16	(ULL(1) << 16)
#define RAINIER_CPUACTLR2_EL1_BIT_59	(ULL(1) << 59)

#define RAINIER_CPUACTLR3_EL1		S3_0_C15_C1_2

#define RAINIER_CPUACTLR3_EL1_BIT_10	(ULL(1) << 10)

/* Instruction patching registers */
#define CPUPSELR_EL3	S3_6_C15_C8_0
#define CPUPCR_EL3	S3_6_C15_C8_1
#define CPUPOR_EL3	S3_6_C15_C8_2
#define CPUPMR_EL3	S3_6_C15_C8_3

#endif /* RAINIER_H */
