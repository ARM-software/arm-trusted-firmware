/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A715_H
#define CORTEX_A715_H

#define CORTEX_A715_MIDR					U(0x410FD4D0)

/* Cortex-A715 loop count for CVE-2022-23960 mitigation */
#define CORTEX_A715_BHB_LOOP_COUNT				U(38)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A715_CPUECTLR_EL1				S3_0_C15_C1_4

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A715_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A715_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#endif /* CORTEX_A715_H */
