/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A725_H
#define CORTEX_A725_H

#define CORTEX_A725_MIDR					U(0x410FD870)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A725_CPUECTLR_EL1				S3_0_C15_C1_4
#define CORTEX_A725_CPUECTLR_EL1_EXTLLC_BIT			U(0)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A725_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A725_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions
 ******************************************************************************/
#define CORTEX_A725_CPUACTLR_EL1				S3_0_C15_C1_0
#define CORTEX_A725_CPUACTLR2_EL1				S3_0_C15_C1_1

/*******************************************************************************
 * CPU Instruction Patch Control register specific definitions
 ******************************************************************************/
#define CORTEX_A725_CPUPSELR_EL3				S3_6_C15_C8_0
#define CORTEX_A725_CPUPCR_EL3					S3_6_C15_C8_1
#define CORTEX_A725_CPUPOR_EL3					S3_6_C15_C8_2
#define CORTEX_A725_CPUPMR_EL3					S3_6_C15_C8_3

#ifndef __ASSEMBLER__
long check_erratum_cortex_a725_3699564(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_A725_H */
