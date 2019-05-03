/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_EXPORT_PLAT_MEDIATEK_COMMON_PLAT_PARAMS_EXP_H
#define ARM_TRUSTED_FIRMWARE_EXPORT_PLAT_MEDIATEK_COMMON_PLAT_PARAMS_EXP_H

/* EXPORT HEADER -- See include/export/README for details! -- EXPORT HEADER */

#include "../../../lib/bl_aux_params/bl_aux_params_exp.h"

/* param type */
enum bl_aux_mtk_param_type {
	BL_AUX_PARAM_MTK_RESET_GPIO = BL_AUX_PARAM_VENDOR_SPECIFIC_FIRST,
};

#endif /* ARM_TRUSTED_FIRMWARE_EXPORT_PLAT_MEDIATEK_COMMON_PLAT_PARAMS_EXP_H */
