/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_N1_H
#define NEOVERSE_N1_H

#include <lib/utils_def.h>

/* Neoverse N1 MIDR for revision 0 */
#define NEOVERSE_N1_MIDR		U(0x410fd0c0)

/*******************************************************************************
 * CPU Power Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_N1_CPUPWRCTLR_EL1	S3_0_C15_C2_7

/* Definitions of register field mask in NEOVERSE_N1_CPUPWRCTLR_EL1 */
#define NEOVERSE_N1_CORE_PWRDN_EN_MASK	U(0x1)

#define NEOVERSE_N1_ACTLR_AMEN_BIT	(U(1) << 4)

#define NEOVERSE_N1_AMU_NR_COUNTERS	U(5)
#define NEOVERSE_N1_AMU_GROUP0_MASK	U(0x1f)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_N1_CPUECTLR_EL1	S3_0_C15_C1_4

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_N1_CPUACTLR2_EL1	S3_0_C15_C1_1

#define NEOVERSE_N1_CPUACTLR2_EL1_BIT_2	(ULL(1) << 2)

/* Instruction patching registers */
#define CPUPSELR_EL3	S3_6_C15_C8_0
#define CPUPCR_EL3	S3_6_C15_C8_1
#define CPUPOR_EL3	S3_6_C15_C8_2
#define CPUPMR_EL3	S3_6_C15_C8_3

#endif /* NEOVERSE_N1_H */
