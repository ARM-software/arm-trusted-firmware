/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
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
#define NEOVERSE_V2_CPUECTLR_EL1_EXTLLC_BIT		(ULL(1) << 0)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define NEOVERSE_V2_CPUPWRCTLR_EL1_CORE_PWRDN_BIT	U(1)
#define NEOVERSE_V2_CPUPWRCTLR_EL1_WFI_RET_CTRL_SHIFT	U(4)
#define NEOVERSE_V2_CPUPWRCTLR_EL1_WFI_RET_CTRL_WIDTH	U(3)
#define NEOVERSE_V2_CPUPWRCTLR_EL1_WFE_RET_CTRL_SHIFT	U(7)
#define NEOVERSE_V2_CPUPWRCTLR_EL1_WFE_RET_CTRL_WIDTH	U(3)

/*******************************************************************************
 * CPU Extended Control register 2 specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_CPUECTLR2_EL1			S3_0_C15_C1_5
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_STATIC_FULL	ULL(0)
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_LSB		U(0)
#define NEOVERSE_V2_CPUECTLR2_EL1_TXREQ_WIDTH		U(3)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR_EL1			S3_0_C15_C1_0

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

/*******************************************************************************
 * CPU Auxiliary control register 6 specific definitions
 ******************************************************************************/
#define NEOVERSE_V2_CPUACTLR6_EL1			S3_0_C15_C8_1

#ifndef __ASSEMBLER__
long check_erratum_neoverse_v2_3701771(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* NEOVERSE_V2_H */
