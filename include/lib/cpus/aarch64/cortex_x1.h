/*
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_X1_H
#define CORTEX_X1_H

/* Cortex-X1 MIDR for r1p0 */
#define CORTEX_X1_MIDR			U(0x411fd440)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_X1_CPUECTLR_EL1		S3_0_C15_C1_4

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_X1_ACTLR2_EL1		S3_0_C15_C1_1

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_X1_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_X1_CORE_PWRDN_EN_MASK	U(0x1)

#endif /* CORTEX_X1_H */
