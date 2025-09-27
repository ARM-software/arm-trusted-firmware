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
#define SCIF_SCFSR_TEND		BIT(6)
#define SCIF_SCFSR_TDFE		BIT(5)
#define TRANS_END_CHECK		(SCIF_SCFSR_TEND | SCIF_SCFSR_TDFE)

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

static uint32_t rcar_putc_fsr;
static uint32_t rcar_putc_tdr;

static inline void scif_clrbits_16(uintptr_t addr, uint32_t clear)
{
	mmio_write_16(addr, mmio_read_16(addr) & ~clear);
}

static void scif_console_trans_end_poll(uint32_t reg)
{
	/* Check that transfer of SCIF is completed */
	while ((mmio_read_16(reg) & TRANS_END_CHECK) != TRANS_END_CHECK)
		;
}

static void scif_console_putc_common(uint8_t chr)
{
	scif_console_trans_end_poll(rcar_putc_fsr);
	mmio_write_8(rcar_putc_tdr, chr);	/* Transfer one character */
	scif_clrbits_16(rcar_putc_fsr, TRANS_END_CHECK); /* TEND,TDFE clear */
	scif_console_trans_end_poll(rcar_putc_fsr);
}

static void scif_console_set_regs(uint32_t fsr, uint32_t tdr)
{
	rcar_putc_fsr = fsr;
	rcar_putc_tdr = tdr;
}

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

int console_rcar_putc(int c, console_t *pconsole)
{
	if (rcar_putc_fsr == 0 || rcar_putc_tdr == 0)
		return -1;

	if (c == '\n')	/* add 'CR' before 'LF' */
		scif_console_putc_common('\r');

	scif_console_putc_common(c);

	return c;
}

int console_rcar_flush(console_t *pconsole)
{
	/* Nothing to do */
	return 0;
}
