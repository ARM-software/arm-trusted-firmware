/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A75_H__
#define __CORTEX_A75_H__

/* Cortex-A75 MIDR */
#define CORTEX_A75_MIDR		0x410fd0a0

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A75_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_A75_CPUECTLR_EL1		S3_0_C15_C1_4

/* Definitions of register field mask in CORTEX_A75_CPUPWRCTLR_EL1 */
#define CORTEX_A75_CORE_PWRDN_EN_MASK	0x1

#endif /* __CORTEX_A75_H__ */
