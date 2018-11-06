/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_SP_MIN_H
#define PLATFORM_SP_MIN_H

#include <stdint.h>

/*******************************************************************************
 * Mandatory SP_MIN functions
 ******************************************************************************/
void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3);
void sp_min_platform_setup(void);
void sp_min_plat_runtime_setup(void);
void sp_min_plat_arch_setup(void);
entry_point_info_t *sp_min_plat_get_bl33_ep_info(void);

/* Platforms that enable SP_MIN_WITH_SECURE_FIQ shall implement this api */
void sp_min_plat_fiq_handler(uint32_t id);

#endif /* PLATFORM_SP_MIN_H */
