/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define QEMU_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

#define PLATFORM_STACK_SIZE 0x1000

#if ARM_ARCH_MAJOR == 7
#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(4)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CLUSTER1_CORE_COUNT	U(0)
#else
/*
 * Define the number of cores per cluster used in calculating core position.
 * The cluster number is shifted by this value and added to the core ID,
 * so its value represents log2(cores/cluster).
 * Default is 2**(4) = 16 cores per cluster.
 */
#define PLATFORM_CPU_PER_CLUSTER_SHIFT	U(4)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	(U(1) << PLATFORM_CPU_PER_CLUSTER_SHIFT)
#define PLATFORM_CLUSTER_COUNT		U(2)
#define PLATFORM_CLUSTER0_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CLUSTER1_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#endif
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER0_CORE_COUNT + \
					 PLATFORM_CLUSTER1_CORE_COUNT)

#define QEMU_PRIMARY_CPU		U(0)

#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1

#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN		U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET		U(1)
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

#define NS_DRAM0_BASE			ULL(0x40000000)
#define NS_DRAM0_SIZE			ULL(0xc0000000)

#define SEC_SRAM_BASE			0x0e000000
#define SEC_SRAM_SIZE			0x00100000

#define SEC_DRAM_BASE			0x0e100000
#define SEC_DRAM_SIZE			0x00f00000

#define SECURE_GPIO_BASE		0x090b0000
#define SECURE_GPIO_SIZE		0x00001000
#define SECURE_GPIO_POWEROFF		0
#define SECURE_GPIO_RESET		1

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

#define TB_FW_CONFIG_BASE		BL_RAM_BASE
#define TB_FW_CONFIG_LIMIT		(TB_FW_CONFIG_BASE + PAGE_SIZE)
#define TOS_FW_CONFIG_BASE		TB_FW_CONFIG_LIMIT
#define TOS_FW_CONFIG_LIMIT		(TOS_FW_CONFIG_BASE + PAGE_SIZE)

/*
 * BL1 specific defines.
 *
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 * Put BL1 RW at the top of the Secure SRAM. BL1_RW_BASE is calculated using
 * the current BL1 RW debug size plus a little space for growth.
 */
#define BL1_RO_BASE			SEC_ROM_BASE
#define BL1_RO_LIMIT			(SEC_ROM_BASE + SEC_ROM_SIZE)
#define BL1_RW_BASE			(BL1_RW_LIMIT - 0x12000)
#define BL1_RW_LIMIT			(BL_RAM_BASE + BL_RAM_SIZE)

/*
 * BL2 specific defines.
 *
 * Put BL2 just below BL3-1. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth.
 */
#define BL2_BASE			(BL31_BASE - 0x35000)
#define BL2_LIMIT			BL31_BASE

/*
 * BL3-1 specific defines.
 *
 * Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth.
 */
#define BL31_BASE			(BL31_LIMIT - 0x60000)
#define BL31_LIMIT			(BL_RAM_BASE + BL_RAM_SIZE - FW_HANDOFF_SIZE)
#define BL31_PROGBITS_LIMIT		BL1_RW_BASE

#if TRANSFER_LIST
#define FW_HANDOFF_BASE			BL31_LIMIT
#define FW_HANDOFF_LIMIT		(FW_HANDOFF_BASE + FW_HANDOFF_SIZE)
#define FW_HANDOFF_SIZE			0x4000
#else
#define FW_HANDOFF_SIZE			0
#endif


/*
 * BL3-2 specific defines.
 *
 * BL3-2 can execute from Secure SRAM, or Secure DRAM.
 */
#define BL32_SRAM_BASE			BL_RAM_BASE
#define BL32_SRAM_LIMIT			BL31_BASE
#define BL32_DRAM_BASE			SEC_DRAM_BASE
#define BL32_DRAM_LIMIT			(SEC_DRAM_BASE + SEC_DRAM_SIZE - \
					 RME_GPT_DRAM_SIZE)

#define SEC_SRAM_ID			0
#define SEC_DRAM_ID			1

#if BL32_RAM_LOCATION_ID == SEC_SRAM_ID
# define BL32_MEM_BASE			BL_RAM_BASE
# define BL32_MEM_SIZE			BL_RAM_SIZE
# define BL32_BASE			BL32_SRAM_BASE
# define BL32_LIMIT			(BL32_SRAM_LIMIT - FW_HANDOFF_SIZE)
#elif BL32_RAM_LOCATION_ID == SEC_DRAM_ID
# define BL32_MEM_BASE			SEC_DRAM_BASE
# define BL32_MEM_SIZE			SEC_DRAM_SIZE
# define BL32_BASE			BL32_DRAM_BASE
# define BL32_LIMIT			(BL32_DRAM_LIMIT - FW_HANDOFF_SIZE)
#else
# error "Unsupported BL32_RAM_LOCATION_ID value"
#endif

#if TRANSFER_LIST
#define FW_NS_HANDOFF_BASE		(NS_IMAGE_OFFSET - FW_HANDOFF_SIZE)
#endif

#define NS_IMAGE_OFFSET			(NS_DRAM0_BASE + 0x20000000)
#define NS_IMAGE_MAX_SIZE		(NS_DRAM0_SIZE - 0x20000000)

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_MMAP_REGIONS		(13 + MAX_MMAP_REGIONS_SPMC)
#define MAX_XLAT_TABLES			(6 + MAX_XLAT_TABLES_SPMC)
#define MAX_IO_DEVICES			4
#define MAX_IO_HANDLES			4

/*
 * PL011 related constants
 */
#define UART0_BASE			0x09000000
#define UART1_BASE			0x09040000
#define UART0_CLK_IN_HZ			1
#define UART1_CLK_IN_HZ			1

#define PLAT_QEMU_BOOT_UART_BASE	UART0_BASE
#define PLAT_QEMU_BOOT_UART_CLK_IN_HZ	UART0_CLK_IN_HZ

#define PLAT_QEMU_CRASH_UART_BASE	UART1_BASE
#define PLAT_QEMU_CRASH_UART_CLK_IN_HZ	UART1_CLK_IN_HZ

#define PLAT_QEMU_CONSOLE_BAUDRATE	115200

#define QEMU_FLASH0_BASE		0x00000000
#define QEMU_FLASH0_SIZE		0x04000000
#define QEMU_FLASH1_BASE		0x04000000
#define QEMU_FLASH1_SIZE		0x04000000

#define PLAT_QEMU_FIP_BASE		0x00040000
#define PLAT_QEMU_FIP_MAX_SIZE		(QEMU_FLASH0_SIZE - PLAT_QEMU_FIP_BASE)

#define DEVICE0_BASE			0x08000000
#define DEVICE0_SIZE			0x01000000
#define DEVICE1_BASE			0x09000000
#define DEVICE1_SIZE			0x00c00000

/*
 * GIC related constants
 */

#define GICD_BASE			0x8000000
#define GICC_BASE			0x8010000
#define GICR_BASE			0x80A0000


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
	DESC_G1S_IRQ_SEC_SGI_0(grp)					\
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

#if SDEI_SUPPORT
#define DESC_G0_IRQ_SEC_SGI(grp)					\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI, (grp), \
					   GIC_INTR_CFG_EDGE)
#define DESC_G1S_IRQ_SEC_SGI_0(grp)
#else
#define DESC_G0_IRQ_SEC_SGI(grp)
#define DESC_G1S_IRQ_SEC_SGI_0(grp)					\
	INTR_PROP_DESC(QEMU_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI, (grp),	\
					   GIC_INTR_CFG_EDGE),
#endif

#define PLATFORM_G0_PROPS(grp)		DESC_G0_IRQ_SEC_SGI(grp)

/*
 * DT related constants
 */
#define PLAT_QEMU_DT_BASE		NS_DRAM0_BASE
#define PLAT_QEMU_DT_MAX_SIZE		0x100000

/*
 * Platforms macros to support SDEI
 */
#define PLAT_PRI_BITS			U(3)
#define PLAT_SDEI_CRITICAL_PRI		0x60
#define PLAT_SDEI_NORMAL_PRI		0x70
#define PLAT_SDEI_SGI_PRIVATE		QEMU_IRQ_SEC_SGI_0

/*
 * Maximum size of Event Log buffer used in Measured Boot Event Log driver
 */
#define	PLAT_EVENT_LOG_MAX_SIZE		UL(0x400)

#if SPMC_AT_EL3
/*
 * Number of Secure Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of
 * supported secure partitions.
 */
#define SECURE_PARTITION_COUNT		1

/*
 * Number of Logical Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of
 * supported logical partitions.
 */
#define MAX_EL3_LP_DESCS_COUNT		0

/*
 * Number of Normal World Partitions supported.
 * SPMC at EL3, uses this count to configure the maximum number of
 * supported normal world partitions.
 */
#define NS_PARTITION_COUNT		1

#define MAX_MMAP_REGIONS_SPMC		2
#define MAX_XLAT_TABLES_SPMC		4
#else
#define MAX_MMAP_REGIONS_SPMC		0
#define MAX_XLAT_TABLES_SPMC		0
#endif

#if ENABLE_RME

/*
 * Reserve some space at the end of secure DRAM for the Granule Protection
 * Tables
 */
#define PLAT_QEMU_L0_GPT_BASE		(PLAT_QEMU_L1_GPT_BASE -	\
					 PLAT_QEMU_L0_GPT_SIZE)
#define PLAT_QEMU_L0_GPT_SIZE		(2 * PAGE_SIZE)

#define PLAT_QEMU_L1_GPT_BASE		(SEC_DRAM_BASE + SEC_DRAM_SIZE - \
					 PLAT_QEMU_L1_GPT_SIZE)
#define PLAT_QEMU_L1_GPT_END		(PLAT_QEMU_L1_GPT_BASE +	\
					 PLAT_QEMU_L1_GPT_SIZE - 1U)
#define PLAT_QEMU_L1_GPT_SIZE		UL(0x00100000)	/* 1MB */

#define RME_GPT_DRAM_BASE		PLAT_QEMU_L0_GPT_BASE
#define RME_GPT_DRAM_SIZE		(PLAT_QEMU_L1_GPT_SIZE +	\
					 PLAT_QEMU_L0_GPT_SIZE)

#ifndef __ASSEMBLER__
/* L0 table greater than 4KB must be naturally aligned */
CASSERT((PLAT_QEMU_L0_GPT_BASE & (PLAT_QEMU_L0_GPT_SIZE - 1)) == 0,
	assert_l0_gpt_naturally_aligned);
#endif

/* Reserved some DRAM space for RMM (24MB) */
#define REALM_DRAM_BASE			(NS_DRAM0_BASE + PLAT_QEMU_DT_MAX_SIZE)
#define REALM_DRAM_SIZE			0x01800000

#define PLAT_QEMU_RMM_SIZE		(REALM_DRAM_SIZE - RMM_SHARED_SIZE)
#define PLAT_QEMU_RMM_SHARED_SIZE	(PAGE_SIZE)	/* 4KB */

#define RMM_BASE			(REALM_DRAM_BASE)
#define RMM_LIMIT			(RMM_BASE + PLAT_QEMU_RMM_SIZE)
#define RMM_SHARED_BASE			(RMM_LIMIT)
#define RMM_SHARED_SIZE			PLAT_QEMU_RMM_SHARED_SIZE

#define MAP_GPT_L0_REGION	MAP_REGION_FLAT(			\
					PLAT_QEMU_L0_GPT_BASE,		\
					PLAT_QEMU_L0_GPT_SIZE,		\
					MT_MEMORY | MT_RW | EL3_PAS)

#define MAP_GPT_L1_REGION	MAP_REGION_FLAT(			\
					PLAT_QEMU_L1_GPT_BASE,		\
					PLAT_QEMU_L1_GPT_SIZE,		\
					MT_MEMORY | MT_RW | EL3_PAS)
/*
 * We add the RMM_SHARED size to RMM mapping to map the region as a block.
 * Else we end up requiring more pagetables in BL2 for ROMLIB build.
 */
#define MAP_RMM_DRAM		MAP_REGION_FLAT(			\
					RMM_BASE,			\
					(PLAT_QEMU_RMM_SIZE +		\
					 RMM_SHARED_SIZE),		\
					MT_MEMORY | MT_RW | MT_REALM)

#define MAP_RMM_SHARED_MEM	MAP_REGION_FLAT(			\
					RMM_SHARED_BASE,		\
					RMM_SHARED_SIZE,		\
					MT_MEMORY | MT_RW | MT_REALM)
#else /* !ENABLE_RME */

#define RME_GPT_DRAM_SIZE		0

#endif /* ENABLE_RME */

#endif /* PLATFORM_DEF_H */
