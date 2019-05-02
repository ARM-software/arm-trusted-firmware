/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <gpio/mtgpio.h>
#include <gpio/mtgpio_cfg.h>
#include <drivers/gpio.h>
#include <mcucfg.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <spm.h>
#include <stdbool.h>

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
#define PULLEN_ADDR_OFFSET	0x060
#define PULLSEL_ADDR_OFFSET	0x080

void mt_set_gpio_dir_chip(uint32_t pin, int dir)
{
	uint32_t pos, bit;

	assert(pin < MAX_GPIO_PIN);
	assert(dir < GPIO_DIR_MAX);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (dir == GPIO_DIR_IN)
		mmio_write_32(DIR_BASE + 0x10 * pos + CLR, 1U << bit);
	else
		mmio_write_32(DIR_BASE + 0x10 * pos + SET, 1U << bit);
}

int mt_get_gpio_dir_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t reg;

	assert(pin < MAX_GPIO_PIN);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = mmio_read_32(DIR_BASE + 0x10 * pos);
	return (((reg & (1U << bit)) != 0) ? GPIO_DIR_OUT : GPIO_DIR_IN);
}

void mt_set_gpio_out_chip(uint32_t pin, int output)
{
	uint32_t pos, bit;

	assert(pin < MAX_GPIO_PIN);
	assert(output < GPIO_OUT_MAX);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (output == GPIO_OUT_ZERO)
		mmio_write_32(DOUT_BASE + 0x10 * pos + CLR, 1U << bit);
	else
		mmio_write_32(DOUT_BASE + 0x10 * pos + SET, 1U << bit);
}

int mt_get_gpio_out_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t reg;

	assert(pin < MAX_GPIO_PIN);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = mmio_read_32(DOUT_BASE + 0x10 * pos);
	return (((reg & (1U << bit)) != 0) ? 1 : 0);
}

int mt_get_gpio_in_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t reg;

	assert(pin < MAX_GPIO_PIN);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = mmio_read_32(DIN_BASE + 0x10 * pos);
	return (((reg & (1U << bit)) != 0) ? 1 : 0);
}

void mt_set_gpio_mode_chip(uint32_t pin, int mode)
{
	uint32_t pos, bit;
	uint32_t data;
	uint32_t mask;

	assert(pin < MAX_GPIO_PIN);
	assert(mode < GPIO_MODE_MAX);

	mask = (1U << GPIO_MODE_BITS) - 1;

	mode = mode & mask;
	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = (pin % MAX_GPIO_MODE_PER_REG) * GPIO_MODE_BITS;

	data = mmio_read_32(MODE_BASE + 0x10 * pos);
	data &= (~(mask << bit));
	data |= (mode << bit);
	mmio_write_32(MODE_BASE + 0x10 * pos, data);
}

int mt_get_gpio_mode_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t data;
	uint32_t mask;

	assert(pin < MAX_GPIO_PIN);

	mask = (1U << GPIO_MODE_BITS) - 1;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = (pin % MAX_GPIO_MODE_PER_REG) * GPIO_MODE_BITS;

	data = mmio_read_32(MODE_BASE + 0x10 * pos);
	return (data >> bit) & mask;
}

int32_t gpio_get_pull_iocfg(uint32_t pin)
{
	switch (pin) {
	case 0 ... 10:
		return IOCFG_5_BASE;
	case 11 ... 12:
		return IOCFG_0_BASE;
	case 13 ... 28:
		return IOCFG_1_BASE;
	case 43 ... 49:
		return IOCFG_2_BASE;
	case 50 ... 60:
		return IOCFG_3_BASE;
	case 61 ... 88:
		return IOCFG_4_BASE;
	case 89 ... 90:
		return IOCFG_5_BASE;
	case 95 ... 106:
		return IOCFG_5_BASE;
	case 107 ... 121:
		return IOCFG_6_BASE;
	case 134 ... 160:
		return IOCFG_0_BASE;
	case 161 ... 166:
		return IOCFG_1_BASE;
	case 167 ... 176:
		return IOCFG_3_BASE;
	case 177 ... 179:
		return IOCFG_5_BASE;
	default:
		return -1;
	}
}

int32_t gpio_get_pupd_iocfg(uint32_t pin)
{
	const int32_t offset = 0x0c0;

	switch (pin) {
	case 29 ... 34:
		return IOCFG_1_BASE + offset;
	case 35 ... 42:
		return IOCFG_2_BASE + offset;
	case 91 ... 94:
		return IOCFG_5_BASE + offset;
	case 122 ... 133:
		return IOCFG_7_BASE + offset;
	default:
		return -1;
	}
}

int gpio_get_pupd_offset(uint32_t pin)
{
	switch (pin) {
	case 29 ... 34:
		return (pin - 29) * 4 % 32;
	case 35 ... 42:
		return (pin - 35) * 4 % 32;
	case 91 ... 94:
		return (pin - 91) * 4 % 32;
	case 122 ... 129:
		return (pin - 122) * 4 % 32;
	case 130 ... 133:
		return (pin - 130) * 4 % 32;
	default:
		return -1;
	}
}

void mt_set_gpio_pull_enable_chip(uint32_t pin, int en)
{
	int pullen_addr = gpio_get_pull_iocfg(pin) + PULLEN_ADDR_OFFSET;
	int pupd_addr = gpio_get_pupd_iocfg(pin);
	int pupd_offset = gpio_get_pupd_offset(pin);

	assert(pin < MAX_GPIO_PIN);

	assert(!((PULL_offset[pin].offset == (int8_t)-1) &&
		(pupd_offset == (int8_t)-1)));

	if (en == GPIO_PULL_DISABLE) {
		if (PULL_offset[pin].offset == (int8_t)-1)
			mmio_clrbits_32(pupd_addr, 3U << pupd_offset);
		else
			mmio_clrbits_32(pullen_addr,
					1U << PULL_offset[pin].offset);
	} else if (en == GPIO_PULL_ENABLE) {
		if (PULL_offset[pin].offset == (int8_t)-1) {
			/* For PUPD+R0+R1 Type, mt_set_gpio_pull_enable
			 * does not know
			 * which one between PU and PD shall be enabled.
			 * Use R0 to guarantee at one resistor is set when lk
			 * apply default setting
			 */
			mmio_setbits_32(pupd_addr, 1U << pupd_offset);
			mmio_clrbits_32(pupd_addr, 1U << (pupd_offset + 1));
		} else {
			/* For PULLEN + PULLSEL Type */
			mmio_setbits_32(pullen_addr,
					1U << PULL_offset[pin].offset);
		}
	} else if (en == GPIO_PULL_ENABLE_R0) {
		assert(!(pupd_offset == (int8_t)-1));
		mmio_setbits_32(pupd_addr, 1U << pupd_offset);
		mmio_clrbits_32(pupd_addr, 1U << (pupd_offset + 1));
	} else if (en == GPIO_PULL_ENABLE_R1) {
		assert(!(pupd_offset == (int8_t)-1));

		mmio_clrbits_32(pupd_addr, 1U << pupd_offset);
		mmio_setbits_32(pupd_addr, 1U << (pupd_offset + 1));
	} else if (en == GPIO_PULL_ENABLE_R0R1) {
		assert(!(pupd_offset == (int8_t)-1));
		mmio_setbits_32(pupd_addr, 3U << pupd_offset);
	}
}

int mt_get_gpio_pull_enable_chip(uint32_t pin)
{
	uint32_t reg;

	int pullen_addr = gpio_get_pull_iocfg(pin) + PULLEN_ADDR_OFFSET;
	int pupd_addr = gpio_get_pupd_iocfg(pin);
	int pupd_offset = gpio_get_pupd_offset(pin);

	assert(pin < MAX_GPIO_PIN);

	assert(!((PULL_offset[pin].offset == (int8_t)-1) &&
		(pupd_offset == (int8_t)-1)));

	if (PULL_offset[pin].offset == (int8_t)-1) {
		reg = mmio_read_32(pupd_addr);
		return ((reg & (3U << pupd_offset)) ? 1 : 0);
	} else if (pupd_offset == (int8_t)-1) {
		reg = mmio_read_32(pullen_addr);
		return ((reg & (1U << PULL_offset[pin].offset)) ? 1 : 0);
	}

	return -ERINVAL;
}

void mt_set_gpio_pull_select_chip(uint32_t pin, int sel)
{
	int pullsel_addr = gpio_get_pull_iocfg(pin) + PULLSEL_ADDR_OFFSET;
	int pupd_addr = gpio_get_pupd_iocfg(pin);
	int pupd_offset = gpio_get_pupd_offset(pin);

	assert(pin < MAX_GPIO_PIN);

	assert(!((PULL_offset[pin].offset == (int8_t) -1) &&
		(pupd_offset == (int8_t)-1)));

	if (sel == GPIO_PULL_NONE) {
		/*  Regard No PULL as PULL disable + pull down */
		mt_set_gpio_pull_enable_chip(pin, GPIO_PULL_DISABLE);
		if (PULL_offset[pin].offset == (int8_t)-1)
			mmio_setbits_32(pupd_addr, 1U << (pupd_offset + 2));
		else
			mmio_clrbits_32(pullsel_addr,
					1U << PULL_offset[pin].offset);
	} else if (sel == GPIO_PULL_UP) {
		mt_set_gpio_pull_enable_chip(pin, GPIO_PULL_ENABLE);
		if (PULL_offset[pin].offset == (int8_t)-1)
			mmio_clrbits_32(pupd_addr, 1U << (pupd_offset + 2));
		else
			mmio_setbits_32(pullsel_addr,
					1U << PULL_offset[pin].offset);
	} else if (sel == GPIO_PULL_DOWN) {
		mt_set_gpio_pull_enable_chip(pin, GPIO_PULL_ENABLE);
		if (PULL_offset[pin].offset == -1)
			mmio_setbits_32(pupd_addr, 1U << (pupd_offset + 2));
		else
			mmio_clrbits_32(pullsel_addr,
					1U << PULL_offset[pin].offset);
	}
}

/* get pull-up or pull-down, regardless of resistor value */
int mt_get_gpio_pull_select_chip(uint32_t pin)
{
	uint32_t reg;

	int pullen_addr = gpio_get_pull_iocfg(pin) + PULLEN_ADDR_OFFSET;
	int pullsel_addr = gpio_get_pull_iocfg(pin) + PULLSEL_ADDR_OFFSET;
	int pupd_addr = gpio_get_pupd_iocfg(pin);
	int pupd_offset = gpio_get_pupd_offset(pin);

	assert(pin < MAX_GPIO_PIN);

	assert(!((PULL_offset[pin].offset == (int8_t)-1) &&
		(pupd_offset == (int8_t)-1)));

	if (PULL_offset[pin].offset == (int8_t)-1) {
		reg = mmio_read_32(pupd_addr);
		if (reg & (3U << pupd_offset)) {
			reg = mmio_read_32(pupd_addr);
			/* Reg value: 0 for PU, 1 for PD -->
			 * reverse return value */
			return ((reg & (1U << (pupd_offset + 2))) ?
					GPIO_PULL_DOWN : GPIO_PULL_UP);
		} else {
			return GPIO_PULL_NONE;
		}
	} else if (pupd_offset == (int8_t)-1) {
		reg = mmio_read_32(pullen_addr);
		if ((reg & (1U << PULL_offset[pin].offset))) {
			reg = mmio_read_32(pullsel_addr);
			return ((reg & (1U << PULL_offset[pin].offset)) ?
					GPIO_PULL_UP : GPIO_PULL_DOWN);
		} else {
			return GPIO_PULL_NONE;
		}
	}

	return -ERINVAL;
}

void mt_set_gpio_dir(int gpio, int direction)
{
	mt_set_gpio_dir_chip((uint32_t)gpio, direction);
}

int mt_get_gpio_dir(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_dir_chip(pin);
}

void mt_set_gpio_pull(int gpio, int pull)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	mt_set_gpio_pull_select_chip(pin, pull);
}

int mt_get_gpio_pull(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_pull_select_chip(pin);
}

void mt_set_gpio_out(int gpio, int value)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	mt_set_gpio_out_chip(pin, value);
}

int mt_get_gpio_out(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_out_chip(pin);
}

int mt_get_gpio_in(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_in_chip(pin);
}

void mt_set_gpio_mode(int gpio, int mode)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	mt_set_gpio_mode_chip(pin, mode);
}

int mt_get_gpio_mode(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_mode_chip(pin);
}

const gpio_ops_t mtgpio_ops = {
	 .get_direction = mt_get_gpio_dir,
	 .set_direction = mt_set_gpio_dir,
	 .get_value = mt_get_gpio_in,
	 .set_value = mt_set_gpio_out,
	 .set_pull = mt_set_gpio_pull,
	 .get_pull = mt_get_gpio_pull,
};
