/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A720_H
#define CORTEX_A720_H

#define CORTEX_A720_MIDR					U(0x410FD810)

/* Cortex A720 loop count for CVE-2022-23960 mitigation */
#define CORTEX_A720_BHB_LOOP_COUNT				U(132)

/*******************************************************************************
 * CPU Auxiliary Control register 1 specific definitions.
 ******************************************************************************/
#define CORTEX_A720_CPUACTLR_EL1				S3_0_C15_C1_0

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 ******************************************************************************/
#define CORTEX_A720_CPUACTLR2_EL1				S3_0_C15_C1_1

/*******************************************************************************
 * CPU Auxiliary Control register 4 specific definitions.
 ******************************************************************************/
#define CORTEX_A720_CPUACTLR4_EL1				S3_0_C15_C1_3

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A720_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A720_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A720_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * CPU Instruction Patching Register Definitions
 ******************************************************************************/
#define CORTEX_A720_CPUPSELR_EL3				S3_6_C15_C8_0
#define CORTEX_A720_CPUPCR_EL3					S3_6_C15_C8_1
#define CORTEX_A720_CPUPOR_EL3					S3_6_C15_C8_2
#define CORTEX_A720_CPUPMR_EL3					S3_6_C15_C8_3

#ifndef __ASSEMBLER__
long check_erratum_cortex_a720_3699561(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_A720_H */
