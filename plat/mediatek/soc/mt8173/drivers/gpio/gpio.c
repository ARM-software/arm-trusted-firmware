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
#include <assert.h>
#include <debug.h>
#include <gpio.h>
#include <mmio.h>

enum {
	MAX_8173_GPIO = 134,
	MAX_GPIO_REG_BITS = 16,
	MAX_GPIO_MODE_PER_REG = 5,
	GPIO_MODE_BITS = 3,
};

enum {
	GPIO_DIRECTION_IN = 0,
	GPIO_DIRECTION_OUT = 1,
};

enum {
	GPIO_MODE = 0,
};

static void pos_bit_calc(uint32_t pin, uint32_t *pos, uint32_t *bit)
{
	*pos = pin / MAX_GPIO_REG_BITS;
	*bit = pin % MAX_GPIO_REG_BITS;
}

static void pos_bit_calc_for_mode(uint32_t pin, uint32_t *pos, uint32_t *bit)
{
	*pos = pin / MAX_GPIO_MODE_PER_REG;
	*bit = (pin % MAX_GPIO_MODE_PER_REG) * GPIO_MODE_BITS;
}

static int gpio_set_dir(uint32_t pin, uint32_t dir)
{
	uint32_t pos;
	uint32_t bit;
	uint16_t *reg;

	assert(pin <= MAX_8173_GPIO);

	pos_bit_calc(pin, &pos, &bit);

	if (dir == GPIO_DIRECTION_IN)
		reg = &mt8173_gpio->dir[pos].rst;
	else
		reg = &mt8173_gpio->dir[pos].set;

	mmio_write_16((uintptr_t)reg, 1L << bit);

	return 0;
}

void gpio_set_pull(uint32_t pin, enum pull_enable enable,
		   enum pull_select select)
{
	uint32_t pos;
	uint32_t bit;
	uint16_t *en_reg, *sel_reg;

	assert(pin <= MAX_8173_GPIO);

	pos_bit_calc(pin, &pos, &bit);

	if (enable == GPIO_PULL_DISABLE) {
		en_reg = &mt8173_gpio->pullen[pos].rst;
	} else {
	/* These pins' pulls can't be set through GPIO controller. */
		assert(pin < 22 || pin > 27);
		assert(pin < 47 || pin > 56);
		assert(pin < 57 || pin > 68);
		assert(pin < 73 || pin > 78);
		assert(pin < 100 || pin > 105);
		assert(pin < 119 || pin > 124);

		en_reg = &mt8173_gpio->pullen[pos].set;
		sel_reg = (select == GPIO_PULL_DOWN) ?
			  (&mt8173_gpio->pullsel[pos].rst) :
			  (&mt8173_gpio->pullsel[pos].set);
		mmio_write_16((uintptr_t)sel_reg, 1L << bit);
	}
	mmio_write_16((uintptr_t)en_reg, 1L << bit);
}

int gpio_get(uint32_t pin)
{
	uint32_t pos;
	uint32_t bit;
	uint16_t *reg;
	int data;

	assert(pin <= MAX_8173_GPIO);

	pos_bit_calc(pin, &pos, &bit);

	reg = &mt8173_gpio->din[pos].val;
	data = mmio_read_32((uintptr_t)reg);

	return (data & (1L << bit)) ? 1 : 0;
}

void gpio_set(uint32_t pin, int output)
{
	uint32_t pos;
	uint32_t bit;
	uint16_t *reg;

	assert(pin <= MAX_8173_GPIO);

	pos_bit_calc(pin, &pos, &bit);

	if (output == 0)
		reg = &mt8173_gpio->dout[pos].rst;
	else
		reg = &mt8173_gpio->dout[pos].set;
	mmio_write_16((uintptr_t)reg, 1L << bit);
}

void gpio_set_mode(uint32_t pin, int mode)
{
	uint32_t pos;
	uint32_t bit;
	uint32_t mask = (1L << GPIO_MODE_BITS) - 1;

	assert(pin <= MAX_8173_GPIO);

	pos_bit_calc_for_mode(pin, &pos, &bit);

	mmio_clrsetbits_32((uintptr_t)&mt8173_gpio->mode[pos].val,
			   mask << bit, mode << bit);
}

void gpio_input_pulldown(uint32_t gpio)
{
	gpio_set_pull(gpio, GPIO_PULL_ENABLE, GPIO_PULL_DOWN);
	gpio_set_dir(gpio, GPIO_DIRECTION_IN);
	gpio_set_mode(gpio, GPIO_MODE);
}

void gpio_input_pullup(uint32_t gpio)
{
	gpio_set_pull(gpio, GPIO_PULL_ENABLE, GPIO_PULL_UP);
	gpio_set_dir(gpio, GPIO_DIRECTION_IN);
	gpio_set_mode(gpio, GPIO_MODE);
}

void gpio_input(uint32_t gpio)
{
	gpio_set_pull(gpio, GPIO_PULL_DISABLE, GPIO_PULL_DOWN);
	gpio_set_dir(gpio, GPIO_DIRECTION_IN);
	gpio_set_mode(gpio, GPIO_MODE);
}

void gpio_output(uint32_t gpio, int value)
{
	gpio_set_pull(gpio, GPIO_PULL_DISABLE, GPIO_PULL_DOWN);
	gpio_set(gpio, value);
	gpio_set_dir(gpio, GPIO_DIRECTION_OUT);
	gpio_set_mode(gpio, GPIO_MODE);
}
