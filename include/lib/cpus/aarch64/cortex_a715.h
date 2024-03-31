/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A715_H
#define CORTEX_A715_H

#define CORTEX_A715_MIDR					U(0x410FD4D0)

/* Cortex-A715 loop count for CVE-2022-23960 mitigation */
#define CORTEX_A715_BHB_LOOP_COUNT				U(38)

/*******************************************************************************
 * CPU Auxiliary Control register 1 specific definitions.
 ******************************************************************************/
#define CORTEX_A715_CPUACTLR_EL1				S3_0_C15_C1_0

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 ******************************************************************************/
#define CORTEX_A715_CPUACTLR2_EL1				S3_0_C15_C1_1

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A715_CPUECTLR_EL1				S3_0_C15_C1_4

#define CORTEX_A715_CPUPSELR_EL3				S3_6_C15_C8_0
#define CORTEX_A715_CPUPCR_EL3					S3_6_C15_C8_1
#define CORTEX_A715_CPUPOR_EL3					S3_6_C15_C8_2
#define CORTEX_A715_CPUPMR_EL3					S3_6_C15_C8_3

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A715_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A715_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* CORTEX_A715_H */
