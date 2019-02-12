/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A72_H
#define CORTEX_A72_H

#include <lib/utils_def.h>

/* Cortex-A72 midr for revision 0 */
#define CORTEX_A72_MIDR		U(0x410FD080)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_ECTLR				p15, 1, c15

#define CORTEX_A72_ECTLR_SMP_BIT			(ULL(1) << 6)
#define CORTEX_A72_ECTLR_DIS_TWD_ACC_PFTCH_BIT		(ULL(1) << 38)
#define CORTEX_A72_ECTLR_L2_IPFTCH_DIST_MASK		(ULL(0x3) << 35)
#define CORTEX_A72_ECTLR_L2_DPFTCH_DIST_MASK		(ULL(0x3) << 32)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_MERRSR				p15, 2, c15

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_CPUACTLR				p15, 0, c15

#define CORTEX_A72_CPUACTLR_DISABLE_L1_DCACHE_HW_PFTCH	(ULL(1) << 56)
#define CORTEX_A72_CPUACTLR_DIS_LOAD_PASS_STORE		(ULL(1) << 55)
#define CORTEX_A72_CPUACTLR_NO_ALLOC_WBWA		(ULL(1) << 49)
#define CORTEX_A72_CPUACTLR_DCC_AS_DCCI			(ULL(1) << 44)
#define CORTEX_A72_CPUACTLR_DIS_INSTR_PREFETCH		(ULL(1) << 32)

/*******************************************************************************
 * L2 Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_L2CTLR				p15, 1, c9, c0, 2

#define CORTEX_A72_L2CTLR_DATA_RAM_LATENCY_SHIFT	U(0)
#define CORTEX_A72_L2CTLR_TAG_RAM_LATENCY_SHIFT		U(6)

#define CORTEX_A72_L2_DATA_RAM_LATENCY_3_CYCLES		U(0x2)
#define CORTEX_A72_L2_TAG_RAM_LATENCY_2_CYCLES		U(0x1)
#define CORTEX_A72_L2_TAG_RAM_LATENCY_3_CYCLES		U(0x2)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_L2MERRSR				p15, 3, c15

#endif /* CORTEX_A72_H */
