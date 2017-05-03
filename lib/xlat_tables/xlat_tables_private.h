/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_PRIVATE_H__
#define __XLAT_TABLES_PRIVATE_H__

#include <cassert.h>
#include <platform_def.h>
#include <utils_def.h>

/*
 * If the platform hasn't defined a physical and a virtual address space size
 * default to ADDR_SPACE_SIZE.
 */
#if ERROR_DEPRECATED
# ifdef ADDR_SPACE_SIZE
#  error "ADDR_SPACE_SIZE is deprecated. Use PLAT_xxx_ADDR_SPACE_SIZE instead."
# endif
#elif defined(ADDR_SPACE_SIZE)
# ifndef PLAT_PHY_ADDR_SPACE_SIZE
#  define PLAT_PHY_ADDR_SPACE_SIZE	ADDR_SPACE_SIZE
# endif
# ifndef PLAT_VIRT_ADDR_SPACE_SIZE
#  define PLAT_VIRT_ADDR_SPACE_SIZE	ADDR_SPACE_SIZE
# endif
#endif

/* The virtual and physical address space sizes must be powers of two. */
CASSERT(IS_POWER_OF_TWO(PLAT_VIRT_ADDR_SPACE_SIZE),
	assert_valid_virt_addr_space_size);
CASSERT(IS_POWER_OF_TWO(PLAT_PHY_ADDR_SPACE_SIZE),
	assert_valid_phy_addr_space_size);

/*
 * In AArch32 state, the MMU only supports 4KB page granularity, which means
 * that the first translation table level is either 1 or 2. Both of them are
 * allowed to have block and table descriptors. See section G4.5.6 of the
 * ARMv8-A Architecture Reference Manual (DDI 0487A.k) for more information.
 *
 * In AArch64 state, the MMU may support 4 KB, 16 KB and 64 KB page
 * granularity. For 4KB granularity, a level 0 table descriptor doesn't support
 * block translation. For 16KB, the same thing happens to levels 0 and 1. For
 * 64KB, same for level 1. See section D4.3.1 of the ARMv8-A Architecture
 * Reference Manual (DDI 0487A.k) for more information.
 *
 * The define below specifies the first table level that allows block
 * descriptors.
 */

#ifdef AARCH32

# define XLAT_BLOCK_LEVEL_MIN 1

#else /* if AArch64 */

# if PAGE_SIZE == (4*1024) /* 4KB */
#  define XLAT_BLOCK_LEVEL_MIN 1
# else /* 16KB or 64KB */
#  define XLAT_BLOCK_LEVEL_MIN 2
# endif

#endif /* AARCH32 */

void print_mmap(void);
void init_xlation_table(uintptr_t base_va, uint64_t *table,
			int level, uintptr_t *max_va,
			unsigned long long *max_pa);

#endif /* __XLAT_TABLES_PRIVATE_H__ */
