/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

#include <common/bl_common.h>

#include "hi3798cv200.h"

void plat_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start,
			    unsigned long ro_limit,
			    unsigned long coh_start,
			    unsigned long coh_limit);

void plat_configure_mmu_el1(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start,
			    unsigned long ro_limit,
			    unsigned long coh_start,
			    unsigned long coh_limit);

void plat_io_setup(void);

unsigned int poplar_calc_core_pos(u_register_t mpidr);

void poplar_gic_driver_init(void);
void poplar_gic_init(void);
void poplar_gic_cpuif_enable(void);
void poplar_gic_pcpu_init(void);

#endif /* PLAT_PRIVATE_H */
