/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SQ_COMMON_H__
#define __SQ_COMMON_H__

#include <stdint.h>
#include <xlat_tables_v2.h>

struct draminfo {
	uint32_t	num_regions;
	uint32_t	reserved;
	uint64_t	base1;
	uint64_t	size1;
	uint64_t	base2;
	uint64_t	size2;
	uint64_t	base3;
	uint64_t	size3;
};

uint32_t scpi_get_draminfo(struct draminfo *info);

void plat_sq_pwrc_setup(void);

void plat_sq_interconnect_init(void);
void plat_sq_interconnect_enter_coherency(void);
void plat_sq_interconnect_exit_coherency(void);

unsigned int sq_calc_core_pos(u_register_t mpidr);

void sq_gic_driver_init(void);
void sq_gic_init(void);
void sq_gic_cpuif_enable(void);
void sq_gic_cpuif_disable(void);
void sq_gic_pcpu_init(void);

void sq_mmap_setup(uintptr_t total_base, size_t total_size,
		   const struct mmap_region *mmap);

#endif /* __SQ_COMMON_H__ */
