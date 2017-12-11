/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A75_H__
#define __CORTEX_A75_H__

/* Cortex-A75 MIDR */
#define CORTEX_A75_MIDR		0x410fd0a0

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A75_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_A75_CPUECTLR_EL1		S3_0_C15_C1_4

/* Definitions of register field mask in CORTEX_A75_CPUPWRCTLR_EL1 */
#define CORTEX_A75_CORE_PWRDN_EN_MASK	0x1

/*******************************************************************************
 * CPU Activity Monitor Unit register specific definitions.
 ******************************************************************************/
#define CPUAMCNTENCLR_EL0	S3_3_C15_C9_7
#define CPUAMCNTENSET_EL0	S3_3_C15_C9_6
#define CPUAMCFGR_EL0		S3_3_C15_C10_6
#define CPUAMUSERENR_EL0	S3_3_C15_C10_7

/* Activity Monitor Event Counter Registers */
#define CPUAMEVCNTR0_EL0	S3_3_C15_C9_0
#define CPUAMEVCNTR1_EL0	S3_3_C15_C9_1
#define CPUAMEVCNTR2_EL0	S3_3_C15_C9_2
#define CPUAMEVCNTR3_EL0	S3_3_C15_C9_3
#define CPUAMEVCNTR4_EL0	S3_3_C15_C9_4

/* Activity Monitor Event Type Registers */
#define CPUAMEVTYPER0_EL0	S3_3_C15_C10_0
#define CPUAMEVTYPER1_EL0	S3_3_C15_C10_1
#define CPUAMEVTYPER2_EL0	S3_3_C15_C10_2
#define CPUAMEVTYPER3_EL0	S3_3_C15_C10_3
#define CPUAMEVTYPER4_EL0	S3_3_C15_C10_4

#define CORTEX_A75_ACTLR_AMEN_BIT	(U(1) << 4)

/*
 * The Cortex-A75 core implements five counters, 0-4. Events 0, 1, 2, are
 * fixed and are enabled (Group 0). Events 3 and 4 (Group 1) are
 * programmable by programming the appropriate Event count bits in
 * CPUAMEVTYPER<n> register and are disabled by default. Platforms may
 * enable this with suitable programming.
 */
#define CORTEX_A75_AMU_NR_COUNTERS	5
#define CORTEX_A75_AMU_GROUP0_MASK	0x7
#define CORTEX_A75_AMU_GROUP1_MASK	(0 << 3)

#ifndef __ASSEMBLY__
#include <stdint.h>

uint64_t cortex_a75_amu_cnt_read(int idx);
void cortex_a75_amu_cnt_write(int idx, uint64_t val);
unsigned int cortex_a75_amu_read_cpuamcntenset_el0(void);
unsigned int cortex_a75_amu_read_cpuamcntenclr_el0(void);
void cortex_a75_amu_write_cpuamcntenset_el0(unsigned int mask);
void cortex_a75_amu_write_cpuamcntenclr_el0(unsigned int mask);
#endif /* __ASSEMBLY__ */

#endif /* __CORTEX_A75_H__ */
