/*
 * Copyright (c) 2014-2016, Linaro Ltd and Contributors. All rights reserved.
 * Copyright (c) 2014-2016, Hisilicon Ltd and Contributors. All rights reserved.
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
#include <errno.h>
#include <gpio.h>
#include <mmio.h>
#include <pl061_gpio.h>

#define MAX_GPIO_DEVICES	32
#define PL061_GPIO_DIR		0x400

#define BIT(nr)			(1UL << (nr))

struct pl061_gpio_device_t {
	uintptr_t	reg_base[MAX_GPIO_DEVICES];
	uint32_t	gpio_index[MAX_GPIO_DEVICES];
	uint32_t	gpio_nums;
	uint32_t	gpio_devs;
};

static struct pl061_gpio_device_t pl061_dev;

static int pl061_get_reg_base(uint32_t gpio, uintptr_t *gpio_base)
{
	int i;

	if (pl061_dev.gpio_nums > MAX_GPIO_DEVICES * GPIOS_PER_PL061)
		return -EINVAL;
	if (gpio >= pl061_dev.gpio_nums)
		return -EINVAL;
	if (gpio_base == NULL)
		return -EINVAL;
	for (i = 0; i < pl061_dev.gpio_devs; i++) {
		if (gpio < pl061_dev.gpio_index[i])
			return -ENOENT;
		if (gpio >= pl061_dev.gpio_index[i] + GPIOS_PER_PL061)
			continue;
		*gpio_base = pl061_dev.reg_base[i];
		return 0;
	}
	return -ENOENT;
}

static int pl061_set_direction(uint32_t gpio, uint32_t direction)
{
	uintptr_t base_addr;
	uint32_t data, offset;

	if (pl061_get_reg_base(gpio, &base_addr))
		return -EINVAL;

	offset = gpio % GPIOS_PER_PL061;
	if (direction == GPIO_DIR_OUT) {
		data = mmio_read_8(base_addr + PL061_GPIO_DIR) | BIT(offset);
		mmio_write_8(base_addr + PL061_GPIO_DIR, data);
	} else if (direction == GPIO_DIR_IN) {
		data = mmio_read_8(base_addr + PL061_GPIO_DIR) & ~BIT(offset);
		mmio_write_8(base_addr + PL061_GPIO_DIR, data);
	} else
		return -EINVAL;
	return 0;
}

static int pl061_get_value(uint32_t gpio)
{
	uintptr_t base_addr;
	uint32_t offset;

	if (pl061_get_reg_base(gpio, &base_addr))
		return -EINVAL;

	offset = gpio % GPIOS_PER_PL061;
	if (mmio_read_8(base_addr + BIT(offset + 2)))
		return GPIO_LEVEL_HIGH;
	return GPIO_LEVEL_LOW;
}

static int pl061_set_value(uint32_t gpio, uint32_t value)
{
	uintptr_t base_addr;
	uint32_t offset;

	if ((value != GPIO_LEVEL_LOW) && (value != GPIO_LEVEL_HIGH))
		return -EINVAL;
	if (pl061_get_reg_base(gpio, &base_addr))
		return -EINVAL;

	offset = gpio % GPIOS_PER_PL061;
	if (value == GPIO_LEVEL_HIGH)
		mmio_write_8(base_addr + BIT(offset + 2), BIT(offset));
	else
		mmio_write_8(base_addr + BIT(offset + 2), 0);
	return 0;
}

static gpio_ops_t pl061_gpio_ops = {
	.set_direction	= pl061_set_direction,
	.get_value	= pl061_get_value,
	.set_value	= pl061_set_value,
};


/*
 * Register the PL061 GPIO controller with a base address and the offset
 * of start pin in this GPIO controller.
 * This function is called after pl061_gpio_ops_init().
 */
int pl061_gpio_register(uintptr_t base_addr, uint32_t gpio_offset)
{
	int i;

	if ((pl061_dev.gpio_nums == 0) || (pl061_dev.gpio_devs == 0))
		return -ENOENT;
	if (gpio_offset % GPIOS_PER_PL061) {
		WARN("Fail to register PL061 GPIO with offset %d\n",
		     gpio_offset);
		return -EINVAL;
	}

	for (i = 0; i < pl061_dev.gpio_devs; i++) {
		if (gpio_offset == pl061_dev.gpio_index[i]) {
			pl061_dev.reg_base[i] = base_addr;
			return 0;
		}
	}
	return -EINVAL;
}

/*
 * Initialize PL061 GPIO controller with the total GPIO numbers in SoC.
 */
int pl061_gpio_init(uint32_t gpio_nums)
{
	int gpio_devs, i;

	if (gpio_nums > (MAX_GPIO_DEVICES * GPIOS_PER_PL061))
		return -EINVAL;

	gpio_devs = (gpio_nums + (GPIOS_PER_PL061 - 1)) / GPIOS_PER_PL061;
	for (i = 0; i < gpio_devs; i++) {
		pl061_dev.reg_base[i] = 0;
		pl061_dev.gpio_index[i] = i * GPIOS_PER_PL061;
	}
	pl061_dev.gpio_nums = gpio_nums;
	pl061_dev.gpio_devs = gpio_devs;

	gpio_init(&pl061_gpio_ops);
	return 0;
}
