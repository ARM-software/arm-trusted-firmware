/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HIKEY960_PRIVATE_H__
#define __HIKEY960_PRIVATE_H__

#include <bl_common.h>

/*
 * Function and variable prototypes
 */
void hikey960_init_mmu_el1(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit,
			unsigned long coh_start,
			unsigned long coh_limit);
void hikey960_init_mmu_el3(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit,
			unsigned long coh_start,
			unsigned long coh_limit);
void hikey960_init_ufs(void);
void hikey960_io_setup(void);
int hikey960_read_boardid(unsigned int *id);
void set_retention_ticks(unsigned int val);
void clr_retention_ticks(unsigned int val);
void clr_ex(void);
void nop(void);

#endif /* __HIKEY960_PRIVATE_H__ */
