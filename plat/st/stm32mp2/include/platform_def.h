/*
 * Copyright (c) 2023, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
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

#define PLAT_MAX_PWR_LVL		U(5)
#define PLAT_MAX_CPU_SUSPEND_PWR_LVL	U(5)
#define PLAT_NUM_PWR_DOMAINS		U(7)

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

/*******************************************************************************
 * BL33 specific defines.
 ******************************************************************************/
#define BL33_BASE			STM32MP_BL33_BASE

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

#endif /* PLATFORM_DEF_H */
