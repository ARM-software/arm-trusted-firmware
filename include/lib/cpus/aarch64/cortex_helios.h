/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_HELIOS_H__
#define __CORTEX_HELIOS_H__

#define CORTEX_HELIOS_MIDR		U(0x410FD060)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_HELIOS_ECTLR_EL1		S3_0_C15_C1_4

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_HELIOS_CPUACTLR_EL1	S3_0_C15_C1_0

/*******************************************************************************
 * CPU Power Control register specific definitions.
 ******************************************************************************/

#define CORTEX_HELIOS_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_HELIOS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		(U(1) << 0)

#endif /* __CORTEX_HELIOS_H__ */
