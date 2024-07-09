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

#define RPI_IO_BASE			ULL(0xFC000000)
#define RPI_IO_SIZE			ULL(0x04000000)

#define RPI_LEGACY_BASE			(ULL(0x02000000) + RPI_IO_BASE)

/*
 * ARM <-> VideoCore mailboxes
 */
#define RPI3_MBOX_OFFSET		ULL(0x0000B880)
#define RPI3_MBOX_BASE			(RPI_LEGACY_BASE + RPI3_MBOX_OFFSET)

/*
 * Power management, reset controller, watchdog.
 */
#define RPI3_IO_PM_OFFSET		ULL(0x00100000)
#define RPI3_PM_BASE			(RPI_LEGACY_BASE + RPI3_IO_PM_OFFSET)

/*
 * Hardware random number generator.
 */
#define RPI3_IO_RNG_OFFSET		ULL(0x00104000)
#define RPI3_RNG_BASE			(RPI_LEGACY_BASE + RPI3_IO_RNG_OFFSET)

/*
 * Serial ports:
 * 'Mini UART' in the BCM docucmentation is the 8250 compatible UART.
 * There is also a PL011 UART, multiplexed to the same pins.
 */
#define RPI4_IO_MINI_UART_OFFSET	ULL(0x00215040)
#define RPI4_MINI_UART_BASE		(RPI_LEGACY_BASE + RPI4_IO_MINI_UART_OFFSET)
#define RPI4_IO_PL011_UART_OFFSET	ULL(0x00201000)
#define RPI4_PL011_UART_BASE		(RPI_LEGACY_BASE + RPI4_IO_PL011_UART_OFFSET)
#define RPI4_PL011_UART_CLOCK		ULL(48000000)

/*
 * GPIO controller
 */
#define RPI3_IO_GPIO_OFFSET		ULL(0x00200000)
#define RPI3_GPIO_BASE			(RPI_LEGACY_BASE + RPI3_IO_GPIO_OFFSET)

/*
 * SDHost controller
 */
#define RPI3_IO_SDHOST_OFFSET           ULL(0x00202000)
#define RPI3_SDHOST_BASE                (RPI_LEGACY_BASE + RPI3_IO_SDHOST_OFFSET)

/*
 * GIC interrupt controller
 */
#define RPI_HAVE_GIC
#define RPI4_GIC_GICD_BASE		ULL(0xff841000)
#define RPI4_GIC_GICC_BASE		ULL(0xff842000)

#define	RPI4_LOCAL_CONTROL_BASE_ADDRESS		ULL(0xff800000)
#define	RPI4_LOCAL_CONTROL_PRESCALER		ULL(0xff800008)

/*
 * PCI Express
 */
#define RPI_PCIE_RC_BASES		(RPI_IO_BASE + ULL(0x01500000))

#define RPI_PCIE_ECAM_SERROR_QUIRK	1

#endif /* RPI_HW_H */
