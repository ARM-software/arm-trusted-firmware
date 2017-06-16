/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_V2_H__
#define __XLAT_TABLES_V2_H__

#include <xlat_tables_defs.h>

#ifndef __ASSEMBLY__
#include <stddef.h>
#include <stdint.h>
#include <xlat_mmu_helpers.h>

/* Helper macro to define entries for mmap_region_t. It creates
 * identity mappings for each region.
 */
#define MAP_REGION_FLAT(adr, sz, attr) MAP_REGION(adr, adr, sz, attr)

/* Helper macro to define entries for mmap_region_t. It allows to
 * re-map address mappings from 'pa' to 'va' for each region.
 */
#define MAP_REGION(pa, va, sz, attr) {(pa), (va), (sz), (attr)}

/*
 * Shifts and masks to access fields of an mmap_attr_t
 */
#define MT_TYPE_MASK		U(0x7)
#define MT_TYPE(_attr)		((_attr) & MT_TYPE_MASK)
/* Access permissions (RO/RW) */
#define MT_PERM_SHIFT		U(3)
/* Security state (SECURE/NS) */
#define MT_SEC_SHIFT		U(4)
/* Access permissions for instruction execution (EXECUTE/EXECUTE_NEVER) */
#define MT_EXECUTE_SHIFT	U(5)
/* All other bits are reserved */

/*
 * Memory mapping attributes
 */
typedef enum  {
	/*
	 * Memory types supported.
	 * These are organised so that, going down the list, the memory types
	 * are getting weaker; conversely going up the list the memory types are
	 * getting stronger.
	 */
	MT_DEVICE,
	MT_NON_CACHEABLE,
	MT_MEMORY,
	/* Values up to 7 are reserved to add new memory types in the future */

	MT_RO		= U(0) << MT_PERM_SHIFT,
	MT_RW		= U(1) << MT_PERM_SHIFT,

	MT_SECURE	= U(0) << MT_SEC_SHIFT,
	MT_NS		= U(1) << MT_SEC_SHIFT,

	/*
	 * Access permissions for instruction execution are only relevant for
	 * normal read-only memory, i.e. MT_MEMORY | MT_RO. They are ignored
	 * (and potentially overridden) otherwise:
	 *  - Device memory is always marked as execute-never.
	 *  - Read-write normal memory is always marked as execute-never.
	 */
	MT_EXECUTE		= U(0) << MT_EXECUTE_SHIFT,
	MT_EXECUTE_NEVER	= U(1) << MT_EXECUTE_SHIFT,
} mmap_attr_t;

#define MT_CODE		(MT_MEMORY | MT_RO | MT_EXECUTE)
#define MT_RO_DATA	(MT_MEMORY | MT_RO | MT_EXECUTE_NEVER)

/*
 * Structure for specifying a single region of memory.
 */
typedef struct mmap_region {
	unsigned long long	base_pa;
	uintptr_t		base_va;
	size_t			size;
	mmap_attr_t		attr;
} mmap_region_t;

/* Generic translation table APIs */

/*
 * Initialize translation tables from the current list of mmap regions. Calling
 * this function marks the transition point after which static regions can no
 * longer be added.
 */
void init_xlat_tables(void);

/*
 * Add a static region with defined base PA and base VA. This function can only
 * be used before initializing the translation tables. The region cannot be
 * removed afterwards.
 */
void mmap_add_region(unsigned long long base_pa, uintptr_t base_va,
				size_t size, mmap_attr_t attr);

/*
 * Add a dynamic region with defined base PA and base VA. This type of region
 * can be added and removed even after the translation tables are initialized.
 *
 * Returns:
 *        0: Success.
 *   EINVAL: Invalid values were used as arguments.
 *   ERANGE: Memory limits were surpassed.
 *   ENOMEM: Not enough space in the mmap array or not enough free xlat tables.
 *    EPERM: It overlaps another region in an invalid way.
 */
int mmap_add_dynamic_region(unsigned long long base_pa, uintptr_t base_va,
				size_t size, mmap_attr_t attr);

/*
 * Add an array of static regions with defined base PA and base VA. This
 * function can only be used before initializing the translation tables. The
 * regions cannot be removed afterwards.
 */
void mmap_add(const mmap_region_t *mm);

/*
 * Remove a region with the specified base VA and size. Only dynamic regions can
 * be removed, and they can be removed even if the translation tables are
 * initialized.
 *
 * Returns:
 *        0: Success.
 *   EINVAL: The specified region wasn't found.
 *    EPERM: Trying to remove a static region.
 */
int mmap_remove_dynamic_region(uintptr_t base_va, size_t size);

#endif /*__ASSEMBLY__*/
#endif /* __XLAT_TABLES_V2_H__ */
