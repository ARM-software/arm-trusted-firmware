/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdint.h>

int console_init(uintptr_t base_addr,
		unsigned int uart_clk, unsigned int baud_rate);
void console_uninit(void);
int console_putc(int c);
int console_getc(void);
int console_flush(void);

#endif /* __CONSOLE_H__ */

