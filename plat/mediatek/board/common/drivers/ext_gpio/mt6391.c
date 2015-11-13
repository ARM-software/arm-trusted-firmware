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
#include <mt6391.h>
#include <pmic_wrap_init.h>

/* API of GPIO in PMIC MT6391 */
enum {
	MAX_GPIO_REG_BITS = 16,
	MAX_GPIO_MODE_PER_REG = 5,
	GPIO_MODE_BITS = 3,
	GPIO_PORT_OFFSET = 3,
	GPIO_SET_OFFSET = 2,
	GPIO_RST_OFFSET = 4,
	MAX_MT6391_GPIO = 40
};

enum {
	MT6391_GPIO_DIRECTION_IN = 0,
	MT6391_GPIO_DIRECTION_OUT = 1,
};

enum {
	MT6391_GPIO_MODE = 0,
};

static void pos_bit_calc(uint32_t pin, uint16_t *pos, uint16_t *bit)
{
	*pos = (pin / MAX_GPIO_REG_BITS) << GPIO_PORT_OFFSET;
	*bit = pin % MAX_GPIO_REG_BITS;
}

static void pos_bit_calc_mode(uint32_t pin, uint16_t *pos, uint16_t *bit)
{
	*pos = (pin / MAX_GPIO_MODE_PER_REG) << GPIO_PORT_OFFSET;
	*bit = pin % MAX_GPIO_MODE_PER_REG;
}

static int mt6391_gpio_set_dir(uint32_t pin, uint32_t dir)
{
	uint16_t pos;
	uint16_t bit;
	uint16_t reg;

	assert(pin <= MAX_MT6391_GPIO);

	pos_bit_calc(pin, &pos, &bit);

	if (dir == MT6391_GPIO_DIRECTION_IN)
		reg = MT6391_GPIO_DIR_BASE + pos + GPIO_RST_OFFSET;
	else
		reg = MT6391_GPIO_DIR_BASE + pos + GPIO_SET_OFFSET;

	if (pwrap_write(reg, 1L << bit) != 0)
		return -1;

	return 0;
}

void mt6391_gpio_set_pull(uint32_t pin, enum mt6391_pull_enable enable,
			  enum mt6391_pull_select select)
{
	uint16_t pos;
	uint16_t bit;
	uint16_t en_reg, sel_reg;

	assert(pin <= MAX_MT6391_GPIO);

	pos_bit_calc(pin, &pos, &bit);

	if (enable == MT6391_GPIO_PULL_DISABLE) {
		en_reg = MT6391_GPIO_PULLEN_BASE + pos + GPIO_RST_OFFSET;
	} else {
		en_reg = MT6391_GPIO_PULLEN_BASE + pos + GPIO_SET_OFFSET;
		sel_reg = (select == MT6391_GPIO_PULL_DOWN) ?
			  (MT6391_GPIO_PULLSEL_BASE + pos + GPIO_RST_OFFSET) :
			  (MT6391_GPIO_PULLSEL_BASE + pos + GPIO_SET_OFFSET);
		pwrap_write(sel_reg, 1L << bit);
	}
	pwrap_write(en_reg, 1L << bit);
}

int mt6391_gpio_get(uint32_t pin)
{
	uint16_t pos;
	uint16_t bit;
	uint16_t reg;
	uint16_t data;

	assert(pin <= MAX_MT6391_GPIO);

	pos_bit_calc(pin, &pos, &bit);

	reg = MT6391_GPIO_DIN_BASE + pos;
	pwrap_read(reg, &data);

	return (data & (1L << bit)) ? 1 : 0;
}

void mt6391_gpio_set(uint32_t pin, int output)
{
	uint16_t pos;
	uint16_t bit;
	uint16_t reg;

	assert(pin <= MAX_MT6391_GPIO);

	pos_bit_calc(pin, &pos, &bit);

	if (output == 0)
		reg = MT6391_GPIO_DOUT_BASE + pos + GPIO_RST_OFFSET;
	else
		reg = MT6391_GPIO_DOUT_BASE + pos + GPIO_SET_OFFSET;

	pwrap_write(reg, 1L << bit);
}

void mt6391_gpio_set_mode(uint32_t pin, int mode)
{
	uint16_t pos;
	uint16_t bit;
	uint16_t mask = (1L << GPIO_MODE_BITS) - 1;
	uint16_t offset;
	uint16_t data;

	assert(pin <= MAX_MT6391_GPIO);

	pos_bit_calc_mode(pin, &pos, &bit);

	offset = GPIO_MODE_BITS * bit;

	pwrap_read((uint16_t)MT6391_GPIO_MODE_BASE + pos, &data);
	data &= ~(mask << (offset));
	data |= (mode << (offset));

	pwrap_write((uint16_t)MT6391_GPIO_MODE_BASE + pos, data);
}

void mt6391_gpio_input_pulldown(uint32_t gpio)
{
	mt6391_gpio_set_pull(gpio, MT6391_GPIO_PULL_ENABLE,
			     MT6391_GPIO_PULL_DOWN);
	mt6391_gpio_set_dir(gpio, MT6391_GPIO_DIRECTION_IN);
	mt6391_gpio_set_mode(gpio, MT6391_GPIO_MODE);
}

void mt6391_gpio_input_pullup(uint32_t gpio)
{
	mt6391_gpio_set_pull(gpio, MT6391_GPIO_PULL_ENABLE,
			     MT6391_GPIO_PULL_UP);
	mt6391_gpio_set_dir(gpio, MT6391_GPIO_DIRECTION_IN);
	mt6391_gpio_set_mode(gpio, MT6391_GPIO_MODE);
}

void mt6391_gpio_input(uint32_t gpio)
{
	mt6391_gpio_set_pull(gpio, MT6391_GPIO_PULL_DISABLE,
			     MT6391_GPIO_PULL_DOWN);
	mt6391_gpio_set_dir(gpio, MT6391_GPIO_DIRECTION_IN);
	mt6391_gpio_set_mode(gpio, MT6391_GPIO_MODE);
}

void mt6391_gpio_output(uint32_t gpio, int value)
{
	mt6391_gpio_set_pull(gpio, MT6391_GPIO_PULL_DISABLE,
			     MT6391_GPIO_PULL_DOWN);
	mt6391_gpio_set(gpio, value);
	mt6391_gpio_set_dir(gpio, MT6391_GPIO_DIRECTION_OUT);
	mt6391_gpio_set_mode(gpio, MT6391_GPIO_MODE);
}
