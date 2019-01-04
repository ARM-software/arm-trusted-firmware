/*
 * Copyright (c) 2015-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32_GPIO_H
#define STM32_GPIO_H

#include <lib/utils_def.h>

#define STM32_GPIOA_BANK	U(0x50002000)
#define STM32_GPIOZ_BANK	U(0x54004000)
#define STM32_GPIO_BANK_OFFSET	U(0x1000)

#define GPIO_MODE_OFFSET	U(0x00)
#define GPIO_TYPE_OFFSET	U(0x04)
#define GPIO_SPEED_OFFSET	U(0x08)
#define GPIO_PUPD_OFFSET	U(0x0C)
#define GPIO_BSRR_OFFSET	U(0x18)
#define GPIO_AFRL_OFFSET	U(0x20)
#define GPIO_AFRH_OFFSET	U(0x24)

#define GPIO_ALT_LOWER_LIMIT	U(0x08)

#define GPIO_BANK_A		U(0x00)
#define GPIO_BANK_B		U(0x01)
#define GPIO_BANK_C		U(0x02)
#define GPIO_BANK_D		U(0x03)
#define GPIO_BANK_E		U(0x04)
#define GPIO_BANK_F		U(0x05)
#define GPIO_BANK_G		U(0x06)
#define GPIO_BANK_H		U(0x07)
#define GPIO_BANK_I		U(0x08)
#define GPIO_BANK_J		U(0x09)
#define GPIO_BANK_K		U(0x0A)
#define GPIO_BANK_Z		U(0x19)

#define GPIO_PIN_0		U(0x00)
#define GPIO_PIN_1		U(0x01)
#define GPIO_PIN_2		U(0x02)
#define GPIO_PIN_3		U(0x03)
#define GPIO_PIN_4		U(0x04)
#define GPIO_PIN_5		U(0x05)
#define GPIO_PIN_6		U(0x06)
#define GPIO_PIN_7		U(0x07)
#define GPIO_PIN_8		U(0x08)
#define GPIO_PIN_9		U(0x09)
#define GPIO_PIN_10		U(0x0A)
#define GPIO_PIN_11		U(0x0B)
#define GPIO_PIN_12		U(0x0C)
#define GPIO_PIN_13		U(0x0D)
#define GPIO_PIN_14		U(0x0E)
#define GPIO_PIN_15		U(0x0F)
#define GPIO_PIN_MAX		GPIO_PIN_15

#define GPIO_ALTERNATE_0	0x00
#define GPIO_ALTERNATE_1	0x01
#define GPIO_ALTERNATE_2	0x02
#define GPIO_ALTERNATE_3	0x03
#define GPIO_ALTERNATE_4	0x04
#define GPIO_ALTERNATE_5	0x05
#define GPIO_ALTERNATE_6	0x06
#define GPIO_ALTERNATE_7	0x07
#define GPIO_ALTERNATE_8	0x08
#define GPIO_ALTERNATE_9	0x09
#define GPIO_ALTERNATE_10	0x0A
#define GPIO_ALTERNATE_11	0x0B
#define GPIO_ALTERNATE_12	0x0C
#define GPIO_ALTERNATE_13	0x0D
#define GPIO_ALTERNATE_14	0x0E
#define GPIO_ALTERNATE_15	0x0F
#define GPIO_ALTERNATE_MASK	U(0x0F)

#define GPIO_MODE_INPUT		0x00
#define GPIO_MODE_OUTPUT	0x01
#define GPIO_MODE_ALTERNATE	0x02
#define GPIO_MODE_ANALOG	0x03
#define GPIO_MODE_MASK		U(0x03)

#define GPIO_OPEN_DRAIN		U(0x10)

#define GPIO_SPEED_LOW		0x00
#define GPIO_SPEED_MEDIUM	0x01
#define GPIO_SPEED_FAST		0x02
#define GPIO_SPEED_HIGH		0x03
#define GPIO_SPEED_MASK		U(0x03)

#define GPIO_NO_PULL		0x00
#define GPIO_PULL_UP		0x01
#define GPIO_PULL_DOWN		0x02
#define GPIO_PULL_MASK		U(0x03)

#ifndef __ASSEMBLY__
#include <stdint.h>

void set_gpio(uint32_t bank, uint32_t pin, uint32_t mode, uint32_t speed,
	      uint32_t pull, uint32_t alternate);
#endif /*__ASSEMBLY__*/

#endif /* STM32_GPIO_H */
