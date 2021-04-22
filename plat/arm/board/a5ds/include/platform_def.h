/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>

/* Memory location options for TSP */
#define ARM_DRAM_ID			2

#define ARM_DRAM1_BASE			UL(0x80000000)
#define ARM_DRAM1_SIZE			UL(0x80000000)
#define ARM_DRAM1_END			(ARM_DRAM1_BASE +		\
					 ARM_DRAM1_SIZE - 1)

#define SRAM_BASE	0x2000000
#define SRAM_SIZE	0x200000

/* The first 4KB of NS DRAM1 are used as shared memory */
#define A5DS_SHARED_RAM_BASE		SRAM_BASE
#define A5DS_SHARED_RAM_SIZE		UL(0x00001000)	/* 4 KB */

/* The next 252 kB of NS DRAM is used to load the BL images */
#define ARM_BL_RAM_BASE	(A5DS_SHARED_RAM_BASE +	\
					 A5DS_SHARED_RAM_SIZE)
#define ARM_BL_RAM_SIZE	(PLAT_ARM_BL_PLUS_SHARED_RAM_SIZE -	\
					 A5DS_SHARED_RAM_SIZE)

#define PERIPHBASE 0x1a000000
#define PERIPH_SIZE  0x00240000
#define A5_PERIPHERALS_BASE 0x1c000000
#define A5_PERIPHERALS_SIZE  0x10000

#define ARM_CACHE_WRITEBACK_SHIFT	5

#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE)

#define ARM_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE)

#define A5DS_IRQ_TZ_WDOG			56
#define A5DS_IRQ_SEC_SYS_TIMER		57

/* Default cluster count for A5DS */
#define A5DS_CLUSTER_COUNT	U(1)

/* Default number of CPUs per cluster on A5DS */
#define A5DS_MAX_CPUS_PER_CLUSTER	U(4)

/* Default number of threads per CPU on A5DS */
#define A5DS_MAX_PE_PER_CPU	U(1)

#define A5DS_CORE_COUNT		U(4)

#define A5DS_PRIMARY_CPU	0x0

#define BOOT_BASE			ARM_DRAM1_BASE
#define BOOT_SIZE			UL(0x2800000)

#define ARM_NS_DRAM1_BASE		(ARM_DRAM1_BASE + BOOT_SIZE)
/*
 * The last 2MB is meant to be NOLOAD and will not be zero
 * initialized.
 */
#define ARM_NS_DRAM1_SIZE		(ARM_DRAM1_SIZE -		\
					 BOOT_SIZE -			\
					 0x00200000)

#define MAP_BOOT_RW          		MAP_REGION_FLAT(		\
						BOOT_BASE,		\
						BOOT_SIZE,    		\
						MT_DEVICE | MT_RW | MT_SECURE)

#define ARM_MAP_SHARED_RAM		MAP_REGION_FLAT(		\
						A5DS_SHARED_RAM_BASE,	\
						A5DS_SHARED_RAM_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)

#define ARM_MAP_NS_DRAM1		MAP_REGION_FLAT(		\
						ARM_NS_DRAM1_BASE,	\
						ARM_NS_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)

#define ARM_MAP_SRAM			MAP_REGION_FLAT(		\
						SRAM_BASE,		\
						SRAM_SIZE,		\
						MT_MEMORY | MT_RW | MT_NS)

/*
 * Mapping for the BL1 RW region. This mapping is needed by BL2 in order to
 * share the Mbed TLS heap. Since the heap is allocated inside BL1, it resides
 * in the BL1 RW region. Hence, BL2 needs access to the BL1 RW region in order
 * to be able to access the heap.
 */

#define ARM_MAP_BL_RO	MAP_REGION_FLAT(\
						BL_CODE_BASE,\
						BL_CODE_END - BL_CODE_BASE,\
						MT_CODE | MT_SECURE),\
					MAP_REGION_FLAT(\
						BL_RO_DATA_BASE,\
						BL_RO_DATA_END\
						- BL_RO_DATA_BASE,	\
						MT_RO_DATA | MT_SECURE)

#if USE_COHERENT_MEM
#define ARM_MAP_BL_COHERENT_RAM		MAP_REGION_FLAT(\
						BL_COHERENT_RAM_BASE,\
						BL_COHERENT_RAM_END	\
						- BL_COHERENT_RAM_BASE, \
						MT_DEVICE | MT_RW | MT_SECURE)
#endif

/*
 * Map the region for device tree configuration with read and write permissions
 */
#define ARM_MAP_BL_CONFIG_REGION	MAP_REGION_FLAT(ARM_BL_RAM_BASE,	\
						(ARM_FW_CONFIGS_LIMIT		\
							- ARM_BL_RAM_BASE),	\
						MT_MEMORY | MT_RW | MT_SECURE)

/*
 * The max number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#define ARM_BL_REGIONS			6

#define MAX_MMAP_REGIONS		(PLAT_ARM_MMAP_ENTRIES +	\
					 ARM_BL_REGIONS)

/* Memory mapped Generic timer interfaces  */
#define A5DS_TIMER_BASE_FREQUENCY		UL(7500000)

#define ARM_CONSOLE_BAUDRATE		115200

#define PLAT_PHY_ADDR_SPACE_SIZE			(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE			(1ULL << 32)

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		1

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		2

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(U(1) << ARM_CACHE_WRITEBACK_SHIFT)

/*
 * To enable FW_CONFIG to be loaded by BL1, define the corresponding base
 * and limit. Leave enough space of BL2 meminfo.
 */
#define ARM_FW_CONFIG_BASE		(ARM_BL_RAM_BASE + sizeof(meminfo_t))
#define ARM_FW_CONFIG_LIMIT		(ARM_BL_RAM_BASE + PAGE_SIZE)

/*
 * Define limit of firmware configuration memory:
 * ARM_FW_CONFIG + ARM_BL2_MEM_DESC memory
 */
#define ARM_FW_CONFIGS_LIMIT		(ARM_BL_RAM_BASE + (PAGE_SIZE * 2))

/*******************************************************************************
 * BL1 specific defines.
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 ******************************************************************************/
#define BL1_RO_BASE			0x00000000
#define BL1_RO_LIMIT			PLAT_ARM_TRUSTED_ROM_SIZE
/*
 * Put BL1 RW at the top of the memory allocated for BL images in NS DRAM.
 */
#define BL1_RW_BASE	(ARM_BL_RAM_BASE + \
						ARM_BL_RAM_SIZE - \
						(PLAT_ARM_MAX_BL1_RW_SIZE))
#define BL1_RW_LIMIT (ARM_BL_RAM_BASE + \
					    (ARM_BL_RAM_SIZE))
/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/

/*
 * Put BL2 just below BL1.
 */
#define BL2_BASE			(BL1_RW_BASE - A5DS_MAX_BL2_SIZE)
#define BL2_LIMIT			BL1_RW_BASE

/* Put BL32 below BL2 in NS DRAM.*/
#define ARM_BL2_MEM_DESC_BASE		ARM_FW_CONFIG_LIMIT
#define ARM_BL2_MEM_DESC_LIMIT		(ARM_BL2_MEM_DESC_BASE \
					+ (PAGE_SIZE / 2U))

#define BL32_BASE			((ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)\
						- PLAT_ARM_MAX_BL32_SIZE)
#define BL32_PROGBITS_LIMIT		BL2_BASE
#define BL32_LIMIT			(ARM_BL_RAM_BASE + ARM_BL_RAM_SIZE)

/* Required platform porting definitions */
#define PLATFORM_CORE_COUNT	A5DS_CORE_COUNT
#define PLAT_NUM_PWR_DOMAINS	(A5DS_CLUSTER_COUNT + \
				PLATFORM_CORE_COUNT) + U(1)

#define PLAT_MAX_PWR_LVL	2

/*
 * Other platform porting definitions are provided by included headers
 */

/*
 * Required ARM standard platform porting definitions
 */

#define PLAT_ARM_BL_PLUS_SHARED_RAM_SIZE	0x00040000	/* 256 KB */

#define PLAT_ARM_TRUSTED_ROM_BASE	0x00000000
#define PLAT_ARM_TRUSTED_ROM_SIZE	0x10000	/* 64KB */

#define PLAT_ARM_DRAM2_SIZE		ULL(0x80000000)

/*
 * Load address of BL33 for this platform port
 */
#define PLAT_ARM_NS_IMAGE_BASE	(ARM_DRAM1_BASE + U(0x8000000))

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage.
 */
#if defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		8
# define MAX_XLAT_TABLES		6
#else
# define PLAT_ARM_MMAP_ENTRIES		12
# define MAX_XLAT_TABLES		6
#endif

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#define PLAT_ARM_MAX_BL1_RW_SIZE	0xB000

/*
 * A5DS_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth.
 */
#define A5DS_MAX_BL2_SIZE		0x11000

/*
 * Since BL32 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL32_SIZE is
 * calculated using the current SP_MIN PROGBITS debug size plus the sizes of
 * BL2 and BL1-RW
 */
#define PLAT_ARM_MAX_BL32_SIZE		0x3B000
/*
 * Size of cacheable stacks
 */
#if defined(IMAGE_BL1)
#  define PLATFORM_STACK_SIZE 0x440
#elif defined(IMAGE_BL2)
#  define PLATFORM_STACK_SIZE 0x400
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE 0x440
#endif

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

/* Reserve the last block of flash for PSCI MEM PROTECT flag */
#define PLAT_ARM_FLASH_IMAGE_BASE	BOOT_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(BOOT_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_ARM_NVM_BASE		BOOT_BASE
#define PLAT_ARM_NVM_SIZE		(BOOT_SIZE - V2M_FLASH_BLOCK_SIZE)

/*
 * PL011 related constants
 */
#define PLAT_ARM_BOOT_UART_BASE		0x1A200000
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	UL(7500000)

#define PLAT_ARM_RUN_UART_BASE		0x1A210000
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	UL(7500000)

#define PLAT_ARM_CRASH_UART_BASE	PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	PLAT_ARM_RUN_UART_CLK_IN_HZ

#define A5DS_TIMER_BASE_FREQUENCY	UL(7500000)

/* System timer related constants */
#define PLAT_ARM_NSTIMER_FRAME_ID		1

/* Mailbox base address */
#define A5DS_TRUSTED_MAILBOX_BASE	A5DS_SHARED_RAM_BASE
#define A5DS_TRUSTED_MAILBOX_SIZE	(8 + A5DS_HOLD_SIZE)
#define A5DS_HOLD_BASE		(A5DS_TRUSTED_MAILBOX_BASE + 8)
#define A5DS_HOLD_SIZE		(PLATFORM_CORE_COUNT * \
					 A5DS_HOLD_ENTRY_SIZE)
#define A5DS_HOLD_ENTRY_SHIFT	3
#define A5DS_HOLD_ENTRY_SIZE	(1 << A5DS_HOLD_ENTRY_SHIFT)
#define A5DS_HOLD_STATE_WAIT	0
#define A5DS_HOLD_STATE_GO	1

/* Snoop Control Unit base address */
#define A5DS_SCU_BASE			0x1C000000

/*
 * GIC related constants to cater for GICv2
 */
#define PLAT_ARM_GICD_BASE		0x1C001000
#define PLAT_ARM_GICC_BASE		0x1C000100

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	ARM_G1S_IRQ_PROPS(grp), \
	INTR_PROP_DESC(A5DS_IRQ_TZ_WDOG, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(A5DS_IRQ_SEC_SYS_TIMER,\
		 GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_LEVEL)

#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#endif /* PLATFORM_DEF_H */
