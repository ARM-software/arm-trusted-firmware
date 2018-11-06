/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A73_H
#define CORTEX_A73_H

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

/*******************************************************************************
 * CPU implementation defined register specific definitions.
 ******************************************************************************/
#define CORTEX_A73_IMP_DEF_REG1		S3_0_C15_C0_0

#define CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE	(1 << 3)

#endif /* CORTEX_A73_H */
