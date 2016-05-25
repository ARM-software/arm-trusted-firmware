/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <gpio.h>
#include <mmio.h>
#include <platform.h>
#include <plat_params.h>
#include <plat_private.h>
#include <string.h>

static struct bl31_plat_param *bl31_params_head;
static struct bl31_gpio_param param_reset;
static struct bl31_gpio_param param_poweroff;
static struct gpio_info *rst_gpio;
static struct gpio_info *poweroff_gpio;

void *plat_get_rockchip_gpio_reset(void)
{
	return rst_gpio;
}

void *plat_get_rockchip_gpio_poweroff(void)
{
	return poweroff_gpio;
}

void params_early_setup(void *plat_param_from_bl2)
{
	struct bl31_plat_param *param;
	struct bl31_plat_param *bl2_param;
	struct bl31_gpio_param *gpio_param;

	/* keep plat parameters for later processing if need */
	bl2_param = (struct bl31_plat_param *)plat_param_from_bl2;
	while (bl2_param) {
		switch (bl2_param->type) {
		case PARAM_RESET:
			param = (struct bl31_plat_param *)&param_reset;
			memcpy((void *)param, (void *)bl2_param,
				sizeof(struct bl31_gpio_param));
			gpio_param = (struct bl31_gpio_param *)param;
			rst_gpio = &gpio_param->gpio;
			break;
		case PARAM_POWEROFF:
			param = (struct bl31_plat_param *)&param_poweroff;
			memcpy((void *)param, (void *)bl2_param,
				sizeof(struct bl31_gpio_param));
			gpio_param = (struct bl31_gpio_param *)param;
			poweroff_gpio = &gpio_param->gpio;
			break;
		default:
			NOTICE("not expected type found\n");
			return; /* don't continue if unexpected type found */
		}
		param->next = bl31_params_head;
		bl31_params_head = param;
		bl2_param = bl2_param->next;
	}
}
