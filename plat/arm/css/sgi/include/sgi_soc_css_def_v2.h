/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI_SOC_CSS_DEF_V2_H
#define SGI_SOC_CSS_DEF_V2_H

#include <lib/utils_def.h>
#include <plat/common/common_def.h>

/*
 * Definitions common to all ARM CSS SoCs
 */

/* Following covers ARM CSS SoC Peripherals */

#define SOC_SYSTEM_PERIPH_BASE		UL(0x0C000000)
#define SOC_SYSTEM_PERIPH_SIZE		UL(0x02000000)

#define SOC_PLATFORM_PERIPH_BASE	UL(0x0E000000)
#define SOC_PLATFORM_PERIPH_SIZE	UL(0x02000000)

#define SOC_CSS_PCIE_CONTROL_BASE	UL(0x0ef20000)

/* PL011 UART related constants */
#define SOC_CSS_UART1_BASE		UL(0x0ef80000)
#define SOC_CSS_UART0_BASE		UL(0x0ef70000)

/* Memory controller */
#define SOC_MEMCNTRL_BASE		UL(0x10000000)
#define SOC_MEMCNTRL_SIZE		UL(0x10000000)

#define SOC_CSS_UART0_CLK_IN_HZ		UL(7372800)
#define SOC_CSS_UART1_CLK_IN_HZ		UL(7372800)

/* SoC NIC-400 Global Programmers View (GPV) */
#define SOC_CSS_NIC400_BASE		UL(0x0ED00000)

#define SOC_CSS_NIC400_USB_EHCI		U(0)
#define SOC_CSS_NIC400_TLX_MASTER	U(1)
#define SOC_CSS_NIC400_USB_OHCI		U(2)
#define SOC_CSS_NIC400_PL354_SMC	U(3)
/*
 * The apb4_bridge controls access to:
 *   - the PCIe configuration registers
 *   - the MMU units for USB, HDLCD and DMA
 */
#define SOC_CSS_NIC400_APB4_BRIDGE	U(4)

/* Non-volatile counters */
#define SOC_TRUSTED_NVCTR_BASE		UL(0x0EE70000)
#define TFW_NVCTR_BASE			(SOC_TRUSTED_NVCTR_BASE + 0x0000)
#define TFW_NVCTR_SIZE			U(4)
#define NTFW_CTR_BASE			(SOC_TRUSTED_NVCTR_BASE + 0x0004)
#define NTFW_CTR_SIZE			U(4)

/* Keys */
#define SOC_KEYS_BASE			UL(0x0EE80000)
#define TZ_PUB_KEY_HASH_BASE		(SOC_KEYS_BASE + 0x0000)
#define TZ_PUB_KEY_HASH_SIZE		U(32)
#define HU_KEY_BASE			(SOC_KEYS_BASE + 0x0020)
#define HU_KEY_SIZE			U(16)
#define END_KEY_BASE			(SOC_KEYS_BASE + 0x0044)
#define END_KEY_SIZE			U(32)

#define SOC_PLATFORM_PERIPH_MAP_DEVICE	MAP_REGION_FLAT(			\
						SOC_PLATFORM_PERIPH_BASE, 	\
						SOC_PLATFORM_PERIPH_SIZE, 	\
						MT_DEVICE | MT_RW | MT_SECURE)

#if SPM_MM
/*
 * Memory map definition for the platform peripheral memory region that is
 * accessible from S-EL0 (with secure user mode access).
 */
#define SOC_PLATFORM_PERIPH_MAP_DEVICE_USER				       \
		MAP_REGION_FLAT(					       \
			SOC_PLATFORM_PERIPH_BASE,			       \
			SOC_PLATFORM_PERIPH_SIZE,			       \
			MT_DEVICE | MT_RW | MT_SECURE | MT_USER)
#endif

#define SOC_SYSTEM_PERIPH_MAP_DEVICE	MAP_REGION_FLAT(			\
						SOC_SYSTEM_PERIPH_BASE,		\
						SOC_SYSTEM_PERIPH_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#define SOC_MEMCNTRL_MAP_DEVICE		MAP_REGION_FLAT(			\
						SOC_MEMCNTRL_BASE,		\
						SOC_MEMCNTRL_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#define SOC_MEMCNTRL_MAP_DEVICE_REMOTE_CHIP(n)					\
		MAP_REGION_FLAT(						\
			CSS_SGI_REMOTE_CHIP_MEM_OFFSET(n) + SOC_MEMCNTRL_BASE,	\
			SOC_MEMCNTRL_SIZE,					\
			MT_DEVICE | MT_RW | MT_SECURE)

/*
 * The bootsec_bridge controls access to a bunch of peripherals, e.g. the UARTs.
 */
#define SOC_CSS_NIC400_BOOTSEC_BRIDGE		U(5)
#define SOC_CSS_NIC400_BOOTSEC_BRIDGE_UART1	UL(1 << 12)

/*
 * Required platform porting definitions common to all ARM CSS SoCs
 */
/* 2MB used for SCP DDR retraining */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	UL(0x00200000)

/* V2M motherboard system registers & offsets */
#define V2M_SYSREGS_BASE		UL(0x0C010000)
#define V2M_SYS_LED			U(0x8)

/*
 * V2M sysled bit definitions. The values written to this
 * register are defined in arch.h & runtime_svc.h. Only
 * used by the primary cpu to diagnose any cold boot issues.
 *
 * SYS_LED[0]   - Security state (S=0/NS=1)
 * SYS_LED[2:1] - Exception Level (EL3-EL0)
 * SYS_LED[7:3] - Exception Class (Sync/Async & origin)
 *
 */
#define V2M_SYS_LED_SS_SHIFT		U(0)
#define V2M_SYS_LED_EL_SHIFT		U(1)
#define V2M_SYS_LED_EC_SHIFT		U(3)

#define V2M_SYS_LED_SS_MASK		U(0x01)
#define V2M_SYS_LED_EL_MASK		U(0x03)
#define V2M_SYS_LED_EC_MASK		U(0x1f)

/* NOR Flash */
#define V2M_FLASH0_BASE			UL(0x08000000)
#define V2M_FLASH0_SIZE			UL(0x04000000)
#define V2M_FLASH_BLOCK_SIZE		UL(0x00040000)	/* 256 KB */

/*
 * The flash can be mapped either as read-only or read-write.
 *
 * If it is read-write then it should also be mapped as device memory because
 * NOR flash programming involves sending a fixed, ordered sequence of commands.
 *
 * If it is read-only then it should also be mapped as:
 * - Normal memory, because reading from NOR flash is transparent, it is like
 *   reading from RAM.
 * - Non-executable by default. If some parts of the flash need to be executable
 *   then platform code is responsible for re-mapping the appropriate portion
 *   of it as executable.
 */
#define V2M_MAP_FLASH0_RW		MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define V2M_MAP_FLASH0_RO		MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_RO_DATA | MT_SECURE)

#define SGI_MAP_FLASH0_RO		MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_DEVICE | MT_RO | MT_SECURE)

/* Platform ID address */
#define BOARD_CSS_PLAT_ID_REG_ADDR		UL(0x0EFE00E0)

/* Platform ID related accessors */
#define BOARD_CSS_PLAT_ID_REG_ID_MASK		U(0x0F)
#define BOARD_CSS_PLAT_ID_REG_ID_SHIFT		U(0x00)
#define BOARD_CSS_PLAT_ID_REG_VERSION_MASK	U(0xF00)
#define BOARD_CSS_PLAT_ID_REG_VERSION_SHIFT	U(0x08)
#define BOARD_CSS_PLAT_TYPE_RTL			U(0x00)
#define BOARD_CSS_PLAT_TYPE_FPGA		U(0x01)
#define BOARD_CSS_PLAT_TYPE_EMULATOR		U(0x02)
#define BOARD_CSS_PLAT_TYPE_FVP			U(0x03)

#ifndef __ASSEMBLER__

#include <lib/mmio.h>

#define BOARD_CSS_GET_PLAT_TYPE(addr)					\
	((mmio_read_32(addr) & BOARD_CSS_PLAT_ID_REG_ID_MASK)		\
	>> BOARD_CSS_PLAT_ID_REG_ID_SHIFT)

#endif /* __ASSEMBLER__ */


#define MAX_IO_DEVICES			U(3)
#define MAX_IO_HANDLES			U(4)

/* Reserve the last block of flash for PSCI MEM PROTECT flag */
#define PLAT_ARM_FLASH_IMAGE_BASE	V2M_FLASH0_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#if ARM_GPT_SUPPORT
/*
 * Offset of the FIP in the GPT image. BL1 component uses this option
 * as it does not load the partition table to get the FIP base
 * address. At sector 34 by default (i.e. after reserved sectors 0-33)
 * Offset = 34 * 512(sector size) = 17408 i.e. 0x4400
 */
#define PLAT_ARM_FIP_OFFSET_IN_GPT		0x4400
#endif /* ARM_GPT_SUPPORT */

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE			SOC_CSS_UART0_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ		SOC_CSS_UART0_CLK_IN_HZ

#define PLAT_ARM_RUN_UART_BASE			SOC_CSS_UART1_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ		SOC_CSS_UART1_CLK_IN_HZ

#define PLAT_ARM_SP_MIN_RUN_UART_BASE		SOC_CSS_UART1_BASE
#define PLAT_ARM_SP_MIN_RUN_UART_CLK_IN_HZ	SOC_CSS_UART1_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE		PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ		PLAT_ARM_RUN_UART_CLK_IN_HZ

#endif /* SGI_SOC_CSS_DEF_V2_H */
