/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CORTEX_A75_H
#define CORTEX_A75_H

#include <lib/utils_def.h>

/* Cortex-A75 MIDR */
#define CORTEX_A75_MIDR		U(0x410fd0a0)

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A75_CPUPWRCTLR_EL1	S3_0_C15_C2_7
#define CORTEX_A75_CPUECTLR_EL1		S3_0_C15_C1_4

/*******************************************************************************
 * CPU Auxiliary Control register specific definitions.
 ******************************************************************************/
#define CORTEX_A75_CPUACTLR_EL1		S3_0_C15_C1_0

#define CORTEX_A75_CPUACTLR_EL1_DISABLE_LOAD_PASS_STORE	(ULL(1) << 35)

/* Definitions of register field mask in CORTEX_A75_CPUPWRCTLR_EL1 */
#define CORTEX_A75_CORE_PWRDN_EN_MASK	U(0x1)

#define CORTEX_A75_ACTLR_AMEN_BIT	(ULL(1) << 4)

/*
 * The Cortex-A75 core implements five counters, 0-4. Events 0, 1, 2, are
 * fixed and are enabled (Group 0). Events 3 and 4 (Group 1) are
 * programmable by programming the appropriate Event count bits in
 * CPUAMEVTYPER<n> register and are disabled by default. Platforms may
 * enable this with suitable programming.
 */
#define CORTEX_A75_AMU_NR_COUNTERS	U(5)
#define CORTEX_A75_AMU_GROUP0_MASK	U(0x7)
#define CORTEX_A75_AMU_GROUP1_MASK	(U(0) << 3)

#ifndef __ASSEMBLY__
#include <stdint.h>

uint64_t cortex_a75_amu_cnt_read(int idx);
void cortex_a75_amu_cnt_write(int idx, uint64_t val);
unsigned int cortex_a75_amu_read_cpuamcntenset_el0(void);
unsigned int cortex_a75_amu_read_cpuamcntenclr_el0(void);
void cortex_a75_amu_write_cpuamcntenset_el0(unsigned int mask);
void cortex_a75_amu_write_cpuamcntenclr_el0(unsigned int mask);
#endif /* __ASSEMBLY__ */

#endif /* CORTEX_A75_H */
