/*
 * Copyright (c) 2015-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>

#include "rcar_def.h"

extern void gicd_set_icenabler(uintptr_t base, unsigned int id);

#define RST_BASE			(0xE6160000U)
#define RST_WDTRSTCR			(RST_BASE + 0x0054U)
#define SWDT_BASE			(0xE6030000U)
#define SWDT_WTCNT			(SWDT_BASE + 0x0000U)
#define SWDT_WTCSRA			(SWDT_BASE + 0x0004U)
#define SWDT_WTCSRB			(SWDT_BASE + 0x0008U)
#define SWDT_GICD_BASE			(0xF1010000U)
#define SWDT_GICC_BASE			(0xF1020000U)
#define SWDT_GICD_CTLR			(SWDT_GICD_BASE + 0x0000U)
#define SWDT_GICD_IGROUPR		(SWDT_GICD_BASE + 0x0080U)
#define SWDT_GICD_ISPRIORITYR		(SWDT_GICD_BASE + 0x0400U)
#define SWDT_GICC_CTLR			(SWDT_GICC_BASE + 0x0000U)
#define SWDT_GICC_PMR			(SWDT_GICC_BASE + 0x0004U)
#define SWDT_GICD_ITARGETSR		(SWDT_GICD_BASE + 0x0800U)
#define IGROUPR_NUM			(16U)
#define ISPRIORITY_NUM			(128U)
#define ITARGET_MASK			(0x03U)

#define WDTRSTCR_UPPER_BYTE		(0xA55A0000U)
#define WTCSRA_UPPER_BYTE		(0xA5A5A500U)
#define WTCSRB_UPPER_BYTE		(0xA5A5A500U)
#define WTCNT_UPPER_BYTE		(0x5A5A0000U)
#define WTCNT_RESET_VALUE		(0xF488U)
#define WTCSRA_BIT_CKS			(0x0007U)
#define WTCSRB_BIT_CKS			(0x003FU)
#define SWDT_RSTMSK			(1U << 1U)
#define WTCSRA_WOVFE			(1U << 3U)
#define WTCSRA_WRFLG			(1U << 5U)
#define SWDT_ENABLE			(1U << 7U)

#define WDTRSTCR_MASK_ALL		(0x0000FFFFU)
#define WTCSRA_MASK_ALL			(0x000000FFU)
#define WTCNT_INIT_DATA			(WTCNT_UPPER_BYTE + WTCNT_RESET_VALUE)
#define WTCSRA_INIT_DATA		(WTCSRA_UPPER_BYTE + 0x0FU)
#define WTCSRB_INIT_DATA		(WTCSRB_UPPER_BYTE + 0x21U)

#if RCAR_LSI == RCAR_D3
#define WTCNT_COUNT_8p13k		(0x10000U - 40760U)
#else
#define WTCNT_COUNT_8p13k		(0x10000U - 40687U)
#endif
#define WTCNT_COUNT_8p13k_H3VER10	(0x10000U - 20343U)
#define WTCNT_COUNT_8p22k		(0x10000U - 41115U)
#define WTCNT_COUNT_7p81k		(0x10000U - 39062U)
#define WTCSRA_CKS_DIV16		(0x00000002U)

static void swdt_disable(void)
{
	uint32_t rmsk;

	rmsk = mmio_read_32(RST_WDTRSTCR) & WDTRSTCR_MASK_ALL;
	rmsk |= SWDT_RSTMSK;
	mmio_write_32(RST_WDTRSTCR, WDTRSTCR_UPPER_BYTE | rmsk);

	mmio_write_32(SWDT_WTCNT, WTCNT_INIT_DATA);
	mmio_write_32(SWDT_WTCSRA, WTCSRA_INIT_DATA);
	mmio_write_32(SWDT_WTCSRB, WTCSRB_INIT_DATA);

	/* Set the interrupt clear enable register */
	gicd_set_icenabler(RCAR_GICD_BASE, ARM_IRQ_SEC_WDT);
}

void rcar_swdt_init(void)
{
	uint32_t rmsk, sr;
#if (RCAR_LSI != RCAR_E3) && (RCAR_LSI != RCAR_D3) && (RCAR_LSI != RZ_G2E)
	uint32_t reg, val, product_cut, chk_data;

	reg = mmio_read_32(RCAR_PRR);
	product_cut = reg & (PRR_PRODUCT_MASK | PRR_CUT_MASK);

	reg = mmio_read_32(RCAR_MODEMR);
	chk_data = reg & CHECK_MD13_MD14;
#endif
	/* stop watchdog */
	if (mmio_read_32(SWDT_WTCSRA) & SWDT_ENABLE)
		mmio_write_32(SWDT_WTCSRA, WTCSRA_UPPER_BYTE);

	mmio_write_32(SWDT_WTCSRA, WTCSRA_UPPER_BYTE |
		      WTCSRA_WOVFE | WTCSRA_CKS_DIV16);

#if (RCAR_LSI == RCAR_E3) || (RCAR_LSI == RZ_G2E)
	mmio_write_32(SWDT_WTCNT, WTCNT_UPPER_BYTE | WTCNT_COUNT_7p81k);
#elif (RCAR_LSI == RCAR_D3)
	mmio_write_32(SWDT_WTCNT, WTCNT_UPPER_BYTE | WTCNT_COUNT_8p13k);
#else
	val = WTCNT_UPPER_BYTE;

	switch (chk_data) {
	case MD14_MD13_TYPE_0:
	case MD14_MD13_TYPE_2:
		val |= WTCNT_COUNT_8p13k;
		break;
	case MD14_MD13_TYPE_1:
		val |= WTCNT_COUNT_8p22k;
		break;
	case MD14_MD13_TYPE_3:
		val |= product_cut == (PRR_PRODUCT_H3 | PRR_PRODUCT_10) ?
		    WTCNT_COUNT_8p13k_H3VER10 : WTCNT_COUNT_8p13k;
		break;
	default:
		ERROR("MODEMR ERROR value = %x\n", chk_data);
		panic();
		break;
	}

	mmio_write_32(SWDT_WTCNT, val);
#endif
	rmsk = mmio_read_32(RST_WDTRSTCR) & WDTRSTCR_MASK_ALL;
	rmsk |= SWDT_RSTMSK | WDTRSTCR_UPPER_BYTE;
	mmio_write_32(RST_WDTRSTCR, rmsk);

	while ((mmio_read_8(SWDT_WTCSRA) & WTCSRA_WRFLG) != 0U)
		;

	/* Start the System WatchDog Timer */
	sr = mmio_read_32(SWDT_WTCSRA) & WTCSRA_MASK_ALL;
	mmio_write_32(SWDT_WTCSRA, (WTCSRA_UPPER_BYTE | sr | SWDT_ENABLE));
}

void rcar_swdt_release(void)
{
	uintptr_t itarget = SWDT_GICD_ITARGETSR +
	    (ARM_IRQ_SEC_WDT & ~ITARGET_MASK);
	uint32_t i;

	/* Disable FIQ interrupt */
	write_daifset(DAIF_FIQ_BIT);
	/* FIQ interrupts are not taken to EL3 */
	write_scr_el3(read_scr_el3() & ~SCR_FIQ_BIT);

	swdt_disable();
	gicv2_cpuif_disable();

	for (i = 0; i < IGROUPR_NUM; i++)
		mmio_write_32(SWDT_GICD_IGROUPR + i * 4, 0U);

	for (i = 0; i < ISPRIORITY_NUM; i++)
		mmio_write_32(SWDT_GICD_ISPRIORITYR + i * 4, 0U);

	mmio_write_32(itarget, 0U);
	mmio_write_32(SWDT_GICD_CTLR, 0U);
	mmio_write_32(SWDT_GICC_CTLR, 0U);
	mmio_write_32(SWDT_GICC_PMR, 0U);
}

void rcar_swdt_exec(uint64_t p)
{
	gicv2_end_of_interrupt(ARM_IRQ_SEC_WDT);
	rcar_swdt_release();
	ERROR("\n");
	ERROR("System WDT overflow, occurred address is %p\n", (void *)p);
	panic();
}
