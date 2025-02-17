/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#define PLAT_ARM_TRUSTED_SRAM_BASE		UL(0x0)
#define PLAT_ARM_TRUSTED_SRAM_SIZE		UL(0x00100000)

/*
 * SMD region assignment
 * AP accesses devices located in SMD via AP SMD region starting from
 * 0x4000_0000.
 */
/* CNTControl frame 64KB */
#define PLAT_ARM_SYS_CNTCTL_BASE		UL(0x40000000)
/* CNTRead frame 64KB */
#define PLAT_ARM_SYS_CNTREAD_BASE		UL(0x40010000)
/* SI MHU regions 6MB in total */
#define SI_MHU_REGION_BASE			UL(0x40020000)

/* AP access AP_REFLCK generic timers via Peripheral Block */
#define PLAT_ARM_SYS_TIMCTL_BASE		UL(0x1A810000)
#define PLAT_ARM_SYS_CNT_BASE_S			UL(0x1A820000)
#define PLAT_ARM_SYS_CNT_BASE_NS		UL(0x1A830000)
#define PLAT_ARM_NSTIMER_FRAME_ID		U(1)

#include <plat/arm/common/arm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <plat/common/common_def.h>

#define PLATFORM_CORE_COUNT			U(16)
#define PLAT_ARM_CLUSTER_COUNT			U(4)
#define PLAT_MAX_CPUS_PER_CLUSTER		U(4)
#define PLAT_MAX_PE_PER_CPU			U(1)

#define RDASPEN_CLUSTER_CORE_COUNT		U(4)

#define PLATFORM_STACK_SIZE			UL(0x1000)

/* BL1 is not supported */
#define PLAT_ARM_TRUSTED_ROM_BASE		UL(0x0)
#define PLAT_ARM_TRUSTED_ROM_SIZE		UL(0x0)
#define PLAT_ARM_MAX_BL1_RW_SIZE		UL(0x0)

/* USE_ROMLIB is not supported */
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE		U(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE		U(0)

#define PLAT_ARM_MAX_BL31_SIZE			UL(0xED000)

#undef ARM_DRAM2_BASE
#define ARM_DRAM2_BASE				ULL(0x20000000000)
#undef ARM_DRAM2_SIZE
#define ARM_DRAM2_SIZE				SZ_2G

/*
 * MHU for SCMI communication with SCP on Safety Island CL0
 * Offset to SI_MHU_REGION_BASE: 0x0008_0000.
 */
#define PLAT_CSS_MHU_BASE			UL(0x400A0000)

/* UART Related Constants */
#define SOC_CSS_SEC_UART_BASE			UL(0x1A410000)
#define SOC_CSS_UART_SIZE			SZ_64K
#define SOC_CSS_UART_CLK_IN_HZ			UL(24000000)
#define PLAT_ARM_BOOT_UART_BASE			SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ		SOC_CSS_UART_CLK_IN_HZ
#define PLAT_ARM_RUN_UART_BASE			SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ		SOC_CSS_UART_CLK_IN_HZ
#define PLAT_ARM_CRASH_UART_BASE		SOC_CSS_SEC_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ		SOC_CSS_UART_CLK_IN_HZ

/* 256 TB */
#define SZ_256T					(1ULL << 48)
/* Physical and virtual address space limits for MMU */
#define PLAT_PHY_ADDR_SPACE_SIZE		SZ_256T
#define PLAT_VIRT_ADDR_SPACE_SIZE		SZ_256T

#define PLAT_ARM_GICD_BASE			UL(0x20000000)
#define PLAT_ARM_GICR_BASE			UL(0x200C0000)
#define PLAT_ARM_G1S_IRQ_PROPS(grp)		CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)		ARM_G0_IRQ_PROPS(grp)

/* Secure Watchdog Constants */
#define SBSA_SECURE_WDOG_BASE			UL(0x1A460000)
#define SBSA_SECURE_WDOG_TIMEOUT		UL(100)

/* Virtual address used by dynamic mem_protect for chunk_base */
#define PLAT_ARM_MEM_PROTEC_VA_FRAME		UL(0xC0000000)

/* SCMI Related Constants */
#define PLAT_ARM_SCMI_CHANNEL_COUNT		U(1)
#define CSS_SYSTEM_PWR_DMN_LVL			ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL			ARM_PWR_LVL1

#define MAX_IO_DEVICES				U(3)
#define MAX_IO_HANDLES				U(4)

#if IMAGE_BL2
#define PLAT_ARM_MMAP_ENTRIES			U(8)
#elif IMAGE_BL31
#define PLAT_ARM_MMAP_ENTRIES			U(7)
#endif
#define MAX_XLAT_TABLES				U(8)

#define PLAT_FW_CONFIG_MAX_SIZE			(ARM_FW_CONFIG_LIMIT - \
						 ARM_FW_CONFIG_BASE)
#define PLAT_FW_CONFIG_BASE			ARM_FW_CONFIG_BASE

/*
 * Map peripherals till GIC regions and devices in Rest of System
 */
#define RDASPEN_DEVICE_BASE		UL(0x10000000)
#define RDASPEN_DEVICE_SIZE		UL(0x28000000)
#define RDASPEN_MAP_DEVICE		MAP_REGION_FLAT(RDASPEN_DEVICE_BASE, \
							RDASPEN_DEVICE_SIZE, \
							MT_DEVICE | MT_RW | \
							MT_SECURE)

/* Flash in Rest of the System */
#define EXT_FLASH_BASE			UL(0x38000000)
#define EXT_FLASH_SIZE			SZ_128M
#define EXT_FLASH_BLOCK_SIZE		SZ_256K
#define PLAT_ARM_FLASH_IMAGE_BASE	EXT_FLASH_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(EXT_FLASH_SIZE - \
					 EXT_FLASH_BLOCK_SIZE)

#define RDASPEN_MAP_EXTERNAL_FLASH	MAP_REGION_FLAT(EXT_FLASH_BASE, \
							PLAT_ARM_FLASH_IMAGE_MAX_SIZE, \
							MT_DEVICE | MT_RO | \
							MT_SECURE)

/* SMD region */
#define RDASPEN_SMD_REGION_BASE		UL(0x40000000)
#define RDASPEN_SMD_REGION_SIZE		UL(0x10000000)
#define RDASPEN_MAP_SMD_REGION		MAP_REGION_FLAT(RDASPEN_SMD_REGION_BASE, \
							RDASPEN_SMD_REGION_SIZE, \
							MT_DEVICE | MT_RW | \
							MT_SECURE)

/*
 * NS DRAM
 * So far the whole DRAM1 is assigned to NS.
 */
#define RDASPEN_MAP_NS_DRAM1		MAP_REGION_FLAT(ARM_DRAM1_BASE, \
							ARM_DRAM1_SIZE, \
							MT_MEMORY | MT_RW | \
							MT_NS)

#define RDASPEN_MAP_NS_DRAM2		MAP_REGION_FLAT(ARM_DRAM2_BASE, \
							ARM_DRAM2_SIZE,	\
							MT_MEMORY | MT_RW | \
							MT_NS)

/* Secure DRAM */
#define RDASPEN_MAP_S_DRAM		MAP_REGION_FLAT(PLAT_ARM_SPMC_BASE, \
							PLAT_ARM_SPMC_SIZE, \
							MT_MEMORY | MT_RW | \
							MT_SECURE)

/* load address of DTB */
#define PLAT_HW_CONFIG_DTB_BASE		ARM_DRAM1_BASE
#define PLAT_ARM_HW_CONFIG_SIZE		UL(0x8000)

/*
 * PSCI memory protect definitions:
 * This variable is stored in a non-secure flash because some ARM reference
 * platforms do not have secure NVRAM. Real systems that provided MEM_PROTECT
 * support must use a secure NVRAM to store the PSCI MEM_PROTECT definitions.
 */
#define PLAT_ARM_MEM_PROT_ADDR		(EXT_FLASH_BASE + \
					 EXT_FLASH_SIZE - \
					 EXT_FLASH_BLOCK_SIZE)

/* Map mem_protect flash region with read and write permissions */
#define RDASPEN_MAP_MEM_PROTECT		MAP_REGION_FLAT(PLAT_ARM_MEM_PROT_ADDR,	\
							EXT_FLASH_BLOCK_SIZE,	\
							MT_DEVICE | MT_RW | \
							MT_SECURE)

/* 125 MHz REFCLK to System Counter */
#define SYS_COUNTER_FREQ_IN_TICKS	UL(125000000)

/* The index of the primary CPU */
#define RDASPEN_PRIMARY_CPU		0x0

#endif  /* PLATFORM_DEF_H */
