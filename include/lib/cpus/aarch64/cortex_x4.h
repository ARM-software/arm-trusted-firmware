/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_X4_H
#define CORTEX_X4_H

#define CORTEX_X4_MIDR					U(0x410FD821)

/* Cortex X4 loop count for CVE-2022-23960 mitigation */
#define CORTEX_X4_BHB_LOOP_COUNT			U(132)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_X4_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_X4_CPUPWRCTLR_EL1			S3_0_C15_C2_7
#define CORTEX_X4_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* CORTEX_X4_H */
