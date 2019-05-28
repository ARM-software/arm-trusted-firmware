/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A55_H
#define CORTEX_A55_H

#include <lib/utils_def.h>

/* Cortex-A55 MIDR for revision 0 */
#define CORTEX_A55_MIDR			U(0x410fd050)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A55_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_A55_CPUECTLR_EL1		S3_0_C15_C1_4

#define CORTEX_A55_CPUECTLR_EL1_L1WSCTL	(ULL(3) << 25)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A55_CPUACTLR_EL1				S3_0_C15_C1_0

#define CORTEX_A55_CPUACTLR_EL1_DISABLE_WRITE_STREAMING	(ULL(1) << 24)
#define CORTEX_A55_CPUACTLR_EL1_DISABLE_DUAL_ISSUE	(ULL(1) << 31)
#define CORTEX_A55_CPUACTLR_EL1_DISABLE_L1_PAGEWALKS	(ULL(1) << 49)

/*******************************************************************************
 * CPU Identification register specific definitions.
 ******************************************************************************/
#define CORTEX_A55_CLIDR_EL1				S3_1_C0_C0_1

#define CORTEX_A55_CLIDR_EL1_CTYPE3			(ULL(7) << 6)

/* Definitions of register field mask in CORTEX_A55_CPUPWRCTLR_EL1 */
#define CORTEX_A55_CORE_PWRDN_EN_MASK	U(0x1)

/* Instruction patching registers */
#define CPUPSELR_EL3	S3_6_C15_C8_0
#define CPUPCR_EL3	S3_6_C15_C8_1
#define CPUPOR_EL3	S3_6_C15_C8_2
#define CPUPMR_EL3	S3_6_C15_C8_3

#endif /* CORTEX_A55_H */
