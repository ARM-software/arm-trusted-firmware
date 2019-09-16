/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
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
/* VideoCore -> ARM */
#define RPI3_MBOX0_READ_OFFSET		ULL(0x00000000)
#define RPI3_MBOX0_PEEK_OFFSET		ULL(0x00000010)
#define RPI3_MBOX0_SENDER_OFFSET	ULL(0x00000014)
#define RPI3_MBOX0_STATUS_OFFSET	ULL(0x00000018)
#define RPI3_MBOX0_CONFIG_OFFSET	ULL(0x0000001C)
/* ARM -> VideoCore */
#define RPI3_MBOX1_WRITE_OFFSET		ULL(0x00000020)
#define RPI3_MBOX1_PEEK_OFFSET		ULL(0x00000030)
#define RPI3_MBOX1_SENDER_OFFSET	ULL(0x00000034)
#define RPI3_MBOX1_STATUS_OFFSET	ULL(0x00000038)
#define RPI3_MBOX1_CONFIG_OFFSET	ULL(0x0000003C)
/* Mailbox status constants */
#define RPI3_MBOX_STATUS_FULL_MASK	U(0x80000000) /* Set if full */
#define RPI3_MBOX_STATUS_EMPTY_MASK	U(0x40000000) /* Set if empty */

/*
 * Power management, reset controller, watchdog.
 */
#define RPI3_IO_PM_OFFSET		ULL(0x00100000)
#define RPI3_PM_BASE			(RPI_IO_BASE + RPI3_IO_PM_OFFSET)
/* Registers on top of RPI3_PM_BASE. */
#define RPI3_PM_RSTC_OFFSET		ULL(0x0000001C)
#define RPI3_PM_RSTS_OFFSET		ULL(0x00000020)
#define RPI3_PM_WDOG_OFFSET		ULL(0x00000024)
/* Watchdog constants */
#define RPI3_PM_PASSWORD		U(0x5A000000)
#define RPI3_PM_RSTC_WRCFG_MASK		U(0x00000030)
#define RPI3_PM_RSTC_WRCFG_FULL_RESET	U(0x00000020)
/*
 * The RSTS register is used by the VideoCore firmware when booting the
 * Raspberry Pi to know which partition to boot from. The partition value is
 * formed by bits 0, 2, 4, 6, 8 and 10. Partition 63 is used by said firmware
 * to indicate halt.
 */
#define RPI3_PM_RSTS_WRCFG_HALT		U(0x00000555)

/*
 * Hardware random number generator.
 */
#define RPI3_IO_RNG_OFFSET		ULL(0x00104000)
#define RPI3_RNG_BASE			(RPI_IO_BASE + RPI3_IO_RNG_OFFSET)
#define RPI3_RNG_CTRL_OFFSET		ULL(0x00000000)
#define RPI3_RNG_STATUS_OFFSET		ULL(0x00000004)
#define RPI3_RNG_DATA_OFFSET		ULL(0x00000008)
#define RPI3_RNG_INT_MASK_OFFSET	ULL(0x00000010)
/* Enable/disable RNG */
#define RPI3_RNG_CTRL_ENABLE		U(0x1)
#define RPI3_RNG_CTRL_DISABLE		U(0x0)
/* Number of currently available words */
#define RPI3_RNG_STATUS_NUM_WORDS_SHIFT	U(24)
#define RPI3_RNG_STATUS_NUM_WORDS_MASK	U(0xFF)
/* Value to mask interrupts caused by the RNG */
#define RPI3_RNG_INT_MASK_DISABLE	U(0x1)

/*
 * Serial port (called 'Mini UART' in the BCM docucmentation).
 */
#define RPI3_IO_MINI_UART_OFFSET	ULL(0x00215040)
#define RPI3_MINI_UART_BASE		(RPI_IO_BASE + RPI3_IO_MINI_UART_OFFSET)
#define RPI3_MINI_UART_CLK_IN_HZ	ULL(500000000)

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
