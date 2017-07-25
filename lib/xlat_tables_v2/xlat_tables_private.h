/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
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
#  define PLAT_PHY_ADDR_SPACE_SIZE	ULL(ADDR_SPACE_SIZE)
# endif
# ifndef PLAT_VIRT_ADDR_SPACE_SIZE
#  define PLAT_VIRT_ADDR_SPACE_SIZE	ULL(ADDR_SPACE_SIZE)
# endif
#endif

/* The virtual and physical address space sizes must be powers of two. */
CASSERT(IS_POWER_OF_TWO(PLAT_VIRT_ADDR_SPACE_SIZE),
	assert_valid_virt_addr_space_size);
CASSERT(IS_POWER_OF_TWO(PLAT_PHY_ADDR_SPACE_SIZE),
	assert_valid_phy_addr_space_size);

/* Struct that holds all information about the translation tables. */
typedef struct {

	/*
	 * Max allowed Virtual and Physical Addresses.
	 */
	unsigned long long pa_max_address;
	uintptr_t va_max_address;

	/*
	 * Array of all memory regions stored in order of ascending end address
	 * and ascending size to simplify the code that allows overlapping
	 * regions. The list is terminated by the first entry with size == 0.
	 * The max size of the list is stored in `mmap_num`. `mmap` points to an
	 * array of mmap_num + 1 elements, so that there is space for the final
	 * null entry.
	 */
	mmap_region_t *mmap;
	unsigned int mmap_num;

	/*
	 * Array of finer-grain translation tables.
	 * For example, if the initial lookup level is 1 then this array would
	 * contain both level-2 and level-3 entries.
	 */
	uint64_t (*tables)[XLAT_TABLE_ENTRIES];
	unsigned int tables_num;
	/*
	 * Keep track of how many regions are mapped in each table. The base
	 * table can't be unmapped so it isn't needed to keep track of it.
	 */
#if PLAT_XLAT_TABLES_DYNAMIC
	int *tables_mapped_regions;
#endif /* PLAT_XLAT_TABLES_DYNAMIC */

	unsigned int next_table;

	/*
	 * Base translation table. It doesn't need to have the same amount of
	 * entries as the ones used for other levels.
	 */
	uint64_t *base_table;
	unsigned int base_table_entries;

	/*
	 * Max Physical and Virtual addresses currently in use by the
	 * translation tables. These might get updated as we map/unmap memory
	 * regions but they will never go beyond pa/va_max_address.
	 */
	unsigned long long max_pa;
	uintptr_t max_va;

	/* Level of the base translation table. */
	unsigned int base_level;

	/* Set to 1 when the translation tables are initialized. */
	unsigned int initialized;

	/*
	 * Bit mask that has to be ORed to the rest of a translation table
	 * descriptor in order to prohibit execution of code at the exception
	 * level of this translation context.
	 */
	uint64_t execute_never_mask;

} xlat_ctx_t;

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

/* Add a dynamic region to the specified context. */
int mmap_add_dynamic_region_ctx(xlat_ctx_t *ctx, mmap_region_t *mm);

/* Remove a dynamic region from the specified context. */
int mmap_remove_dynamic_region_ctx(xlat_ctx_t *ctx, uintptr_t base_va,
			size_t size);

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

/* Print VA, PA, size and attributes of all regions in the mmap array. */
void print_mmap(mmap_region_t *const mmap);

/*
 * Print the current state of the translation tables by reading them from
 * memory.
 */
void xlat_tables_print(xlat_ctx_t *ctx);

/*
 * Initialize the translation tables by mapping all regions added to the
 * specified context.
 */
void init_xlation_table(xlat_ctx_t *ctx);

/* Add a static region to the specified context. */
void mmap_add_region_ctx(xlat_ctx_t *ctx, mmap_region_t *mm);

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
