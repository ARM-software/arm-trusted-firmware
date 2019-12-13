/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <drivers/arm/gic_common.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#include "../stm32mp1_def.h"

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#if defined(IMAGE_BL32)
#define PLATFORM_STACK_SIZE		0x600
#else
#define PLATFORM_STACK_SIZE		0xC00
#endif

#ifdef AARCH32_SP_OPTEE
#define OPTEE_HEADER_IMAGE_NAME		"teeh"
#define OPTEE_PAGED_IMAGE_NAME		"teed"
#define OPTEE_PAGER_IMAGE_NAME		"teex"
#define OPTEE_HEADER_BINARY_TYPE	U(0x20)
#define OPTEE_PAGER_BINARY_TYPE		U(0x21)
#define OPTEE_PAGED_BINARY_TYPE		U(0x22)
#endif

/* SSBL = second stage boot loader */
#define BL33_IMAGE_NAME			"ssbl"
#define BL33_BINARY_TYPE		U(0x0)

#define STM32MP_PRIMARY_CPU		U(0x0)
#define STM32MP_SECONDARY_CPU		U(0x1)

#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT	U(2)
#define PLATFORM_CLUSTER1_CORE_COUNT	U(0)
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER1_CORE_COUNT + \
					 PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	2

#define MAX_IO_DEVICES			U(4)
#define MAX_IO_HANDLES			U(4)
#define MAX_IO_BLOCK_DEVICES		U(1)
#define MAX_IO_MTD_DEVICES		U(1)

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
/*
 * Put BL2 just below BL3-1. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth.
 */
#define BL2_BASE			STM32MP_BL2_BASE
#define BL2_LIMIT			(STM32MP_BL2_BASE + \
					 STM32MP_BL2_SIZE)

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
#ifndef AARCH32_SP_OPTEE
#define BL32_BASE			STM32MP_BL32_BASE
#define BL32_LIMIT			(STM32MP_BL32_BASE + \
					 STM32MP_BL32_SIZE)
#endif

/*******************************************************************************
 * BL33 specific defines.
 ******************************************************************************/
#define BL33_BASE			STM32MP_BL33_BASE

/*
 * Load address of BL33 for this platform port
 */
#define PLAT_STM32MP_NS_IMAGE_OFFSET	BL33_BASE

/*******************************************************************************
 * DTB specific defines.
 ******************************************************************************/
#define DTB_BASE			STM32MP_DTB_BASE
#define DTB_LIMIT			(STM32MP_DTB_BASE + \
					 STM32MP_DTB_SIZE)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 32)

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

/*
 * Secure Interrupt: based on the standard ARM mapping
 */
#define ARM_IRQ_SEC_PHY_TIMER		U(29)

#define ARM_IRQ_SEC_SGI_0		U(8)
#define ARM_IRQ_SEC_SGI_1		U(9)
#define ARM_IRQ_SEC_SGI_2		U(10)
#define ARM_IRQ_SEC_SGI_3		U(11)
#define ARM_IRQ_SEC_SGI_4		U(12)
#define ARM_IRQ_SEC_SGI_5		U(13)
#define ARM_IRQ_SEC_SGI_6		U(14)
#define ARM_IRQ_SEC_SGI_7		U(15)

#define STM32MP1_IRQ_TZC400		U(36)
#define STM32MP1_IRQ_TAMPSERRS		U(229)
#define STM32MP1_IRQ_AXIERRIRQ		U(244)

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLATFORM_G1S_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(STM32MP1_IRQ_AXIERRIRQ,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(STM32MP1_IRQ_TZC400,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_EDGE),		\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_EDGE),		\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_EDGE),		\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_EDGE),		\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_EDGE),		\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_EDGE)

#define PLATFORM_G0_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_EDGE),		\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       grp, GIC_INTR_CFG_EDGE)

/*
 * Power
 */
#define PLAT_MAX_PWR_LVL	U(1)

/* Local power state for power domains in Run state. */
#define ARM_LOCAL_STATE_RUN	U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define ARM_LOCAL_STATE_RET	U(1)
/* Local power state for power-down. Valid for CPU and cluster power domains */
#define ARM_LOCAL_STATE_OFF	U(2)
/*
 * This macro defines the deepest retention state possible.
 * A higher state id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		ARM_LOCAL_STATE_RET
/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		ARM_LOCAL_STATE_OFF

/*******************************************************************************
 * Size of the per-cpu data in bytes that should be reserved in the generic
 * per-cpu data structure for the FVP port.
 ******************************************************************************/
#define PLAT_PCPU_DATA_SIZE	2

#endif /* PLATFORM_DEF_H */
