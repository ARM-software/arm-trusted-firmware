/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
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
#define CORTEX_A72_ECTLR_EL1			S3_1_C15_C2_1

#define CORTEX_A72_ECTLR_SMP_BIT		(1 << 6)
#define CORTEX_A72_ECTLR_DIS_TWD_ACC_PFTCH_BIT	(1 << 38)
#define CORTEX_A72_ECTLR_L2_IPFTCH_DIST_MASK	(0x3 << 35)
#define CORTEX_A72_ECTLR_L2_DPFTCH_DIST_MASK	(0x3 << 32)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_MERRSR_EL1			S3_1_C15_C2_2

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_ACTLR_EL1			S3_1_C15_C2_0

#define CORTEX_A72_ACTLR_DISABLE_L1_DCACHE_HW_PFTCH	(1 << 56)
#define CORTEX_A72_ACTLR_NO_ALLOC_WBWA		(1 << 49)
#define CORTEX_A72_ACTLR_DCC_AS_DCCI		(1 << 44)

/*******************************************************************************
 * L2 Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_L2CTLR_EL1			S3_1_C11_C0_2

#define CORTEX_A72_L2CTLR_DATA_RAM_LATENCY_SHIFT	0
#define CORTEX_A72_L2CTLR_TAG_RAM_LATENCY_SHIFT	6

#define CORTEX_A72_L2_DATA_RAM_LATENCY_3_CYCLES	0x2
#define CORTEX_A72_L2_TAG_RAM_LATENCY_2_CYCLES	0x1
#define CORTEX_A72_L2_TAG_RAM_LATENCY_3_CYCLES	0x2

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A72_L2MERRSR_EL1			S3_1_C15_C2_3

#endif /* __CORTEX_A72_H__ */
