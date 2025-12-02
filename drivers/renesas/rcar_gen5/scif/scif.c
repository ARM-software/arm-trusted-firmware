/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/console.h>
#include "scif.h"

/* SCIF/HSCIF */
#define HSCIF0_BASE	0xC0710000UL

/* HSCIF */
#define HSCIF_BASE	HSCIF0_BASE
#define HSCIF_HSFTDR	(HSCIF_BASE + 0x000CU) /*  8 Transmit FIFO data register */
#define HSCIF_HSFSR	(HSCIF_BASE + 0x0010U) /* 16 Serial status register */

int console_rcar_init(uintptr_t base_addr, uint32_t uart_clk,
		      uint32_t baud_rate)
{
	scif_console_set_regs(HSCIF_HSFSR, HSCIF_HSFTDR);

	return 1;
}
