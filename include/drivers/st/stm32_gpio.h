/*
 * Copyright (c) 2015-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_GPIO_H
#define STM32_GPIO_H

#include <lib/utils_def.h>

#define GPIO_MODE_OFFSET	U(0x00)
#define GPIO_TYPE_OFFSET	U(0x04)
#define GPIO_SPEED_OFFSET	U(0x08)
#define GPIO_PUPD_OFFSET	U(0x0C)
#define GPIO_OD_OFFSET		U(0x14)
#define GPIO_BSRR_OFFSET	U(0x18)
#define GPIO_AFRL_OFFSET	U(0x20)
#define GPIO_AFRH_OFFSET	U(0x24)
#define GPIO_SECR_OFFSET	U(0x30)

#define GPIO_ALT_LOWER_LIMIT	U(0x08)

#define GPIO_PIN_(_x)		U(_x)
#define GPIO_PIN_MAX		GPIO_PIN_(15)

#define GPIO_ALTERNATE_(_x)	U(_x)
#define GPIO_ALTERNATE_MASK	U(0x0F)

#define GPIO_MODE_INPUT		U(0x00)
#define GPIO_MODE_OUTPUT	U(0x01)
#define GPIO_MODE_ALTERNATE	U(0x02)
#define GPIO_MODE_ANALOG	U(0x03)
#define GPIO_MODE_MASK		U(0x03)

#define GPIO_TYPE_PUSH_PULL	U(0x00)
#define GPIO_TYPE_OPEN_DRAIN	U(0x01)
#define GPIO_TYPE_MASK		U(0x01)

#define GPIO_SPEED_LOW		U(0x00)
#define GPIO_SPEED_MEDIUM	U(0x01)
#define GPIO_SPEED_HIGH		U(0x02)
#define GPIO_SPEED_VERY_HIGH	U(0x03)
#define GPIO_SPEED_MASK		U(0x03)

#define GPIO_NO_PULL		U(0x00)
#define GPIO_PULL_UP		U(0x01)
#define GPIO_PULL_DOWN		U(0x02)
#define GPIO_PULL_MASK		U(0x03)

#define GPIO_OD_OUTPUT_LOW	U(0x00)
#define GPIO_OD_OUTPUT_HIGH	U(0x01)
#define GPIO_OD_MASK		U(0x01)

#ifndef __ASSEMBLER__
#include <stdint.h>

int dt_set_pinctrl_config(int node);
void set_gpio_secure_cfg(uint32_t bank, uint32_t pin, bool secure);
void set_gpio_reset_cfg(uint32_t bank, uint32_t pin);
#endif /*__ASSEMBLER__*/

#endif /* STM32_GPIO_H */
