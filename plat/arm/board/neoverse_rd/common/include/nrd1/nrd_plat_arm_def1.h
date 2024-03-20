/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the platform port definitions for the
 * first generation platforms based on the A75, N1 and V1 CPUs.
 */

#ifndef NRD_PLAT_ARM_DEF1_H
#define NRD_PLAT_ARM_DEF1_H

#ifndef __ASSEMBLER__
#include <lib/mmio.h>
#endif /* __ASSEMBLER__ */

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/common/arm_spm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <plat/arm/soc/common/soc_css_def.h>
#include <plat/common/common_def.h>
#include <nrd_css_fw_def1.h>
#include <nrd_ros_fw_def1.h>

/*******************************************************************************
 * Core count
 ******************************************************************************/

#define PLATFORM_CORE_COUNT	(NRD_CHIP_COUNT *			\
				PLAT_ARM_CLUSTER_COUNT *		\
				NRD_MAX_CPUS_PER_CLUSTER *		\
				NRD_MAX_PE_PER_CPU)

/*******************************************************************************
 * PA/VA config
 ******************************************************************************/

#ifdef __aarch64__
#define PLAT_PHY_ADDR_SPACE_SIZE	NRD_REMOTE_CHIP_MEM_OFFSET( \
						NRD_CHIP_COUNT)
#define PLAT_VIRT_ADDR_SPACE_SIZE	NRD_REMOTE_CHIP_MEM_OFFSET( \
						NRD_CHIP_COUNT)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#endif

/*******************************************************************************
 * XLAT definitions
 ******************************************************************************/

#if defined(IMAGE_BL31)
# if SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP)
#  define PLAT_ARM_MMAP_ENTRIES		(10 + ((NRD_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(11 + ((NRD_CHIP_COUNT - 1) * 3))
#  define PLAT_SP_IMAGE_MMAP_REGIONS	(12)
#  define PLAT_SP_IMAGE_MAX_XLAT_TABLES	(14)
# else
#  define PLAT_ARM_MMAP_ENTRIES		(5 + ((NRD_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(6 + ((NRD_CHIP_COUNT - 1) * 3))
# endif
#elif defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		(8)
# define MAX_XLAT_TABLES		(5)
#elif defined(IMAGE_BL2)
# define PLAT_ARM_MMAP_ENTRIES		(11 + (NRD_CHIP_COUNT - 1))

/*
 * MAX_XLAT_TABLES entries need to be doubled because when the address width
 * exceeds 40 bits an additional level of translation is required. In case of
 * multichip platforms peripherals also fall into address space with width
 * > 40 bits.
 */
# define MAX_XLAT_TABLES		(11  + ((NRD_CHIP_COUNT - 1) * 2))
#elif !USE_ROMLIB
# define PLAT_ARM_MMAP_ENTRIES		(11)
# define MAX_XLAT_TABLES		(7)
#else
# define PLAT_ARM_MMAP_ENTRIES		(12)
# define MAX_XLAT_TABLES		(6)
#endif

/*******************************************************************************
 * Stack size
 ******************************************************************************/

#if defined(IMAGE_BL1)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		(0x1000)
# else
#  define PLATFORM_STACK_SIZE		(0x440)
# endif
#elif defined(IMAGE_BL2)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		(0x1000)
# else
#  define PLATFORM_STACK_SIZE		(0x400)
# endif
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE		(0x400)
#elif defined(IMAGE_BL31)
# if SPM_MM
#  define PLATFORM_STACK_SIZE		(0x500)
# else
#  define PLATFORM_STACK_SIZE		(0x400)
# endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE		(0x440)
#endif

#if (SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP))
/*
 * Secure partition stack follows right after the memory region that is shared
 * between EL3 and S-EL0.
 */
#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)
#endif /* SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP) */

/*******************************************************************************
 * BL sizes
 ******************************************************************************/

#define PLAT_ARM_MAX_BL1_RW_SIZE	(64 * 1024)	/* 64 KB */
#if USE_ROMLIB
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	(0x1000)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	(0xe000)
#else
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	(0)
#endif

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth. Additional 8KiB space is added per chip in
 * order to accommodate the additional level of translation required for "TZC"
 * peripheral access which lies in >4TB address space.
 *
 */
#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL2_SIZE		(0x28000 + ((NRD_CHIP_COUNT - 1) * \
							0x2000))
#else
# define PLAT_ARM_MAX_BL2_SIZE		(0x14000 + ((NRD_CHIP_COUNT - 1) * \
							0x2000))
#endif

#define PLAT_ARM_MAX_BL31_SIZE		(NRD_CSS_BL31_SIZE +		\
						PLAT_ARM_MAX_BL2_SIZE +	\
						PLAT_ARM_MAX_BL1_RW_SIZE)

/*******************************************************************************
 * ROM, SRAM and DRAM config
 ******************************************************************************/

#define PLAT_ARM_TRUSTED_SRAM_SIZE	(0x00080000)	/* 512 KB */

#define PLAT_ARM_TRUSTED_ROM_BASE	(0x0)
#define PLAT_ARM_TRUSTED_ROM_SIZE	(0x00080000)	/* 512KB */

#define PLAT_ARM_NSRAM_BASE		(0x06000000)
#define PLAT_ARM_NSRAM_SIZE		(0x00080000)	/* 512KB */

#define PLAT_ARM_DRAM2_BASE		ULL(0x8080000000)
#define PLAT_ARM_DRAM2_SIZE		ULL(0x180000000)

/*******************************************************************************
 * Console config
 ******************************************************************************/

#define PLAT_ARM_BOOT_UART_BASE		NRD_CSS_SEC_UART_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

#define PLAT_ARM_RUN_UART_BASE		NRD_CSS_SEC_UART_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE	NRD_CSS_SEC_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

/*******************************************************************************
 * Timer config
 ******************************************************************************/

#define PLAT_ARM_NSTIMER_FRAME_ID	(0)

/*******************************************************************************
 * Power config
 ******************************************************************************/

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

/*******************************************************************************
 * Flash config
 ******************************************************************************/

#define PLAT_ARM_FLASH_IMAGE_BASE	V2M_FLASH0_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)
#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)
#define PLAT_ARM_MEM_PROT_ADDR		(V2M_FLASH0_BASE + \
					 V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)
/* IO storage framework */
#define MAX_IO_DEVICES			(3)
#define MAX_IO_HANDLES			(4)

/*******************************************************************************
 * SCMI config
 ******************************************************************************/

/* Number of SCMI channels on the platform */
#define PLAT_ARM_SCMI_CHANNEL_COUNT	NRD_CHIP_COUNT

/*******************************************************************************
 * SDS config
 ******************************************************************************/

/* Index of SDS region used in the communication with SCP */
#define SDS_SCP_AP_REGION_ID		U(0)
/* SDS ID for unusable CPU MPID list structure */
#define SDS_ISOLATED_CPU_LIST_ID	U(128)

/*******************************************************************************
 * GIC/EHF config
 ******************************************************************************/

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)
#define PLAT_SP_PRI			(0x10)

/*******************************************************************************
 * Platform type identification macro
 ******************************************************************************/

/* Platform ID related accessors */
#define BOARD_CSS_PLAT_ID_REG_ID_MASK		0x0f
#define BOARD_CSS_PLAT_ID_REG_ID_SHIFT		0x0
#define BOARD_CSS_PLAT_TYPE_EMULATOR		0x02

#ifndef __ASSEMBLER__
#define BOARD_CSS_GET_PLAT_TYPE(addr)					\
		((mmio_read_32(addr) & BOARD_CSS_PLAT_ID_REG_ID_MASK)	\
		>> BOARD_CSS_PLAT_ID_REG_ID_SHIFT)
#endif /* __ASSEMBLER__ */

/* Platform ID address */
#define BOARD_CSS_PLAT_ID_REG_ADDR		UL(0x7ffe00e0)

#endif /* NRD_PLAT_ARM_DEF1_H */
