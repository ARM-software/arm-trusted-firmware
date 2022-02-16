/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A510_H
#define CORTEX_A510_H

#define CORTEX_A510_MIDR					U(0x410FD460)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A510_CPUECTLR_EL1				S3_0_C15_C1_4
#define CORTEX_A510_CPUECTLR_EL1_READPREFERUNIQUE_SHIFT		U(19)
#define CORTEX_A510_CPUECTLR_EL1_READPREFERUNIQUE_DISABLE	U(1)
#define CORTEX_A510_CPUECTLR_EL1_RSCTL_SHIFT			U(23)
#define CORTEX_A510_CPUECTLR_EL1_NTCTL_SHIFT			U(46)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A510_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A510_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * Complex auxiliary control register specific definitions
 ******************************************************************************/
#define CORTEX_A510_CMPXACTLR_EL1				S3_0_C15_C1_3

/*******************************************************************************
 * Auxiliary control register specific definitions
 ******************************************************************************/
#define CORTEX_A510_CPUACTLR_EL1				S3_0_C15_C1_0

#endif /* CORTEX_A510_H */
