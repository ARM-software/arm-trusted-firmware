/*
 * Copyright (c) 2019, Linaro Limited
 * Copyright (c) 2019, Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPI3_GPIO_H
#define RPI3_GPIO_H

#include <stdint.h>
#include <drivers/gpio.h>

void rpi3_gpio_init(void);
int rpi3_gpio_get_select(int gpio);
void rpi3_gpio_set_select(int gpio, int fsel);

#define RPI3_GPIO_GPFSEL(n)	((n) * U(0x04))
#define RPI3_GPIO_GPSET(n)	(((n) * U(0x04)) + U(0x1C))
#define RPI3_GPIO_GPCLR(n)	(((n) * U(0x04)) + U(0x28))
#define RPI3_GPIO_GPLEV(n) 	(((n) * U(0x04)) + U(0x34))
#define RPI3_GPIO_GPPUD		U(0x94)
#define RPI3_GPIO_GPPUDCLK(n)	(((n) * U(0x04)) + U(0x98))

#define RPI3_GPIO_FUNC_INPUT	U(0)
#define RPI3_GPIO_FUNC_OUTPUT	U(1)
#define RPI3_GPIO_FUNC_ALT0	U(4)
#define RPI3_GPIO_FUNC_ALT1	U(5)
#define RPI3_GPIO_FUNC_ALT2	U(6)
#define RPI3_GPIO_FUNC_ALT3	U(7)
#define RPI3_GPIO_FUNC_ALT4	U(3)
#define RPI3_GPIO_FUNC_ALT5	U(2)

#endif  /* RPI3_GPIO_H */
