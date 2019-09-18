/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <uart.h>

static struct mt_uart uart_save_addr[DRV_SUPPORT_UART_PORTS];

static const unsigned int uart_base_addr[DRV_SUPPORT_UART_PORTS] = {
	UART0_BASE,
	UART1_BASE
};

void mt_uart_restore(void)
{
	int uart_idx = UART_PORT0;
	struct mt_uart *uart;
	unsigned long base;

	/* Must NOT print any debug log before UART restore */
	for (uart_idx = UART_PORT0; uart_idx < HW_SUPPORT_UART_PORTS;
	     uart_idx++) {

		uart = &uart_save_addr[uart_idx];
		base = uart->base;

		mmio_write_32(UART_LCR(base), UART_LCR_MODE_B);
		mmio_write_32(UART_EFR(base), uart->registers.efr);
		mmio_write_32(UART_LCR(base), uart->registers.lcr);
		mmio_write_32(UART_FCR(base), uart->registers.fcr);

		/* baudrate */
		mmio_write_32(UART_HIGHSPEED(base), uart->registers.highspeed);
		mmio_write_32(UART_FRACDIV_L(base), uart->registers.fracdiv_l);
		mmio_write_32(UART_FRACDIV_M(base), uart->registers.fracdiv_m);
		mmio_write_32(UART_LCR(base),
			      uart->registers.lcr | UART_LCR_DLAB);
		mmio_write_32(UART_DLL(base), uart->registers.dll);
		mmio_write_32(UART_DLH(base), uart->registers.dlh);
		mmio_write_32(UART_LCR(base), uart->registers.lcr);
		mmio_write_32(UART_SAMPLE_COUNT(base),
			      uart->registers.sample_count);
		mmio_write_32(UART_SAMPLE_POINT(base),
			      uart->registers.sample_point);
		mmio_write_32(UART_GUARD(base), uart->registers.guard);

		/* flow control */
		mmio_write_32(UART_ESCAPE_EN(base), uart->registers.escape_en);
		mmio_write_32(UART_MCR(base), uart->registers.mcr);
		mmio_write_32(UART_IER(base), uart->registers.ier);
		mmio_write_32(UART_SCR(base), uart->registers.scr);
	}
}

void mt_uart_save(void)
{
	int uart_idx = UART_PORT0;
	struct mt_uart *uart;
	unsigned long base;

	for (uart_idx = UART_PORT0; uart_idx < HW_SUPPORT_UART_PORTS;
	     uart_idx++) {

		uart_save_addr[uart_idx].base = uart_base_addr[uart_idx];
		base = uart_base_addr[uart_idx];
		uart = &uart_save_addr[uart_idx];
		uart->registers.lcr = mmio_read_32(UART_LCR(base));

		mmio_write_32(UART_LCR(base), UART_LCR_MODE_B);
		uart->registers.efr = mmio_read_32(UART_EFR(base));
		mmio_write_32(UART_LCR(base), uart->registers.lcr);
		uart->registers.fcr = mmio_read_32(UART_FCR_RD(base));

		/* baudrate */
		uart->registers.highspeed = mmio_read_32(UART_HIGHSPEED(base));
		uart->registers.fracdiv_l = mmio_read_32(UART_FRACDIV_L(base));
		uart->registers.fracdiv_m = mmio_read_32(UART_FRACDIV_M(base));
		mmio_write_32(UART_LCR(base),
			      uart->registers.lcr | UART_LCR_DLAB);
		uart->registers.dll = mmio_read_32(UART_DLL(base));
		uart->registers.dlh = mmio_read_32(UART_DLH(base));
		mmio_write_32(UART_LCR(base), uart->registers.lcr);
		uart->registers.sample_count = mmio_read_32(
						UART_SAMPLE_COUNT(base));
		uart->registers.sample_point = mmio_read_32(
						UART_SAMPLE_POINT(base));
		uart->registers.guard = mmio_read_32(UART_GUARD(base));

		/* flow control */
		uart->registers.escape_en = mmio_read_32(UART_ESCAPE_EN(base));
		uart->registers.mcr = mmio_read_32(UART_MCR(base));
		uart->registers.ier = mmio_read_32(UART_IER(base));
		uart->registers.scr = mmio_read_32(UART_SCR(base));
	}
}

void mt_console_uart_cg(int on)
{
	if (on)
		mmio_write_32(UART_CLOCK_GATE_CLR, UART0_CLOCK_GATE_BIT);
	else
		mmio_write_32(UART_CLOCK_GATE_SET, UART0_CLOCK_GATE_BIT);
}

int mt_console_uart_cg_status(void)
{
	return mmio_read_32(UART_CLOCK_GATE_STA) & UART0_CLOCK_GATE_BIT;
}
