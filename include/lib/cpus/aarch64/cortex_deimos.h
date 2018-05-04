/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_DEIMOS_H__
#define __CORTEX_DEIMOS_H__

#define CORTEX_DEIMOS_MIDR					U(0x410FD0D0)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_DEIMOS_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions.
 ******************************************************************************/
#define CORTEX_DEIMOS_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_DEIMOS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		(U(1) << 0)

#endif /* __CORTEX_DEIMOS_H__ */
