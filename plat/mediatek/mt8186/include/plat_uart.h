/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_UART_H__
#define __PLAT_UART_H__

/* UART error code */
#define UART_DONE	U(0)
#define UART_PM_ERROR	U(1)

/* UART HW information */
#ifndef HW_SUPPORT_UART_PORTS
#define HW_SUPPORT_UART_PORTS	(2U)	/* the UART PORTs current HW have */
#endif
#define MTK_UART_SEND_SLEEP_REQ	(1U)	/* Request uart to sleep */
#define MTK_UART_SLEEP_ACK_IDLE	(1U)	/* uart in idle state */
#define MTK_UART_WAIT_ACK_TIMES	(50U)

#define UART_BASE0	(0x11002000)
#define UART_BASE1	(0x11003000)

#endif /* __PLAT_UART_H__ */
