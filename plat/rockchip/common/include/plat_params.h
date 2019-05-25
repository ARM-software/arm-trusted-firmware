/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PARAMS_H
#define PLAT_PARAMS_H

#include <lib/bl_aux_params/bl_aux_params.h>
#include <stdint.h>

/* param type */
enum bl_aux_rk_param_type {
	BL_AUX_PARAM_RK_RESET_GPIO = BL_AUX_PARAM_VENDOR_SPECIFIC_FIRST,
	BL_AUX_PARAM_RK_POWEROFF_GPIO,
	BL_AUX_PARAM_RK_SUSPEND_GPIO,
	BL_AUX_PARAM_RK_SUSPEND_APIO,
};

struct bl_aux_rk_apio_info {
	uint8_t apio1 : 1;
	uint8_t apio2 : 1;
	uint8_t apio3 : 1;
	uint8_t apio4 : 1;
	uint8_t apio5 : 1;
};

struct bl_aux_param_rk_apio {
	struct bl_aux_param_header h;
	struct bl_aux_rk_apio_info apio;
};

#endif /* PLAT_PARAMS_H */
