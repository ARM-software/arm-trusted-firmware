/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A520_H
#define CORTEX_A520_H

#define CORTEX_A520_MIDR					U(0x410FD800)

/*******************************************************************************
 * CPU Extended Control register specific definitions
 ******************************************************************************/
#define CORTEX_A520_CPUACTLR_EL1				S3_0_C15_C1_0

#define CORTEX_A520_CPUECTLR_EL1				S3_0_C15_C1_4
#define CORTEX_A520_CPUECTLR_EL1_EXTLLC_BIT			U(0)

/*******************************************************************************
 * CPU Auxiliary Control register 1 specific definitions.
 ******************************************************************************/
#define CORTEX_A520_CPUACTLR_EL1				S3_0_C15_C1_0

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A520_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A520_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

#ifndef __ASSEMBLER__
#if ERRATA_A520_2938996
long  check_erratum_cortex_a520_2938996(long cpu_rev);
#else
static inline long  check_erratum_cortex_a520_2938996(long cpu_rev)
{
       return 0;
}
#endif /* ERRATA_A520_2938996 */
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_A520_H */
