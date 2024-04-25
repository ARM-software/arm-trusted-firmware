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

/* Boot ROM */
#define NRD_CSS_SECURE_ROM_BASE			UL(0x00000000)

/* DRAM2 */
#define NRD_CSS_DRAM2_BASE			ULL(0x8080000000)

/* NS SRAM */
#define NRD_CSS_NS_SRAM_BASE			UL(0x06000000)

/* PL011 UART */
#define NRD_CSS_SEC_UART_BASE			UL(0x2A410000)
#define NRD_CSS_NSEC_UART_BASE			UL(0x2A400000)
#define NRD_CSS_UART_SIZE			UL(0x10000)

/* General Peripherals */
#define NRD_CSS_PERIPH_BASE			UL(0x20000000)
#define NRD_CSS_PERIPH_SIZE			UL(0x20000000)

/* NS RAM Error record */
#define NRD_CSS_NS_RAM_ERR_REC_BASE		UL(0x2A4C0000)

/*Secure Watchdog */
#define NRD_CSS_SECURE_WDOG_BASE		UL(0x2A480000)

/* MHU */
#define NRD_CSS_AP_SCP_S_MHU_BASE		UL(0x2A920000)

/* GIC */
#define NRD_CSS_GIC_BASE			UL(0x30000000)

#endif /* NRD_CSS_DEF2_H */
