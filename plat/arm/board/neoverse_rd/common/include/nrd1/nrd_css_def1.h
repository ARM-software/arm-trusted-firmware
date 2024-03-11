/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the CSS specific memory and interrupt map
 * definitions for the first generation platforms based on the A75, N1 and V1
 * CPUs. There are minor differences in the memory map of these platforms and
 * those differences are not in the scope of this file.
 */

#ifndef NRD_CSS_DEF1_H
#define NRD_CSS_DEF1_H

/*******************************************************************************
 * CSS memory map related defines
 ******************************************************************************/

/* PL011 UART */
#define NRD_CSS_SEC_UART_BASE		UL(0x2A410000)
#define NRD_CSS_UART_SIZE		UL(0x10000)

/* CSS peripherals */
#define NRD_CSS_PERIPH_BASE		UL(0x20000000)
#define NRD_CSS_PERIPH_SIZE		UL(0x40000000)

/* Secure Watchdog */
#define NRD_CSS_WDOG_BASE		UL(0x2A480000)

#endif /* NRD_CSS_DEF1_H */
