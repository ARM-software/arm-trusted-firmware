/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void plat_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start,
			    unsigned long ro_limit,
			    unsigned long coh_start,
			    unsigned long coh_limit);

void plat_cci_init(void);
void plat_cci_enable(void);
void plat_cci_disable(void);
void plat_cci_init_sf(void);
void plat_gic_init(void);

/* Declarations for plat_topology.c */
int mt_setup_topology(void);

#endif /* PLAT_PRIVATE_H */
