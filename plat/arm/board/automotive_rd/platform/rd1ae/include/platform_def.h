/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

/* These are referenced by arm_def.h #included next, so #define first. */
#define PLAT_ARM_TRUSTED_SRAM_BASE		UL(0x0)

#include <plat/arm/common/arm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <plat/common/common_def.h>

#define PLATFORM_CORE_COUNT			U(16)
#define PLAT_ARM_CLUSTER_COUNT			U(16)
#define PLAT_MAX_CPUS_PER_CLUSTER		U(1)
#define PLAT_MAX_PE_PER_CPU			U(1)

#define PLATFORM_STACK_SIZE			UL(0x1000)

/* BL1 is not supported */
#define PLAT_ARM_TRUSTED_ROM_BASE		UL(0x0)
#define PLAT_ARM_TRUSTED_ROM_SIZE		UL(0x0)

#define PLAT_ARM_TRUSTED_SRAM_SIZE		UL(0x00080000)

/* USE_ROMLIB is not supported */
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE		U(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE		U(0)

/* Defined based on actual binary sizes */
#define PLAT_ARM_MAX_BL1_RW_SIZE		0x0
#define PLAT_ARM_MAX_BL2_SIZE			0x20000
#define PLAT_ARM_MAX_BL31_SIZE			0x70000

#define PLAT_ARM_DRAM2_BASE			ULL(0x8080000000)
#define PLAT_ARM_DRAM2_SIZE			ULL(0x180000000)

#define PLAT_CSS_MHU_BASE			UL(0x2A920000)
#define PLAT_ARM_NSTIMER_FRAME_ID		U(0)

#define PLAT_ARM_SPMC_BASE		        UL(0xFFC00000)
#define PLAT_ARM_SPMC_SIZE		        UL(0x00400000)

#define SOC_CSS_SEC_UART_BASE			UL(0x2A410000)
#define SOC_CSS_NSEC_UART_BASE			UL(0x2A400000)
#define SOC_CSS_UART_SIZE			UL(0x10000)
#define SOC_CSS_UART_CLK_IN_HZ			UL(7372800)
#define PLAT_ARM_BOOT_UART_BASE			SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ		SOC_CSS_UART_CLK_IN_HZ
#define PLAT_ARM_RUN_UART_BASE			SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ		SOC_CSS_UART_CLK_IN_HZ
#define PLAT_ARM_CRASH_UART_BASE		SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ		SOC_CSS_UART_CLK_IN_HZ

/* Physical and virtual address space limits for MMU */
#define PLAT_PHY_ADDR_SPACE_SIZE		(1ULL << 42)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ULL << 42)

/* GIC related constants */
#define PLAT_ARM_GICD_BASE			UL(0x30000000)
#define PLAT_ARM_GICR_BASE			UL(0x301C0000)
#define PLAT_ARM_GICC_BASE			UL(0x2C000000)
#define PLAT_ARM_G1S_IRQ_PROPS(grp)		CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)		ARM_G0_IRQ_PROPS(grp)

/* Virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME		UL(0xC0000000)

/* Secure Watchdog Constants */
#define SBSA_SECURE_WDOG_BASE			UL(0x2A480000)
#define SBSA_SECURE_WDOG_TIMEOUT		UL(100)

#define V2M_SYS_LED_SS_SHIFT			U(0)
#define V2M_SYS_LED_EL_SHIFT			U(1)
#define V2M_SYS_LED_EC_SHIFT			U(3)

#define V2M_SYS_LED_SS_MASK			U(0x01)
#define V2M_SYS_LED_EL_MASK			U(0x03)
#define V2M_SYS_LED_EC_MASK			U(0x1f)

#define V2M_SYSREGS_BASE			UL(0x0C010000)
#define V2M_SYS_LED				U(0x8)

#define PLAT_ARM_SCMI_CHANNEL_COUNT		U(1)
#define CSS_SYSTEM_PWR_DMN_LVL			ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL				ARM_PWR_LVL1

#define MAX_IO_DEVICES				U(3)
#define MAX_IO_HANDLES				U(4)

#ifdef IMAGE_BL2
#define PLAT_ARM_MMAP_ENTRIES			U(5)
#else
#define PLAT_ARM_MMAP_ENTRIES			U(6)
#endif
#define MAX_XLAT_TABLES				U(6)

#define V2M_FLASH0_BASE				UL(0x08000000)
#define V2M_FLASH0_SIZE				UL(0x04000000)
#define V2M_FLASH_BLOCK_SIZE			UL(0x00040000)	/* 256 KB */
#define PLAT_ARM_FLASH_IMAGE_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_FW_CONFIG_MAX_SIZE			(ARM_FW_CONFIG_LIMIT - ARM_FW_CONFIG_BASE)
#define PLAT_FW_CONFIG_BASE			ARM_FW_CONFIG_BASE

/* RD1AE-specific memory mappings */
#define RD1AE_EXTERNAL_FLASH	MAP_REGION_FLAT(V2M_FLASH0_BASE, \
						V2M_FLASH0_SIZE, \
						MT_DEVICE | MT_RO | \
						MT_SECURE)

#define RD1AE_MAP_NS_DRAM1	MAP_REGION_FLAT(ARM_DRAM1_BASE,	\
						ARM_DRAM1_SIZE - PLAT_ARM_SPMC_SIZE, \
						MT_MEMORY | MT_RW | \
						MT_NS)

#define RD1AE_MAP_S_DRAM1	MAP_REGION_FLAT(PLAT_ARM_SPMC_BASE, \
						PLAT_ARM_SPMC_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)

#define RD1AE_DEVICE_BASE	(0x20000000)
#define RD1AE_DEVICE_SIZE	(0x20000000)
#define RD1AE_MAP_DEVICE	MAP_REGION_FLAT(RD1AE_DEVICE_BASE, \
						RD1AE_DEVICE_SIZE, \
						MT_DEVICE | MT_RW | \
						MT_SECURE)

#define SOC_PLATFORM_PERIPH_BASE	UL(0x0E000000)
#define SOC_PLATFORM_PERIPH_SIZE	UL(0x02000000)
#define SOC_PLATFORM_PERIPH_MAP_DEVICE	MAP_REGION_FLAT(SOC_PLATFORM_PERIPH_BASE, \
							SOC_PLATFORM_PERIPH_SIZE, \
							MT_DEVICE | MT_RW | MT_SECURE)

/* Non-volatile counters */
#define TRUSTED_NVCTR_BASE_OFFSET	UL(0x00E70000)
#define TFW_NVCTR_BASE_OFFSET		0x0000
#define NTFW_CTR_BASE_OFFSET		0x0004
#define SOC_TRUSTED_NVCTR_BASE		(SOC_PLATFORM_PERIPH_BASE + TRUSTED_NVCTR_BASE_OFFSET)
#define TFW_NVCTR_BASE			(SOC_TRUSTED_NVCTR_BASE + TFW_NVCTR_BASE_OFFSET)
#define TFW_NVCTR_SIZE			U(4)
#define NTFW_CTR_BASE			(SOC_TRUSTED_NVCTR_BASE + NTFW_CTR_BASE_OFFSET)
#define NTFW_CTR_SIZE			U(4)

/*******************************************************************************
 * Memprotect definitions
 ******************************************************************************/
/* PSCI memory protect definitions:
 * This variable is stored in a non-secure flash because some ARM reference
 * platforms do not have secure NVRAM. Real systems that provided MEM_PROTECT
 * support must use a secure NVRAM to store the PSCI MEM_PROTECT definitions.
 */
#define PLAT_ARM_MEM_PROT_ADDR	(V2M_FLASH0_BASE + \
					V2M_FLASH0_SIZE - \
					V2M_FLASH_BLOCK_SIZE)

#endif /* PLATFORM_DEF_H */
