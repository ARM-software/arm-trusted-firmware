/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A76_H
#define CORTEX_A76_H

#include <lib/utils_def.h>

/* Cortex-A76 MIDR for revision 0 */
#define CORTEX_A76_MIDR		U(0x410fd0b0)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A76_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_A76_CPUECTLR_EL1		S3_0_C15_C1_4

#define CORTEX_A76_CPUECTLR_EL1_WS_THR_L2	(ULL(3) << 24)
#define CORTEX_A76_CPUECTLR_EL1_BIT_51		(ULL(1) << 51)

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A76_CPUACTLR_EL1		S3_0_C15_C1_0

#define CORTEX_A76_CPUACTLR_EL1_DISABLE_STATIC_PREDICTION	(ULL(1) << 6)

#define CORTEX_A76_CPUACTLR_EL1_BIT_13	(ULL(1) << 13)

#define CORTEX_A76_CPUACTLR2_EL1	S3_0_C15_C1_1

#define CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE	(ULL(1) << 16)

#define CORTEX_A76_CPUACTLR3_EL1	S3_0_C15_C1_2

#define CORTEX_A76_CPUACTLR3_EL1_BIT_10	(ULL(1) << 10)


/* Definitions of register field mask in CORTEX_A76_CPUPWRCTLR_EL1 */
#define CORTEX_A76_CORE_PWRDN_EN_MASK	U(0x1)

#endif /* CORTEX_A76_H */
