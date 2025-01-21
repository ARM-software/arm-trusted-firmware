/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A725_H
#define CORTEX_A725_H

#define CORTEX_A725_MIDR					U(0x410FD870)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A725_CPUECTLR_EL1				S3_0_C15_C1_4
#define CORTEX_A725_CPUECTLR_EL1_EXTLLC_BIT			U(0)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A725_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A725_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#ifndef __ASSEMBLER__
long check_erratum_cortex_a725_3699564(long cpu_rev);
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_A725_H */
