/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <plat/common/common_def.h>

#include <board_def.h>

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stack */
#if IMAGE_BL31
#define PLATFORM_STACK_SIZE		0x800
#else
#define PLATFORM_STACK_SIZE		0x1000
#endif

#define PLATFORM_SYSTEM_COUNT		1
#define PLATFORM_CORE_COUNT		(K3_CLUSTER0_CORE_COUNT + \
					K3_CLUSTER1_CORE_COUNT + \
					K3_CLUSTER2_CORE_COUNT + \
					K3_CLUSTER3_CORE_COUNT)

#define PLATFORM_CLUSTER_COUNT		((K3_CLUSTER0_CORE_COUNT != 0) + \
					(K3_CLUSTER1_CORE_COUNT != 0) + \
					(K3_CLUSTER2_CORE_COUNT != 0) + \
					(K3_CLUSTER3_CORE_COUNT != 0))

#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_SYSTEM_COUNT + \
					PLATFORM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)

/*******************************************************************************
 * Memory layout constants
 ******************************************************************************/

/*
 * This RAM will be used for the bootloader including code, bss, and stacks.
 * It may need to be increased if BL31 grows in size.
 *
 * The link addresses are determined by BL31_BASE + offset.
 * When ENABLE_PIE is set, the TF images can be loaded anywhere, so
 * BL31_BASE is really arbitrary.
 *
 * When ENABLE_PIE is unset, BL31_BASE should be chosen so that
 * it matches to the physical address where BL31 is loaded, that is,
 * BL31_BASE should be the base address of the RAM region.
 *
 * Lets make things explicit by mapping BL31_BASE to 0x0 since ENABLE_PIE is
 * defined as default for our platform.
 */
#define BL31_BASE	UL(0x00000000) /* PIE remapped on fly */
#define BL31_SIZE	UL(0x00020000) /* 128k */
#define BL31_LIMIT	(BL31_BASE + BL31_SIZE)

/*
 * Defines the maximum number of translation tables that are allocated by the
 * translation table library code. To minimize the amount of runtime memory
 * used, choose the smallest value needed to map the required virtual addresses
 * for each BL stage.
 */
#define MAX_XLAT_TABLES		4

/*
 * Defines the maximum number of regions that are allocated by the translation
 * table library code. A region consists of physical base address, virtual base
 * address, size and attributes (Device/Memory, RO/RW, Secure/Non-Secure), as
 * defined in the `mmap_region_t` structure. The platform defines the regions
 * that should be mapped. Then, the translation table library will create the
 * corresponding tables and descriptors at runtime. To minimize the amount of
 * runtime memory used, choose the smallest value needed to register the
 * required regions for each BL stage.
 */
#if USE_COHERENT_MEM
#define MAX_MMAP_REGIONS	11
#else
#define MAX_MMAP_REGIONS	10
#endif

/*
 * Defines the total size of the address space in bytes. For example, for a 32
 * bit address space, this value should be `(1ull << 32)`.
 */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 32)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

/* Platform default console definitions */
#ifndef K3_USART_BASE
#define K3_USART_BASE			(0x02800000 + 0x10000 * K3_USART)
#endif

/* USART has a default size for address space */
#define K3_USART_SIZE 0x1000

#ifndef K3_USART_CLK_SPEED
#define K3_USART_CLK_SPEED 48000000
#endif

/* Crash console defaults */
#define CRASH_CONSOLE_BASE K3_USART_BASE
#define CRASH_CONSOLE_CLK K3_USART_CLK_SPEED
#define CRASH_CONSOLE_BAUD_RATE K3_USART_BAUD

/* Timer frequency */
#ifndef SYS_COUNTER_FREQ_IN_TICKS
#define SYS_COUNTER_FREQ_IN_TICKS 200000000
#endif

/* Interrupt numbers */
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
 * Define properties of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_ARM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

#define PLAT_ARM_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)


#define K3_GTC_BASE		0x00A90000
/* We just need 20 byte offset, but simpler to just remap the 64K page in */
#define K3_GTC_SIZE		0x10000
#define K3_GTC_CNTCR_OFFSET	0x00
#define K3_GTC_CNTCR_EN_MASK	0x01
#define K3_GTC_CNTCR_HDBG_MASK	0x02
#define K3_GTC_CNTFID0_OFFSET	0x20

#define K3_GIC_BASE	0x01800000
#define K3_GIC_SIZE	0x200000

#if !K3_SEC_PROXY_LITE
#define SEC_PROXY_DATA_BASE	0x32C00000
#define SEC_PROXY_DATA_SIZE	0x80000
#define SEC_PROXY_SCFG_BASE	0x32800000
#define SEC_PROXY_SCFG_SIZE	0x80000
#define SEC_PROXY_RT_BASE	0x32400000
#define SEC_PROXY_RT_SIZE	0x80000
#else
#define SEC_PROXY_DATA_BASE	0x4D000000
#define SEC_PROXY_DATA_SIZE	0x80000
#define SEC_PROXY_SCFG_BASE	0x4A400000
#define SEC_PROXY_SCFG_SIZE	0x80000
#define SEC_PROXY_RT_BASE	0x4A600000
#define SEC_PROXY_RT_SIZE	0x80000
#endif /* K3_SEC_PROXY_LITE */

#define SEC_PROXY_TIMEOUT_US		1000000
#define SEC_PROXY_MAX_MESSAGE_SIZE	56

#define TI_SCI_HOST_ID			10
#define TI_SCI_MAX_MESSAGE_SIZE		52

#endif /* PLATFORM_DEF_H */
