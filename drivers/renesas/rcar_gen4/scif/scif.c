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

/* CPG */
#define CPG_BASE		0xE6150000UL
#define CPG_CPGWPR		(CPG_BASE + 0x0000UL)
#define CPG_CPGWPCR		(CPG_BASE + 0x0004UL)
#define CPG_MSTPCR5		(CPG_BASE + 0x2D14UL)
#define CPG_MSTPSR5		(CPG_BASE + 0x2E14UL)
#define CPG_MSTPSR5_HSCIF0	BIT(14)
#define CPG_MSTPCR7		(CPG_BASE + 0x2D1CUL)
#define CPG_MSTPSR7		(CPG_BASE + 0x2E1CUL)
#define CPG_MSTPSR7_SCIF0	BIT(2)
#define CPG_MSTPSR7_SCIF3	BIT(4)

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
#define CPG_MSTPSR7_BIT	CPG_MSTPSR7_SCIF3
#else
#define SCIF_BASE	SCIF0_BASE
#define CPG_MSTPSR7_BIT	CPG_MSTPSR7_SCIF0
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
	uint32_t modemr, mstpcr, mstpsr, mstpbit;

	modemr = ((mmio_read_32(RST_MODEMR0) & RST_MODEMR0_MD31) >> 31U) |
		 ((mmio_read_32(RST_MODEMR1) & RST_MODEMR1_MD32) << 1U);

	if (modemr == MODEMR_HSCIF_DLMODE_3000000 ||
	    modemr == MODEMR_HSCIF_DLMODE_1843200 ||
	    modemr == MODEMR_HSCIF_DLMODE_921600) {
		mstpcr = CPG_MSTPCR5;
		mstpsr = CPG_MSTPSR5;
		mstpbit = CPG_MSTPSR5_HSCIF0;
		scif_console_set_regs(HSCIF_HSFSR, HSCIF_HSFTDR);
	} else {
		mstpcr = CPG_MSTPCR7;
		mstpsr = CPG_MSTPSR7;
		mstpbit = CPG_MSTPSR7_BIT;
		scif_console_set_regs(SCIF_SCFSR, SCIF_SCFTDR);
	}

	/* Turn SCIF/HSCIF clock ON. */
	mmio_clrbits_32(mstpcr, mstpbit);
	while (mmio_read_32(mstpsr) & mstpbit)
		;

	return 1;
}
