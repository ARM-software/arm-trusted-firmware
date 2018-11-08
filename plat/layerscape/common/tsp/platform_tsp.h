/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_TSP_H
#define PLATFORM_TSP_H

/*******************************************************************************
 * Mandatory TSP functions (only if platform contains a TSP)
 ******************************************************************************/
void tsp_early_platform_setup(void);
void tsp_plat_arch_setup(void);
void tsp_platform_setup(void);

#endif /* PLATFORM_TSP_H */
