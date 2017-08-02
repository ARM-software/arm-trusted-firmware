/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A57_H__
#define __CORTEX_A57_H__
#include <utils_def.h>

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
#define CORTEX_A57_ECTLR			p15, 1, c15

#define CORTEX_A57_ECTLR_SMP_BIT		(ULL(1) << 6)
#define CORTEX_A57_ECTLR_DIS_TWD_ACC_PFTCH_BIT	(ULL(1) << 38)
#define CORTEX_A57_ECTLR_L2_IPFTCH_DIST_MASK	(ULL(0x3) << 35)
#define CORTEX_A57_ECTLR_L2_DPFTCH_DIST_MASK	(ULL(0x3) << 32)

#define CORTEX_A57_ECTLR_CPU_RET_CTRL_SHIFT	0
#define CORTEX_A57_ECTLR_CPU_RET_CTRL_MASK	(ULL(0x7) << CORTEX_A57_ECTLR_CPU_RET_CTRL_SHIFT)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A57_CPUMERRSR			p15, 2, c15

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A57_CPUACTLR				p15, 0, c15

#define CORTEX_A57_CPUACTLR_DIS_LOAD_PASS_DMB		(ULL(1) << 59)
#define CORTEX_A57_CPUACTLR_GRE_NGRE_AS_NGNRE		(ULL(1) << 54)
#define CORTEX_A57_CPUACTLR_DIS_OVERREAD		(ULL(1) << 52)
#define CORTEX_A57_CPUACTLR_NO_ALLOC_WBWA		(ULL(1) << 49)
#define CORTEX_A57_CPUACTLR_DCC_AS_DCCI			(ULL(1) << 44)
#define CORTEX_A57_CPUACTLR_FORCE_FPSCR_FLUSH		(ULL(1) << 38)
#define CORTEX_A57_CPUACTLR_DIS_INSTR_PREFETCH		(ULL(1) << 32)
#define CORTEX_A57_CPUACTLR_DIS_STREAMING		(ULL(3) << 27)
#define CORTEX_A57_CPUACTLR_DIS_L1_STREAMING		(ULL(3) << 25)
#define CORTEX_A57_CPUACTLR_DIS_INDIRECT_PREDICTOR	(ULL(1) << 4)

/*******************************************************************************
 * L2 Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A57_L2CTLR				p15, 1, c9, c0, 2

#define CORTEX_A57_L2CTLR_DATA_RAM_LATENCY_SHIFT	0
#define CORTEX_A57_L2CTLR_TAG_RAM_LATENCY_SHIFT		6

#define CORTEX_A57_L2_DATA_RAM_LATENCY_3_CYCLES		0x2
#define CORTEX_A57_L2_TAG_RAM_LATENCY_3_CYCLES		0x2

/*******************************************************************************
 * L2 Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A57_L2ECTLR			p15, 1, c9, c0, 3

#define CORTEX_A57_L2ECTLR_RET_CTRL_SHIFT	0
#define CORTEX_A57_L2ECTLR_RET_CTRL_MASK	(ULL(0x7) << CORTEX_A57_L2ECTLR_RET_CTRL_SHIFT)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A57_L2MERRSR			p15, 3, c15

#if !ERROR_DEPRECATED
/*
 * These registers were previously wrongly named. Provide previous definitions so
 * as not to break platforms that continue using them.
 */
#define CORTEX_A57_ACTLR			CORTEX_A57_CPUACTLR

#define CORTEX_A57_ACTLR_DIS_LOAD_PASS_DMB	CORTEX_A57_CPUACTLR_DIS_LOAD_PASS_DMB
#define CORTEX_A57_ACTLR_GRE_NGRE_AS_NGNRE	CORTEX_A57_CPUACTLR_GRE_NGRE_AS_NGNRE
#define CORTEX_A57_ACTLR_DIS_OVERREAD		CORTEX_A57_CPUACTLR_DIS_OVERREAD
#define CORTEX_A57_ACTLR_NO_ALLOC_WBWA		CORTEX_A57_CPUACTLR_NO_ALLOC_WBWA
#define CORTEX_A57_ACTLR_DCC_AS_DCCI		CORTEX_A57_CPUACTLR_DCC_AS_DCCI
#define CORTEX_A57_ACTLR_FORCE_FPSCR_FLUSH	CORTEX_A57_CPUACTLR_FORCE_FPSCR_FLUSH
#define CORTEX_A57_ACTLR_DIS_STREAMING		CORTEX_A57_CPUACTLR_DIS_STREAMING
#define CORTEX_A57_ACTLR_DIS_L1_STREAMING	CORTEX_A57_CPUACTLR_DIS_L1_STREAMING
#define CORTEX_A57_ACTLR_DIS_INDIRECT_PREDICTOR	CORTEX_A57_CPUACTLR_DIS_INDIRECT_PREDICTOR
#endif /* !ERROR_DEPRECATED */

#endif /* __CORTEX_A57_H__ */
