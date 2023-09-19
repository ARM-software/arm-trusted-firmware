/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_V2_H
#define NEOVERSE_V2_H

#define NEOVERSE_V2_MIDR				U(0x410FD4F0)

/* Neoverse V2 loop count for CVE-2022-23960 mitigation */
#define NEOVERSE_V2_BHB_LOOP_COUNT			U(132)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUECTLR_EL1			S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEOVERSE_V2_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)

/*******************************************************************************
 * CPU Extended Control register 2 specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_CPUECTLR2_EL1			S3_0_C15_C1_5
#define NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_CNSRV		ULL(9)
#define NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_LSB		U(11)
#define NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_WIDTH		U(4)

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR2_EL1			S3_0_C15_C1_1
#define NEOVERSE_V2_CPUACTLR2_EL1_BIT_0			(ULL(1) << 0)

/*******************************************************************************
 * CPU Auxiliary Control register 3 specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR3_EL1			S3_0_C15_C1_2
#define NEOVERSE_V2_CPUACTLR3_EL1_BIT_47		(ULL(1) << 47)

/*******************************************************************************
 * CPU Auxiliary Control register 5 specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR5_EL1			S3_0_C15_C8_0
#define NEOVERSE_V2_CPUACTLR5_EL1_BIT_56		(ULL(1) << 56)
#define NEOVERSE_V2_CPUACTLR5_EL1_BIT_55		(ULL(1) << 55)

#endif /* NEOVERSE_V2_H */
