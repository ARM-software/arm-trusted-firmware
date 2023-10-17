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

typedef struct dt_uart_info_s {
	char compatible[30];
	uintptr_t base;
	uint32_t baud_rate;
	int32_t status;
} dt_uart_info_t;

void setup_console(void);

#endif /* PLAT_DT_UART_H */
