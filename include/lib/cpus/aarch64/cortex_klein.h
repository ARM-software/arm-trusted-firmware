/*
 * Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_KLEIN_H
#define CORTEX_KLEIN_H

#define CORTEX_KLEIN_MIDR					U(0x410FD460)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_KLEIN_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_KLEIN_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_KLEIN_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* CORTEX_KLEIN_H */
