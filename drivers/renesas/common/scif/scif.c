/*
 * Copyright (c) 2026, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include "scif.h"

#include "cpg_registers.h"
#include "rcar_def.h"
#include "rcar_private.h"

/* CPG */
#define CPG_MSTPSR2_SCIF0	BIT(7)
#define CPG_MSTPSR3_SCIF2	BIT(10)

/* SCIF */
#define SCIF0_BASE		0xE6E60000UL
#define SCIF2_BASE		0xE6E88000UL

/* SCIF */
#define SCIF_SCSMR		0x00
#define SCIF_SCBRR		0x04
#define SCIF_SCSCR		0x08
#define SCIF_SCFTDR		0x0C
#define SCIF_SCFSR		0x10
#define SCIF_SCFCR		0x18
#define SCIF_SCLSR		0x24
#define SCIF_DL			0x30
#define SCIF_CKS		0x34

/* MODE pin */
#define MODEMR_MD12		BIT(12)

#define SCBRR_115200BPS		17
#define SCBRR_115200BPS_D3_SSCG	16
#define SCBRR_115200BPS_E3_SSCG	15
#define SCBRR_230400BPS		8

#define SCSCR_TE_EN		BIT(5)
#define SCSCR_RE_EN		BIT(4)
#define SCSCR_CKE_MASK		3
#define SCFSR_TEND_MASK		BIT(6)
#define SCFSR_TEND_TRANS_END	BIT(6)
#define SCSCR_CKE_INT_CLK	0
#define SCFCR_TFRST_EN		BIT(2)
#define SCFCR_RFRS_EN		BIT(1)

int console_rcar_init(uintptr_t base_addr, uint32_t uart_clk,
		      uint32_t baud_rate)
{
	uint32_t prr = mmio_read_32(PRR);
	uint32_t base;
	int i;

	if ((prr & PRR_PRODUCT_MASK) == PRR_PRODUCT_V3M) { /* V3M */
		base = SCIF0_BASE;
		/* Enable SCIF clock */
		mstpcr_write(CPG_SMSTPCR2, CPG_MSTPSR2, CPG_MSTPSR2_SCIF0);
	} else {
		base = SCIF2_BASE;
		/* Enable SCIF clock */
		mstpcr_write(CPG_SMSTPCR3, CPG_MSTPSR3, CPG_MSTPSR3_SCIF2);
	}

	scif_console_set_regs(base + SCIF_SCFSR, base + SCIF_SCFTDR);

	/* Clear bits TE and RE in SCSCR to 0 */
	mmio_write_16(base + SCIF_SCSCR, 0);

	/* Set bits TFRST and RFRST in SCFCR to 1 */
	mmio_clrsetbits_16(base + SCIF_SCFCR,
			   SCFCR_TFRST_EN | SCFCR_RFRS_EN,
			   SCFCR_TFRST_EN | SCFCR_RFRS_EN);

	/*
	 * Read flags of ER, DR, BRK, and RDF in SCFSR and those
	 * of TO and ORER in SCLSR, then clear them to 0.
	 */
	mmio_write_16(base + SCIF_SCFSR, 0);
	mmio_write_16(base + SCIF_SCLSR, 0);

	/* Set bits CKE[1:0] in SCSCR */
	mmio_clrsetbits_16(base + SCIF_SCSCR, SCSCR_CKE_MASK,
			   SCSCR_CKE_INT_CLK);

	/* Set data transfer format in SCSMR */
	mmio_write_16(base + SCIF_SCSMR, 0);

	/* Set value in SCBRR */
	if ((prr & (PRR_PRODUCT_MASK | PRR_CUT_MASK)) == PRR_PRODUCT_H3_CUT10) {
		/* H3 ES 1.0 */
		mmio_write_8(base + SCIF_SCBRR, SCBRR_230400BPS);
	} else if (((prr & PRR_PRODUCT_MASK) == PRR_PRODUCT_D3) &&
		   (mmio_read_32(RST_MODEMR) & MODEMR_MD12)) {
		/* D3 with SSCG(MD12) ON */
		mmio_write_8(base + SCIF_SCBRR, SCBRR_115200BPS_D3_SSCG);
	} else if (((prr & PRR_PRODUCT_MASK) == PRR_PRODUCT_E3) &&
		   (mmio_read_32(RST_MODEMR) & MODEMR_MD12)) {
		/* E3 with SSCG(MD12) ON */
		mmio_write_8(base + SCIF_SCBRR, SCBRR_115200BPS_E3_SSCG);
	} else {
		/* H3/M3/M3N or when SSCG(MD12) is off in E3/D3 */
		mmio_write_8(base + SCIF_SCBRR, SCBRR_115200BPS);
	}

	/* 1-bit interval elapsed */
	for (i = 0; i < 100; i++)
		asm volatile("nop");

	/*
	 * Set bits RTRG[1:0], TTRG[1:0], and MCE in SCFCR
	 * Clear bits FRST and RFRST to 0
	 */
	mmio_write_16(base + SCIF_SCFCR, 0);

	/* Set bits TE and RE in SCSCR to 1 */
	mmio_clrsetbits_16(base + SCIF_SCSCR, SCSCR_TE_EN | SCSCR_RE_EN,
			   SCSCR_TE_EN | SCSCR_RE_EN);

	return 1;
}
