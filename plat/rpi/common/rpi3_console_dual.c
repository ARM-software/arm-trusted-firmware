/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2024, Mario Bălănică <mariobalanica02@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <drivers/rpi3/gpio/rpi3_gpio.h>
#include <drivers/ti/uart/uart_16550.h>
#include <platform_def.h>

#include <rpi_shared.h>

static bool rpi3_use_mini_uart(void)
{
	return rpi3_gpio_get_select(14) == RPI3_GPIO_FUNC_ALT5;
}

int rpi3_register_used_uart(console_t *console)
{
	rpi3_gpio_init();

	if (rpi3_use_mini_uart())
		return console_16550_register(PLAT_RPI_MINI_UART_BASE,
					      0,
					      PLAT_RPI_UART_BAUDRATE,
					      console);
	else
		return console_pl011_register(PLAT_RPI_PL011_UART_BASE,
					      PLAT_RPI_PL011_UART_CLOCK,
					      PLAT_RPI_UART_BAUDRATE,
					      console);
}
