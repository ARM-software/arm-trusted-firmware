/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A57_H__
#define __CORTEX_A57_H__

/* Cortex-A57 midr for revision 0 */
#define CORTEX_A57_MIDR 0x410FD070

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
#define CPUECTLR			p15, 1, c15	/* Instruction def. */

#define CPUECTLR_SMP_BIT		(1 << 6)
#define CPUECTLR_DIS_TWD_ACC_PFTCH_BIT	(1 << 38)
#define CPUECTLR_L2_IPFTCH_DIST_MASK	(0x3 << 35)
#define CPUECTLR_L2_DPFTCH_DIST_MASK	(0x3 << 32)

#define CPUECTLR_CPU_RET_CTRL_SHIFT	0
#define CPUECTLR_CPU_RET_CTRL_MASK	(0x7 << CPUECTLR_CPU_RET_CTRL_SHIFT)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CPUMERRSR			p15, 2, c15 /* Instruction def. */

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CPUACTLR			p15, 0, c15 /* Instruction def. */

#define CPUACTLR_DIS_LOAD_PASS_DMB	(1 << 59)
#define CPUACTLR_GRE_NGRE_AS_NGNRE	(1 << 54)
#define CPUACTLR_DIS_OVERREAD		(1 << 52)
#define CPUACTLR_NO_ALLOC_WBWA		(1 << 49)
#define CPUACTLR_DCC_AS_DCCI		(1 << 44)
#define CPUACTLR_FORCE_FPSCR_FLUSH	(1 << 38)
#define CPUACTLR_DIS_STREAMING		(3 << 27)
#define CPUACTLR_DIS_L1_STREAMING	(3 << 25)
#define CPUACTLR_DIS_INDIRECT_PREDICTOR	(1 << 4)

/*******************************************************************************
 * L2 Control register specific definitions.
 ******************************************************************************/
#define L2CTLR			p15, 1, c9, c0, 3 /* Instruction def. */

#define L2CTLR_DATA_RAM_LATENCY_SHIFT	0
#define L2CTLR_TAG_RAM_LATENCY_SHIFT	6

#define L2_DATA_RAM_LATENCY_3_CYCLES	0x2
#define L2_TAG_RAM_LATENCY_3_CYCLES	0x2

/*******************************************************************************
 * L2 Extended Control register specific definitions.
 ******************************************************************************/
#define L2ECTLR			p15, 1, c9, c0, 3	/* Instruction def. */

#define L2ECTLR_RET_CTRL_SHIFT		0
#define L2ECTLR_RET_CTRL_MASK		(0x7 << L2ECTLR_RET_CTRL_SHIFT)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define L2MERRSR			p15, 3, c15 /* Instruction def. */

#endif /* __CORTEX_A57_H__ */
