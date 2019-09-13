/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/bl_aux_params/bl_aux_params.h>
#include <common/debug.h>
#include <plat_params.h>
#include <string.h>

static struct bl_aux_gpio_info rst_gpio;

struct bl_aux_gpio_info *plat_get_mtk_gpio_reset(void)
{
	return &rst_gpio;
}

static bool mtk_aux_param_handler(struct bl_aux_param_header *param)
{
	/* Store platform parameters for later processing if needed. */
	switch (param->type) {
	case BL_AUX_PARAM_MTK_RESET_GPIO:
		rst_gpio = ((struct bl_aux_param_gpio *)param)->gpio;
		return true;
	}

	return false;
}

void params_early_setup(u_register_t plat_param_from_bl2)
{
	bl_aux_params_parse(plat_param_from_bl2, mtk_aux_param_handler);
}

