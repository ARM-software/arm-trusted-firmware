/*
 * Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPI_HW_H
#define RPI_HW_H

#include <lib/utils_def.h>

/*
 * Peripherals
 */

#define RPI_IO_BASE			ULL(0x3F000000)
#define RPI_IO_SIZE			ULL(0x01000000)

/*
 * ARM <-> VideoCore mailboxes
 */
#define RPI3_MBOX_OFFSET		ULL(0x0000B880)
#define RPI3_MBOX_BASE			(RPI_IO_BASE + RPI3_MBOX_OFFSET)

/*
 * Power management, reset controller, watchdog.
 */
#define RPI3_IO_PM_OFFSET		ULL(0x00100000)
#define RPI3_PM_BASE			(RPI_IO_BASE + RPI3_IO_PM_OFFSET)

/*
 * Hardware random number generator.
 */
#define RPI3_IO_RNG_OFFSET		ULL(0x00104000)
#define RPI3_RNG_BASE			(RPI_IO_BASE + RPI3_IO_RNG_OFFSET)

/*
 * Serial ports:
 * 'Mini UART' in the BCM docucmentation is the 8250 compatible UART.
 * There is also a PL011 UART, multiplexed to the same pins.
 */
#define RPI3_IO_MINI_UART_OFFSET	ULL(0x00215040)
#define RPI3_MINI_UART_BASE		(RPI_IO_BASE + RPI3_IO_MINI_UART_OFFSET)
#define RPI3_IO_PL011_UART_OFFSET	ULL(0x00201000)
#define RPI3_PL011_UART_BASE		(RPI_IO_BASE + RPI3_IO_PL011_UART_OFFSET)
#define RPI3_PL011_UART_CLOCK		ULL(48000000)

/*
 * GPIO controller
 */
#define RPI3_IO_GPIO_OFFSET		ULL(0x00200000)
#define RPI3_GPIO_BASE			(RPI_IO_BASE + RPI3_IO_GPIO_OFFSET)

/*
 * SDHost controller
 */
#define RPI3_IO_SDHOST_OFFSET           ULL(0x00202000)
#define RPI3_SDHOST_BASE                (RPI_IO_BASE + RPI3_IO_SDHOST_OFFSET)

/*
 * Local interrupt controller
 */
#define RPI3_INTC_BASE_ADDRESS			ULL(0x40000000)
/* Registers on top of RPI3_INTC_BASE_ADDRESS */
#define RPI3_INTC_CONTROL_OFFSET		ULL(0x00000000)
#define RPI3_INTC_PRESCALER_OFFSET		ULL(0x00000008)
#define RPI3_INTC_MBOX_CONTROL_OFFSET		ULL(0x00000050)
#define RPI3_INTC_MBOX_CONTROL_SLOT3_FIQ	ULL(0x00000080)
#define RPI3_INTC_PENDING_FIQ_OFFSET		ULL(0x00000070)
#define RPI3_INTC_PENDING_FIQ_MBOX3		ULL(0x00000080)

#endif /* RPI_HW_H */
