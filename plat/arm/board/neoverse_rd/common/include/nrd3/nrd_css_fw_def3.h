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

#define NRD_CSS_BL1_RO_BASE		NRD_CSS_SHARED_SRAM_BASE
#define NRD_CSS_BL1_RO_SIZE		UL(0x00019000)

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

#define NRD_CSS_DRAM1_CARVEOUT_SIZE	UL(0x0C000000)	/* 117MB */

/*******************************************************************************
 * Console config
 ******************************************************************************/

#define NRD_CSS_UART_CLK_IN_HZ		UL(7372800)

/*******************************************************************************
 * Watchdog config
 ******************************************************************************/

#define NRD_CSS_AP_SECURE_WDOG_TIMEOUT	UL(100)

/*******************************************************************************
 * RMM Console Config
 ******************************************************************************/

#define NRD_CSS_RMM_CONSOLE_BASE		NRD_CSS_REALM_UART_BASE
#define NRD_CSS_RMM_CONSOLE_BAUD		ARM_CONSOLE_BAUDRATE
#define NRD_CSS_RMM_CONSOLE_CLK_IN_HZ		UL(14745600)
#define NRD_CSS_RMM_CONSOLE_NAME		"pl011"
#define NRD_CSS_RMM_CONSOLE_COUNT		UL(1)

/*******************************************************************************
 * MMU mapping
 ******************************************************************************/

#define NRD_CSS_PERIPH_MMAP(n)						\
		MAP_REGION_FLAT(					\
			NRD_REMOTE_CHIP_MEM_OFFSET(n) +			\
			NRD_CSS_PERIPH_BASE,				\
			NRD_CSS_PERIPH_SIZE,				\
			MT_DEVICE | MT_RW | EL3_PAS)

#define NRD_CSS_SHARED_RAM_MMAP(n)					\
		MAP_REGION_FLAT(					\
			NRD_REMOTE_CHIP_MEM_OFFSET(n) +			\
			ARM_SHARED_RAM_BASE,				\
			ARM_SHARED_RAM_SIZE,				\
			MT_MEMORY | MT_RW | EL3_PAS)

#define NRD_CSS_GPC_SMMU_SMMUV3_MMAP					\
		MAP_REGION_FLAT(					\
			NRD_CSS_GPC_SMMUV3_BASE,			\
			NRD_CSS_GPC_SMMUV3_SIZE,			\
			MT_DEVICE | MT_RW | EL3_PAS)

#define NRD_CSS_BL1_RW_MMAP						\
		MAP_REGION_FLAT(					\
			BL1_RW_BASE,					\
			BL1_RW_LIMIT - BL1_RW_BASE,			\
			MT_MEMORY | MT_RW | EL3_PAS)

#define NRD_CSS_NS_DRAM1_MMAP						\
		MAP_REGION_FLAT(					\
			ARM_NS_DRAM1_BASE,				\
			ARM_NS_DRAM1_SIZE,				\
			MT_MEMORY | MT_RW | MT_NS)

#define NRD_CSS_GPT_L1_DRAM_MMAP					\
		MAP_REGION_FLAT(					\
			ARM_L1_GPT_BASE,				\
			ARM_L1_GPT_SIZE,				\
			MT_MEMORY | MT_RW | EL3_PAS)

#define NRD_CSS_EL3_RMM_SHARED_MEM_MMAP					\
		MAP_REGION_FLAT(					\
			ARM_EL3_RMM_SHARED_BASE,			\
			ARM_EL3_RMM_SHARED_SIZE,			\
			MT_MEMORY | MT_RW | MT_REALM)

#define NRD_CSS_RMM_REGION_MMAP						\
		MAP_REGION_FLAT(					\
			ARM_REALM_BASE,					\
			ARM_REALM_SIZE,					\
			MT_MEMORY | MT_RW | MT_REALM)

#endif /* NRD_CSS_FW_DEF3_H */
