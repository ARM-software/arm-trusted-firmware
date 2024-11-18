/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPIO_SPI_H
#define GPIO_SPI_H

#include <stdint.h>

struct gpio_spi_data {
	uint8_t cs_gpio, sclk_gpio, mosi_gpio, miso_gpio, reset_gpio;
	uint32_t spi_delay_us;
	unsigned int spi_mode;
};

struct spi_ops {
	void (*get_access)(void);
	void (*start)(void);
	void (*stop)(void);
	int (*xfer)(unsigned int bitlen, const void *dout, void *din);
};

struct spi_plat {
	struct gpio_spi_data gpio_data;
	const struct spi_ops *ops;
};

struct spi_plat *gpio_spi_init(struct gpio_spi_data *gpio_spi_data);

#endif /* GPIO_SPI_H */
