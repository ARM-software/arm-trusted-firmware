/*
 * Copyright (c) 2020-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_N2_H
#define NEOVERSE_N2_H

/* Neoverse N2 ID register for revision r0p0 */
#define NEOVERSE_N2_MIDR				U(0x410FD490)

/* Neoverse N2 loop count for CVE-2022-23960 mitigation */
#define NEOVERSE_N2_BHB_LOOP_COUNT			U(32)

/*******************************************************************************
 * CPU Power control register
 ******************************************************************************/
#define NEOVERSE_N2_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEOVERSE_N2_CORE_PWRDN_EN_BIT			(ULL(1) << 0)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_N2_CPUECTLR_EL1			S3_0_C15_C1_4
#define NEOVERSE_N2_CPUECTLR_EL1_EXTLLC_BIT		(ULL(1) << 0)
#define NEOVERSE_N2_CPUECTLR_EL1_PFSTIDIS_BIT		(ULL(1) << 8)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_N2_CPUACTLR_EL1			S3_0_C15_C1_0
#define NEOVERSE_N2_CPUACTLR_EL1_BIT_46			(ULL(1) << 46)
#define NEOVERSE_N2_CPUACTLR_EL1_BIT_22			(ULL(1) << 22)

/*******************************************************************************
 * CPU Auxiliary Control register 2 specific definitions.
 ******************************************************************************/
#define NEOVERSE_N2_CPUACTLR2_EL1			S3_0_C15_C1_1
#define NEOVERSE_N2_CPUACTLR2_EL1_BIT_2			(ULL(1) << 2)
#define NEOVERSE_N2_CPUACTLR2_EL1_BIT_40		(ULL(1) << 40)

/*******************************************************************************
 * CPU Auxiliary Control register 5 specific definitions.
 ******************************************************************************/
#define NEOVERSE_N2_CPUACTLR5_EL1			S3_0_C15_C8_0
#define NEOVERSE_N2_CPUACTLR5_EL1_BIT_44		(ULL(1) << 44)
#define NEOVERSE_N2_CPUACTLR5_EL1_BIT_13		(ULL(1) << 13)
#define NEOVERSE_N2_CPUACTLR5_EL1_BIT_17		(ULL(1) << 17)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_N2_CPUECTLR2_EL1			S3_0_C15_C1_5
#define NEOVERSE_N2_CPUECTLR2_EL1_PF_MODE_CNSRV		ULL(9)
#define CPUECTLR2_EL1_PF_MODE_LSB			U(11)
#define CPUECTLR2_EL1_PF_MODE_WIDTH			U(4)

#endif /* NEOVERSE_N2_H */
