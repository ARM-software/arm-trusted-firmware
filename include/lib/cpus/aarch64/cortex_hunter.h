/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_HUNTER_H
#define CORTEX_HUNTER_H

#define CORTEX_HUNTER_MIDR					U(0x410FD810)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_HUNTER_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_HUNTER_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_HUNTER_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* CORTEX_HUNTER_H */
