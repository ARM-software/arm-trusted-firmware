/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file contains the CSS-firmware specific definitions for the third
 * generation of platforms.
 */

#ifndef NRD_CSS_FW_DEF3_H
#define NRD_CSS_FW_DEF3_H

#include <nrd_css_def3.h>

/*******************************************************************************
 * BL sizes
 ******************************************************************************/

#define NRD_CSS_BL1_RW_SIZE		UL(64 * 1024)	/* 64KB */

# define NRD_CSS_BL2_SIZE		UL(0x30000)

/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of BL2
 * and BL1-RW. NRD_BL31_SIZE - is tuned with respect to the actual BL31
 * PROGBITS size which is around 64-68KB at the time this change is being made.
 * A buffer of ~35KB is added to account for future expansion of the image,
 * making it a total of 100KB.
 */
#define NRD_CSS_BL31_SIZE		UL(116 * 1024)	/* 116 KB */

/*******************************************************************************
 * Console config
 ******************************************************************************/

#define NRD_CSS_UART_CLK_IN_HZ		UL(7372800)

/*******************************************************************************
 * Watchdog config
 ******************************************************************************/

#define NRD_CSS_AP_SECURE_WDOG_TIMEOUT	UL(100)

/*******************************************************************************
 * MMU mapping
 ******************************************************************************/

#define NRD_CSS_PERIPH_MMAP(n)						\
		MAP_REGION_FLAT(					\
			NRD_REMOTE_CHIP_MEM_OFFSET(n) +			\
			NRD_CSS_PERIPH_BASE,				\
			NRD_CSS_PERIPH_SIZE,				\
			MT_DEVICE | MT_RW | EL3_PAS)

#endif /* NRD_CSS_FW_DEF3_H */
