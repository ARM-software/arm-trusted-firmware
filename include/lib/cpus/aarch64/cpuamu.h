/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPUAMU_H
#define CPUAMU_H

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

#ifndef __ASSEMBLY__
#include <stdint.h>

uint64_t cpuamu_cnt_read(unsigned int idx);
void cpuamu_cnt_write(unsigned int idx, uint64_t val);
unsigned int cpuamu_read_cpuamcntenset_el0(void);
unsigned int cpuamu_read_cpuamcntenclr_el0(void);
void cpuamu_write_cpuamcntenset_el0(unsigned int mask);
void cpuamu_write_cpuamcntenclr_el0(unsigned int mask);

int midr_match(unsigned int cpu_midr);
void cpuamu_context_save(unsigned int nr_counters);
void cpuamu_context_restore(unsigned int nr_counters);

#endif /* __ASSEMBLY__ */

#endif /* CPUAMU_H */
