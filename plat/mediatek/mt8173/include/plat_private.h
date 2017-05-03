/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_PRIVATE_H__
#define __PLAT_PRIVATE_H__

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void plat_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long,
			    unsigned long,
			    unsigned long,
			    unsigned long);

void plat_cci_init(void);
void plat_cci_enable(void);
void plat_cci_disable(void);

/* Declarations for plat_topology.c */
int mt_setup_topology(void);

#endif /* __PLAT_PRIVATE_H__ */
