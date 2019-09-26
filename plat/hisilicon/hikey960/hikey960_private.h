/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HIKEY960_PRIVATE_H
#define HIKEY960_PRIVATE_H

#include <common/bl_common.h>

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
void hikey960_io_setup(void);
int hikey960_read_boardid(unsigned int *id);
int hikey960_set_fip_addr(unsigned int image_id, const char *name);
void hikey960_clk_init(void);
void hikey960_pmu_init(void);
void hikey960_regulator_enable(void);
void hikey960_tzc_init(void);
void hikey960_peri_init(void);
void hikey960_pinmux_init(void);
void hikey960_gpio_init(void);
void set_retention_ticks(unsigned int val);
void clr_retention_ticks(unsigned int val);
void clr_ex(void);
void nop(void);

#endif /* HIKEY960_PRIVATE_H */
