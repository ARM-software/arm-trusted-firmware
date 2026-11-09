/*
 * Copyright (c) 2019-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_V1_H
#define NEOVERSE_V1_H

#define NEOVERSE_V1_MIDR					U(0x410FD400)

/* Neoverse V1 loop count for CVE-2022-23960 mitigation */
#define NEOVERSE_V1_BHB_LOOP_COUNT				U(32)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_V1_CPUECTLR_EL1				S3_0_C15_C1_4
#define NEOVERSE_V1_CPUPSELR_EL3				S3_6_C15_C8_0
#define NEOVERSE_V1_CPUPOR_EL3					S3_6_C15_C8_2
#define NEOVERSE_V1_CPUPMR_EL3					S3_6_C15_C8_3
#define NEOVERSE_V1_CPUPCR_EL3					S3_6_C15_C8_1

#define NEOVERSE_V1_CPUECTLR2_EL1				S3_0_C15_C1_5
#define NEOVERSE_V1_CPUECTLR2_EL1_TXREQ_MAX_BIT			U(0)
#define NEOVERSE_V1_CPUECTLR2_EL1_TXREQ_MAX_SHIFT		U(0)
#define NEOVERSE_V1_CPUECTLR2_EL1_TXREQ_MAX_WIDTH		U(2)
/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V1_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define NEOVERSE_V1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		BIT(0)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_V1_CPUACTLR_EL1				S3_0_C15_C1_0

#define NEOVERSE_V1_CPUACTLR2_EL1				S3_0_C15_C1_1
#define NEOVERSE_V1_CPUACTLR3_EL1				S3_0_C15_C1_2
#define NEOVERSE_V1_CPUACTLR4_EL1				S3_0_C15_C1_3
#define NEOVERSE_V1_CPUACTLR5_EL1				S3_0_C15_C9_0

#endif /* NEOVERSE_V1_H */
