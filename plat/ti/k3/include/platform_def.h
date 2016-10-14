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

#define PLATFORM_SYSTEM_COUNT		1
#define PLATFORM_CORE_COUNT		(K3_CLUSTER0_CORE_COUNT + \
					K3_CLUSTER1_CORE_COUNT + \
					K3_CLUSTER2_CORE_COUNT + \
					K3_CLUSTER3_CORE_COUNT)

#define PLATFORM_CLUSTER_COUNT		((K3_CLUSTER0_MSMC_PORT != UNUSED) + \
					(K3_CLUSTER1_MSMC_PORT != UNUSED) + \
					(K3_CLUSTER2_MSMC_PORT != UNUSED) + \
					(K3_CLUSTER3_MSMC_PORT != UNUSED))

#define UNUSED				-1

#if !defined(K3_CLUSTER1_CORE_COUNT) || !defined(K3_CLUSTER1_MSMC_PORT)
#define K3_CLUSTER1_CORE_COUNT		0
#define K3_CLUSTER1_MSMC_PORT		UNUSED
#endif

#if !defined(K3_CLUSTER2_CORE_COUNT) || !defined(K3_CLUSTER2_MSMC_PORT)
#define K3_CLUSTER2_CORE_COUNT		0
#define K3_CLUSTER2_MSMC_PORT		UNUSED
#endif

#if !defined(K3_CLUSTER3_CORE_COUNT) || !defined(K3_CLUSTER3_MSMC_PORT)
#define K3_CLUSTER3_CORE_COUNT		0
#define K3_CLUSTER3_MSMC_PORT		UNUSED
#endif

#if K3_CLUSTER0_MSMC_PORT == UNUSED
#error "ARM cluster 0 must be used"
#endif

#if ((K3_CLUSTER1_MSMC_PORT == UNUSED) && (K3_CLUSTER1_CORE_COUNT != 0)) || \
    ((K3_CLUSTER2_MSMC_PORT == UNUSED) && (K3_CLUSTER2_CORE_COUNT != 0)) || \
    ((K3_CLUSTER3_MSMC_PORT == UNUSED) && (K3_CLUSTER3_CORE_COUNT != 0))
#error "Unused ports must have 0 ARM cores"
#endif

#define PLATFORM_CLUSTER_OFFSET		K3_CLUSTER0_MSMC_PORT

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
 * Defines the maximum number of translation tables that are allocated by the
 * translation table library code. To minimize the amount of runtime memory
 * used, choose the smallest value needed to map the required virtual addresses
 * for each BL stage.
 */
#define MAX_XLAT_TABLES		8

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
#define MAX_MMAP_REGIONS	8

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
#ifndef K3_USART_BASE_ADDRESS
#define K3_USART_BASE_ADDRESS 0x02800000
#endif

/* USART has a default size for address space */
#define K3_USART_SIZE 0x1000

#ifndef K3_USART_CLK_SPEED
#define K3_USART_CLK_SPEED 48000000
#endif

#ifndef K3_USART_BAUD
#define K3_USART_BAUD 115200
#endif

/* Crash console defaults */
#define CRASH_CONSOLE_BASE K3_USART_BASE_ADDRESS
#define CRASH_CONSOLE_CLK K3_USART_CLK_SPEED
#define CRASH_CONSOLE_BAUD_RATE K3_USART_BAUD

/* Timer frequency */
#ifndef SYS_COUNTER_FREQ_IN_TICKS
#define SYS_COUNTER_FREQ_IN_TICKS 200000000
#endif

#endif /* __PLATFORM_DEF_H__ */
