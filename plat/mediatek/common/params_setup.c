/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat_params.h>
#include <string.h>

static struct gpio_info param_reset;

struct gpio_info *plat_get_gpio_reset(void)
{
	return &param_reset;
}

void params_early_setup(void *plat_param_from_bl2)
{
	struct bl31_plat_param *bl2_param;
	struct bl31_gpio_param *gpio_param;

	bl2_param = (struct bl31_plat_param *)plat_param_from_bl2;
	while (bl2_param) {
		switch (bl2_param->type) {
		case PARAM_RESET:
			gpio_param = (struct bl31_gpio_param *)bl2_param;
			memcpy(&param_reset, &gpio_param->gpio,
			       sizeof(struct gpio_info));
			break;
		default:
			ERROR("not expected type found %lld\n",
			      bl2_param->type);
			break;
		}
		bl2_param = bl2_param->next;
	}
}
