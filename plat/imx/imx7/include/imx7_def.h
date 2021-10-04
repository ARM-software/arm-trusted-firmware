/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX7_DEF_H
#define IMX7_DEF_H

#include <stdint.h>


/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void plat_imx_io_setup(void);
void imx7_platform_setup(u_register_t arg1, u_register_t arg2,
			 u_register_t arg3, u_register_t arg4);

#endif /*IMX7_DEF_H */
