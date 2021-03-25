/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_CONSOLE_H
#define PLAT_CONSOLE_H

#include <stdint.h>
#include <drivers/console.h>

#if (NXP_CONSOLE == NS16550)
/*
 * NXP specific UART - 16550 configuration
 *
 * Initialize a NXP 16550 console instance and register it with the console
 * framework. The |console| pointer must point to storage that will be valid
 * for the lifetime of the console, such as a global or static local variable.
 * Its contents will be reinitialized from scratch.
 * When |clock| has a value of 0, the UART will *not* be initialised. This
 * means the UART should already be enabled and the baudrate and clock setup
 * should have been done already, either by platform specific code or by
 * previous firmware stages. The |baud| parameter will be ignored in this
 * case as well.
 */
int nxp_console_16550_register(uintptr_t baseaddr, uint32_t clock,
			       uint32_t baud, console_t *console);
#endif
/*
 * Function to initialize platform's console
 * and register with console framework
 */
void plat_console_init(uintptr_t nxp_console_addr, uint32_t uart_clk_div,
			uint32_t baud);

#endif
