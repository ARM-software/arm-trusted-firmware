/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2024, Mario Bălănică <mariobalanica02@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <platform_def.h>

#include <rpi_shared.h>

int rpi3_register_used_uart(console_t *console)
{
	return console_pl011_register(PLAT_RPI_PL011_UART_BASE,
				      PLAT_RPI_PL011_UART_CLOCK,
				      PLAT_RPI_UART_BAUDRATE,
				      console);
}
