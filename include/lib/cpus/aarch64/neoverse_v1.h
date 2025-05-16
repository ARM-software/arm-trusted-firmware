/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
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
#define NEOVERSE_V1_CPUECTLR_EL1_BIT_8				(ULL(1) << 8)
#define NEOVERSE_V1_CPUECTLR_EL1_BIT_53				(ULL(1) << 53)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V1_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define NEOVERSE_V1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_V1_ACTLR2_EL1					S3_0_C15_C1_1
#define NEOVERSE_V1_ACTLR2_EL1_BIT_0				ULL(1)
#define NEOVERSE_V1_ACTLR2_EL1_BIT_2				(ULL(1) << 2)
#define NEOVERSE_V1_ACTLR2_EL1_BIT_28				(ULL(1) << 28)
#define NEOVERSE_V1_ACTLR2_EL1_BIT_40				(ULL(1) << 40)

#define NEOVERSE_V1_ACTLR3_EL1					S3_0_C15_C1_2
#define NEOVERSE_V1_ACTLR3_EL1_BIT_47				(ULL(1) << 47)

#define NEOVERSE_V1_ACTLR5_EL1					S3_0_C15_C9_0
#define NEOVERSE_V1_ACTLR5_EL1_BIT_55				(ULL(1) << 55)
#define NEOVERSE_V1_ACTLR5_EL1_BIT_56				(ULL(1) << 56)
#define NEOVERSE_V1_ACTLR5_EL1_BIT_61				(ULL(1) << 61)

#endif /* NEOVERSE_V1_H */
