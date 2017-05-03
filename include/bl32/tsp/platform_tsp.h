/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_TSP_H__


/*******************************************************************************
 * Mandatory TSP functions (only if platform contains a TSP)
 ******************************************************************************/
void tsp_early_platform_setup(void);
void tsp_plat_arch_setup(void);
void tsp_platform_setup(void);


#define __PLATFORM_H__

#endif
