/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PRIVATE_H
#define PLAT_PRIVATE_H

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void plat_configure_mmu_el3(uintptr_t total_base,
			    uintptr_t total_size,
			    uintptr_t ro_start,
			    uintptr_t ro_limit,
			    uintptr_t coh_start,
			    uintptr_t coh_limit);

void plat_mtk_cci_init(void);
void plat_mtk_cci_enable(void);
void plat_mtk_cci_disable(void);
void plat_mtk_cci_init_sf(void);

/* Declarations for plat_topology.c */
int mt_setup_topology(void);

#endif /* PLAT_PRIVATE_H */
