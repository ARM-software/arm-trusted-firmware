/*
 * Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2024, Mario Bălănică <mariobalanica02@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPI_HW_H
#define RPI_HW_H

#include <lib/utils_def.h>

/*
 * Peripherals
 */

#define RPI_IO_BASE			ULL(0x1000000000)
#define RPI_IO_SIZE			ULL(0x1000000000)

/*
 * ARM <-> VideoCore mailboxes
 */
#define RPI3_MBOX_BASE			(RPI_IO_BASE + ULL(0x7c013880))

/*
 * Power management, reset controller, watchdog.
 */
#define RPI3_PM_BASE			(RPI_IO_BASE + ULL(0x7d200000))

/*
 * Hardware random number generator.
 */
#define RPI3_RNG_BASE			(RPI_IO_BASE + ULL(0x7d208000))

/*
 * PL011 system serial port
 */
#define RPI4_PL011_UART_BASE		(RPI_IO_BASE + ULL(0x7d001000))
#define RPI4_PL011_UART_CLOCK		ULL(44000000)

/*
 * GIC interrupt controller
 */
#define RPI_HAVE_GIC
#define RPI4_GIC_GICD_BASE		(RPI_IO_BASE + ULL(0x7fff9000))
#define RPI4_GIC_GICC_BASE		(RPI_IO_BASE + ULL(0x7fffa000))

#define	RPI4_LOCAL_CONTROL_BASE_ADDRESS		(RPI_IO_BASE + ULL(0x7c280000))
#define	RPI4_LOCAL_CONTROL_PRESCALER		(RPI_IO_BASE + ULL(0x7c280008))

/*
 * PCI Express
 */
#define RPI_PCIE_RC_BASES		RPI_IO_BASE + ULL(0x00100000), \
					RPI_IO_BASE + ULL(0x00110000), \
					RPI_IO_BASE + ULL(0x00120000)

#endif /* RPI_HW_H */
