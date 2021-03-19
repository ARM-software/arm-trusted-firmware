/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
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
			    uintptr_t ro_limit);

#endif /* PLAT_PRIVATE_H */
