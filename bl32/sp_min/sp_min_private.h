/*
 * Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SP_MIN_PRIVATE_H
#define SP_MIN_PRIVATE_H

#include <stdint.h>

void sp_min_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
		  u_register_t arg3);
void sp_min_main(void);
void sp_min_warm_boot(void);
void sp_min_fiq(void);

#endif /* SP_MIN_PRIVATE_H */
