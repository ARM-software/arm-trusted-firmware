/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOVERSE_V3AE_H
#define NEOVERSE_V3AE_H


#define NEOVERSE_V3_VNAE_MIDR				U(0x410FD830)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define NEOVERSE_V3AE_CPUECTLR_EL1				                S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define NEOVERSE_V3AE_CPUPWRCTLR_EL1				            S3_0_C15_C2_7
#define NEOVERSE_V3AE_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		        U(1)
#define NEOVERSE_V3AE_CPUPWRCTLR_EL1_WFI_RET_CTRL_SHIFT	        U(4)
#define NEOVERSE_V3AE_CPUPWRCTLR_EL1_WFI_RET_CTRL_WIDTH	        U(3)
#define NEOVERSE_V3AE_CPUPWRCTLR_EL1_WFE_RET_CTRL_SHIFT	        U(7)
#define NEOVERSE_V3AE_CPUPWRCTLR_EL1_WFE_RET_CTRL_WIDTH	        U(3)

/*******************************************************************************
 * CPU Auxiliary control register definitions
 ******************************************************************************/
#define NEOVERSE_V3AE_CPUACTLR2_EL1				                S3_0_C15_C1_1
#define NEOVERSE_V3AE_CPUACTLR4_EL1				                S3_0_C15_C1_3
#define NEOVERSE_V3AE_CPUACTLR6_EL1				                S3_0_C15_C8_1

/*******************************************************************************
 * CPU instruction patching register definitions
 ******************************************************************************/
#define NEOVERSE_V3AE_CPUPSELR_EL3				                S3_6_C15_C8_0
#define NEOVERSE_V3AE_CPUPCR_EL3				                S3_6_C15_C8_1
#define NEOVERSE_V3AE_CPUPOR_EL3				                S3_6_C15_C8_2
#define NEOVERSE_V3AE_CPUPMR_EL3				                S3_6_C15_C8_3
#define NEOVERSE_V3AE_CPUACTLR5_EL1				                S3_0_C15_C8_0

#ifndef __ASSEMBLER__
long check_erratum_neoverse_v3ae_3701767(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* NEOVERSE_V3AE_H */
