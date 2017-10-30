/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This header file contains internal definitions that are not supposed to be
 * used outside of this library code.
 */

#ifndef __XLAT_TABLES_V2_HELPERS_H__
#define __XLAT_TABLES_V2_HELPERS_H__

#ifndef __XLAT_TABLES_V2_H__
#error "Do not include this header file directly. Include xlat_tables_v2.h instead."
#endif

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <platform_def.h>
#include <stddef.h>
#include <xlat_tables_arch.h>
#include <xlat_tables_defs.h>

/* Forward declaration */
struct mmap_region;

/*
 * Helper macro to define an mmap_region_t.  This macro allows to specify all
 * the fields of the structure but its parameter list is not guaranteed to
 * remain stable as we add members to mmap_region_t.
 */
#define _MAP_REGION_FULL_SPEC(_pa, _va, _sz, _attr, _gr)	\
	{							\
		.base_pa = (_pa),				\
		.base_va = (_va),				\
		.size = (_sz),					\
		.attr = (_attr),				\
		.granularity = (_gr),				\
	}

/* Struct that holds all information about the translation tables. */
struct xlat_ctx {
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
	struct mmap_region *mmap;
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
	 * Translation regime managed by this xlat_ctx_t. It takes the values of
	 * the enumeration xlat_regime_t. The type is "int" to avoid a circular
	 * dependency on xlat_tables_v2.h, but this member must be treated as
	 * xlat_regime_t.
	 */
	int xlat_regime;
};

#if PLAT_XLAT_TABLES_DYNAMIC
#define _ALLOC_DYNMAP_STRUCT(_ctx_name, _xlat_tables_count)		\
	static int _ctx_name##_mapped_regions[_xlat_tables_count];

#define _REGISTER_DYNMAP_STRUCT(_ctx_name)				\
	.tables_mapped_regions = _ctx_name##_mapped_regions,
#else
#define _ALLOC_DYNMAP_STRUCT(_ctx_name, _xlat_tables_count)		\
	/* do nothing */

#define _REGISTER_DYNMAP_STRUCT(_ctx_name)				\
	/* do nothing */
#endif /* PLAT_XLAT_TABLES_DYNAMIC */

#define _REGISTER_XLAT_CONTEXT_FULL_SPEC(_ctx_name, _mmap_count, _xlat_tables_count,	\
			_virt_addr_space_size, _phy_addr_space_size,		\
			_xlat_regime, _section_name)				\
	CASSERT(CHECK_VIRT_ADDR_SPACE_SIZE(_virt_addr_space_size),		\
		assert_invalid_virtual_addr_space_size_for_##_ctx_name);	\
										\
	CASSERT(CHECK_PHY_ADDR_SPACE_SIZE(_phy_addr_space_size),		\
		assert_invalid_physical_addr_space_sizefor_##_ctx_name);	\
										\
	static mmap_region_t _ctx_name##_mmap[_mmap_count + 1];			\
										\
	static uint64_t _ctx_name##_xlat_tables[_xlat_tables_count]		\
		[XLAT_TABLE_ENTRIES]						\
		__aligned(XLAT_TABLE_SIZE) __section(_section_name);		\
										\
	static uint64_t _ctx_name##_base_xlat_table				\
		[GET_NUM_BASE_LEVEL_ENTRIES(_virt_addr_space_size)]		\
		__aligned(GET_NUM_BASE_LEVEL_ENTRIES(_virt_addr_space_size)	\
			* sizeof(uint64_t));					\
										\
	_ALLOC_DYNMAP_STRUCT(_ctx_name, _xlat_tables_count)			\
										\
	static xlat_ctx_t _ctx_name##_xlat_ctx = {				\
		.va_max_address = (_virt_addr_space_size) - 1,			\
		.pa_max_address = (_phy_addr_space_size) - 1,			\
		.mmap = _ctx_name##_mmap,					\
		.mmap_num = _mmap_count,					\
		.base_level = GET_XLAT_TABLE_LEVEL_BASE(_virt_addr_space_size),	\
		.base_table = _ctx_name##_base_xlat_table,			\
		.base_table_entries =						\
			GET_NUM_BASE_LEVEL_ENTRIES(_virt_addr_space_size),	\
		.tables = _ctx_name##_xlat_tables,				\
		.tables_num = _xlat_tables_count,				\
		 _REGISTER_DYNMAP_STRUCT(_ctx_name)				\
		.xlat_regime = (_xlat_regime),					\
		.max_pa = 0,							\
		.max_va = 0,							\
		.next_table = 0,						\
		.initialized = 0,						\
	}

#if AARCH64

/*
 * This IMAGE_EL macro must not to be used outside the library, and it is only
 * used in AArch64.
 */
#if defined(IMAGE_BL1) || defined(IMAGE_BL31) || (defined(IMAGE_BL2) && BL2_AT_EL3)
# define IMAGE_EL	3
# define IMAGE_XLAT_DEFAULT_REGIME EL3_REGIME
#else
# define IMAGE_EL	1
# define IMAGE_XLAT_DEFAULT_REGIME EL1_EL0_REGIME
#endif

#else /* if AARCH32 */

/*
 * The PL1&0 translation regime in AArch32 behaves like the EL1&0 regime in
 * AArch64 except for the XN bits, but we set and unset them at the same time,
 * so there's no difference in practice.
 */
#define IMAGE_XLAT_DEFAULT_REGIME EL1_EL0_REGIME

#endif /* AARCH64 */

#endif /*__ASSEMBLY__*/

#endif /* __XLAT_TABLES_V2_HELPERS_H__ */
