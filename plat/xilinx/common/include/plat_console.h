/*
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DT_UART_H
#define PLAT_DT_UART_H

#define DT_UART_DCC_COMPAT	"arm,dcc"

#if defined(PLAT_zynqmp)
#define DT_UART_COMPAT	"xlnx,zynqmp-uart"
#else
#define DT_UART_COMPAT	"arm,pl011"
#endif

/*Default console type is either CADENCE0 or CADENCE1 or PL011_0 or PL011_1
 * Debug console type is DCC
 **/
#define PLAT_XLNX_CONSOLE_TYPE_DEFAULT 0
#define PLAT_XLNX_CONSOLE_TYPE_DEBUG   1

typedef struct dt_uart_info_s {
	char compatible[30];
	uintptr_t base;
	uint32_t baud_rate;
	int32_t status;
} dt_uart_info_t;

void setup_console(void);

#if defined(CONSOLE_RUNTIME)
void console_runtime_init(void);
#endif

#endif /* PLAT_DT_UART_H */
