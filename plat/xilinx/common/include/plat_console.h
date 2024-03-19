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
#define CONSOLE_CDNS	0
#define CONSOLE_PL011   1
#define CONSOLE_DCC	2

typedef struct console_hd {
	uint8_t console_state;
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
	int32_t status;
	uint8_t console_type;
} dt_uart_info_t;

void setup_console(void);

#if defined(CONSOLE_RUNTIME)
void console_runtime_init(void);
#endif

#endif /* PLAT_DT_UART_H */
