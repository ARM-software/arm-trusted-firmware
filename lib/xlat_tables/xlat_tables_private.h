/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_PRIVATE_H__
#define __XLAT_TABLES_PRIVATE_H__

#include <cassert.h>
#include <platform_def.h>
#include <xlat_tables_arch.h>

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

CASSERT(CHECK_VIRT_ADDR_SPACE_SIZE(PLAT_VIRT_ADDR_SPACE_SIZE),
	assert_valid_virt_addr_space_size);

CASSERT(CHECK_PHY_ADDR_SPACE_SIZE(PLAT_PHY_ADDR_SPACE_SIZE),
	assert_valid_phy_addr_space_size);

/* Alias to retain compatibility with the old #define name */
#define XLAT_BLOCK_LEVEL_MIN	MIN_LVL_BLOCK_DESC

void print_mmap(void);

/* Returns the current Exception Level. The returned EL must be 1 or higher. */
int xlat_arch_current_el(void);

/*
 * Returns the bit mask that has to be ORed to the rest of a translation table
 * descriptor so that execution of code is prohibited at the given Exception
 * Level.
 */
uint64_t xlat_arch_get_xn_desc(int el);

void init_xlation_table(uintptr_t base_va, uint64_t *table,
			unsigned int level, uintptr_t *max_va,
			unsigned long long *max_pa);

#endif /* __XLAT_TABLES_PRIVATE_H__ */
