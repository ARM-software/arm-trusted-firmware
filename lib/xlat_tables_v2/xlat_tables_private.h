/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_PRIVATE_H__
#define __XLAT_TABLES_PRIVATE_H__

#include <platform_def.h>
#include <xlat_tables_defs.h>

#if PLAT_XLAT_TABLES_DYNAMIC
/*
 * Shifts and masks to access fields of an mmap_attr_t
 */
/* Dynamic or static */
#define MT_DYN_SHIFT		30 /* 31 would cause undefined behaviours */

/*
 * Memory mapping private attributes
 *
 * Private attributes not exposed in the mmap_attr_t enum.
 */
typedef enum  {
	/*
	 * Regions mapped before the MMU can't be unmapped dynamically (they are
	 * static) and regions mapped with MMU enabled can be unmapped. This
	 * behaviour can't be overridden.
	 *
	 * Static regions can overlap each other, dynamic regions can't.
	 */
	MT_STATIC	= 0 << MT_DYN_SHIFT,
	MT_DYNAMIC	= 1 << MT_DYN_SHIFT
} mmap_priv_attr_t;

/*
 * Function used to invalidate all levels of the translation walk for a given
 * virtual address. It must be called for every translation table entry that is
 * modified.
 */
void xlat_arch_tlbi_va(uintptr_t va);

/*
 * This function has to be called at the end of any code that uses the function
 * xlat_arch_tlbi_va().
 */
void xlat_arch_tlbi_va_sync(void);

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

/* Print VA, PA, size and attributes of all regions in the mmap array. */
void print_mmap(mmap_region_t *const mmap);

/*
 * Print the current state of the translation tables by reading them from
 * memory.
 */
void xlat_tables_print(xlat_ctx_t *ctx);

/*
 * Architecture-specific initialization code.
 */

/* Returns the current Exception Level. The returned EL must be 1 or higher. */
int xlat_arch_current_el(void);

/*
 * Returns the bit mask that has to be ORed to the rest of a translation table
 * descriptor so that execution of code is prohibited at the given Exception
 * Level.
 */
uint64_t xlat_arch_get_xn_desc(int el);

/* Execute architecture-specific translation table initialization code. */
void init_xlat_tables_arch(unsigned long long max_pa);

/* Enable MMU and configure it to use the specified translation tables. */
void enable_mmu_arch(unsigned int flags, uint64_t *base_table);

/* Return 1 if the MMU of this Exception Level is enabled, 0 otherwise. */
int is_mmu_enabled(void);

#endif /* __XLAT_TABLES_PRIVATE_H__ */
