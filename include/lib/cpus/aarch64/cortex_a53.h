/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A53_H__
#define __CORTEX_A53_H__

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

#define CORTEX_A53_ECTLR_SMP_BIT			(U(1) << 6)

#define CORTEX_A53_ECTLR_CPU_RET_CTRL_SHIFT		U(0)
#define CORTEX_A53_ECTLR_CPU_RET_CTRL_MASK		(U(0x7) << CORTEX_A53_ECTLR_CPU_RET_CTRL_SHIFT)

#define CORTEX_A53_ECTLR_FPU_RET_CTRL_SHIFT		U(3)
#define CORTEX_A53_ECTLR_FPU_RET_CTRL_MASK		(U(0x7) << CORTEX_A53_ECTLR_FPU_RET_CTRL_SHIFT)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_MERRSR_EL1				S3_1_C15_C2_2

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_CPUACTLR_EL1				S3_1_C15_C2_0

#define CORTEX_A53_CPUACTLR_EL1_ENDCCASCI_SHIFT		U(44)
#define CORTEX_A53_CPUACTLR_EL1_ENDCCASCI		(U(1) << CORTEX_A53_CPUACTLR_EL1_ENDCCASCI_SHIFT)
#define CORTEX_A53_CPUACTLR_EL1_RADIS_SHIFT		U(27)
#define CORTEX_A53_CPUACTLR_EL1_RADIS			(U(3) << CORTEX_A53_CPUACTLR_EL1_RADIS_SHIFT)
#define CORTEX_A53_CPUACTLR_EL1_L1RADIS_SHIFT		U(25)
#define CORTEX_A53_CPUACTLR_EL1_L1RADIS			(U(3) << CORTEX_A53_CPUACTLR_EL1_L1RADIS_SHIFT)
#define CORTEX_A53_CPUACTLR_EL1_DTAH_SHIFT		U(24)
#define CORTEX_A53_CPUACTLR_EL1_DTAH			(U(1) << CORTEX_A53_CPUACTLR_EL1_DTAH_SHIFT)

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

#if !ERROR_DEPRECATED
/*
 * These registers were previously wrongly named. Provide previous definitions
 * so as not to break platforms that continue using them.
 */
#define CORTEX_A53_ACTLR_EL1			CORTEX_A53_CPUACTLR_EL1

#define CORTEX_A53_ACTLR_ENDCCASCI_SHIFT	CORTEX_A53_CPUACTLR_EL1_ENDCCASCI_SHIFT
#define CORTEX_A53_ACTLR_ENDCCASCI		CORTEX_A53_CPUACTLR_EL1_ENDCCASCI
#define CORTEX_A53_ACTLR_RADIS_SHIFT		CORTEX_A53_CPUACTLR_EL1_RADIS_SHIFT
#define CORTEX_A53_ACTLR_RADIS			CORTEX_A53_CPUACTLR_EL1_RADIS
#define CORTEX_A53_ACTLR_L1RADIS_SHIFT		CORTEX_A53_CPUACTLR_EL1_L1RADIS_SHIFT
#define CORTEX_A53_ACTLR_L1RADIS		CORTEX_A53_CPUACTLR_EL1_L1RADIS
#define CORTEX_A53_ACTLR_DTAH_SHIFT		CORTEX_A53_CPUACTLR_EL1_DTAH_SHIFT
#define CORTEX_A53_ACTLR_DTAH			CORTEX_A53_CPUACTLR_EL1_DTAH
#endif /* !ERROR_DEPRECATED */

#endif /* __CORTEX_A53_H__ */
