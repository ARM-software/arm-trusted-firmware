/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <platform_def.h>
#include <mtgpio.h>

typedef enum {
	REG_0 = 0,
	REG_1,
	REG_2,
	REG_3,
	REG_4,
	REG_5,
	REG_6,
	REG_7,
	REG_8,
	REG_9,
	REG_10,
	REG_11,
	REG_12,
	REG_13,
	REG_14,
	REG_15
} RegEnum;

uintptr_t mt_gpio_find_reg_addr(uint32_t pin)
{
	uintptr_t reg_addr = 0U;
	struct mt_pin_info gpio_info;

	assert(pin < MAX_GPIO_PIN);

	gpio_info = mt_pin_infos[pin];

	switch (gpio_info.base & 0xF) {
	case REG_0:
		reg_addr = GPIO_BASE;
		break;
	case REG_1:
		reg_addr = IOCFG_RT_BASE;
		break;
	case REG_2:
		reg_addr = IOCFG_RM1_BASE;
		break;
	case REG_3:
		reg_addr = IOCFG_RM2_BASE;
		break;
	case REG_4:
		reg_addr = IOCFG_RB_BASE;
		break;
	case REG_5:
		reg_addr = IOCFG_BM1_BASE;
		break;
	case REG_6:
		reg_addr = IOCFG_BM2_BASE;
		break;
	case REG_7:
		reg_addr = IOCFG_BM3_BASE;
		break;
	case REG_8:
		reg_addr = IOCFG_LT_BASE;
		break;
	case REG_9:
		reg_addr = IOCFG_LM1_BASE;
		break;
	case REG_10:
		reg_addr = IOCFG_LM2_BASE;
		break;
	case REG_11:
		reg_addr = IOCFG_LB1_BASE;
		break;
	case REG_12:
		reg_addr = IOCFG_LB2_BASE;
		break;
	case REG_13:
		reg_addr = IOCFG_TM1_BASE;
		break;
	case REG_14:
		reg_addr = IOCFG_TM2_BASE;
		break;
	case REG_15:
		reg_addr = IOCFG_TM3_BASE;
		break;
	default:
		break;
	}

	return reg_addr;
}
