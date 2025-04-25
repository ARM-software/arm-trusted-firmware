/*
 * Copyright (c) 2022-2025, Arm Limited. All rights reserved.
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
#define CORTEX_A510_CPUECTLR_EL1_READPREFERUNIQUE_WIDTH		U(1)
#define CORTEX_A510_CPUECTLR_EL1_READPREFERUNIQUE_DISABLE	U(1)
#define CORTEX_A510_CPUECTLR_EL1_RSCTL_SHIFT			U(23)
#define CORTEX_A510_CPUECTLR_EL1_NTCTL_SHIFT			U(46)
#define CORTEX_A510_CPUECTLR_EL1_ATOM_EXECALLINSTRNEAR		U(2)
#define CORTEX_A510_CPUECTLR_EL1_ATOM_SHIFT			U(38)
#define CORTEX_A510_CPUECTLR_EL1_ATOM_WIDTH			U(3)

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CORTEX_A510_CPUPWRCTLR_EL1				S3_0_C15_C2_7
#define CORTEX_A510_CPUPWRCTLR_EL1_CORE_PWRDN_BIT		U(1)

/*******************************************************************************
 * Complex auxiliary control register specific definitions
 ******************************************************************************/
#define CORTEX_A510_CMPXACTLR_EL1				S3_0_C15_C1_3
#define CORTEX_A510_CMPXACTLR_EL1_ALIAS_LOADSTORE_DISABLE	U(1)
#define CORTEX_A510_CMPXACTLR_EL1_ALIAS_LOADSTORE_SHIFT		U(25)
#define CORTEX_A510_CMPXACTLR_EL1_ALIAS_LOADSTORE_WIDTH		U(1)
#define CORTEX_A510_CMPXACTLR_EL1_SNPPREFERUNIQUE_DISABLE	U(3)
#define CORTEX_A510_CMPXACTLR_EL1_SNPPREFERUNIQUE_SHIFT		U(10)
#define CORTEX_A510_CMPXACTLR_EL1_SNPPREFERUNIQUE_WIDTH		U(2)

/*******************************************************************************
 * Auxiliary control register specific definitions
 ******************************************************************************/
#define CORTEX_A510_CPUACTLR_EL1				S3_0_C15_C1_0
#define CORTEX_A510_CPUACTLR_EL1_BIT_17				(ULL(1) << 17)
#define CORTEX_A510_CPUACTLR_EL1_BIT_38				(ULL(1) << 38)
#define CORTEX_A510_CPUACTLR_EL1_ALIAS_LOADSTORE_DISABLE	U(1)
#define CORTEX_A510_CPUACTLR_EL1_ALIAS_LOADSTORE_SHIFT		U(18)
#define CORTEX_A510_CPUACTLR_EL1_ALIAS_LOADSTORE_WIDTH		U(1)
#define CORTEX_A510_CPUACTLR_EL1_DATA_CORRUPT_DISABLE		U(1)
#define CORTEX_A510_CPUACTLR_EL1_DATA_CORRUPT_SHIFT		U(18)
#define CORTEX_A510_CPUACTLR_EL1_DATA_CORRUPT_WIDTH		U(1)

#ifndef __ASSEMBLER__

#if ERRATA_A510_2971420
long check_erratum_cortex_a510_2971420(long cpu_rev);
#endif

#endif /* __ASSEMBLER__ */

#endif /* CORTEX_A510_H */
