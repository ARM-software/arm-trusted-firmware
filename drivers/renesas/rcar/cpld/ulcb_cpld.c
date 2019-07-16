/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include "ulcb_cpld.h"

#define SCLK			8	/* GP_6_8 */
#define SSTBZ			3	/* GP_2_3 */
#define MOSI			7	/* GP_6_7 */

#define CPLD_ADDR_RESET		0x80	/* RW */

/* LSI Multiplexed Pin Setting Mask Register */
#define PFC_PMMR		0xE6060000

/* General output registers */
#define GPIO_OUTDT2		0xE6052008
#define GPIO_OUTDT6		0xE6055408

/* General input/output switching registers */
#define GPIO_INOUTSEL2		0xE6052004
#define GPIO_INOUTSEL6		0xE6055404

/* General IO/Interrupt Switching Register */
#define GPIO_IOINTSEL6		0xE6055400

/* GPIO/perihperal function select */
#define PFC_GPSR2		0xE6060108
#define PFC_GPSR6		0xE6060118

static void gpio_set_value(uint32_t addr, uint8_t gpio, uint32_t val)
{
	uint32_t reg;

	reg = mmio_read_32(addr);
	if (val)
		reg |= (1 << gpio);
	else
		reg &= ~(1 << gpio);
	mmio_write_32(addr, reg);
}

static void gpio_direction_output(uint32_t addr, uint8_t gpio)
{
	uint32_t reg;

	reg = mmio_read_32(addr);
	reg |= (1 << gpio);
	mmio_write_32(addr, reg);
}

static void gpio_pfc(uint32_t addr, uint8_t gpio)
{
	uint32_t reg;

	reg = mmio_read_32(addr);
	reg &= ~(1 << gpio);
	mmio_write_32(PFC_PMMR, ~reg);
	mmio_write_32(addr, reg);
}

static void cpld_write(uint8_t addr, uint32_t data)
{
	int i;

	for (i = 0; i < 32; i++) {
		/* MSB first */
		gpio_set_value(GPIO_OUTDT6, MOSI, data & (1U << 31));
		gpio_set_value(GPIO_OUTDT6, SCLK, 1);
		data <<= 1;
		gpio_set_value(GPIO_OUTDT6, SCLK, 0);
	}

	for (i = 0; i < 8; i++) {
		/* MSB first */
		gpio_set_value(GPIO_OUTDT6, MOSI, addr & 0x80);
		gpio_set_value(GPIO_OUTDT6, SCLK, 1);
		addr <<= 1;
		gpio_set_value(GPIO_OUTDT6, SCLK, 0);
	}

	/* WRITE */
	gpio_set_value(GPIO_OUTDT6, MOSI, 1);
	gpio_set_value(GPIO_OUTDT2, SSTBZ, 0);
	gpio_set_value(GPIO_OUTDT6, SCLK, 1);
	gpio_set_value(GPIO_OUTDT6, SCLK, 0);
	gpio_set_value(GPIO_OUTDT2, SSTBZ, 1);
}

static void cpld_init(void)
{
	gpio_pfc(PFC_GPSR6, SCLK);
	gpio_pfc(PFC_GPSR2, SSTBZ);
	gpio_pfc(PFC_GPSR6, MOSI);

	gpio_set_value(GPIO_IOINTSEL6, SCLK, 0);
	gpio_set_value(GPIO_OUTDT6, SCLK, 0);
	gpio_set_value(GPIO_OUTDT2, SSTBZ, 1);
	gpio_set_value(GPIO_OUTDT6, MOSI, 0);

	gpio_direction_output(GPIO_INOUTSEL6, SCLK);
	gpio_direction_output(GPIO_INOUTSEL2, SSTBZ);
	gpio_direction_output(GPIO_INOUTSEL6, MOSI);
}

void rcar_cpld_reset_cpu(void)
{
	cpld_init();

	cpld_write(CPLD_ADDR_RESET, 1);
}
