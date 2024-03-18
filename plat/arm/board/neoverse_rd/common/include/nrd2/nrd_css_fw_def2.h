/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file contains the CSS-firmware specific definitions for the second
 * generation platforms based on the N2/V2 CPU.
 */

#ifndef NRD_CSS_FW_DEF2_H
#define NRD_CSS_FW_DEF2_H

#include <nrd_css_def2.h>

/*******************************************************************************
 * BL sizes
 ******************************************************************************/

/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE is
 * calculated using the current BL31 PROGBITS debug size plus the sizes of BL2
 * and BL1-RW. NRD_BL31_SIZE - is tuned with respect to the actual BL31
 * PROGBITS size which is around 64-68KB at the time this change is being made.
 * A buffer of ~35KB is added to account for future expansion of the image,
 * making it a total of 100KB.
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
 * RAS config
 ******************************************************************************/

#define NS_RAM_ECC_CE_INT		U(87)
#define NS_RAM_ECC_UE_INT		U(88)

#if (SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP))			\
	&& ENABLE_FEAT_RAS && FFH_SUPPORT
/*
 * CPER buffer memory of 128KB is reserved and it is placed adjacent to the
 * memory shared between EL3 and S-EL0.
 */
#define NRD_SP_CPER_BUF_BASE		(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)
#define NRD_SP_CPER_BUF_SIZE		UL(0x10000)
#endif /* SPM_MM && ENABLE_FEAT_RAS && FFH_SUPPORT */

/*******************************************************************************
 * MMU mapping
 ******************************************************************************/

#define NRD_CSS_SHARED_RAM_REMOTE_CHIP_MMAP(n)				\
		MAP_REGION_FLAT(					\
			NRD_REMOTE_CHIP_MEM_OFFSET(n) +			\
			ARM_SHARED_RAM_BASE,				\
			ARM_SHARED_RAM_SIZE,				\
			MT_NON_CACHEABLE | MT_RW | MT_SECURE)

#define NRD_CSS_PERIPH_MMAP						\
		MAP_REGION_FLAT(					\
			NRD_DEVICE_BASE,				\
			NRD_DEVICE_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE)

#define NRD_CSS_PERIPH_REMOTE_CHIP_MMAP(n)				\
		MAP_REGION_FLAT(					\
			NRD_REMOTE_CHIP_MEM_OFFSET(n) +			\
			NRD_DEVICE_BASE,				\
			NRD_DEVICE_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE)

#if (SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP)) &&			\
ENABLE_FEAT_RAS && FFH_SUPPORT
/*
 * CPER buffer memory of 128KB is reserved and it is placed adjacent to the
 * memory shared between EL3 and S-EL0.
 */
#define NRD_CSS_SP_CPER_BUF_MMAP					\
		MAP_REGION2(						\
			NRD_SP_CPER_BUF_BASE,				\
			NRD_SP_CPER_BUF_BASE,				\
			NRD_SP_CPER_BUF_SIZE,				\
			MT_RW_DATA | MT_NS | MT_USER,			\
			PAGE_SIZE)
#endif

#if SPM_MM
#define NRD_CSS_SECURE_UART_USER_MMAP					\
		MAP_REGION_FLAT(					\
			SOC_CSS_SEC_UART_BASE,				\
			SOC_CSS_UART_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE | MT_USER)
#endif

#endif /* NRD_CSS_FW_DEF2_H */
