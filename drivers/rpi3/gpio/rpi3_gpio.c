/*
 * Copyright (c) 2019, Linaro Limited
 * Copyright (c) 2019, Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <assert.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>
#include <drivers/rpi3/gpio/rpi3_gpio.h>
#include <platform_def.h>

static uintptr_t reg_base;

static int rpi3_gpio_get_direction(int gpio);
static void rpi3_gpio_set_direction(int gpio, int direction);
static int rpi3_gpio_get_value(int gpio);
static void rpi3_gpio_set_value(int gpio, int value);
static void rpi3_gpio_set_pull(int gpio, int pull);

static const gpio_ops_t rpi3_gpio_ops = {
	.get_direction  = rpi3_gpio_get_direction,
	.set_direction  = rpi3_gpio_set_direction,
	.get_value      = rpi3_gpio_get_value,
	.set_value      = rpi3_gpio_set_value,
	.set_pull       = rpi3_gpio_set_pull,
};

/**
 * Get selection of GPIO pinmux settings.
 *
 * @param gpio The pin number of GPIO. From 0 to 53.
 * @return The selection of pinmux. RPI3_GPIO_FUNC_INPUT: input,
 *                                  RPI3_GPIO_FUNC_OUTPUT: output,
 *                                  RPI3_GPIO_FUNC_ALT0: alt-0,
 *                                  RPI3_GPIO_FUNC_ALT1: alt-1,
 *                                  RPI3_GPIO_FUNC_ALT2: alt-2,
 *                                  RPI3_GPIO_FUNC_ALT3: alt-3,
 *                                  RPI3_GPIO_FUNC_ALT4: alt-4,
 *                                  RPI3_GPIO_FUNC_ALT5: alt-5
 */
int rpi3_gpio_get_select(int gpio)
{
	int ret;
	int regN = gpio / 10;
	int shift = 3 * (gpio % 10);
	uintptr_t reg_sel = reg_base + RPI3_GPIO_GPFSEL(regN);
	uint32_t sel = mmio_read_32(reg_sel);

	ret = (sel >> shift) & 0x07;

	return ret;
}

/**
 * Set selection of GPIO pinmux settings.
 *
 * @param gpio The pin number of GPIO. From 0 to 53.
 * @param fsel The selection of pinmux. RPI3_GPIO_FUNC_INPUT: input,
 *                                      RPI3_GPIO_FUNC_OUTPUT: output,
 *                                      RPI3_GPIO_FUNC_ALT0: alt-0,
 *                                      RPI3_GPIO_FUNC_ALT1: alt-1,
 *                                      RPI3_GPIO_FUNC_ALT2: alt-2,
 *                                      RPI3_GPIO_FUNC_ALT3: alt-3,
 *                                      RPI3_GPIO_FUNC_ALT4: alt-4,
 *                                      RPI3_GPIO_FUNC_ALT5: alt-5
 */
void rpi3_gpio_set_select(int gpio, int fsel)
{
	int regN = gpio / 10;
	int shift = 3 * (gpio % 10);
	uintptr_t reg_sel = reg_base + RPI3_GPIO_GPFSEL(regN);
	uint32_t sel = mmio_read_32(reg_sel);
	uint32_t mask = U(0x07) << shift;

	sel = (sel & (~mask)) | ((fsel << shift) & mask);
	mmio_write_32(reg_sel, sel);
}

static int rpi3_gpio_get_direction(int gpio)
{
	int result = rpi3_gpio_get_select(gpio);

	if (result == RPI3_GPIO_FUNC_INPUT)
		return GPIO_DIR_IN;
	else if (result == RPI3_GPIO_FUNC_OUTPUT)
		return GPIO_DIR_OUT;

	return GPIO_DIR_IN;
}

static void rpi3_gpio_set_direction(int gpio, int direction)
{
	switch (direction) {
	case GPIO_DIR_IN:
		rpi3_gpio_set_select(gpio, RPI3_GPIO_FUNC_INPUT);
		break;
	case GPIO_DIR_OUT:
		rpi3_gpio_set_select(gpio, RPI3_GPIO_FUNC_OUTPUT);
		break;
	}
}

static int rpi3_gpio_get_value(int gpio)
{
	int regN = gpio / 32;
	int shift = gpio % 32;
	uintptr_t reg_lev = reg_base + RPI3_GPIO_GPLEV(regN);
	uint32_t value = mmio_read_32(reg_lev);

	if ((value >> shift) & 0x01)
		return GPIO_LEVEL_HIGH;
	return GPIO_LEVEL_LOW;
}

static void rpi3_gpio_set_value(int gpio, int value)
{
	int regN = gpio / 32;
	int shift = gpio % 32;
	uintptr_t reg_set = reg_base + RPI3_GPIO_GPSET(regN);
	uintptr_t reg_clr = reg_base + RPI3_GPIO_GPSET(regN);

	switch (value) {
	case GPIO_LEVEL_LOW:
		mmio_write_32(reg_clr, U(1) << shift);
		break;
	case GPIO_LEVEL_HIGH:
		mmio_write_32(reg_set, U(1) << shift);
		break;
	}
}

static void rpi3_gpio_set_pull(int gpio, int pull)
{
	int regN = gpio / 32;
	int shift = gpio % 32;
	uintptr_t reg_pud = reg_base + RPI3_GPIO_GPPUD;
	uintptr_t reg_clk = reg_base + RPI3_GPIO_GPPUDCLK(regN);

	switch (pull) {
	case GPIO_PULL_NONE:
		mmio_write_32(reg_pud, 0x0);
		break;
	case GPIO_PULL_UP:
		mmio_write_32(reg_pud, 0x2);
		break;
	case GPIO_PULL_DOWN:
		mmio_write_32(reg_pud, 0x1);
		break;
	}
	mdelay(150);
	mmio_write_32(reg_clk, U(1) << shift);
	mdelay(150);
	mmio_write_32(reg_clk, 0x0);
	mmio_write_32(reg_pud, 0x0);
}

void rpi3_gpio_init(void)
{
	reg_base = RPI3_GPIO_BASE;
	gpio_init(&rpi3_gpio_ops);
}
