/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_TABLES_V2_H
#define XLAT_TABLES_V2_H

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2_helpers.h>

#ifndef __ASSEMBLER__
#include <stddef.h>
#include <stdint.h>

#include <lib/xlat_tables/xlat_mmu_helpers.h>

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
	MAP_REGION_FULL_SPEC(_pa, _va, _sz, _attr, REGION_DEFAULT_GRANULARITY)

/* Helper macro to define an mmap_region_t with an identity mapping. */
#define MAP_REGION_FLAT(_adr, _sz, _attr)			\
	MAP_REGION(_adr, _adr, _sz, _attr)

/*
 * Helper macro to define entries for mmap_region_t. It allows to define 'pa'
 * and sets 'va' to 0 for each region. To be used with mmap_add_alloc_va().
 */
#define MAP_REGION_ALLOC_VA(pa, sz, attr)	MAP_REGION(pa, 0, sz, attr)

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
	MAP_REGION_FULL_SPEC(_pa, _va, _sz, _attr, _gr)

/*
 * Shifts and masks to access fields of an mmap attribute
 */
#define MT_TYPE_MASK		U(0x7)
#define MT_TYPE(_attr)		((_attr) & MT_TYPE_MASK)
/* Access permissions (RO/RW) */
#define MT_PERM_SHIFT		U(3)

/* Physical address space (SECURE/NS/Root/Realm) */
#define	MT_PAS_SHIFT		U(4)
#define MT_PAS_MASK		(U(3) << MT_PAS_SHIFT)
#define MT_PAS(_attr)		((_attr) & MT_PAS_MASK)

/* Access permissions for instruction execution (EXECUTE/EXECUTE_NEVER) */
#define MT_EXECUTE_SHIFT	U(6)
/* In the EL1&0 translation regime, User (EL0) or Privileged (EL1). */
#define MT_USER_SHIFT		U(7)

/* Shareability attribute for the memory region */
#define MT_SHAREABILITY_SHIFT	U(8)
#define MT_SHAREABILITY_MASK	(U(3) << MT_SHAREABILITY_SHIFT)
#define MT_SHAREABILITY(_attr)	((_attr) & MT_SHAREABILITY_MASK)

/* All other bits are reserved */

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

#define MT_SECURE		(U(0) << MT_PAS_SHIFT)
#define MT_NS			(U(1) << MT_PAS_SHIFT)
#define MT_ROOT			(U(2) << MT_PAS_SHIFT)
#define MT_REALM		(U(3) << MT_PAS_SHIFT)

/*
 * Access permissions for instruction execution are only relevant for normal
 * read-only memory, i.e. MT_MEMORY | MT_RO. They are ignored (and potentially
 * overridden) otherwise:
 *  - Device memory is always marked as execute-never.
 *  - Read-write normal memory is always marked as execute-never.
 */
#define MT_EXECUTE		(U(0) << MT_EXECUTE_SHIFT)
#define MT_EXECUTE_NEVER	(U(1) << MT_EXECUTE_SHIFT)

/*
 * When mapping a region at EL0 or EL1, this attribute will be used to determine
 * if a User mapping (EL0) will be created or a Privileged mapping (EL1).
 */
#define MT_USER			(U(1) << MT_USER_SHIFT)
#define MT_PRIVILEGED		(U(0) << MT_USER_SHIFT)

/*
 * Shareability defines the visibility of any cache changes to
 * all masters belonging to a shareable domain.
 *
 * MT_SHAREABILITY_ISH: For inner shareable domain
 * MT_SHAREABILITY_OSH: For outer shareable domain
 * MT_SHAREABILITY_NSH: For non shareable domain
 */
#define MT_SHAREABILITY_ISH	(U(1) << MT_SHAREABILITY_SHIFT)
#define MT_SHAREABILITY_OSH	(U(2) << MT_SHAREABILITY_SHIFT)
#define MT_SHAREABILITY_NSH	(U(3) << MT_SHAREABILITY_SHIFT)

/* Compound attributes for most common usages */
#define MT_CODE			(MT_MEMORY | MT_RO | MT_EXECUTE)
#define MT_RO_DATA		(MT_MEMORY | MT_RO | MT_EXECUTE_NEVER)
#define MT_RW_DATA		(MT_MEMORY | MT_RW | MT_EXECUTE_NEVER)

/*
 * Structure for specifying a single region of memory.
 */
typedef struct mmap_region {
	unsigned long long	base_pa;
	uintptr_t		base_va;
	size_t			size;
	unsigned int		attr;
	/* Desired granularity. See the MAP_REGION2() macro for more details. */
	size_t			granularity;
} mmap_region_t;

/*
 * Translation regimes supported by this library. EL_REGIME_INVALID tells the
 * library to detect it at runtime.
 */
#define EL1_EL0_REGIME		1
#define EL2_REGIME		2
#define EL3_REGIME		3
#define EL_REGIME_INVALID	-1

/* Memory type for EL3 regions. With RME, EL3 is in ROOT PAS */
#if ENABLE_RME
#define EL3_PAS			MT_ROOT
#else
#define EL3_PAS			MT_SECURE
#endif /* ENABLE_RME */

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
#define REGISTER_XLAT_CONTEXT(_ctx_name, _mmap_count, _xlat_tables_count, \
			      _virt_addr_space_size, _phy_addr_space_size) \
	REGISTER_XLAT_CONTEXT_FULL_SPEC(_ctx_name, (_mmap_count),	\
					 (_xlat_tables_count),		\
					 (_virt_addr_space_size),	\
					 (_phy_addr_space_size),	\
					 EL_REGIME_INVALID,		\
					 "xlat_table", "base_xlat_table")

/*
 * Same as REGISTER_XLAT_CONTEXT plus the additional parameters:
 *
 * _xlat_regime:
 *   Specify the translation regime managed by this xlat_ctx_t instance. The
 *   values are the one from the EL*_REGIME definitions.
 *
 * _section_name:
 *   Specify the name of the section where the translation tables have to be
 *   placed by the linker.
 *
 * _base_table_section_name:
 *   Specify the name of the section where the base translation tables have to
 *   be placed by the linker.
 */
#define REGISTER_XLAT_CONTEXT2(_ctx_name, _mmap_count, _xlat_tables_count, \
			_virt_addr_space_size, _phy_addr_space_size,	\
			_xlat_regime, _section_name, _base_table_section_name) \
	REGISTER_XLAT_CONTEXT_FULL_SPEC(_ctx_name, (_mmap_count),	\
					 (_xlat_tables_count),		\
					 (_virt_addr_space_size),	\
					 (_phy_addr_space_size),	\
					 (_xlat_regime),		\
					 (_section_name), (_base_table_section_name) \
)

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
 * Fill all fields of a dynamic translation tables context. It must be done
 * either statically with REGISTER_XLAT_CONTEXT() or at runtime with this
 * function.
 */
void xlat_setup_dynamic_ctx(xlat_ctx_t *ctx, unsigned long long pa_max,
			    uintptr_t va_max, struct mmap_region *mmap,
			    unsigned int mmap_num, uint64_t **tables,
			    unsigned int tables_num, uint64_t *base_table,
			    int xlat_regime, int *mapped_regions);

/*
 * Add a static region with defined base PA and base VA. This function can only
 * be used before initializing the translation tables. The region cannot be
 * removed afterwards.
 */
void mmap_add_region(unsigned long long base_pa, uintptr_t base_va,
		     size_t size, unsigned int attr);
void mmap_add_region_ctx(xlat_ctx_t *ctx, const mmap_region_t *mm);

/*
 * Add an array of static regions with defined base PA and base VA. This
 * function can only be used before initializing the translation tables. The
 * regions cannot be removed afterwards.
 */
void mmap_add(const mmap_region_t *mm);
void mmap_add_ctx(xlat_ctx_t *ctx, const mmap_region_t *mm);

/*
 * Add a region with defined base PA. Returns base VA calculated using the
 * highest existing region in the mmap array even if it fails to allocate the
 * region.
 */
void mmap_add_region_alloc_va(unsigned long long base_pa, uintptr_t *base_va,
			      size_t size, unsigned int attr);
void mmap_add_region_alloc_va_ctx(xlat_ctx_t *ctx, mmap_region_t *mm);

/*
 * Add an array of static regions with defined base PA, and fill the base VA
 * field on the array of structs. This function can only be used before
 * initializing the translation tables. The regions cannot be removed afterwards.
 */
void mmap_add_alloc_va(mmap_region_t *mm);

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
			    size_t size, unsigned int attr);
int mmap_add_dynamic_region_ctx(xlat_ctx_t *ctx, mmap_region_t *mm);

/*
 * Add a dynamic region with defined base PA. Returns base VA calculated using
 * the highest existing region in the mmap array even if it fails to allocate
 * the region.
 *
 * mmap_add_dynamic_region_alloc_va() returns the allocated VA in 'base_va'.
 * mmap_add_dynamic_region_alloc_va_ctx() returns it in 'mm->base_va'.
 *
 * It returns the same error values as mmap_add_dynamic_region().
 */
int mmap_add_dynamic_region_alloc_va(unsigned long long base_pa,
				     uintptr_t *base_va,
				     size_t size, unsigned int attr);
int mmap_add_dynamic_region_alloc_va_ctx(xlat_ctx_t *ctx, mmap_region_t *mm);

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
int xlat_change_mem_attributes_ctx(const xlat_ctx_t *ctx, uintptr_t base_va,
				   size_t size, uint32_t attr);
int xlat_change_mem_attributes(uintptr_t base_va, size_t size, uint32_t attr);

#if PLAT_RO_XLAT_TABLES
/*
 * Change the memory attributes of the memory region encompassing the higher
 * level translation tables to secure read-only data.
 *
 * Return 0 on success, a negative error code on error.
 */
int xlat_make_tables_readonly(void);
#endif

/*
 * Query the memory attributes of a memory page in a set of translation tables.
 *
 * Return 0 on success, a negative error code on error.
 * On success, the attributes are stored into *attr.
 *
 * ctx
 *   Translation context to work on.
 * base_va
 *   Virtual address of the page to get the attributes of.
 *   There are no alignment restrictions on this address. The attributes of the
 *   memory page it lies within are returned.
 * attr
 *   Output parameter where to store the attributes of the targeted memory page.
 */
int xlat_get_mem_attributes_ctx(const xlat_ctx_t *ctx, uintptr_t base_va,
				uint32_t *attr);
int xlat_get_mem_attributes(uintptr_t base_va, uint32_t *attr);

#endif /*__ASSEMBLER__*/
#endif /* XLAT_TABLES_V2_H */
