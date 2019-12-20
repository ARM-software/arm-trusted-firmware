/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_SPM_DEF_H
#define ARM_SPM_DEF_H

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

/*
 * If BL31 is placed in DRAM, place the Secure Partition in DRAM right after the
 * region used by BL31. If BL31 it is placed in SRAM, put the Secure Partition
 * at the base of DRAM.
 */
#define ARM_SP_IMAGE_BASE		BL32_BASE
#define ARM_SP_IMAGE_LIMIT		BL32_LIMIT
/* The maximum size of the S-EL0 payload can be 3MB */
#define ARM_SP_IMAGE_SIZE		ULL(0x300000)

#ifdef IMAGE_BL2
/* SPM Payload memory. Mapped as RW in BL2. */
#define ARM_SP_IMAGE_MMAP		MAP_REGION_FLAT(			\
						ARM_SP_IMAGE_BASE,		\
						ARM_SP_IMAGE_SIZE,		\
						MT_MEMORY | MT_RW | MT_SECURE)
#endif

#ifdef IMAGE_BL31
/* SPM Payload memory. Mapped as code in S-EL1 */
#define ARM_SP_IMAGE_MMAP		MAP_REGION2(				\
						ARM_SP_IMAGE_BASE,		\
						ARM_SP_IMAGE_BASE,		\
						ARM_SP_IMAGE_SIZE,		\
						MT_CODE | MT_SECURE | MT_USER,	\
						PAGE_SIZE)
#endif

/*
 * Memory shared between EL3 and S-EL0. It is used by EL3 to push data into
 * S-EL0, so it is mapped with RW permission from EL3 and with RO permission
 * from S-EL0. Placed after SPM Payload memory.
 */
#define PLAT_SPM_BUF_BASE		(ARM_SP_IMAGE_BASE + ARM_SP_IMAGE_SIZE)
#define PLAT_SPM_BUF_SIZE		ULL(0x100000)

#define ARM_SPM_BUF_EL3_MMAP		MAP_REGION_FLAT(			\
						PLAT_SPM_BUF_BASE,		\
						PLAT_SPM_BUF_SIZE,		\
						MT_RW_DATA | MT_SECURE)
#define ARM_SPM_BUF_EL0_MMAP		MAP_REGION2(			\
						PLAT_SPM_BUF_BASE,		\
						PLAT_SPM_BUF_BASE,		\
						PLAT_SPM_BUF_SIZE,		\
						MT_RO_DATA | MT_SECURE | MT_USER,\
						PAGE_SIZE)

/*
 * Memory shared between Normal world and S-EL0 for passing data during service
 * requests. Mapped as RW and NS. Placed after the shared memory between EL3 and
 * S-EL0.
 */
#define PLAT_SP_IMAGE_NS_BUF_BASE	(PLAT_SPM_BUF_BASE + PLAT_SPM_BUF_SIZE)
#define PLAT_SP_IMAGE_NS_BUF_SIZE	ULL(0x10000)
#define ARM_SP_IMAGE_NS_BUF_MMAP	MAP_REGION2(				\
						PLAT_SP_IMAGE_NS_BUF_BASE,	\
						PLAT_SP_IMAGE_NS_BUF_BASE,	\
						PLAT_SP_IMAGE_NS_BUF_SIZE,	\
						MT_RW_DATA | MT_NS | MT_USER,	\
						PAGE_SIZE)

/*
 * RW memory, which uses the remaining Trusted DRAM. Placed after the memory
 * shared between Secure and Non-secure worlds, or after the platform specific
 * buffers, if defined. First there is the stack memory for all CPUs and then
 * there is the common heap memory. Both are mapped with RW permissions.
 */
#define PLAT_SP_IMAGE_STACK_BASE	PLAT_ARM_SP_IMAGE_STACK_BASE
#define PLAT_SP_IMAGE_STACK_PCPU_SIZE	ULL(0x2000)
#define ARM_SP_IMAGE_STACK_TOTAL_SIZE	(PLATFORM_CORE_COUNT *			\
					 PLAT_SP_IMAGE_STACK_PCPU_SIZE)

#define ARM_SP_IMAGE_HEAP_BASE		(PLAT_SP_IMAGE_STACK_BASE +		\
					 ARM_SP_IMAGE_STACK_TOTAL_SIZE)
#define ARM_SP_IMAGE_HEAP_SIZE		(ARM_SP_IMAGE_LIMIT - ARM_SP_IMAGE_HEAP_BASE)

#define ARM_SP_IMAGE_RW_MMAP		MAP_REGION2(				\
						PLAT_SP_IMAGE_STACK_BASE,	\
						PLAT_SP_IMAGE_STACK_BASE,	\
						(ARM_SP_IMAGE_LIMIT -		\
						 PLAT_SP_IMAGE_STACK_BASE),	\
						MT_RW_DATA | MT_SECURE | MT_USER,\
						PAGE_SIZE)

/* Total number of memory regions with distinct properties */
#define ARM_SP_IMAGE_NUM_MEM_REGIONS	6

/* Cookies passed to the Secure Partition at boot. Not used by ARM platforms. */
#define PLAT_SPM_COOKIE_0		ULL(0)
#define PLAT_SPM_COOKIE_1		ULL(0)

#endif /* ARM_SPM_DEF_H */
