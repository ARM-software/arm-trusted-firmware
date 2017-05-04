/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A72_H__
#define __CORTEX_A72_H__

/* Cortex-A72 midr for revision 0 */
#define CORTEX_A72_MIDR 0x410FD080

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CPUECTLR			p15, 1, c15	/* Instruction def. */

#define CPUECTLR_SMP_BIT		(1 << 6)
#define CPUECTLR_DIS_TWD_ACC_PFTCH_BIT	(1 << 38)
#define CPUECTLR_L2_IPFTCH_DIST_MASK	(0x3 << 35)
#define CPUECTLR_L2_DPFTCH_DIST_MASK	(0x3 << 32)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CPUMERRSR			p15, 2, c15 /* Instruction def. */

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CPUACTLR			p15, 0, c15 /* Instruction def. */

#define CPUACTLR_DISABLE_L1_DCACHE_HW_PFTCH	(1 << 56)
#define CPUACTLR_NO_ALLOC_WBWA         (1 << 49)
#define CPUACTLR_DCC_AS_DCCI           (1 << 44)

/*******************************************************************************
 * L2 Control register specific definitions.
 ******************************************************************************/
#define L2CTLR			p15, 1, c9, c0, 3 /* Instruction def. */

#define L2CTLR_DATA_RAM_LATENCY_SHIFT	0
#define L2CTLR_TAG_RAM_LATENCY_SHIFT	6

#define L2_DATA_RAM_LATENCY_3_CYCLES	0x2
#define L2_TAG_RAM_LATENCY_2_CYCLES	0x1
#define L2_TAG_RAM_LATENCY_3_CYCLES	0x2

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define L2MERRSR			p15, 3, c15 /* Instruction def. */

#endif /* __CORTEX_A72_H__ */
