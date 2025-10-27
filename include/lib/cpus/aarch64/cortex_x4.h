/*
 * Copyright (c) 2022-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_X4_H
#define CORTEX_X4_H

#define CORTEX_X4_MIDR					U(0x410FD821)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_X4_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_X4_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_X4_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * CPU Auxiliary control register specific definitions
 ******************************************************************************/
#define CORTEX_X4_CPUACTLR_EL1				S3_0_C15_C1_0
#define CORTEX_X4_CPUACTLR2_EL1				S3_0_C15_C1_1
#define CORTEX_X4_CPUACTLR3_EL1				S3_0_C15_C1_2
#define CORTEX_X4_CPUACTLR4_EL1				S3_0_C15_C1_3

/*******************************************************************************
 * CPU Auxiliary control register 5 specific definitions
 ******************************************************************************/
#define CORTEX_X4_CPUACTLR5_EL1				S3_0_C15_C8_0
#define CORTEX_X4_CPUACTLR5_EL1_BIT_14			(ULL(1) << 14)

/*******************************************************************************
 * CPU Auxiliary control register 6 specific definitions
 ******************************************************************************/
#define CORTEX_X4_CPUACTLR6_EL1				S3_0_C15_C8_1

#ifndef __ASSEMBLER__
#if ERRATA_X4_2726228
long check_erratum_cortex_x4_2726228(long cpu_rev);
#else
static inline long check_erratum_cortex_x4_2726228(long cpu_rev)
{
       return 0;
}
#endif /* ERRATA_X4_2726228 */

long check_erratum_cortex_x4_3701758(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_X4_H */
