/*
 * Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_TABLES_H
#define XLAT_TABLES_H

#include <lib/xlat_tables/xlat_tables_defs.h>

#ifndef __ASSEMBLER__
#include <stddef.h>
#include <stdint.h>

#include <lib/xlat_tables/xlat_mmu_helpers.h>

/* Helper macro to define entries for mmap_region_t. It creates
 * identity mappings for each region.
 */
#define MAP_REGION_FLAT(adr, sz, attr) MAP_REGION(adr, adr, sz, attr)

/* Helper macro to define entries for mmap_region_t. It allows to
 * re-map address mappings from 'pa' to 'va' for each region.
 */
#define MAP_REGION(pa, va, sz, attr) {(pa), (va), (sz), (attr)}

/*
 * Shifts and masks to access fields of an mmap attribute
 */
#define MT_TYPE_MASK	U(0x7)
#define MT_TYPE(_attr)	((_attr) & MT_TYPE_MASK)
/* Access permissions (RO/RW) */
#define MT_PERM_SHIFT	U(3)
/* Security state (SECURE/NS) */
#define MT_SEC_SHIFT	U(4)
/* Access permissions for instruction execution (EXECUTE/EXECUTE_NEVER) */
#define MT_EXECUTE_SHIFT	U(5)

/*
 * Memory mapping attributes
 */

/*
 * Memory types supported.
 * These are organised so that, going down the list, the memory types are
 * getting weaker; conversely going up the list the memory types are getting
 * stronger.
 */
#define MT_DEVICE		U(0)
#define MT_NON_CACHEABLE	U(1)
#define MT_MEMORY		U(2)
/* Values up to 7 are reserved to add new memory types in the future */

#define MT_RO			(U(0) << MT_PERM_SHIFT)
#define MT_RW			(U(1) << MT_PERM_SHIFT)

#define MT_SECURE		(U(0) << MT_SEC_SHIFT)
#define MT_NS			(U(1) << MT_SEC_SHIFT)

/*
 * Access permissions for instruction execution are only relevant for normal
 * read-only memory, i.e. MT_MEMORY | MT_RO. They are ignored (and potentially
 * overridden) otherwise:
 *  - Device memory is always marked as execute-never.
 *  - Read-write normal memory is always marked as execute-never.
 */
#define MT_EXECUTE		(U(0) << MT_EXECUTE_SHIFT)
#define MT_EXECUTE_NEVER	(U(1) << MT_EXECUTE_SHIFT)

/* Compound attributes for most common usages */
#define MT_CODE			(MT_MEMORY | MT_RO | MT_EXECUTE)
#define MT_RO_DATA		(MT_MEMORY | MT_RO | MT_EXECUTE_NEVER)

/* Memory type for EL3 regions */
#if ENABLE_RME
#error FEAT_RME requires version 2 of the Translation Tables Library
#else
#define EL3_PAS			MT_SECURE
#endif

/*
 * Structure for specifying a single region of memory.
 */
typedef struct mmap_region {
	unsigned long long	base_pa;
	uintptr_t		base_va;
	size_t			size;
	unsigned int		attr;
} mmap_region_t;

/* Generic translation table APIs */
void init_xlat_tables(void);
void mmap_add_region(unsigned long long base_pa, uintptr_t base_va,
		     size_t size, unsigned int attr);
void mmap_add(const mmap_region_t *mm);

#endif /*__ASSEMBLER__*/
#endif /* XLAT_TABLES_H */
