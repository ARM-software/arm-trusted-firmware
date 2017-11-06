/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RPI3_HW__
#define __RPI3_HW__

#include <utils_def.h>

/*
 * Peripherals
 */

#define RPI3_IO_BASE			ULL(0x3F000000)
#define RPI3_IO_SIZE			ULL(0x01000000)

/*
 * Serial port (called 'Mini UART' in the BCM docucmentation).
 */
#define RPI3_IO_MINI_UART_OFFSET	ULL(0x00215040)
#define RPI3_MINI_UART_BASE		(RPI3_IO_BASE + RPI3_IO_MINI_UART_OFFSET)
#define RPI3_MINI_UART_CLK_IN_HZ	ULL(500000000)

/*
 * Power management, reset controller, watchdog.
 */
#define RPI3_IO_PM_OFFSET		ULL(0x00100000)
#define RPI3_PM_BASE			(RPI3_IO_BASE + RPI3_IO_PM_OFFSET)
/* Registers on top of RPI3_PM_BASE. */
#define RPI3_PM_RSTC_OFFSET		ULL(0x0000001C)
#define RPI3_PM_WDOG_OFFSET		ULL(0x00000024)
/* Watchdog constants */
#define RPI3_PM_PASSWORD		ULL(0x5A000000)
#define RPI3_PM_RSTC_WRCFG_MASK		ULL(0x00000030)
#define RPI3_PM_RSTC_WRCFG_FULL_RESET	ULL(0x00000020)

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

#endif /* __RPI3_HW__ */
