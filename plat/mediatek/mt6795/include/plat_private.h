/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_PRIVATE_H__
#define __PLAT_PRIVATE_H__
#include <stdint.h>
#include <xlat_tables.h>

void plat_configure_mmu_el3(unsigned long total_base,
					unsigned long total_size,
					unsigned long,
					unsigned long,
					unsigned long,
					unsigned long);

void plat_cci_init(void);
void plat_cci_enable(void);
void plat_cci_disable(void);

/* Declarations for plat_mt_gic.c */
void plat_mt_gic_init(void);

/* Declarations for plat_topology.c */
int mt_setup_topology(void);
void plat_delay_timer_init(void);

void plat_mt_gic_driver_init(void);
void plat_mt_gic_init(void);
void plat_mt_gic_cpuif_enable(void);
void plat_mt_gic_cpuif_disable(void);
void plat_mt_gic_pcpu_init(void);

#endif /* __PLAT_PRIVATE_H__ */
