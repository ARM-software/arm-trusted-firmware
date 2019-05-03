/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_PARAMS_H
#define PLAT_PARAMS_H

#include <stdint.h>

#include <export/plat/mediatek/common/plat_params_exp.h>

struct bl_aux_gpio_info *plat_get_mtk_gpio_reset(void);
void params_early_setup(u_register_t plat_param_from_bl2);

#endif
