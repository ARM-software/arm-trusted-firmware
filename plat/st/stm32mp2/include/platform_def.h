/*
 * Copyright (c) 2023-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <drivers/arm/gic_common.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#include "../stm32mp2_def.h"

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE		0xC00

#define STM32MP_PRIMARY_CPU		U(0x0)
#define STM32MP_SECONDARY_CPU		U(0x1)

#define MAX_IO_DEVICES			U(4)
#define MAX_IO_HANDLES			U(4)
#define MAX_IO_BLOCK_DEVICES		U(1)
#define MAX_IO_MTD_DEVICES		U(1)

#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CORE_COUNT		U(2)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(2)

#define PLAT_MAX_PWR_LVL		U(1)
#define PLAT_MIN_SUSPEND_PWR_LVL	U(2)
#define PLAT_NUM_PWR_DOMAINS		U(6)

/* Local power state for power domains in Run state. */
#define STM32MP_LOCAL_STATE_RUN		U(0)
/* Local power state for retention. */
#define STM32MP_LOCAL_STATE_RET		U(1)
#define STM32MP_LOCAL_STATE_LP		U(2)
#define PLAT_MAX_RET_STATE		STM32MP_LOCAL_STATE_LP
/* Local power state for OFF/power-down. */
#define STM32MP_LOCAL_STATE_OFF		U(3)
#define PLAT_MAX_OFF_STATE		STM32MP_LOCAL_STATE_OFF

/* Macros to parse the state information from State-ID (recommended encoding) */
#define PLAT_LOCAL_PSTATE_WIDTH		U(4)
#define PLAT_LOCAL_PSTATE_MASK		GENMASK(PLAT_LOCAL_PSTATE_WIDTH - 1U, 0)

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

#define BL2_RO_BASE			STM32MP_BL2_RO_BASE
#define BL2_RO_LIMIT			(STM32MP_BL2_RO_BASE + \
					 STM32MP_BL2_RO_SIZE)

#define BL2_RW_BASE			STM32MP_BL2_RW_BASE
#define BL2_RW_LIMIT			(STM32MP_BL2_RW_BASE + \
					 STM32MP_BL2_RW_SIZE)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
#if ENABLE_PIE
#define BL31_BASE			0
#else
#define BL31_BASE			STM32MP_SYSRAM_BASE
#endif

#define BL31_LIMIT			(BL31_BASE + (STM32MP_SYSRAM_SIZE / 2))

#define BL31_PROGBITS_LIMIT		(BL31_BASE + STM32MP_BL31_SIZE)

/*******************************************************************************
 * BL33 specific defines.
 ******************************************************************************/
#define BL33_BASE			STM32MP_BL33_BASE

#if STM32MP_DDR_FIP_IO_STORAGE
#define DWL_DDR_BUFFER_BASE		STM32MP_SYSRAM_BASE
#define DWL_DDR_BUFFER_SIZE		U(0x0000A000)
#endif

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 33)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 33)

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

#define ARM_IRQ_NON_SEC_SGI_0		U(0)

#define ARM_IRQ_SEC_SGI_0		U(8)
#define ARM_IRQ_SEC_SGI_1		U(9)
#define ARM_IRQ_SEC_SGI_2		U(10)
#define ARM_IRQ_SEC_SGI_3		U(11)
#define ARM_IRQ_SEC_SGI_4		U(12)
#define ARM_IRQ_SEC_SGI_5		U(13)
#define ARM_IRQ_SEC_SGI_6		U(14)
#define ARM_IRQ_SEC_SGI_7		U(15)

/* Platform IRQ Priority */
#define STM32MP_IRQ_SEC_SPI_PRIO	U(0x10)

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLATFORM_G1S_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_LEVEL),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_EDGE)

#define PLATFORM_G0_PROPS(grp) \
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_EDGE),	\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6,		\
		       GIC_HIGHEST_SEC_PRIORITY,	\
		       (grp), GIC_INTR_CFG_EDGE)

#endif /* PLATFORM_DEF_H */
