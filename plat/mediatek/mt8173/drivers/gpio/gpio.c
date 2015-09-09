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
	MAX_GPIO_REG_BITS = 16
};

static void pos_bit_calc(uint32_t pin, uint32_t *pos, uint32_t *bit)
{
	*pos = pin / MAX_GPIO_REG_BITS;
	*bit = pin % MAX_GPIO_REG_BITS;
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
