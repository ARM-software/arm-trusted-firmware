/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <gpio.h>
#include <mmio.h>
#include <plat_params.h>
#include <plat_private.h>
#include <platform.h>
#include <string.h>

static struct gpio_info param_reset;
static struct gpio_info param_poweroff;
static struct bl31_apio_param param_apio;
static struct gpio_info *rst_gpio;
static struct gpio_info *poweroff_gpio;
static struct gpio_info suspend_gpio[10];
uint32_t suspend_gpio_cnt;
static struct apio_info *suspend_apio;

struct gpio_info *plat_get_rockchip_gpio_reset(void)
{
	return rst_gpio;
}

struct gpio_info *plat_get_rockchip_gpio_poweroff(void)
{
	return poweroff_gpio;
}

struct gpio_info *plat_get_rockchip_suspend_gpio(uint32_t *count)
{
	*count = suspend_gpio_cnt;

	return &suspend_gpio[0];
}

struct apio_info *plat_get_rockchip_suspend_apio(void)
{
	return suspend_apio;
}

void params_early_setup(void *plat_param_from_bl2)
{
	struct bl31_plat_param *bl2_param;
	struct bl31_gpio_param *gpio_param;

	/* keep plat parameters for later processing if need */
	bl2_param = (struct bl31_plat_param *)plat_param_from_bl2;
	while (bl2_param) {
		switch (bl2_param->type) {
		case PARAM_RESET:
			gpio_param = (struct bl31_gpio_param *)bl2_param;
			memcpy(&param_reset, &gpio_param->gpio,
			       sizeof(struct gpio_info));
			rst_gpio = &param_reset;
			break;
		case PARAM_POWEROFF:
			gpio_param = (struct bl31_gpio_param *)bl2_param;
			memcpy(&param_poweroff, &gpio_param->gpio,
				sizeof(struct gpio_info));
			poweroff_gpio = &param_poweroff;
			break;
		case PARAM_SUSPEND_GPIO:
			if (suspend_gpio_cnt >= ARRAY_SIZE(suspend_gpio)) {
				ERROR("exceed support suspend gpio number\n");
				break;
			}
			gpio_param = (struct bl31_gpio_param *)bl2_param;
			memcpy(&suspend_gpio[suspend_gpio_cnt],
			       &gpio_param->gpio,
			       sizeof(struct gpio_info));
			suspend_gpio_cnt++;
			break;
		case PARAM_SUSPEND_APIO:
			memcpy(&param_apio, bl2_param,
			       sizeof(struct bl31_apio_param));
			suspend_apio = &param_apio.apio;
			break;
		default:
			ERROR("not expected type found %ld\n",
			      bl2_param->type);
			break;
		}
		bl2_param = bl2_param->next;
	}
}
