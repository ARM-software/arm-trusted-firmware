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
/*
 * In the EL1&0 translation regime, mark the region as User (EL0) or
 * Privileged (EL1). In the EL3 translation regime this has no effect.
 */
#define MT_USER_SHIFT		U(6)
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

	/*
	 * When mapping a region at EL0 or EL1, this attribute will be used to
	 * determine if a User mapping (EL0) will be created or a Privileged
	 * mapping (EL1).
	 */
	MT_USER				= U(1) << MT_USER_SHIFT,
	MT_PRIVILEGED			= U(0) << MT_USER_SHIFT,
} mmap_attr_t;

/* Compound attributes for most common usages */
#define MT_CODE		(MT_MEMORY | MT_RO | MT_EXECUTE)
#define MT_RO_DATA	(MT_MEMORY | MT_RO | MT_EXECUTE_NEVER)
#define MT_RW_DATA	(MT_MEMORY | MT_RW | MT_EXECUTE_NEVER)

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
	_REGISTER_XLAT_CONTEXT_FULL_SPEC(_ctx_name, _mmap_count,	\
					 _xlat_tables_count,		\
					 _virt_addr_space_size,		\
					 _phy_addr_space_size,		\
					 IMAGE_XLAT_DEFAULT_REGIME,	\
					"xlat_table")

/*
 * Same as REGISTER_XLAT_CONTEXT plus the additional parameters:
 *
 * _xlat_regime:
 *   Specify the translation regime managed by this xlat_ctx_t instance. The
 *   values are the one from xlat_regime_t enumeration.
 *
 * _section_name:
 *   Specify the name of the section where the translation tables have to be
 *   placed by the linker.
 */
#define REGISTER_XLAT_CONTEXT2(_ctx_name, _mmap_count, _xlat_tables_count,	\
			_virt_addr_space_size, _phy_addr_space_size,		\
			_xlat_regime, _section_name)				\
	_REGISTER_XLAT_CONTEXT_FULL_SPEC(_ctx_name, _mmap_count,	\
					 _xlat_tables_count,		\
					 _virt_addr_space_size,		\
					 _phy_addr_space_size,		\
					 _xlat_regime, _section_name)

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

/*
 * Change the memory attributes of the memory region starting from a given
 * virtual address in a set of translation tables.
 *
 * This function can only be used after the translation tables have been
 * initialized.
 *
 * The base address of the memory region must be aligned on a page boundary.
 * The size of this memory region must be a multiple of a page size.
 * The memory region must be already mapped by the given translation tables
 * and it must be mapped at the granularity of a page.
 *
 * Return 0 on success, a negative value on error.
 *
 * In case of error, the memory attributes remain unchanged and this function
 * has no effect.
 *
 * ctx
 *   Translation context to work on.
 * base_va:
 *   Virtual address of the 1st page to change the attributes of.
 * size:
 *   Size in bytes of the memory region.
 * attr:
 *   New attributes of the page tables. The attributes that can be changed are
 *   data access (MT_RO/MT_RW), instruction access (MT_EXECUTE_NEVER/MT_EXECUTE)
 *   and user/privileged access (MT_USER/MT_PRIVILEGED) in the case of contexts
 *   that are used in the EL1&0 translation regime. Also, note that this
 *   function doesn't allow to remap a region as RW and executable, or to remap
 *   device memory as executable.
 *
 * NOTE: The caller of this function must be able to write to the translation
 * tables, i.e. the memory where they are stored must be mapped with read-write
 * access permissions. This function assumes it is the case. If this is not
 * the case then this function might trigger a data abort exception.
 *
 * NOTE2: The caller is responsible for making sure that the targeted
 * translation tables are not modified by any other code while this function is
 * executing.
 */
int change_mem_attributes(xlat_ctx_t *ctx, uintptr_t base_va, size_t size,
			mmap_attr_t attr);

/*
 * Query the memory attributes of a memory page in a set of translation tables.
 *
 * Return 0 on success, a negative error code on error.
 * On success, the attributes are stored into *attributes.
 *
 * ctx
 *   Translation context to work on.
 * base_va
 *   Virtual address of the page to get the attributes of.
 *   There are no alignment restrictions on this address. The attributes of the
 *   memory page it lies within are returned.
 * attributes
 *   Output parameter where to store the attributes of the targeted memory page.
 */
int get_mem_attributes(const xlat_ctx_t *ctx, uintptr_t base_va,
		mmap_attr_t *attributes);

#endif /*__ASSEMBLY__*/
#endif /* __XLAT_TABLES_V2_H__ */
