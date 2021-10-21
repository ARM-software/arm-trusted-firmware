/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A53_H
#define CORTEX_A53_H

#include <lib/utils_def.h>

/* Cortex-A53 midr for revision 0 */
#define CORTEX_A53_MIDR			U(0x410FD030)

/* Retention timer tick definitions */
#define RETENTION_ENTRY_TICKS_2		U(0x1)
#define RETENTION_ENTRY_TICKS_8		U(0x2)
#define RETENTION_ENTRY_TICKS_32	U(0x3)
#define RETENTION_ENTRY_TICKS_64	U(0x4)
#define RETENTION_ENTRY_TICKS_128	U(0x5)
#define RETENTION_ENTRY_TICKS_256	U(0x6)
#define RETENTION_ENTRY_TICKS_512	U(0x7)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_ECTLR_EL1				S3_1_C15_C2_1

#define CORTEX_A53_ECTLR_SMP_BIT			(ULL(1) << 6)

#define CORTEX_A53_ECTLR_CPU_RET_CTRL_SHIFT		U(0)
#define CORTEX_A53_ECTLR_CPU_RET_CTRL_MASK		(ULL(0x7) << CORTEX_A53_ECTLR_CPU_RET_CTRL_SHIFT)

#define CORTEX_A53_ECTLR_FPU_RET_CTRL_SHIFT		U(3)
#define CORTEX_A53_ECTLR_FPU_RET_CTRL_MASK		(ULL(0x7) << CORTEX_A53_ECTLR_FPU_RET_CTRL_SHIFT)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_MERRSR_EL1				S3_1_C15_C2_2

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_CPUACTLR_EL1				S3_1_C15_C2_0

#define CORTEX_A53_CPUACTLR_EL1_ENDCCASCI_SHIFT		U(44)
#define CORTEX_A53_CPUACTLR_EL1_ENDCCASCI		(ULL(1) << CORTEX_A53_CPUACTLR_EL1_ENDCCASCI_SHIFT)
#define CORTEX_A53_CPUACTLR_EL1_RADIS_SHIFT		U(27)
#define CORTEX_A53_CPUACTLR_EL1_RADIS			(ULL(3) << CORTEX_A53_CPUACTLR_EL1_RADIS_SHIFT)
#define CORTEX_A53_CPUACTLR_EL1_L1RADIS_SHIFT		U(25)
#define CORTEX_A53_CPUACTLR_EL1_L1RADIS			(ULL(3) << CORTEX_A53_CPUACTLR_EL1_L1RADIS_SHIFT)
#define CORTEX_A53_CPUACTLR_EL1_DTAH_SHIFT		U(24)
#define CORTEX_A53_CPUACTLR_EL1_DTAH			(ULL(1) << CORTEX_A53_CPUACTLR_EL1_DTAH_SHIFT)
#define CORTEX_A53_CPUACTLR_EL1_L1PCTL_SHIFT		U(13)
#define CORTEX_A53_CPUACTLR_EL1_L1PCTL			(ULL(7) << CORTEX_A53_CPUACTLR_EL1_L1PCTL_SHIFT)

/*******************************************************************************
 * L2 Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2ACTLR_EL1				S3_1_C15_C0_0

#define CORTEX_A53_L2ACTLR_ENABLE_UNIQUECLEAN		(U(1) << 14)
#define CORTEX_A53_L2ACTLR_DISABLE_CLEAN_PUSH		(U(1) << 3)
/*******************************************************************************
 * L2 Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2ECTLR_EL1				S3_1_C11_C0_3

#define CORTEX_A53_L2ECTLR_RET_CTRL_SHIFT		U(0)
#define CORTEX_A53_L2ECTLR_RET_CTRL_MASK		(U(0x7) << L2ECTLR_RET_CTRL_SHIFT)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2MERRSR_EL1				S3_1_C15_C2_3

/*******************************************************************************
 * Helper function to access a53_cpuectlr_el1 register on Cortex-A53 CPUs
 ******************************************************************************/
#ifndef __ASSEMBLER__
DEFINE_RENAME_SYSREG_RW_FUNCS(a53_cpuectlr_el1, CORTEX_A53_ECTLR_EL1)
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_A53_H */
