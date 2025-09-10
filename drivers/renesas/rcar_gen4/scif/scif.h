/*
 * Copyright (c) 2021-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCIF_H
#define SCIF_H

int console_rcar_register(uintptr_t base, uint32_t clk, uint32_t baud,
			  console_t *pconsole);
int console_rcar_init(uintptr_t base_addr, uint32_t uart_clk,
		      uint32_t baud_rate);
int console_rcar_putc(int c, console_t *pconsole);
int console_rcar_flush(console_t *pconsole);

#endif /* SCIF_H */
