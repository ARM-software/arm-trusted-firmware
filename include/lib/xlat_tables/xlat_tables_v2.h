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
#include <xlat_tables_v2_helpers.h>

/*
 * Default granularity size for an mmap_region_t.
 * Useful when no specific granularity is required.
 *
 * By default, choose the biggest possible block size allowed by the
 * architectural state and granule size in order to minimize the number of page
 * tables required for the mapping.
 */
#define REGION_DEFAULT_GRANULARITY	XLAT_BLOCK_SIZE(MIN_LVL_BLOCK_DESC)

/* Helper macro to define an mmap_region_t. */
#define MAP_REGION(_pa, _va, _sz, _attr)	\
	_MAP_REGION_FULL_SPEC(_pa, _va, _sz, _attr, REGION_DEFAULT_GRANULARITY)

/* Helper macro to define an mmap_region_t with an identity mapping. */
#define MAP_REGION_FLAT(_adr, _sz, _attr)			\
	MAP_REGION(_adr, _adr, _sz, _attr)

/*
 * Helper macro to define an mmap_region_t to map with the desired granularity
 * of translation tables.
 *
 * The granularity value passed to this macro must be a valid block or page
 * size. When using a 4KB translation granule, this might be 4KB, 2MB or 1GB.
 * Passing REGION_DEFAULT_GRANULARITY is also allowed and means that the library
 * is free to choose the granularity for this region. In this case, it is
 * equivalent to the MAP_REGION() macro.
 */
#define MAP_REGION2(_pa, _va, _sz, _attr, _gr)			\
	_MAP_REGION_FULL_SPEC(_pa, _va, _sz, _attr, _gr)

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
	/* Desired granularity. See the MAP_REGION2() macro for more details. */
	size_t			granularity;
} mmap_region_t;

/*
 * Translation regimes supported by this library.
 */
typedef enum xlat_regime {
	EL1_EL0_REGIME,
	EL3_REGIME,
} xlat_regime_t;

/*
 * Declare the translation context type.
 * Its definition is private.
 */
typedef struct xlat_ctx xlat_ctx_t;

/*
 * Statically allocate a translation context and associated structures. Also
 * initialize them.
 *
 * _ctx_name:
 *   Prefix for the translation context variable.
 *   E.g. If _ctx_name is 'foo', the variable will be called 'foo_xlat_ctx'.
 *   Useful to distinguish multiple contexts from one another.
 *
 * _mmap_count:
 *   Number of mmap_region_t to allocate.
 *   Would typically be MAX_MMAP_REGIONS for the translation context describing
 *   the BL image currently executing.
 *
 * _xlat_tables_count:
 *   Number of sub-translation tables to allocate.
 *   Would typically be MAX_XLAT_TABLES for the translation context describing
 *   the BL image currently executing.
 *   Note that this is only for sub-tables ; at the initial lookup level, there
 *   is always a single table.
 *
 * _virt_addr_space_size, _phy_addr_space_size:
 *   Size (in bytes) of the virtual (resp. physical) address space.
 *   Would typically be PLAT_VIRT_ADDR_SPACE_SIZE
 *   (resp. PLAT_PHY_ADDR_SPACE_SIZE) for the translation context describing the
 *   BL image currently executing.
 */
#define REGISTER_XLAT_CONTEXT(_ctx_name, _mmap_count, _xlat_tables_count,	\
			_virt_addr_space_size, _phy_addr_space_size)		\
	_REGISTER_XLAT_CONTEXT(_ctx_name, _mmap_count, _xlat_tables_count,	\
		_virt_addr_space_size, _phy_addr_space_size)

/******************************************************************************
 * Generic translation table APIs.
 * Each API comes in 2 variants:
 * - one that acts on the current translation context for this BL image
 * - another that acts on the given translation context instead. This variant
 *   is named after the 1st version, with an additional '_ctx' suffix.
 *****************************************************************************/

/*
 * Initialize translation tables from the current list of mmap regions. Calling
 * this function marks the transition point after which static regions can no
 * longer be added.
 */
void init_xlat_tables(void);
void init_xlat_tables_ctx(xlat_ctx_t *ctx);

/*
 * Add a static region with defined base PA and base VA. This function can only
 * be used before initializing the translation tables. The region cannot be
 * removed afterwards.
 */
void mmap_add_region(unsigned long long base_pa, uintptr_t base_va,
				size_t size, mmap_attr_t attr);
void mmap_add_region_ctx(xlat_ctx_t *ctx, const mmap_region_t *mm);

/*
 * Add an array of static regions with defined base PA and base VA. This
 * function can only be used before initializing the translation tables. The
 * regions cannot be removed afterwards.
 */
void mmap_add(const mmap_region_t *mm);
void mmap_add_ctx(xlat_ctx_t *ctx, const mmap_region_t *mm);


#if PLAT_XLAT_TABLES_DYNAMIC
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
int mmap_add_dynamic_region_ctx(xlat_ctx_t *ctx, mmap_region_t *mm);

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
int mmap_remove_dynamic_region_ctx(xlat_ctx_t *ctx,
				uintptr_t base_va,
				size_t size);

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

#endif /*__ASSEMBLY__*/
#endif /* __XLAT_TABLES_V2_H__ */
