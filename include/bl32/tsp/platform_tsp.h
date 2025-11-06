/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_TSP_H
#define PLATFORM_TSP_H

#include <stdint.h>

/*******************************************************************************
 * Mandatory TSP functions (only if platform contains a TSP)
 ******************************************************************************/
void tsp_early_platform_setup(u_register_t arg0, u_register_t arg1,
			      u_register_t arg2, u_register_t arg3);
void tsp_plat_arch_setup(void);
void tsp_platform_setup(void);

#endif /* PLATFORM_TSP_H */
