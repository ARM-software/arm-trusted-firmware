/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the CSS firmware specific definitions for
 * the first generation platforms based on the A75, N1 and V1 CPUs.
 */

#ifndef NRD1_CSS_FW_DEF1_H
#define NRD1_CSS_FW_DEF1_H

#include <nrd_css_def1.h>

/*******************************************************************************
 * BL sizes
 ******************************************************************************/

/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of BL2
 * and BL1-RW.
 */
#define NRD_BL31_SIZE			UL(116 * 1024)	/* 116 KB */

/*******************************************************************************
 * Console config
 ******************************************************************************/

#define SOC_CSS_UART_CLK_IN_HZ		UL(7372800)

/*******************************************************************************
 * Watchdog config
 ******************************************************************************/

#define SBSA_SECURE_WDOG_TIMEOUT	UL(100)

/*******************************************************************************
 * Platform ID
 ******************************************************************************/

/* Platform ID address */
#define SSC_VERSION		(SSC_REG_BASE + SSC_VERSION_OFFSET)
#ifndef __ASSEMBLER__
/* SSC_VERSION related accessors */
/* Returns the part number of the platform */
#define GET_NRD_PART_NUM						\
			GET_SSC_VERSION_PART_NUM(mmio_read_32(SSC_VERSION))
/* Returns the configuration number of the platform */
#define GET_NRD_CONFIG_NUM						\
			GET_SSC_VERSION_CONFIG(mmio_read_32(SSC_VERSION))
#endif /* __ASSEMBLER__ */

/*******************************************************************************
 * MMU mappings
 ******************************************************************************/

#define NRD_CSS_PERIPH_MMAP(n)						\
		MAP_REGION_FLAT(					\
			NRD_REMOTE_CHIP_MEM_OFFSET(n) +			\
			NRD_CSS_PERIPH_BASE,				\
			NRD_CSS_PERIPH_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_MAP_SHARED_RAM_REMOTE_CHIP(n)				\
		MAP_REGION_FLAT(					\
			NRD_REMOTE_CHIP_MEM_OFFSET(n) +			\
			ARM_SHARED_RAM_BASE,				\
			ARM_SHARED_RAM_SIZE,				\
			MT_NON_CACHEABLE | MT_RW | MT_SECURE)

#if SPM_MM
/*
 * Stand-alone MM logs would be routed via secure UART. Define page table
 * entry for secure UART which would be common to all platforms.
 */
#define SOC_PLATFORM_SECURE_UART					\
		MAP_REGION_FLAT(					\
			SOC_CSS_SEC_UART_BASE,				\
			SOC_CSS_UART_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE | MT_USER)
#endif

#endif /* NRD_CSS_FW_DEF1_H */
