/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_ALTO_H
#define CORTEX_ALTO_H

#define CORTEX_ALTO_MIDR					U(0x411FD900)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_ALTO_IMP_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_ALTO_IMP_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_ALTO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT	U(1)

#endif /* CORTEX_ALTO_H */
