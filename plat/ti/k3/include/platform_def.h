/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <board_def.h>
#include <common_def.h>

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stack */
#if IMAGE_BL31
#define PLATFORM_STACK_SIZE		0x800
#else
#define PLATFORM_STACK_SIZE		0x1000
#endif

#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1

/*******************************************************************************
 * Memory layout constants
 ******************************************************************************/

/*
 * ARM-TF lives in SRAM, partition it here
 */

#define SHARED_RAM_BASE			BL31_LIMIT
#define SHARED_RAM_SIZE			0x00001000

/*
 * BL3-1 specific defines.
 *
 * Put BL3-1 at the base of the Trusted SRAM, before SHARED_RAM.
 */
#define BL31_BASE			SEC_SRAM_BASE
#define BL31_SIZE			(SEC_SRAM_SIZE - SHARED_RAM_SIZE)
#define BL31_LIMIT			(BL31_BASE + BL31_SIZE)
#define BL31_PROGBITS_LIMIT		BL31_LIMIT

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#endif /* __PLATFORM_DEF_H__ */
