/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <mtgpio.h>
#include <platform_def.h>

uintptr_t mt_gpio_find_reg_addr(uint32_t pin)
{
	uintptr_t reg_addr = 0U;
	struct mt_pin_info gpio_info;

	assert(pin < MAX_GPIO_PIN);

	gpio_info = mt_pin_infos[pin];

	switch (gpio_info.base & 0x0f) {
	case 0:
		reg_addr = IOCFG_BM_BASE;
		break;
	case 1:
		reg_addr = IOCFG_BL_BASE;
		break;
	case 2:
		reg_addr = IOCFG_BR_BASE;
		break;
	case 3:
		reg_addr = IOCFG_LM_BASE;
		break;
	case 4:
		reg_addr = IOCFG_RB_BASE;
		break;
	case 5:
		reg_addr = IOCFG_TL_BASE;
		break;
	default:
		break;
	}

	return reg_addr;
}
