/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio.h>
#include <lib/mmio.h>
#include <mtgpio.h>
#include <platform_def.h>

/******************************************************************************
 *Macro Definition
 ******************************************************************************/
#define GPIO_MODE_BITS		4
#define MAX_GPIO_MODE_PER_REG	8
#define MAX_GPIO_REG_BITS	32
#define DIR_BASE		(GPIO_BASE + 0x000)
#define DOUT_BASE		(GPIO_BASE + 0x100)
#define DIN_BASE		(GPIO_BASE + 0x200)
#define MODE_BASE		(GPIO_BASE + 0x300)
#define SET			0x4
#define CLR			0x8

static void mt_set_gpio_dir_chip(uint32_t pin, int dir)
{
	uint32_t pos, bit;

	assert(pin < MAX_GPIO_PIN);
	assert(dir < MT_GPIO_DIR_MAX);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (dir == MT_GPIO_DIR_IN) {
		mmio_write_32(DIR_BASE + 0x10U * pos + CLR, 1U << bit);
	} else {
		mmio_write_32(DIR_BASE + 0x10U * pos + SET, 1U << bit);
	}
}

static int mt_get_gpio_dir_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t reg;

	assert(pin < MAX_GPIO_PIN);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = mmio_read_32(DIR_BASE + 0x10U * pos);
	return (((reg & (1U << bit)) != 0U) ? MT_GPIO_DIR_OUT : MT_GPIO_DIR_IN);
}

static void mt_set_gpio_out_chip(uint32_t pin, int output)
{
	uint32_t pos, bit;

	assert(pin < MAX_GPIO_PIN);
	assert(output < MT_GPIO_OUT_MAX);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (output == MT_GPIO_OUT_ZERO) {
		mmio_write_32(DOUT_BASE + 0x10U * pos + CLR, 1U << bit);
	} else {
		mmio_write_32(DOUT_BASE + 0x10U * pos + SET, 1U << bit);
	}
}

static int mt_get_gpio_in_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t reg;

	assert(pin < MAX_GPIO_PIN);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = mmio_read_32(DIN_BASE + 0x10U * pos);
	return (((reg & (1U << bit)) != 0U) ? 1 : 0);
}

static void mt_gpio_set_spec_pull_pupd(uint32_t pin, int enable,
			       int select)
{
	uintptr_t reg1;
	uintptr_t reg2;
	struct mt_pin_info gpio_info;

	gpio_info = mt_pin_infos[pin];
	uint32_t bit = gpio_info.bit;

	reg1 = mt_gpio_find_reg_addr(pin) + gpio_info.offset;
	reg2 = reg1 + (gpio_info.base & 0xf0);
	if (enable == MT_GPIO_PULL_ENABLE) {
		mmio_write_32(reg2 + SET, (1U << bit));
		if (select == MT_GPIO_PULL_DOWN) {
			mmio_write_32(reg1 + SET, (1U << bit));
		} else {
			mmio_write_32(reg1 + CLR, (1U << bit));
		}
	} else {
		mmio_write_32(reg2 + CLR, (1U << bit));
		mmio_write_32((reg2 + 0x010U) + CLR, (1U << bit));
	}
}

static void mt_gpio_set_pull_pu_pd(uint32_t pin, int enable,
				 int select)
{
	uintptr_t reg1;
	uintptr_t reg2;
	struct mt_pin_info gpio_info;

	gpio_info = mt_pin_infos[pin];
	uint32_t bit = gpio_info.bit;

	reg1 = mt_gpio_find_reg_addr(pin) + gpio_info.offset;
	reg2 = reg1 - (gpio_info.base & 0xf0);

	if (enable == MT_GPIO_PULL_ENABLE) {
		if (select == MT_GPIO_PULL_DOWN) {
			mmio_write_32(reg1 + CLR, (1U << bit));
			mmio_write_32(reg2 + SET, (1U << bit));
		} else {
			mmio_write_32(reg2 + CLR, (1U << bit));
			mmio_write_32(reg1 + SET, (1U << bit));
		}
	} else {
		mmio_write_32(reg1 + CLR, (1U << bit));
		mmio_write_32(reg2 + CLR, (1U << bit));
	}
}

static void mt_gpio_set_pull_chip(uint32_t pin, int enable,
		   int select)
{
	struct mt_pin_info gpio_info;

	gpio_info = mt_pin_infos[pin];
	if (gpio_info.flag) {
		mt_gpio_set_spec_pull_pupd(pin, enable, select);
	} else {
		mt_gpio_set_pull_pu_pd(pin, enable, select);
	}
}

static int mt_gpio_get_spec_pull_pupd(uint32_t pin)
{
	uintptr_t reg1;
	uintptr_t reg2;
	uint32_t r0;
	uint32_t r1;

	struct mt_pin_info gpio_info;

	gpio_info = mt_pin_infos[pin];
	uint32_t bit = gpio_info.bit;

	reg1 = mt_gpio_find_reg_addr(pin) + gpio_info.offset;
	reg2 = reg1 + (gpio_info.base & 0xf0);

	r0 = (mmio_read_32(reg2) >> bit) & 1U;
	r1 = (mmio_read_32(reg2 + 0x010) >> bit) & 1U;
	if (r0 == 0U && r1 == 0U) {
		return MT_GPIO_PULL_NONE;
	} else {
		if (mmio_read_32(reg1) & (1U << bit)) {
			return MT_GPIO_PULL_DOWN;
		} else {
			return MT_GPIO_PULL_UP;
		}
	}
}

static int mt_gpio_get_pull_pu_pd(uint32_t pin)
{
	uintptr_t reg1;
	uintptr_t reg2;
	uint32_t pu;
	uint32_t pd;

	struct mt_pin_info gpio_info;

	gpio_info = mt_pin_infos[pin];
	uint32_t bit = gpio_info.bit;

	reg1 = mt_gpio_find_reg_addr(pin) + gpio_info.offset;
	reg2 = reg1 - (gpio_info.base & 0xf0);
	pu = (mmio_read_32(reg1) >> bit) & 1U;
	pd = (mmio_read_32(reg2) >> bit) & 1U;
	if (pu == 1U) {
		return MT_GPIO_PULL_UP;
	} else if (pd == 1U) {
		return MT_GPIO_PULL_DOWN;
	} else {
		return MT_GPIO_PULL_NONE;
	}
}

static int mt_gpio_get_pull_chip(uint32_t pin)
{
	struct mt_pin_info gpio_info;

	gpio_info = mt_pin_infos[pin];
	if (gpio_info.flag) {
		return mt_gpio_get_spec_pull_pupd(pin);
	} else {
		return mt_gpio_get_pull_pu_pd(pin);
	}
}

static void mt_set_gpio_pull_select_chip(uint32_t pin, int sel)
{
	assert(pin < MAX_GPIO_PIN);

	if (sel == MT_GPIO_PULL_NONE) {
		mt_gpio_set_pull_chip(pin, MT_GPIO_PULL_DISABLE, MT_GPIO_PULL_DOWN);
	} else if (sel == MT_GPIO_PULL_UP) {
		mt_gpio_set_pull_chip(pin, MT_GPIO_PULL_ENABLE, MT_GPIO_PULL_UP);
	} else if (sel == MT_GPIO_PULL_DOWN) {
		mt_gpio_set_pull_chip(pin, MT_GPIO_PULL_ENABLE, MT_GPIO_PULL_DOWN);
	}
}

/* get pull-up or pull-down, regardless of resistor value */
static int mt_get_gpio_pull_select_chip(uint32_t pin)
{
	assert(pin < MAX_GPIO_PIN);

	return mt_gpio_get_pull_chip(pin);
}

static void mt_set_gpio_dir(int gpio, int direction)
{
	mt_set_gpio_dir_chip((uint32_t)gpio, direction);
}

static int mt_get_gpio_dir(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_dir_chip(pin);
}

static void mt_set_gpio_pull(int gpio, int pull)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	mt_set_gpio_pull_select_chip(pin, pull);
}

static int mt_get_gpio_pull(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_pull_select_chip(pin);
}

static void mt_set_gpio_out(int gpio, int value)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	mt_set_gpio_out_chip(pin, value);
}

static int mt_get_gpio_in(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_in_chip(pin);
}

const gpio_ops_t mtgpio_ops = {
	 .get_direction = mt_get_gpio_dir,
	 .set_direction = mt_set_gpio_dir,
	 .get_value = mt_get_gpio_in,
	 .set_value = mt_set_gpio_out,
	 .set_pull = mt_set_gpio_pull,
	 .get_pull = mt_get_gpio_pull,
};

void mt_gpio_init(void)
{
	gpio_init(&mtgpio_ops);
}
