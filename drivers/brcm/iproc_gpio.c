/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/gpio.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <iproc_gpio.h>
#include <platform_def.h>

#define IPROC_GPIO_DATA_IN_OFFSET     0x00
#define IPROC_GPIO_DATA_OUT_OFFSET    0x04
#define IPROC_GPIO_OUT_EN_OFFSET      0x08
#define IPROC_GPIO_PAD_RES_OFFSET     0x34
#define IPROC_GPIO_RES_EN_OFFSET      0x38

#define PINMUX_OFFSET(gpio)           ((gpio) * 4)
#define PINCONF_OFFSET(gpio)          ((gpio) * 4)
#define PINCONF_PULL_UP               BIT(4)
#define PINCONF_PULL_DOWN             BIT(5)

/*
 * iProc GPIO bank is always 0x200 per bank,
 * with each bank supporting 32 GPIOs.
 */
#define GPIO_BANK_SIZE                0x200
#define NGPIOS_PER_BANK               32
#define GPIO_BANK(pin)                ((pin) / NGPIOS_PER_BANK)

#define IPROC_GPIO_REG(pin, reg)      (GPIO_BANK(pin) * GPIO_BANK_SIZE + (reg))
#define IPROC_GPIO_SHIFT(pin)         ((pin) % NGPIOS_PER_BANK)

#define MUX_GPIO_MODE                 0x3

/*
 * @base: base address of the gpio controller
 * @pinconf_base: base address of the pinconf
 * @pinmux_base: base address of the mux controller
 * @nr_gpios: maxinum number of GPIOs
 */
struct iproc_gpio {
	uintptr_t base;
	uintptr_t pinconf_base;
	uintptr_t pinmux_base;
	int nr_gpios;
};

static struct iproc_gpio iproc_gpio;

static void gpio_set_bit(uintptr_t base, unsigned int reg, int gpio, bool set)
{
	unsigned int offset = IPROC_GPIO_REG(gpio, reg);
	unsigned int shift = IPROC_GPIO_SHIFT(gpio);
	uint32_t val;

	val = mmio_read_32(base + offset);
	if (set)
		val |= BIT(shift);
	else
		val &= ~BIT(shift);

	mmio_write_32(base + offset, val);
}

static bool gpio_get_bit(uintptr_t base, unsigned int reg, int gpio)
{
	unsigned int offset = IPROC_GPIO_REG(gpio, reg);
	unsigned int shift = IPROC_GPIO_SHIFT(gpio);

	return !!(mmio_read_32(base + offset) & BIT(shift));
}

static void mux_to_gpio(struct iproc_gpio *g, int gpio)
{
	/* mux pad to GPIO if IOPAD configuration is mandatory */
	if (g->pinmux_base)
		mmio_write_32(g->pinmux_base + PINMUX_OFFSET(gpio),
			      MUX_GPIO_MODE);
}

static void set_direction(int gpio, int direction)
{
	struct iproc_gpio *g = &iproc_gpio;
	bool dir = (direction == GPIO_DIR_OUT) ? true : false;

	assert(gpio < g->nr_gpios);

	mux_to_gpio(g, gpio);
	gpio_set_bit(g->base, IPROC_GPIO_OUT_EN_OFFSET, gpio, dir);
}

static int get_direction(int gpio)
{
	struct iproc_gpio *g = &iproc_gpio;
	int dir;

	assert(gpio < g->nr_gpios);

	mux_to_gpio(g, gpio);
	dir = gpio_get_bit(g->base, IPROC_GPIO_OUT_EN_OFFSET, gpio) ?
		GPIO_DIR_OUT : GPIO_DIR_IN;

	return dir;
}

static int get_value(int gpio)
{
	struct iproc_gpio *g = &iproc_gpio;
	unsigned int offset;

	assert(gpio < g->nr_gpios);

	mux_to_gpio(g, gpio);

	/*
	 * If GPIO is configured as output, read from the GPIO_OUT register;
	 * otherwise, read from the GPIO_IN register
	 */
	offset = gpio_get_bit(g->base, IPROC_GPIO_OUT_EN_OFFSET, gpio) ?
		IPROC_GPIO_DATA_OUT_OFFSET : IPROC_GPIO_DATA_IN_OFFSET;

	return gpio_get_bit(g->base, offset, gpio);
}

static void set_value(int gpio, int val)
{
	struct iproc_gpio *g = &iproc_gpio;

	assert(gpio < g->nr_gpios);

	mux_to_gpio(g, gpio);

	/* make sure GPIO is configured to output, and then set the value */
	gpio_set_bit(g->base, IPROC_GPIO_OUT_EN_OFFSET, gpio, true);
	gpio_set_bit(g->base, IPROC_GPIO_DATA_OUT_OFFSET, gpio, !!(val));
}

static int get_pull(int gpio)
{
	struct iproc_gpio *g = &iproc_gpio;
	uint32_t val;

	assert(gpio < g->nr_gpios);
	mux_to_gpio(g, gpio);

	/* when there's a valid pinconf_base, use it */
	if (g->pinconf_base) {
		val = mmio_read_32(g->pinconf_base + PINCONF_OFFSET(gpio));

		if (val & PINCONF_PULL_UP)
			return GPIO_PULL_UP;
		else if (val & PINCONF_PULL_DOWN)
			return GPIO_PULL_DOWN;
		else
			return GPIO_PULL_NONE;
	}

	/* no pinconf_base. fall back to GPIO internal pull control */
	if (!gpio_get_bit(g->base, IPROC_GPIO_RES_EN_OFFSET, gpio))
		return GPIO_PULL_NONE;

	return gpio_get_bit(g->base, IPROC_GPIO_PAD_RES_OFFSET, gpio) ?
		GPIO_PULL_UP : GPIO_PULL_DOWN;
}

static void set_pull(int gpio, int pull)
{
	struct iproc_gpio *g = &iproc_gpio;
	uint32_t val;

	assert(gpio < g->nr_gpios);
	mux_to_gpio(g, gpio);

	/* when there's a valid pinconf_base, use it */
	if (g->pinconf_base) {
		val = mmio_read_32(g->pinconf_base + PINCONF_OFFSET(gpio));

		if (pull == GPIO_PULL_NONE) {
			val &= ~(PINCONF_PULL_UP | PINCONF_PULL_DOWN);
		} else if (pull == GPIO_PULL_UP) {
			val |= PINCONF_PULL_UP;
			val &= ~PINCONF_PULL_DOWN;
		} else if (pull == GPIO_PULL_DOWN) {
			val |= PINCONF_PULL_DOWN;
			val &= ~PINCONF_PULL_UP;
		} else {
			return;
		}
		mmio_write_32(g->pinconf_base + PINCONF_OFFSET(gpio), val);
	}

	/* no pinconf_base. fall back to GPIO internal pull control */
	if (pull == GPIO_PULL_NONE) {
		gpio_set_bit(g->base, IPROC_GPIO_RES_EN_OFFSET, gpio, false);
		return;
	}

	/* enable pad register and pull up or down */
	gpio_set_bit(g->base, IPROC_GPIO_RES_EN_OFFSET, gpio, true);
	gpio_set_bit(g->base, IPROC_GPIO_PAD_RES_OFFSET, gpio,
		     !!(pull == GPIO_PULL_UP));
}

const gpio_ops_t iproc_gpio_ops = {
	.get_direction = get_direction,
	.set_direction = set_direction,
	.get_value = get_value,
	.set_value = set_value,
	.get_pull = get_pull,
	.set_pull = set_pull,
};

void iproc_gpio_init(uintptr_t base, int nr_gpios, uintptr_t pinmux_base,
		     uintptr_t pinconf_base)
{
	iproc_gpio.base = base;
	iproc_gpio.nr_gpios = nr_gpios;

	/* pinmux/pinconf base is optional for some SoCs */
	if (pinmux_base)
		iproc_gpio.pinmux_base = pinmux_base;

	if (pinconf_base)
		iproc_gpio.pinconf_base = pinconf_base;

	gpio_init(&iproc_gpio_ops);
}
