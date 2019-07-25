/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_EXPORT_PLAT_ROCKCHIP_COMMON_PLAT_PARAMS_EXP_H
#define ARM_TRUSTED_FIRMWARE_EXPORT_PLAT_ROCKCHIP_COMMON_PLAT_PARAMS_EXP_H

/* EXPORT HEADER -- See include/export/README for details! -- EXPORT HEADER */

#include "../../../lib/bl_aux_params/bl_aux_params_exp.h"

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

#endif /* ARM_TRUSTED_FIRMWARE_EXPORT_PLAT_ROCKCHIP_COMMON_PLAT_PARAMS_EXP_H */
