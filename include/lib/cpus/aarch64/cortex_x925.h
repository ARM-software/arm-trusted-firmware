/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_X925_H
#define CORTEX_X925_H

#define CORTEX_X925_MIDR					U(0x410FD850)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_X925_CPUECTLR_EL1				S3_0_C15_C1_4
#define CORTEX_X925_CPUECTLR_EL1_EXTLLC_BIT			U(0)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_X925_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_X925_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * CPU Auxiliary control register 6 specific definitions
 ******************************************************************************/
#define CORTEX_X925_CPUACTLR6_EL1                                S3_0_C15_C8_1

#ifndef __ASSEMBLER__
long check_erratum_cortex_x925_3701747(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_X925_H */
