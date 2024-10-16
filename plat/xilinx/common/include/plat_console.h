/*
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DT_UART_H
#define PLAT_DT_UART_H

#define DT_UART_DCC_COMPAT	"arm,dcc"
#define DT_UART_CAD_COMPAT	"xlnx,zynqmp-uart"
#define DT_UART_PL011_COMPAT	"arm,pl011"

/* Default console type is either CADENCE0 or CADENCE1 or PL011_0 or PL011_1
 * Debug console type is DCC
 */
#define CONSOLE_NONE	0
#define CONSOLE_CDNS	1
#define CONSOLE_PL011   2
#define CONSOLE_DCC	3

typedef struct console_hd {
	uint32_t clk;
	uint32_t baud_rate;
	uintptr_t base;
	uint32_t console_scope;
	uint8_t console_type;
} console_holder;

typedef struct dt_uart_info_s {
	char compatible[30];
	uintptr_t base;
	uint32_t baud_rate;
	uint8_t console_type;
} dt_uart_info_t;

void setup_console(void);

#endif /* PLAT_DT_UART_H */
