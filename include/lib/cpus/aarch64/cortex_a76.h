/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A76_H__
#define __CORTEX_A76_H__

/* Cortex-A76 MIDR for revision 0 */
#define CORTEX_A76_MIDR		0x410fd0b0

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A76_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_A76_CPUECTLR_EL1	S3_0_C15_C1_4

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A76_CPUACTLR2_EL1	S3_0_C15_C1_1

#define CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE	(1 << 16)

/* Definitions of register field mask in CORTEX_A76_CPUPWRCTLR_EL1 */
#define CORTEX_A76_CORE_PWRDN_EN_MASK	0x1

#endif /* __CORTEX_A76_H__ */
