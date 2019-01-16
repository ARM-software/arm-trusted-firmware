/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_UART_H
#define IMX_UART_H

#include <drivers/console.h>

#ifndef __ASSEMBLY__

typedef struct {
	console_t console;
	uintptr_t base;
} console_uart_t;

int console_imx_uart_register(uintptr_t baseaddr, uint32_t clock, uint32_t baud,
			   console_uart_t *console);
#endif /*__ASSEMBLY__*/

#endif  /* IMX_UART_H */
