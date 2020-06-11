/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019-2020, Linaro Limited and Contributors.
 * All rights reserved.
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <plat/common/common_def.h>
#include <tbbr_img_def.h>

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define QEMU_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define PLATFORM_STACK_SIZE		0x1000

#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(4)
#define PLATFORM_CLUSTER_COUNT		U(2)
#define PLATFORM_CLUSTER0_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CLUSTER1_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER0_CORE_COUNT + \
					 PLATFORM_CLUSTER1_CORE_COUNT)

#define QEMU_PRIMARY_CPU		U(0)

#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1

#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		2

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN		0
/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET		1
/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains.
 */
#define PLAT_LOCAL_STATE_OFF		2

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define PLAT_LOCAL_PSTATE_WIDTH		4
#define PLAT_LOCAL_PSTATE_MASK		((1 << PLAT_LOCAL_PSTATE_WIDTH) - 1)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

/*
 * Partition memory into secure ROM, non-secure DRAM, secure "SRAM",
 * and secure DRAM.
 */
#define SEC_ROM_BASE			0x00000000
#define SEC_ROM_SIZE			0x00020000

#define NS_DRAM0_BASE			0x10000000000ULL
#define NS_DRAM0_SIZE			0x00020000000

#define SEC_SRAM_BASE			0x20000000
#define SEC_SRAM_SIZE			0x20000000

/*
 * RAD just placeholders, need to be chosen after finalizing mem map
 */
#define SEC_DRAM_BASE			0x1000
#define SEC_DRAM_SIZE			0x1000

/* Load pageable part of OP-TEE 2MB above secure DRAM base */
#define QEMU_OPTEE_PAGEABLE_LOAD_BASE	(SEC_DRAM_BASE + 0x00200000)
#define QEMU_OPTEE_PAGEABLE_LOAD_SIZE	0x00400000

/*
 * ARM-TF lives in SRAM, partition it here
 */

#define SHARED_RAM_BASE			SEC_SRAM_BASE
#define SHARED_RAM_SIZE			0x00001000

#define PLAT_QEMU_TRUSTED_MAILBOX_BASE	SHARED_RAM_BASE
#define PLAT_QEMU_TRUSTED_MAILBOX_SIZE	(8 + PLAT_QEMU_HOLD_SIZE)
#define PLAT_QEMU_HOLD_BASE		(PLAT_QEMU_TRUSTED_MAILBOX_BASE + 8)
#define PLAT_QEMU_HOLD_SIZE		(PLATFORM_CORE_COUNT * \
					 PLAT_QEMU_HOLD_ENTRY_SIZE)
#define PLAT_QEMU_HOLD_ENTRY_SHIFT	3
#define PLAT_QEMU_HOLD_ENTRY_SIZE	(1 << PLAT_QEMU_HOLD_ENTRY_SHIFT)
#define PLAT_QEMU_HOLD_STATE_WAIT	0
#define PLAT_QEMU_HOLD_STATE_GO		1

#define BL_RAM_BASE			(SHARED_RAM_BASE + SHARED_RAM_SIZE)
#define BL_RAM_SIZE			(SEC_SRAM_SIZE - SHARED_RAM_SIZE)

/*
 * BL1 specific defines.
 *
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 * Put BL1 RW at the top of the Secure SRAM. BL1_RW_BASE is calculated using
 * the current BL1 RW debug size plus a little space for growth.
 */
#define BL1_SIZE			0x12000
#define BL1_RO_BASE			SEC_ROM_BASE
#define BL1_RO_LIMIT			(SEC_ROM_BASE + SEC_ROM_SIZE)
#define BL1_RW_BASE			(BL1_RW_LIMIT - BL1_SIZE)
#define BL1_RW_LIMIT			(BL_RAM_BASE + BL_RAM_SIZE)

/*
 * BL2 specific defines.
 *
 * Put BL2 just below BL3-1. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth.
 */
#define BL2_SIZE			0x1D000
#define BL2_BASE			(BL31_BASE - BL2_SIZE)
#define BL2_LIMIT			BL31_BASE

/*
 * BL3-1 specific defines.
 *
 * Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth.
 */
#define BL31_SIZE			0x50000
#define BL31_BASE			(BL31_LIMIT - BL31_SIZE)
#define BL31_LIMIT			(BL1_RW_BASE)
#define BL31_PROGBITS_LIMIT		BL1_RW_BASE


/*
 * BL3-2 specific defines.
 *
 * BL3-2 can execute from Secure SRAM, or Secure DRAM.
 */
#define BL32_SRAM_BASE			BL_RAM_BASE
#define BL32_SRAM_LIMIT			BL2_BASE

#define BL32_MEM_BASE			BL_RAM_BASE
#define BL32_MEM_SIZE			(BL_RAM_SIZE - BL1_SIZE - \
					BL2_SIZE - BL31_SIZE)
#define BL32_BASE			BL32_SRAM_BASE
#define BL32_LIMIT			BL32_SRAM_LIMIT

#define NS_IMAGE_OFFSET			(NS_DRAM0_BASE + 0x20000000)
#define NS_IMAGE_MAX_SIZE		(NS_DRAM0_SIZE - 0x20000000)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 42)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 42)
#if SPM_MM
#define MAX_MMAP_REGIONS		12
#define MAX_XLAT_TABLES			11
#else
#define MAX_MMAP_REGIONS		11
#define MAX_XLAT_TABLES			10
#endif
#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

#if SPM_MM && defined(IMAGE_BL31)
# define PLAT_SP_IMAGE_MMAP_REGIONS	30
# define PLAT_SP_IMAGE_MAX_XLAT_TABLES	20
#endif

/*
 * PL011 related constants
 */
#define UART0_BASE			0x60000000
#define UART1_BASE			0x60030000
#define UART0_CLK_IN_HZ			1
#define UART1_CLK_IN_HZ			1

/* Secure UART */
#define UART2_BASE			0x60040000
#define UART2_CLK_IN_HZ			1

#define PLAT_QEMU_BOOT_UART_BASE	UART0_BASE
#define PLAT_QEMU_BOOT_UART_CLK_IN_HZ	UART0_CLK_IN_HZ

#define PLAT_QEMU_CRASH_UART_BASE	UART1_BASE
#define PLAT_QEMU_CRASH_UART_CLK_IN_HZ	UART1_CLK_IN_HZ

#define PLAT_QEMU_CONSOLE_BAUDRATE	115200

#define QEMU_FLASH0_BASE		0x00000000
#define QEMU_FLASH0_SIZE		0x10000000
#define QEMU_FLASH1_BASE		0x10000000
#define QEMU_FLASH1_SIZE		0x10000000

#define PLAT_QEMU_FIP_BASE		0x00008000
#define PLAT_QEMU_FIP_MAX_SIZE		0x00400000

/* This is map from GIC_DIST up to last CPU (255) GIC_REDISTR */
#define DEVICE0_BASE			0x40000000
#define DEVICE0_SIZE			0x04080000
/* This is map from NORMAL_UART up to SECURE_UART_MM */
#define DEVICE1_BASE			0x60000000
#define DEVICE1_SIZE			0x00041000

/*
 * GIC related constants
 * We use GICv3 where CPU Interface registers are not memory mapped
 */
#define GICD_BASE			0x40060000
#define GICR_BASE			0x40080000
#define GICC_BASE			0x0

#define QEMU_IRQ_SEC_SGI_0		8
#define QEMU_IRQ_SEC_SGI_1		9
#define QEMU_IRQ_SEC_SGI_2		10
#define QEMU_IRQ_SEC_SGI_3		11
#define QEMU_IRQ_SEC_SGI_4		12
#define QEMU_IRQ_SEC_SGI_5		13
#define QEMU_IRQ_SEC_SGI_6		14
#define QEMU_IRQ_SEC_SGI_7		15

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
#define PLATFORM_G1S_PROPS(grp)						\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,	\
					   grp, GIC_INTR_CFG_EDGE)

#define PLATFORM_G0_PROPS(grp)

/*
 * DT related constants
 */
#define PLAT_QEMU_DT_BASE		NS_DRAM0_BASE
#define PLAT_QEMU_DT_MAX_SIZE		0x100000

/*
 * System counter
 */
#define SYS_COUNTER_FREQ_IN_TICKS	((1000 * 1000 * 1000) / 16)

#if SPM_MM
#define PLAT_QEMU_SP_IMAGE_BASE		BL_RAM_BASE
#define PLAT_QEMU_SP_IMAGE_SIZE		ULL(0x300000)

#ifdef IMAGE_BL2
/* In BL2 all memory allocated to the SPM Payload image is marked as RW. */
# define QEMU_SP_IMAGE_MMAP		MAP_REGION_FLAT( \
						PLAT_QEMU_SP_IMAGE_BASE, \
						PLAT_QEMU_SP_IMAGE_SIZE, \
						MT_MEMORY | MT_RW | \
						MT_SECURE)
#elif IMAGE_BL31
/* All SPM Payload memory is marked as code in S-EL0 */
# define QEMU_SP_IMAGE_MMAP		MAP_REGION2(PLAT_QEMU_SP_IMAGE_BASE, \
						PLAT_QEMU_SP_IMAGE_BASE, \
						PLAT_QEMU_SP_IMAGE_SIZE, \
						MT_CODE | MT_SECURE | \
						MT_USER,		\
						PAGE_SIZE)
#endif

/*
 * EL3 -> S-EL0 secure shared memory
 */
#define PLAT_SPM_BUF_PCPU_SIZE		ULL(0x10000)
#define PLAT_SPM_BUF_SIZE		(PLATFORM_CORE_COUNT * \
					PLAT_SPM_BUF_PCPU_SIZE)
#define PLAT_SPM_BUF_BASE		(BL32_LIMIT - PLAT_SPM_BUF_SIZE)

#define QEMU_SPM_BUF_EL3_MMAP		MAP_REGION_FLAT(PLAT_SPM_BUF_BASE, \
						PLAT_SPM_BUF_SIZE, \
						MT_RW_DATA | MT_SECURE)

#define QEMU_SPM_BUF_EL0_MMAP		MAP_REGION2(PLAT_SPM_BUF_BASE,	\
						PLAT_SPM_BUF_BASE,	\
						PLAT_SPM_BUF_SIZE,	\
						MT_RO_DATA | MT_SECURE | \
						MT_USER,		\
						PAGE_SIZE)

/*
 * Shared memory between Normal world and S-EL0 for
 * passing data during service requests. It will be marked as RW and NS.
 */
#define PLAT_QEMU_SP_IMAGE_NS_BUF_BASE	(PLAT_QEMU_DT_BASE +		\
						PLAT_QEMU_DT_MAX_SIZE)
#define PLAT_QEMU_SP_IMAGE_NS_BUF_SIZE	ULL(0x10000)
#define QEMU_SP_IMAGE_NS_BUF_MMAP	MAP_REGION2( \
					PLAT_QEMU_SP_IMAGE_NS_BUF_BASE, \
					PLAT_QEMU_SP_IMAGE_NS_BUF_BASE, \
					PLAT_QEMU_SP_IMAGE_NS_BUF_SIZE, \
					MT_RW_DATA | MT_NS | \
					MT_USER, \
					PAGE_SIZE)

#define PLAT_SP_IMAGE_NS_BUF_BASE	PLAT_QEMU_SP_IMAGE_NS_BUF_BASE
#define PLAT_SP_IMAGE_NS_BUF_SIZE	PLAT_QEMU_SP_IMAGE_NS_BUF_SIZE

#define PLAT_QEMU_SP_IMAGE_HEAP_BASE	(PLAT_QEMU_SP_IMAGE_BASE + \
					PLAT_QEMU_SP_IMAGE_SIZE)
#define PLAT_QEMU_SP_IMAGE_HEAP_SIZE	ULL(0x800000)

#define PLAT_SP_IMAGE_STACK_BASE	(PLAT_QEMU_SP_IMAGE_HEAP_BASE + \
						PLAT_QEMU_SP_IMAGE_HEAP_SIZE)
#define PLAT_SP_IMAGE_STACK_PCPU_SIZE	ULL(0x10000)
#define QEMU_SP_IMAGE_STACK_TOTAL_SIZE	(PLATFORM_CORE_COUNT * \
						PLAT_SP_IMAGE_STACK_PCPU_SIZE)

#define QEMU_SP_IMAGE_RW_MMAP		MAP_REGION2( \
					PLAT_QEMU_SP_IMAGE_HEAP_BASE, \
					PLAT_QEMU_SP_IMAGE_HEAP_BASE, \
					(QEMU_SP_IMAGE_STACK_TOTAL_SIZE + \
					PLAT_QEMU_SP_IMAGE_HEAP_SIZE), \
					MT_RW_DATA | MT_SECURE | \
					MT_USER, \
					PAGE_SIZE)

/* Total number of memory regions with distinct properties */
#define PLAT_QEMU_SP_IMAGE_NUM_MEM_REGIONS	6

/*
 * Name of the section to put the translation tables used by the S-EL1/S-EL0
 * context of a Secure Partition.
 */
#define PLAT_SP_IMAGE_XLAT_SECTION_NAME		"qemu_sp_xlat_table"
#define PLAT_SP_IMAGE_BASE_XLAT_SECTION_NAME	"qemu_sp_xlat_table"

/* Cookies passed to the Secure Partition at boot. Not used by QEMU platforms.*/
#define PLAT_SPM_COOKIE_0		ULL(0)
#define PLAT_SPM_COOKIE_1		ULL(0)
#endif

#define QEMU_PRI_BITS		2
#define PLAT_SP_PRI		0x20

#endif /* PLATFORM_DEF_H */
