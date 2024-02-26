/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file contains the CSS specific definitions for the second generation
 * platforms based on the N2/V2 CPU.
 */

#ifndef NRD_CSS_DEF2_H
#define NRD_CSS_DEF2_H

/*******************************************************************************
 * CSS memory map related defines
 ******************************************************************************/

/* PL011 UART */
#define SOC_CSS_SEC_UART_BASE			UL(0x2A410000)
#define SOC_CSS_NSEC_UART_BASE			UL(0x2A400000)
#define SOC_CSS_UART_SIZE			UL(0x10000)

/* General Peripherals */
#define NRD_DEVICE_BASE				UL(0x20000000)
#define NRD_DEVICE_SIZE				UL(0x20000000)

/*Secure Watchdog */
#define SBSA_SECURE_WDOG_BASE			UL(0x2A480000)

#endif /* NRD_CSS_DEF2_H */
