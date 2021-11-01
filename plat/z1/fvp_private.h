/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_PRIVATE_H
#define FVP_PRIVATE_H

#include <plat/arm/common/plat_arm.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/

void fvp_config_setup(void);

void fvp_interconnect_init(void);
void fvp_interconnect_enable(void);
void fvp_interconnect_disable(void);
void fvp_timer_init(void);
void tsp_early_platform_setup(void);

#endif /* FVP_PRIVATE_H */
