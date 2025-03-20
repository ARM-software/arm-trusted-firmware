/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdlib.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio.h>
#include <drivers/gpio_spi.h>
#include <platform_def.h>

static struct spi_plat gpio_spidev;

static void gpio_spi_delay_us(void)
{
	udelay(gpio_spidev.gpio_data.spi_delay_us);
}

static int gpio_spi_miso(void)
{
	return gpio_get_value(gpio_spidev.gpio_data.miso_gpio);
}

static void gpio_spi_sclk(int bit)
{
	gpio_set_value(gpio_spidev.gpio_data.sclk_gpio, bit);
}

static void gpio_spi_mosi(int bit)
{
	gpio_set_value(gpio_spidev.gpio_data.mosi_gpio, bit);
}

static void gpio_spi_cs(int bit)
{
	gpio_set_value(gpio_spidev.gpio_data.cs_gpio, bit);
}

static void gpio_spi_start(void)
{
	gpio_spi_cs(1);
	gpio_spi_sclk(0);
	gpio_spi_cs(0);
}

static void gpio_spi_stop(void)
{
	gpio_spi_cs(1);
}

/* set sclk to a known state (0) before performing any further action */
static void gpio_spi_get_access(void)
{
	gpio_spi_sclk(0);
}

static void xfer(unsigned int bytes, const void *out, void *in, int cpol, int cpha)
{
	for (unsigned int j = 0U; j < bytes; j++) {
		unsigned char in_byte  = 0U;
		unsigned char out_byte = (out != NULL) ? *(const uint8_t *)out++ : 0xFF;

		for (int i = 7; i >= 0; i--) {
			if (cpha) {
				gpio_spi_sclk(!cpol);
			}

			gpio_spi_mosi(!!(out_byte & (1 << i)));

			gpio_spi_delay_us();
			gpio_spi_sclk(cpha ? cpol : !cpol);
			gpio_spi_delay_us();

			in_byte |= gpio_spi_miso() << i;

			if (!cpha) {
				gpio_spi_sclk(cpol);
			}
		}

		if (in != NULL) {
			*(uint8_t *)in++ = in_byte;
		}
	}
}

static int gpio_spi_xfer(unsigned int bytes, const void *out, void *in)
{
	if ((out == NULL) && (in == NULL)) {
		return -1;
	}

	switch (gpio_spidev.gpio_data.spi_mode) {
	case 0:
		xfer(bytes, out, in, 0, 0);
		break;
	case 1:
		xfer(bytes, out, in, 0, 1);
		break;
	case 2:
		xfer(bytes, out, in, 1, 0);
		break;
	case 3:
		xfer(bytes, out, in, 1, 1);
		break;
	default:
		return -1;
	}

	return 0;
}

struct spi_ops gpio_spidev_ops = {
	.get_access	= gpio_spi_get_access,
	.start		= gpio_spi_start,
	.stop		= gpio_spi_stop,
	.xfer		= gpio_spi_xfer,
};

struct spi_plat *gpio_spi_init(struct gpio_spi_data *gpio_spi_data)
{
	gpio_spidev.gpio_data = *gpio_spi_data;
	gpio_spidev.ops = &gpio_spidev_ops;

	return &gpio_spidev;
}
