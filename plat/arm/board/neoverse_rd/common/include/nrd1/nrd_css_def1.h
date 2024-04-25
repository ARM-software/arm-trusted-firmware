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

/* On-Chip ROM */
#define NRD_CSS_TRUSTED_ROM_BASE	UL(0x00000000)
#define NRD_CSS_TRUSTED_ROM_SIZE	UL(0x00080000)	/* 512KB */

/* On-Chip RAM */
#define	NRD_CSS_TRUSTED_SRAM_SIZE	UL(0x00080000)	/* 512KB */
#define NRD_CSS_NONTRUSTED_SRAM_BASE	UL(0x06000000)
#define NRD_CSS_NONTRUSTED_SRAM_SIZE	UL(0x00080000)	/* 512KB */

/* PL011 UART */
#define NRD_CSS_SEC_UART_BASE		UL(0x2A410000)
#define NRD_CSS_UART_SIZE		UL(0x10000)

/* CSS peripherals */
#define NRD_CSS_PERIPH_BASE		UL(0x20000000)
#define NRD_CSS_PERIPH_SIZE		UL(0x40000000)

/* Secure Watchdog */
#define NRD_CSS_WDOG_BASE		UL(0x2A480000)

/* DRAM2 */
#define NRD_CSS_DRAM2_BASE		ULL(0x8080000000)
#define NRD_CSS_DRAM2_SIZE		ULL(0x180000000)

#endif /* NRD_CSS_DEF1_H */
