/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (C) 2017-2023 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <npcm845x_clock.h>
#include <npcm845x_gcr.h>
#include <npcm845x_lpuart.h>
#include <plat_npcm845x.h>


uintptr_t npcm845x_get_base_uart(UART_DEV_T devNum)
{
	return 0xF0000000 + devNum * 0x1000;
}

uintptr_t npcm845x_get_base_clk(void)
{
	return 0xF0801000;
}

uintptr_t npcm845x_get_base_gcr(void)
{
	return 0xF0800000;
}

void npcm845x_wait_for_empty(int uart_n)
{
	volatile struct npcmX50_uart *uart = (struct npcmX50_uart *)npcm845x_get_base_uart(uart_n);

	while ((*(uint8_t *)(uintptr_t)(&uart->lsr) & 0x40) == 0x00) {
/*
 * wait for THRE (Transmitter Holding Register Empty)
 * and TSR (Transmitter Shift Register) to be empty.
 * Some delay. notice needed some delay so UartUpdateTool
 * will pass w/o error log
 */
	}

	volatile int delay;

	for (delay = 0; delay < 10000; delay++) {
		;
	}
}

int UART_Init(UART_DEV_T devNum,  UART_BAUDRATE_T baudRate)
{
	uint32_t val = 0;
	uintptr_t clk_base = npcm845x_get_base_clk();
	uintptr_t gcr_base =  npcm845x_get_base_gcr();
	uintptr_t uart_base = npcm845x_get_base_uart(devNum);
	volatile struct npcmX50_uart *uart = (struct npcmX50_uart *)uart_base;

/* Use  CLKREF to be independent of CPU frequency */
	volatile struct clk_ctl *clk_ctl_obj = (struct clk_ctl *)clk_base;
	volatile struct npcm845x_gcr *gcr_ctl_obj =
		(struct npcm845x_gcr *)gcr_base;

	clk_ctl_obj->clksel = clk_ctl_obj->clksel & ~(0x3 << 8);
	clk_ctl_obj->clksel = clk_ctl_obj->clksel | (0x2 << 8);

	/* Set devider according to baudrate */
	clk_ctl_obj->clkdiv1 =
		(unsigned int)(clk_ctl_obj->clkdiv1 & ~(0x1F << 16));

	/* clear bits 11-15 - set value 0 */
	if (devNum == UART3_DEV) {
		clk_ctl_obj->clkdiv2 =
			(unsigned int)(clk_ctl_obj->clkdiv2 & ~(0x1F << 11));
	}

	npcm845x_wait_for_empty(devNum);

	val = (uint32_t)LCR_WLS_8bit;
	mmio_write_8((uintptr_t)&uart->lcr, (uint8_t)val);

	/* disable all interrupts */
	mmio_write_8((uintptr_t)&uart->ier, 0);

	/*
	 * Set the RX FIFO trigger level, reset RX, TX FIFO
	 */
	val = (uint32_t)(FCR_FME | FCR_RFR | FCR_TFR | FCR_RFITL_4B);

	/* reset TX and RX FIFO */
	mmio_write_8((uintptr_t)(&uart->fcr), (uint8_t)val);

	/* Set port for 8 bit, 1 stop, no parity */
	val = (uint32_t)LCR_WLS_8bit;

	/* Set DLAB bit; Accesses the Divisor Latch Registers (DLL, DLM). */
	val |= 0x80;
	mmio_write_8((uintptr_t)(&uart->lcr), (uint8_t)val);

	/* Baud Rate = UART Clock 24MHz / (16 * (11+2)) = 115384 */
	mmio_write_8((uintptr_t)(&uart->dll), 11);
	mmio_write_8((uintptr_t)(&uart->dlm), 0x00);

	val = mmio_read_8((uintptr_t)&uart->lcr);

	/* Clear DLAB bit; Accesses RBR, THR or IER registers. */
	val &= 0x7F;
	mmio_write_8((uintptr_t)(&uart->lcr), (uint8_t)val);

	if (devNum == UART0_DEV) {
		gcr_ctl_obj->mfsel4 &= ~(1 << 1);
		gcr_ctl_obj->mfsel1 |= 1 << 9;
	} else if (devNum == UART3_DEV) {
		/* Pin Mux */
		gcr_ctl_obj->mfsel4 &= ~(1 << 1);
		gcr_ctl_obj->mfsel1 |= 1 << 11;
		gcr_ctl_obj->spswc &= (7 << 0);
		gcr_ctl_obj->spswc |= (2 << 0);
	} else {
		/* halt */
		while (1) {
			;
		}
	}

	return 0;
}
