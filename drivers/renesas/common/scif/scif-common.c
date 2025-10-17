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

/* SCIF/HSCIF */
#define SCIF_SCFSR_TEND		BIT(6)
#define SCIF_SCFSR_TDFE		BIT(5)
#define TRANS_END_CHECK		(SCIF_SCFSR_TEND | SCIF_SCFSR_TDFE)

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

void scif_console_set_regs(uint32_t fsr, uint32_t tdr)
{
	rcar_putc_fsr = fsr;
	rcar_putc_tdr = tdr;
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
