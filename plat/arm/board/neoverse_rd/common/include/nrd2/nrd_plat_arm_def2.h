/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the trusted firmware required platform port
 * definitions for the second generation platforms based on the N2/V2 CPUs. The
 * common platform support for Arm platforms expect platforms to define certain
 * definitions and those definitions are referred to as the platform port
 * definitions.
 */

#ifndef NRD_PLAT_ARM_DEF2_H
#define NRD_PLAT_ARM_DEF2_H

#include <plat/arm/common/arm_def.h>
#include <plat/arm/common/arm_spm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <nrd_css_fw_def2.h>
#include <nrd_ros_fw_def2.h>

/*******************************************************************************
 * Core count
 ******************************************************************************/

#define PLATFORM_CORE_COUNT		(NRD_CHIP_COUNT *		\
					PLAT_ARM_CLUSTER_COUNT *	\
					NRD_MAX_CPUS_PER_CLUSTER *	\
					NRD_MAX_PE_PER_CPU)

/*******************************************************************************
 * XLAT definitions
 ******************************************************************************/

#if defined(IMAGE_BL31)
# if SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP)
#  define PLAT_ARM_MMAP_ENTRIES		(10  + ((NRD_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(8  + ((NRD_CHIP_COUNT - 1) * 3))
#  define PLAT_SP_IMAGE_MMAP_REGIONS	U(12)
#  define PLAT_SP_IMAGE_MAX_XLAT_TABLES	U(14)
# else
#  define PLAT_ARM_MMAP_ENTRIES		(5 + ((NRD_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(6 + ((NRD_CHIP_COUNT - 1) * 3))
# endif
#elif defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		U(8)
# define MAX_XLAT_TABLES		U(5)
#elif defined(IMAGE_BL2)
# define PLAT_ARM_MMAP_ENTRIES		(11 + (NRD_CHIP_COUNT - 1))

/*
 * MAX_XLAT_TABLES entries need to be doubled because when the address width
 * exceeds 40 bits an additional level of translation is required. In case of
 * multichip platforms peripherals also fall into address space with width
 * > 40 bits
 *
 */
# define MAX_XLAT_TABLES		(7  + ((NRD_CHIP_COUNT - 1) * 2))
#elif !USE_ROMLIB
# define PLAT_ARM_MMAP_ENTRIES		U(11)
# define MAX_XLAT_TABLES		U(7)
#else
# define PLAT_ARM_MMAP_ENTRIES		U(12)
# define MAX_XLAT_TABLES		U(6)
#endif

/*******************************************************************************
 * BL sizes
 ******************************************************************************/

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#define PLAT_ARM_MAX_BL1_RW_SIZE	UL(64 * 1024)	/* 64 KB */

/*
 * PLAT_ARM_MAX_ROMLIB_RW_SIZE is define to use a full page
 */

#if USE_ROMLIB
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0x1000)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0xe000)
#else
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0)
#endif

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth. Additional 8KiB space is added per chip in
 * order to accommodate the additional level of translation required for "TZC"
 * peripheral access which lies in >4TB address space.
 *
 */
#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL2_SIZE		(0x20000 + ((NRD_CHIP_COUNT - 1) * \
							0x2000))
#else
# define PLAT_ARM_MAX_BL2_SIZE		(0x14000 + ((NRD_CHIP_COUNT - 1) * \
							0x2000))
#endif

#define PLAT_ARM_MAX_BL31_SIZE		(NRD_BL31_SIZE +		\
					PLAT_ARM_MAX_BL2_SIZE +		\
					PLAT_ARM_MAX_BL1_RW_SIZE)

/*******************************************************************************
 * Stack sizes
 ******************************************************************************/

#if defined(IMAGE_BL1)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x440)
# endif
#elif defined(IMAGE_BL2)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x400)
# endif
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE		UL(0x400)
#elif defined(IMAGE_BL31)
# if SPM_MM
#  define PLATFORM_STACK_SIZE		UL(0x500)
# else
#  define PLATFORM_STACK_SIZE		UL(0x400)
# endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE		UL(0x440)
#endif

#if (SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP)) &&			\
ENABLE_FEAT_RAS && FFH_SUPPORT
/*
 * Secure partition stack follows right after the memory space reserved for
 * CPER buffer memory.
 */
#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +   \
					 PLAT_SP_IMAGE_NS_BUF_SIZE +   \
					 NRD_SP_CPER_BUF_SIZE)
#elif (SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP))
/*
 * Secure partition stack follows right after the memory region that is shared
 * between EL3 and S-EL0.
 */
#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)
#endif /* SPM_MM && ENABLE_FEAT_RAS && FFH_SUPPORT */

/*******************************************************************************
 * Console config
 ******************************************************************************/

#define PLAT_ARM_BOOT_UART_BASE		SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	SOC_CSS_UART_CLK_IN_HZ

#define PLAT_ARM_RUN_UART_BASE		SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	SOC_CSS_UART_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE	SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	SOC_CSS_UART_CLK_IN_HZ

/*******************************************************************************
 * SCMI config
 ******************************************************************************/

/* Number of SCMI channels on the platform */
#define PLAT_ARM_SCMI_CHANNEL_COUNT	NRD_CHIP_COUNT

/*******************************************************************************
 * ROM, SRAM and DRAM config
 ******************************************************************************/

#define PLAT_ARM_TRUSTED_ROM_BASE	UL(0x00000000)
#define PLAT_ARM_TRUSTED_ROM_SIZE	UL(0x00080000)	/* 512KB */

#define PLAT_ARM_TRUSTED_SRAM_SIZE	UL(0x00080000)	/* 512 KB */
#define PLAT_ARM_NSTIMER_FRAME_ID	UL(0)

#define PLAT_ARM_NSRAM_BASE		UL(0x06000000)
#define PLAT_ARM_NSRAM_SIZE		UL(0x00080000)	/* 512KB */

#define PLAT_ARM_DRAM2_BASE		ULL(0x8080000000)
#define PLAT_ARM_DRAM2_SIZE		ULL(0x180000000) /* 6GB */

/*
 * Required platform porting definitions common to all ARM CSS SoCs
 */
/* 2MB used for SCP DDR retraining */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	UL(0x00200000)

/*******************************************************************************
 * GIC/EHF config
 ******************************************************************************/

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#if ENABLE_FEAT_RAS && FFH_SUPPORT
#define PLAT_SP_PRI			PLAT_RAS_PRI
#else
#define PLAT_SP_PRI			(0x10)
#endif

/*******************************************************************************
 * SDS config
 ******************************************************************************/

/* SDS ID for unusable CPU MPID list structure */
#define SDS_ISOLATED_CPU_LIST_ID	U(128)

/* Index of SDS region used in the communication with SCP */
#define SDS_SCP_AP_REGION_ID		U(0)

/*******************************************************************************
 * Flash config
 ******************************************************************************/

#define PLAT_ARM_MEM_PROT_ADDR		(V2M_FLASH0_BASE +	\
					V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#endif /* NRD_PLAT_ARM_DEF2_H */
