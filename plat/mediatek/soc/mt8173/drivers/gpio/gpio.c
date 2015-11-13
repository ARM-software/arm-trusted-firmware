/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <debug.h>
#include <gpio.h>
#include <mmio.h>
#include <mt8173_def.h>
#include <pmic_wrap_init.h>

enum {
	MAX_GPIO_REG_BITS = 16,
};

struct mt_gpio_obj {
	struct gpio_regs *reg;
};

static struct mt_gpio_obj gpio_dat = {
	.reg = (struct gpio_regs *)(GPIO_BASE),
};

static struct mt_gpio_obj *gpio_obj = &gpio_dat;

struct mt_gpioext_obj {
	struct gpioext_regs *reg;
};

static struct mt_gpioext_obj gpioext_dat = {
	.reg = (struct gpioext_regs *)(GPIOEXT_BASE),
};

static struct mt_gpioext_obj *gpioext_obj = &gpioext_dat;

static inline struct mt_gpio_obj *mt_get_gpio_obj(void)
{
	return gpio_obj;
}

static inline struct mt_gpioext_obj *mt_get_gpioext_obj(void)
{
	return gpioext_obj;
}

enum {
	GPIO_PRO_DIR = 0,
	GPIO_PRO_DOUT,
	GPIO_PRO_DIN,
	GPIO_PRO_PULLEN,
	GPIO_PRO_PULLSEL,
	GPIO_PRO_MODE,
	GPIO_PRO_MAX,
};

static inline int32_t gpioext_write(uint16_t *addr, int64_t data)
{
	return pwrap_write((uint32_t)(uintptr_t)addr, data);
}

static inline int32_t gpioext_set_bits(uint32_t bit, uint16_t *reg)
{
	return gpioext_write(reg, bit);
}

static int32_t mt_set_gpio_chip(uint32_t pin, uint32_t property, uint32_t val)
{
	uint32_t pos = 0;
	uint32_t bit = 0;
	struct mt_gpio_obj *obj = mt_get_gpio_obj();
	uint16_t *reg;
	uint32_t data = 0;

	if (!obj)
		return -ERACCESS;

	if (pin >= GPIO_EXTEND_START)
		return -ERINVAL;

	if (property >= GPIO_PRO_MAX)
		return -ERINVAL;

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;
	data = 1L << bit;

	switch (property) {
	case GPIO_PRO_DIR:
		if (val == GPIO_DIR_IN)
			reg = &obj->reg->dir[pos].rst;
		else
			reg = &obj->reg->dir[pos].set;
		break;
	case GPIO_PRO_DOUT:
		if (val == GPIO_OUT_ZERO)
			reg = &obj->reg->dout[pos].rst;
		else
			reg = &obj->reg->dout[pos].set;
		break;
	default:
		return -ERINVAL;
	}

	mmio_write_16((uintptr_t)reg, data);

	return RSUCCESS;
}

static int32_t mt_set_gpio_ext(uint32_t pin, uint32_t property, uint32_t val)
{
	uint32_t pos = 0;
	uint32_t bit = 0;
	struct mt_gpioext_obj *obj = mt_get_gpioext_obj();
	uint16_t *reg;
	uint32_t data = 0;
	int ret = 0;

	if (!obj)
		return -ERACCESS;

	if (pin >= MAX_GPIO_PIN)
		return -ERINVAL;

	if (property >= GPIO_PRO_MAX)
		return -ERINVAL;

	pin -= GPIO_EXTEND_START;
	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	switch (property) {
	case GPIO_PRO_DIR:
		if (val == GPIO_DIR_IN)
			reg = &obj->reg->dir[pos].rst;
		else
			reg = &obj->reg->dir[pos].set;
		break;
	case GPIO_PRO_DOUT:
		if (val == GPIO_OUT_ZERO)
			reg = &obj->reg->dout[pos].rst;
		else
			reg = &obj->reg->dout[pos].set;
		break;
	default:
		return -ERINVAL;
	}
	data = (1L << bit);
	ret = gpioext_set_bits(data, reg);

	return ret ? -ERWRAPPER : RSUCCESS;
}

static void mt_gpio_pin_decrypt(uint32_t *cipher)
{
	if ((*cipher & (0x80000000)) == 0)
		INFO("Pin %u decrypt warning!\n", *cipher);
	*cipher &= ~(0x80000000);
}

int32_t mt_set_gpio_out(uint32_t pin, uint32_t output)
{
	uint32_t gp = GPIO_PRO_DOUT;

	mt_gpio_pin_decrypt(&pin);

	return (pin >= GPIO_EXTEND_START) ?
		mt_set_gpio_ext(pin, gp, output) :
		mt_set_gpio_chip(pin, gp, output);
}

void gpio_set(uint32_t gpio, int32_t value)
{
	mt_set_gpio_out(gpio, value);
}
