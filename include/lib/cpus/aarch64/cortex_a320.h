/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A320_H
#define CORTEX_A320_H

#define CORTEX_A320_MIDR					U(0x410FD8F0)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A320_CPUECTLR_EL1				S3_0_C15_C1_4
#define CORTEX_A320_CPUECTLR_EL1_EXTLLC_BIT			U(0)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A320_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A320_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* CORTEX_A320_H */
