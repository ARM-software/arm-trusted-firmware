/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A53_H__
#define __CORTEX_A53_H__

/* Cortex-A53 midr for revision 0 */
#define CORTEX_A53_MIDR 0x410FD030

/* Retention timer tick definitions */
#define RETENTION_ENTRY_TICKS_2		0x1
#define RETENTION_ENTRY_TICKS_8		0x2
#define RETENTION_ENTRY_TICKS_32	0x3
#define RETENTION_ENTRY_TICKS_64	0x4
#define RETENTION_ENTRY_TICKS_128	0x5
#define RETENTION_ENTRY_TICKS_256	0x6
#define RETENTION_ENTRY_TICKS_512	0x7

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_ECTLR			p15, 1, c15

#define CORTEX_A53_ECTLR_SMP_BIT		(1 << 6)

#define CORTEX_A53_ECTLR_CPU_RET_CTRL_SHIFT	0
#define CORTEX_A53_ECTLR_CPU_RET_CTRL_MASK	(0x7 << CORTEX_A53_ECTLR_CPU_RET_CTRL_SHIFT)

#define CORTEX_A53_ECTLR_FPU_RET_CTRL_SHIFT	3
#define CORTEX_A53_ECTLR_FPU_RET_CTRL_MASK	(0x7 << CORTEX_A53_ECTLR_FPU_RET_CTRL_SHIFT)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_MERRSR			p15, 2, c15

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_CPUACTLR			p15, 0, c15

#define CORTEX_A53_CPUACTLR_ENDCCASCI_SHIFT	44
#define CORTEX_A53_CPUACTLR_ENDCCASCI		(1 << CORTEX_A53_CPUACTLR_ENDCCASCI_SHIFT)
#define CORTEX_A53_CPUACTLR_DTAH		(1 << 24)

/*******************************************************************************
 * L2 Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2ACTLR			p15, 1, c15, c0, 0

#define CORTEX_A53_L2ACTLR_ENABLE_UNIQUECLEAN	(1 << 14)
#define CORTEX_A53_L2ACTLR_DISABLE_CLEAN_PUSH	(1 << 3)

/*******************************************************************************
 * L2 Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2ECTLR			p15, 1, c9, c0, 3

#define CORTEX_A53_L2ECTLR_RET_CTRL_SHIFT	0
#define CORTEX_A53_L2ECTLR_RET_CTRL_MASK	(0x7 << L2ECTLR_RET_CTRL_SHIFT)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A53_L2MERRSR			p15, 3, c15

#if !ERROR_DEPRECATED
/*
 * These registers were previously wrongly named. Provide previous definitions so
 * as not to break platforms that continue using them.
 */
#define CORTEX_A53_ACTLR			CORTEX_A53_CPUACTLR

#define CORTEX_A53_ACTLR_ENDCCASCI_SHIFT	CORTEX_A53_CPUACTLR_ENDCCASCI_SHIFT
#define CORTEX_A53_ACTLR_ENDCCASCI		CORTEX_A53_CPUACTLR_ENDCCASCI
#define CORTEX_A53_ACTLR_DTAH			CORTEX_A53_CPUACTLR_DTAH
#endif /* !ERROR_DEPRECATED */

#endif /* __CORTEX_A53_H__ */
