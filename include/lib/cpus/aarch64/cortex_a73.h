/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A73_H__
#define __CORTEX_A73_H__

/* Cortex-A73 midr for revision 0 */
#define CORTEX_A73_MIDR	0x410FD090

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A73_CPUECTLR_EL1		S3_1_C15_C2_1	/* Instruction def. */

#define CORTEX_A73_CPUECTLR_SMP_BIT	(1 << 6)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A73_L2MERRSR_EL1		S3_1_C15_C2_3   /* Instruction def. */

#endif /* __CORTEX_A73_H__ */
