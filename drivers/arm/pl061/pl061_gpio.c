/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
 *
 * ARM PL061 GPIO Driver.
 * Reference to ARM DDI 0190B document.
 *
 */

#include <assert.h>
#include <cassert.h>
#include <debug.h>
#include <errno.h>
#include <gpio.h>
#include <mmio.h>
#include <pl061_gpio.h>

#if !PLAT_PL061_MAX_GPIOS
# define PLAT_PL061_MAX_GPIOS	32
#endif	/* PLAT_PL061_MAX_GPIOS */

CASSERT(PLAT_PL061_MAX_GPIOS > 0, assert_plat_pl061_max_gpios);

#define MAX_GPIO_DEVICES	((PLAT_PL061_MAX_GPIOS +		\
				 (GPIOS_PER_PL061 - 1)) / GPIOS_PER_PL061)

#define PL061_GPIO_DIR		0x400

#define GPIOS_PER_PL061		8
#define BIT(nr)			(1UL << (nr))

static int pl061_get_direction(int gpio);
static void pl061_set_direction(int gpio, int direction);
static int pl061_get_value(int gpio);
static void pl061_set_value(int gpio, int value);

static uintptr_t pl061_reg_base[MAX_GPIO_DEVICES];

static const gpio_ops_t pl061_gpio_ops = {
	.get_direction	= pl061_get_direction,
	.set_direction	= pl061_set_direction,
	.get_value	= pl061_get_value,
	.set_value	= pl061_set_value,
};

static int pl061_get_direction(int gpio)
{
	uintptr_t base_addr;
	unsigned int data, offset;

	assert((gpio >= 0) && (gpio < PLAT_PL061_MAX_GPIOS));

	base_addr = pl061_reg_base[gpio / GPIOS_PER_PL061];
	offset = gpio % GPIOS_PER_PL061;
	data = mmio_read_8(base_addr + PL061_GPIO_DIR);
	if (data & BIT(offset))
		return GPIO_DIR_OUT;
	return GPIO_DIR_IN;
}

static void pl061_set_direction(int gpio, int direction)
{
	uintptr_t base_addr;
	unsigned int data, offset;

	assert((gpio >= 0) && (gpio < PLAT_PL061_MAX_GPIOS));

	base_addr = pl061_reg_base[gpio / GPIOS_PER_PL061];
	offset = gpio % GPIOS_PER_PL061;
	if (direction == GPIO_DIR_OUT) {
		data = mmio_read_8(base_addr + PL061_GPIO_DIR) | BIT(offset);
		mmio_write_8(base_addr + PL061_GPIO_DIR, data);
	} else {
		data = mmio_read_8(base_addr + PL061_GPIO_DIR) & ~BIT(offset);
		mmio_write_8(base_addr + PL061_GPIO_DIR, data);
	}
}

/*
 * The offset of GPIODATA register is 0.
 * The values read from GPIODATA are determined for each bit, by the mask bit
 * derived from the address used to access the data register, PADDR[9:2].
 * Bits that are 1 in the address mask cause the corresponding bits in GPIODATA
 * to be read, and bits that are 0 in the address mask cause the corresponding
 * bits in GPIODATA to be read as 0, regardless of their value.
 */
static int pl061_get_value(int gpio)
{
	uintptr_t base_addr;
	unsigned int offset;

	assert((gpio >= 0) && (gpio < PLAT_PL061_MAX_GPIOS));

	base_addr = pl061_reg_base[gpio / GPIOS_PER_PL061];
	offset = gpio % GPIOS_PER_PL061;
	if (mmio_read_8(base_addr + BIT(offset + 2)))
		return GPIO_LEVEL_HIGH;
	return GPIO_LEVEL_LOW;
}

/*
 * In order to write GPIODATA, the corresponding bits in the mask, resulting
 * from the address bus, PADDR[9:2], must be HIGH. Otherwise the bit values
 * remain unchanged by the write.
 */
static void pl061_set_value(int gpio, int value)
{
	uintptr_t base_addr;
	int offset;

	assert((gpio >= 0) && (gpio < PLAT_PL061_MAX_GPIOS));

	base_addr = pl061_reg_base[gpio / GPIOS_PER_PL061];
	offset = gpio % GPIOS_PER_PL061;
	if (value == GPIO_LEVEL_HIGH)
		mmio_write_8(base_addr + BIT(offset + 2), BIT(offset));
	else
		mmio_write_8(base_addr + BIT(offset + 2), 0);
}


/*
 * Register the PL061 GPIO controller with a base address and the offset
 * of start pin in this GPIO controller.
 * This function is called after pl061_gpio_ops_init().
 */
void pl061_gpio_register(uintptr_t base_addr, int gpio_dev)
{
	assert((gpio_dev >= 0) && (gpio_dev < MAX_GPIO_DEVICES));

	pl061_reg_base[gpio_dev] = base_addr;
}

/*
 * Initialize PL061 GPIO controller with the total GPIO numbers in SoC.
 */
void pl061_gpio_init(void)
{
	gpio_init(&pl061_gpio_ops);
}
