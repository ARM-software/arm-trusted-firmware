/*
 * Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_MATTERHORN_H
#define CORTEX_MATTERHORN_H

#define CORTEX_MATTERHORN_MIDR					U(0x410FD470)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_MATTERHORN_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_MATTERHORN_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_MATTERHORN_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* CORTEX_MATTERHORN_H */
