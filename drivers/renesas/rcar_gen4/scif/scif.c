/*
 * Copyright (c) 2021-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include "scif.h"

#include "rcar_def.h"

/* RST */
#define RST_BASE		(0xE6160000UL + (RCAR_DOMAIN * 0x4000UL))
#define RST_MODEMR0		RST_BASE
#define RST_MODEMR1		(RST_BASE + 4UL)
#define RST_MODEMR0_MD31	BIT(31)
#define RST_MODEMR1_MD32	BIT(0)

/* SCIF/HSCIF */
#define SCIF0_BASE		0xE6E60000UL
#define SCIF3_BASE		0xE6C50000UL
#define HSCIF0_BASE		0xE6540000UL

/* SCIF */
#if (RCAR_LSI == RCAR_S4) /* S4 */
#define SCIF_BASE	SCIF3_BASE
#else
#define SCIF_BASE	SCIF0_BASE
#endif
#define SCIF_SCFTDR	(SCIF_BASE + 0x000CU)	/*  8 Transmit FIFO data register */
#define SCIF_SCFSR	(SCIF_BASE + 0x0010U)	/* 16 Serial status register */

/* HSCIF */
#define HSCIF_BASE	HSCIF0_BASE
#define HSCIF_HSFTDR	(HSCIF_BASE + 0x000CU) /*  8 Transmit FIFO data register */
#define HSCIF_HSFSR	(HSCIF_BASE + 0x0010U) /* 16 Serial status register */

/* Mode */
#define MODEMR_SCIF_DLMODE		0U
#define MODEMR_HSCIF_DLMODE_921600	1U
#define MODEMR_HSCIF_DLMODE_1843200	2U
#define MODEMR_HSCIF_DLMODE_3000000	3U

int console_rcar_init(uintptr_t base_addr, uint32_t uart_clk,
		      uint32_t baud_rate)
{
	uint32_t modemr;

	modemr = ((mmio_read_32(RST_MODEMR0) & RST_MODEMR0_MD31) >> 31U) |
		 ((mmio_read_32(RST_MODEMR1) & RST_MODEMR1_MD32) << 1U);

	if (modemr == MODEMR_HSCIF_DLMODE_3000000 ||
	    modemr == MODEMR_HSCIF_DLMODE_1843200 ||
	    modemr == MODEMR_HSCIF_DLMODE_921600) {
		scif_console_set_regs(HSCIF_HSFSR, HSCIF_HSFTDR);
	} else {
		scif_console_set_regs(SCIF_SCFSR, SCIF_SCFTDR);
	}

	return 1;
}
