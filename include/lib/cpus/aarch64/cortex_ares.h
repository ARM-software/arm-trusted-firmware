/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_ARES_H__
#define __CORTEX_ARES_H__

/* Cortex-ARES MIDR for revision 0 */
#define CORTEX_ARES_MIDR		0x410fd0c0

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_ARES_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_ARES_CPUECTLR_EL1	S3_0_C15_C1_4

/* Definitions of register field mask in CORTEX_ARES_CPUPWRCTLR_EL1 */
#define CORTEX_ARES_CORE_PWRDN_EN_MASK	0x1

#define CORTEX_ARES_ACTLR_AMEN_BIT	(U(1) << 4)

#define CORTEX_ARES_AMU_NR_COUNTERS	U(5)
#define CORTEX_ARES_AMU_GROUP0_MASK	U(0x1f)

/* Instruction patching registers */
#define CPUPSELR_EL3	S3_6_C15_C8_0
#define CPUPCR_EL3	S3_6_C15_C8_1
#define CPUPOR_EL3	S3_6_C15_C8_2
#define CPUPMR_EL3	S3_6_C15_C8_3

#endif /* __CORTEX_ARES_H__ */
