/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
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
#define CORTEX_A53_ECTLR			p15, 1, c15

#define CORTEX_A53_ECTLR_SMP_BIT		(U(1) << 6)

#define CORTEX_A53_ECTLR_CPU_RET_CTRL_SHIFT	U(0)
#define CORTEX_A53_ECTLR_CPU_RET_CTRL_MASK	(ULL(0x7) << CORTEX_A53_ECTLR_CPU_RET_CTRL_SHIFT)

#define CORTEX_A53_ECTLR_FPU_RET_CTRL_SHIFT	U(3)
#define CORTEX_A53_ECTLR_FPU_RET_CTRL_MASK	(ULL(0x7) << CORTEX_A53_ECTLR_FPU_RET_CTRL_SHIFT)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_MERRSR			p15, 2, c15

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_CPUACTLR			p15, 0, c15

#define CORTEX_A53_CPUACTLR_ENDCCASCI_SHIFT	U(44)
#define CORTEX_A53_CPUACTLR_ENDCCASCI		(ULL(1) << CORTEX_A53_CPUACTLR_ENDCCASCI_SHIFT)
#define CORTEX_A53_CPUACTLR_DTAH_SHIFT		U(24)
#define CORTEX_A53_CPUACTLR_DTAH		(ULL(1) << CORTEX_A53_CPUACTLR_DTAH_SHIFT)

/*******************************************************************************
 * L2 Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2ACTLR			p15, 1, c15, c0, 0

#define CORTEX_A53_L2ACTLR_ENABLE_UNIQUECLEAN	(U(1) << 14)
#define CORTEX_A53_L2ACTLR_DISABLE_CLEAN_PUSH	(U(1) << 3)

/*******************************************************************************
 * L2 Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2ECTLR			p15, 1, c9, c0, 3

#define CORTEX_A53_L2ECTLR_RET_CTRL_SHIFT	U(0)
#define CORTEX_A53_L2ECTLR_RET_CTRL_MASK	(U(0x7) << L2ECTLR_RET_CTRL_SHIFT)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2MERRSR			p15, 3, c15

#endif /* CORTEX_A53_H */
