/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <drivers/console.h>
#include <drivers/ti/uart/uart_16550.h>

#include <k3_console.h>

void bl31_console_setup(void)
{
	static console_16550_t console;

	/* Initialize the console to provide early debug support */
	console_16550_register(K3_USART_BASE, K3_USART_CLK_SPEED,
			       K3_USART_BAUD, &console);
}
