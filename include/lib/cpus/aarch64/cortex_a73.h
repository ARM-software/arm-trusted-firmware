/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A73_H
#define CORTEX_A73_H

#include <lib/utils_def.h>

/* Cortex-A73 midr for revision 0 */
#define CORTEX_A73_MIDR			U(0x410FD090)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A73_CPUECTLR_EL1		S3_1_C15_C2_1	/* Instruction def. */

#define CORTEX_A73_CPUECTLR_SMP_BIT	(ULL(1) << 6)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CORTEX_A73_L2MERRSR_EL1		S3_1_C15_C2_3   /* Instruction def. */

/*******************************************************************************
 * CPU implementation defined register specific definitions.
 ******************************************************************************/
#define CORTEX_A73_IMP_DEF_REG1		S3_0_C15_C0_0

#define CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE	(ULL(1) << 3)

#define CORTEX_A73_DIAGNOSTIC_REGISTER	S3_0_C15_C0_1

#define CORTEX_A73_IMP_DEF_REG2		S3_0_C15_C0_2

/*******************************************************************************
 * Helper function to access a73_cpuectlr_el1 register on Cortex-A73 CPUs
 ******************************************************************************/
#ifndef __ASSEMBLER__
DEFINE_RENAME_SYSREG_RW_FUNCS(a73_cpuectlr_el1, CORTEX_A73_CPUECTLR_EL1)
#endif /* __ASSEMBLER__ */

#endif /* CORTEX_A73_H */
