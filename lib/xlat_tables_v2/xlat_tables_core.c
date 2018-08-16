/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <platform_def.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <utils_def.h>
#include <xlat_tables_defs.h>
#include <xlat_tables_v2.h>

#include "xlat_tables_private.h"

/* Helper function that cleans the data cache only if it is enabled. */
static inline void xlat_clean_dcache_range(uintptr_t addr, size_t size)
{
	if (is_dcache_enabled())
		clean_dcache_range(addr, size);
}

#if PLAT_XLAT_TABLES_DYNAMIC

/*
 * The following functions assume that they will be called using subtables only.
 * The base table can't be unmapped, so it is not needed to do any special
 * handling for it.
 */

/*
 * Returns the index of the array corresponding to the specified translation
 * table.
 */
static int xlat_table_get_index(const xlat_ctx_t *ctx, const uint64_t *table)
{
	for (int i = 0; i < ctx->tables_num; i++)
		if (ctx->tables[i] == table)
			return i;

	/*
	 * Maybe we were asked to get the index of the base level table, which
	 * should never happen.
	 */
	assert(false);

	return -1;
}

/* Returns a pointer to an empty translation table. */
static uint64_t *xlat_table_get_empty(const xlat_ctx_t *ctx)
{
	for (int i = 0; i < ctx->tables_num; i++)
		if (ctx->tables_mapped_regions[i] == 0)
			return ctx->tables[i];

	return NULL;
}

/* Increments region count for a given table. */
static void xlat_table_inc_regions_count(const xlat_ctx_t *ctx,
					 const uint64_t *table)
{
	int idx = xlat_table_get_index(ctx, table);

	ctx->tables_mapped_regions[idx]++;
}

/* Decrements region count for a given table. */
static void xlat_table_dec_regions_count(const xlat_ctx_t *ctx,
					 const uint64_t *table)
{
	int idx = xlat_table_get_index(ctx, table);

	ctx->tables_mapped_regions[idx]--;
}

/* Returns 0 if the specified table isn't empty, otherwise 1. */
static bool xlat_table_is_empty(const xlat_ctx_t *ctx, const uint64_t *table)
{
	return ctx->tables_mapped_regions[xlat_table_get_index(ctx, table)] == 0;
}

#else /* PLAT_XLAT_TABLES_DYNAMIC */

/* Returns a pointer to the first empty translation table. */
static uint64_t *xlat_table_get_empty(xlat_ctx_t *ctx)
{
	assert(ctx->next_table < ctx->tables_num);

	return ctx->tables[ctx->next_table++];
}

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

/*
 * Returns a block/page table descriptor for the given level and attributes.
 */
uint64_t xlat_desc(const xlat_ctx_t *ctx, uint32_t attr,
		   unsigned long long addr_pa, unsigned int level)
{
	uint64_t desc;
	uint32_t mem_type;

	/* Make sure that the granularity is fine enough to map this address. */
	assert((addr_pa & XLAT_BLOCK_MASK(level)) == 0U);

	desc = addr_pa;
	/*
	 * There are different translation table descriptors for level 3 and the
	 * rest.
	 */
	desc |= (level == XLAT_TABLE_LEVEL_MAX) ? PAGE_DESC : BLOCK_DESC;
	/*
	 * Always set the access flag, as this library assumes access flag
	 * faults aren't managed.
	 */
	desc |= LOWER_ATTRS(ACCESS_FLAG);
	/*
	 * Deduce other fields of the descriptor based on the MT_NS and MT_RW
	 * memory region attributes.
	 */
	desc |= ((attr & MT_NS) != 0U) ? LOWER_ATTRS(NS) : 0U;
	desc |= ((attr & MT_RW) != 0U) ? LOWER_ATTRS(AP_RW) : LOWER_ATTRS(AP_RO);

	/*
	 * Do not allow unprivileged access when the mapping is for a privileged
	 * EL. For translation regimes that do not have mappings for access for
	 * lower exception levels, set AP[2] to AP_NO_ACCESS_UNPRIVILEGED.
	 */
	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		if ((attr & MT_USER) != 0U) {
			/* EL0 mapping requested, so we give User access */
			desc |= LOWER_ATTRS(AP_ACCESS_UNPRIVILEGED);
		} else {
			/* EL1 mapping requested, no User access granted */
			desc |= LOWER_ATTRS(AP_NO_ACCESS_UNPRIVILEGED);
		}
	} else {
		assert((ctx->xlat_regime == EL2_REGIME) ||
		       (ctx->xlat_regime == EL3_REGIME));
		desc |= LOWER_ATTRS(AP_ONE_VA_RANGE_RES1);
	}

	/*
	 * Deduce shareability domain and executability of the memory region
	 * from the memory type of the attributes (MT_TYPE).
	 *
	 * Data accesses to device memory and non-cacheable normal memory are
	 * coherent for all observers in the system, and correspondingly are
	 * always treated as being Outer Shareable. Therefore, for these 2 types
	 * of memory, it is not strictly needed to set the shareability field
	 * in the translation tables.
	 */
	mem_type = MT_TYPE(attr);
	if (mem_type == MT_DEVICE) {
		desc |= LOWER_ATTRS(ATTR_DEVICE_INDEX | OSH);
		/*
		 * Always map device memory as execute-never.
		 * This is to avoid the possibility of a speculative instruction
		 * fetch, which could be an issue if this memory region
		 * corresponds to a read-sensitive peripheral.
		 */
		desc |= xlat_arch_regime_get_xn_desc(ctx->xlat_regime);

	} else { /* Normal memory */
		/*
		 * Always map read-write normal memory as execute-never.
		 * This library assumes that it is used by software that does
		 * not self-modify its code, therefore R/W memory is reserved
		 * for data storage, which must not be executable.
		 *
		 * Note that setting the XN bit here is for consistency only.
		 * The function that enables the MMU sets the SCTLR_ELx.WXN bit,
		 * which makes any writable memory region to be treated as
		 * execute-never, regardless of the value of the XN bit in the
		 * translation table.
		 *
		 * For read-only memory, rely on the MT_EXECUTE/MT_EXECUTE_NEVER
		 * attribute to figure out the value of the XN bit.  The actual
		 * XN bit(s) to set in the descriptor depends on the context's
		 * translation regime and the policy applied in
		 * xlat_arch_regime_get_xn_desc().
		 */
		if (((attr & MT_RW) != 0U) || ((attr & MT_EXECUTE_NEVER) != 0U)) {
			desc |= xlat_arch_regime_get_xn_desc(ctx->xlat_regime);
		}

		if (mem_type == MT_MEMORY) {
			desc |= LOWER_ATTRS(ATTR_IWBWA_OWBWA_NTR_INDEX | ISH);
		} else {
			assert(mem_type == MT_NON_CACHEABLE);
			desc |= LOWER_ATTRS(ATTR_NON_CACHEABLE_INDEX | OSH);
		}
	}

	return desc;
}

/*
 * Enumeration of actions that can be made when mapping table entries depending
 * on the previous value in that entry and information about the region being
 * mapped.
 */
typedef enum {

	/* Do nothing */
	ACTION_NONE,

	/* Write a block (or page, if in level 3) entry. */
	ACTION_WRITE_BLOCK_ENTRY,

	/*
	 * Create a new table and write a table entry pointing to it. Recurse
	 * into it for further processing.
	 */
	ACTION_CREATE_NEW_TABLE,

	/*
	 * There is a table descriptor in this entry, read it and recurse into
	 * that table for further processing.
	 */
	ACTION_RECURSE_INTO_TABLE,

} action_t;

#if PLAT_XLAT_TABLES_DYNAMIC

/*
 * Recursive function that writes to the translation tables and unmaps the
 * specified region.
 */
static void xlat_tables_unmap_region(xlat_ctx_t *ctx, mmap_region_t *mm,
				     const uintptr_t table_base_va,
				     uint64_t *const table_base,
				     const unsigned int table_entries,
				     const unsigned int level)
{
	assert((level >= ctx->base_level) && (level <= XLAT_TABLE_LEVEL_MAX));

	uint64_t *subtable;
	uint64_t desc;

	uintptr_t table_idx_va;
	uintptr_t table_idx_end_va; /* End VA of this entry */

	uintptr_t region_end_va = mm->base_va + mm->size - 1U;

	unsigned int table_idx;

	if (mm->base_va > table_base_va) {
		/* Find the first index of the table affected by the region. */
		table_idx_va = mm->base_va & ~XLAT_BLOCK_MASK(level);

		table_idx = (unsigned int)((table_idx_va - table_base_va) >>
			    XLAT_ADDR_SHIFT(level));

		assert(table_idx < table_entries);
	} else {
		/* Start from the beginning of the table. */
		table_idx_va = table_base_va;
		table_idx = 0;
	}

	while (table_idx < table_entries) {

		table_idx_end_va = table_idx_va + XLAT_BLOCK_SIZE(level) - 1U;

		desc = table_base[table_idx];
		uint64_t desc_type = desc & DESC_MASK;

		action_t action;

		if ((mm->base_va <= table_idx_va) &&
		    (region_end_va >= table_idx_end_va)) {
			/* Region covers all block */

			if (level == 3U) {
				/*
				 * Last level, only page descriptors allowed,
				 * erase it.
				 */
				assert(desc_type == PAGE_DESC);

				action = ACTION_WRITE_BLOCK_ENTRY;
			} else {
				/*
				 * Other levels can have table descriptors. If
				 * so, recurse into it and erase descriptors
				 * inside it as needed. If there is a block
				 * descriptor, just erase it. If an invalid
				 * descriptor is found, this table isn't
				 * actually mapped, which shouldn't happen.
				 */
				if (desc_type == TABLE_DESC) {
					action = ACTION_RECURSE_INTO_TABLE;
				} else {
					assert(desc_type == BLOCK_DESC);
					action = ACTION_WRITE_BLOCK_ENTRY;
				}
			}

		} else if ((mm->base_va <= table_idx_end_va) ||
			   (region_end_va >= table_idx_va)) {
			/*
			 * Region partially covers block.
			 *
			 * It can't happen in level 3.
			 *
			 * There must be a table descriptor here, if not there
			 * was a problem when mapping the region.
			 */
			assert(level < 3U);
			assert(desc_type == TABLE_DESC);

			action = ACTION_RECURSE_INTO_TABLE;
		} else {
			/* The region doesn't cover the block at all */
			action = ACTION_NONE;
		}

		if (action == ACTION_WRITE_BLOCK_ENTRY) {

			table_base[table_idx] = INVALID_DESC;
			xlat_arch_tlbi_va(table_idx_va, ctx->xlat_regime);

		} else if (action == ACTION_RECURSE_INTO_TABLE) {

			subtable = (uint64_t *)(uintptr_t)(desc & TABLE_ADDR_MASK);

			/* Recurse to write into subtable */
			xlat_tables_unmap_region(ctx, mm, table_idx_va,
						 subtable, XLAT_TABLE_ENTRIES,
						 level + 1U);
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
			xlat_clean_dcache_range((uintptr_t)subtable,
				XLAT_TABLE_ENTRIES * sizeof(uint64_t));
#endif
			/*
			 * If the subtable is now empty, remove its reference.
			 */
			if (xlat_table_is_empty(ctx, subtable)) {
				table_base[table_idx] = INVALID_DESC;
				xlat_arch_tlbi_va(table_idx_va,
						  ctx->xlat_regime);
			}

		} else {
			assert(action == ACTION_NONE);
		}

		table_idx++;
		table_idx_va += XLAT_BLOCK_SIZE(level);

		/* If reached the end of the region, exit */
		if (region_end_va <= table_idx_va)
			break;
	}

	if (level > ctx->base_level)
		xlat_table_dec_regions_count(ctx, table_base);
}

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

/*
 * From the given arguments, it decides which action to take when mapping the
 * specified region.
 */
static action_t xlat_tables_map_region_action(const mmap_region_t *mm,
		unsigned int desc_type, unsigned long long dest_pa,
		uintptr_t table_entry_base_va, unsigned int level)
{
	uintptr_t mm_end_va = mm->base_va + mm->size - 1U;
	uintptr_t table_entry_end_va =
			table_entry_base_va + XLAT_BLOCK_SIZE(level) - 1U;

	/*
	 * The descriptor types allowed depend on the current table level.
	 */

	if ((mm->base_va <= table_entry_base_va) &&
	    (mm_end_va >= table_entry_end_va)) {

		/*
		 * Table entry is covered by region
		 * --------------------------------
		 *
		 * This means that this table entry can describe the whole
		 * translation with this granularity in principle.
		 */

		if (level == 3U) {
			/*
			 * Last level, only page descriptors are allowed.
			 */
			if (desc_type == PAGE_DESC) {
				/*
				 * There's another region mapped here, don't
				 * overwrite.
				 */
				return ACTION_NONE;
			} else {
				assert(desc_type == INVALID_DESC);
				return ACTION_WRITE_BLOCK_ENTRY;
			}

		} else {

			/*
			 * Other levels. Table descriptors are allowed. Block
			 * descriptors too, but they have some limitations.
			 */

			if (desc_type == TABLE_DESC) {
				/* There's already a table, recurse into it. */
				return ACTION_RECURSE_INTO_TABLE;

			} else if (desc_type == INVALID_DESC) {
				/*
				 * There's nothing mapped here, create a new
				 * entry.
				 *
				 * Check if the destination granularity allows
				 * us to use a block descriptor or we need a
				 * finer table for it.
				 *
				 * Also, check if the current level allows block
				 * descriptors. If not, create a table instead.
				 */
				if (((dest_pa & XLAT_BLOCK_MASK(level)) != 0U)
				    || (level < MIN_LVL_BLOCK_DESC) ||
				    (mm->granularity < XLAT_BLOCK_SIZE(level)))
					return ACTION_CREATE_NEW_TABLE;
				else
					return ACTION_WRITE_BLOCK_ENTRY;

			} else {
				/*
				 * There's another region mapped here, don't
				 * overwrite.
				 */
				assert(desc_type == BLOCK_DESC);

				return ACTION_NONE;
			}
		}

	} else if ((mm->base_va <= table_entry_end_va) ||
		   (mm_end_va >= table_entry_base_va)) {

		/*
		 * Region partially covers table entry
		 * -----------------------------------
		 *
		 * This means that this table entry can't describe the whole
		 * translation, a finer table is needed.

		 * There cannot be partial block overlaps in level 3. If that
		 * happens, some of the preliminary checks when adding the
		 * mmap region failed to detect that PA and VA must at least be
		 * aligned to PAGE_SIZE.
		 */
		assert(level < 3U);

		if (desc_type == INVALID_DESC) {
			/*
			 * The block is not fully covered by the region. Create
			 * a new table, recurse into it and try to map the
			 * region with finer granularity.
			 */
			return ACTION_CREATE_NEW_TABLE;

		} else {
			assert(desc_type == TABLE_DESC);
			/*
			 * The block is not fully covered by the region, but
			 * there is already a table here. Recurse into it and
			 * try to map with finer granularity.
			 *
			 * PAGE_DESC for level 3 has the same value as
			 * TABLE_DESC, but this code can't run on a level 3
			 * table because there can't be overlaps in level 3.
			 */
			return ACTION_RECURSE_INTO_TABLE;
		}
	} else {

		/*
		 * This table entry is outside of the region specified in the
		 * arguments, don't write anything to it.
		 */
		return ACTION_NONE;
	}
}

/*
 * Recursive function that writes to the translation tables and maps the
 * specified region. On success, it returns the VA of the last byte that was
 * successfully mapped. On error, it returns the VA of the next entry that
 * should have been mapped.
 */
static uintptr_t xlat_tables_map_region(xlat_ctx_t *ctx, mmap_region_t *mm,
				   uintptr_t table_base_va,
				   uint64_t *const table_base,
				   unsigned int table_entries,
				   unsigned int level)
{
	assert((level >= ctx->base_level) && (level <= XLAT_TABLE_LEVEL_MAX));

	uintptr_t mm_end_va = mm->base_va + mm->size - 1U;

	uintptr_t table_idx_va;
	unsigned long long table_idx_pa;

	uint64_t *subtable;
	uint64_t desc;

	unsigned int table_idx;

	if (mm->base_va > table_base_va) {
		/* Find the first index of the table affected by the region. */
		table_idx_va = mm->base_va & ~XLAT_BLOCK_MASK(level);

		table_idx = (unsigned int)((table_idx_va - table_base_va) >>
			    XLAT_ADDR_SHIFT(level));

		assert(table_idx < table_entries);
	} else {
		/* Start from the beginning of the table. */
		table_idx_va = table_base_va;
		table_idx = 0U;
	}

#if PLAT_XLAT_TABLES_DYNAMIC
	if (level > ctx->base_level)
		xlat_table_inc_regions_count(ctx, table_base);
#endif

	while (table_idx < table_entries) {

		desc = table_base[table_idx];

		table_idx_pa = mm->base_pa + table_idx_va - mm->base_va;

		action_t action = xlat_tables_map_region_action(mm,
			(uint32_t)(desc & DESC_MASK), table_idx_pa,
			table_idx_va, level);

		if (action == ACTION_WRITE_BLOCK_ENTRY) {

			table_base[table_idx] =
				xlat_desc(ctx, (uint32_t)mm->attr, table_idx_pa,
					  level);

		} else if (action == ACTION_CREATE_NEW_TABLE) {
			uintptr_t end_va;

			subtable = xlat_table_get_empty(ctx);
			if (subtable == NULL) {
				/* Not enough free tables to map this region */
				return table_idx_va;
			}

			/* Point to new subtable from this one. */
			table_base[table_idx] = TABLE_DESC | (unsigned long)subtable;

			/* Recurse to write into subtable */
			end_va = xlat_tables_map_region(ctx, mm, table_idx_va,
					       subtable, XLAT_TABLE_ENTRIES,
					       level + 1U);
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
			xlat_clean_dcache_range((uintptr_t)subtable,
				XLAT_TABLE_ENTRIES * sizeof(uint64_t));
#endif
			if (end_va !=
				(table_idx_va + XLAT_BLOCK_SIZE(level) - 1U))
				return end_va;

		} else if (action == ACTION_RECURSE_INTO_TABLE) {
			uintptr_t end_va;

			subtable = (uint64_t *)(uintptr_t)(desc & TABLE_ADDR_MASK);
			/* Recurse to write into subtable */
			end_va = xlat_tables_map_region(ctx, mm, table_idx_va,
					       subtable, XLAT_TABLE_ENTRIES,
					       level + 1U);
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
			xlat_clean_dcache_range((uintptr_t)subtable,
				XLAT_TABLE_ENTRIES * sizeof(uint64_t));
#endif
			if (end_va !=
				(table_idx_va + XLAT_BLOCK_SIZE(level) - 1U))
				return end_va;

		} else {

			assert(action == ACTION_NONE);

		}

		table_idx++;
		table_idx_va += XLAT_BLOCK_SIZE(level);

		/* If reached the end of the region, exit */
		if (mm_end_va <= table_idx_va)
			break;
	}

	return table_idx_va - 1U;
}

/*
 * Function that verifies that a region can be mapped.
 * Returns:
 *        0: Success, the mapping is allowed.
 *   EINVAL: Invalid values were used as arguments.
 *   ERANGE: The memory limits were surpassed.
 *   ENOMEM: There is not enough memory in the mmap array.
 *    EPERM: Region overlaps another one in an invalid way.
 */
static int mmap_add_region_check(const xlat_ctx_t *ctx, const mmap_region_t *mm)
{
	unsigned long long base_pa = mm->base_pa;
	uintptr_t base_va = mm->base_va;
	size_t size = mm->size;
	size_t granularity = mm->granularity;

	unsigned long long end_pa = base_pa + size - 1U;
	uintptr_t end_va = base_va + size - 1U;

	if (!IS_PAGE_ALIGNED(base_pa) || !IS_PAGE_ALIGNED(base_va) ||
			!IS_PAGE_ALIGNED(size))
		return -EINVAL;

	if ((granularity != XLAT_BLOCK_SIZE(1U)) &&
		(granularity != XLAT_BLOCK_SIZE(2U)) &&
		(granularity != XLAT_BLOCK_SIZE(3U))) {
		return -EINVAL;
	}

	/* Check for overflows */
	if ((base_pa > end_pa) || (base_va > end_va))
		return -ERANGE;

	if ((base_va + (uintptr_t)size - (uintptr_t)1) > ctx->va_max_address)
		return -ERANGE;

	if ((base_pa + (unsigned long long)size - 1ULL) > ctx->pa_max_address)
		return -ERANGE;

	/* Check that there is space in the ctx->mmap array */
	if (ctx->mmap[ctx->mmap_num - 1].size != 0U)
		return -ENOMEM;

	/* Check for PAs and VAs overlaps with all other regions */
	for (const mmap_region_t *mm_cursor = ctx->mmap;
	     mm_cursor->size != 0U; ++mm_cursor) {

		uintptr_t mm_cursor_end_va = mm_cursor->base_va
							+ mm_cursor->size - 1U;

		/*
		 * Check if one of the regions is completely inside the other
		 * one.
		 */
		bool fully_overlapped_va =
			((base_va >= mm_cursor->base_va) &&
					(end_va <= mm_cursor_end_va)) ||
			((mm_cursor->base_va >= base_va) &&
						(mm_cursor_end_va <= end_va));

		/*
		 * Full VA overlaps are only allowed if both regions are
		 * identity mapped (zero offset) or have the same VA to PA
		 * offset. Also, make sure that it's not the exact same area.
		 * This can only be done with static regions.
		 */
		if (fully_overlapped_va) {

#if PLAT_XLAT_TABLES_DYNAMIC
			if (((mm->attr & MT_DYNAMIC) != 0U) ||
			    ((mm_cursor->attr & MT_DYNAMIC) != 0U))
				return -EPERM;
#endif /* PLAT_XLAT_TABLES_DYNAMIC */
			if ((mm_cursor->base_va - mm_cursor->base_pa) !=
							(base_va - base_pa))
				return -EPERM;

			if ((base_va == mm_cursor->base_va) &&
						(size == mm_cursor->size))
				return -EPERM;

		} else {
			/*
			 * If the regions do not have fully overlapping VAs,
			 * then they must have fully separated VAs and PAs.
			 * Partial overlaps are not allowed
			 */

			unsigned long long mm_cursor_end_pa =
				     mm_cursor->base_pa + mm_cursor->size - 1U;

			bool separated_pa = (end_pa < mm_cursor->base_pa) ||
				(base_pa > mm_cursor_end_pa);
			bool separated_va = (end_va < mm_cursor->base_va) ||
				(base_va > mm_cursor_end_va);

			if (!separated_va || !separated_pa)
				return -EPERM;
		}
	}

	return 0;
}

void mmap_add_region_ctx(xlat_ctx_t *ctx, const mmap_region_t *mm)
{
	mmap_region_t *mm_cursor = ctx->mmap, *mm_destination;
	const mmap_region_t *mm_end = ctx->mmap + ctx->mmap_num;
	const mmap_region_t *mm_last;
	unsigned long long end_pa = mm->base_pa + mm->size - 1U;
	uintptr_t end_va = mm->base_va + mm->size - 1U;
	int ret;

	/* Ignore empty regions */
	if (mm->size == 0U)
		return;

	/* Static regions must be added before initializing the xlat tables. */
	assert(!ctx->initialized);

	ret = mmap_add_region_check(ctx, mm);
	if (ret != 0) {
		ERROR("mmap_add_region_check() failed. error %d\n", ret);
		assert(false);
		return;
	}

	/*
	 * Find correct place in mmap to insert new region.
	 *
	 * 1 - Lower region VA end first.
	 * 2 - Smaller region size first.
	 *
	 * VA  0                                   0xFF
	 *
	 * 1st |------|
	 * 2nd |------------|
	 * 3rd                 |------|
	 * 4th                            |---|
	 * 5th                                   |---|
	 * 6th                            |----------|
	 * 7th |-------------------------------------|
	 *
	 * This is required for overlapping regions only. It simplifies adding
	 * regions with the loop in xlat_tables_init_internal because the outer
	 * ones won't overwrite block or page descriptors of regions added
	 * previously.
	 *
	 * Overlapping is only allowed for static regions.
	 */

	while (((mm_cursor->base_va + mm_cursor->size - 1U) < end_va)
	       && (mm_cursor->size != 0U)) {
		++mm_cursor;
	}

	while (((mm_cursor->base_va + mm_cursor->size - 1U) == end_va) &&
	       (mm_cursor->size != 0U) && (mm_cursor->size < mm->size)) {
		++mm_cursor;
	}

	/*
	 * Find the last entry marker in the mmap
	 */
	mm_last = ctx->mmap;
	while ((mm_last->size != 0U) && (mm_last < mm_end)) {
		++mm_last;
	}

	/*
	 * Check if we have enough space in the memory mapping table.
	 * This shouldn't happen as we have checked in mmap_add_region_check
	 * that there is free space.
	 */
	assert(mm_last->size == 0U);

	/* Make room for new region by moving other regions up by one place */
	mm_destination = mm_cursor + 1;
	(void)memmove(mm_destination, mm_cursor,
		(uintptr_t)mm_last - (uintptr_t)mm_cursor);

	/*
	 * Check we haven't lost the empty sentinel from the end of the array.
	 * This shouldn't happen as we have checked in mmap_add_region_check
	 * that there is free space.
	 */
	assert(mm_end->size == 0U);

	*mm_cursor = *mm;

	if (end_pa > ctx->max_pa)
		ctx->max_pa = end_pa;
	if (end_va > ctx->max_va)
		ctx->max_va = end_va;
}

void mmap_add_ctx(xlat_ctx_t *ctx, const mmap_region_t *mm)
{
	const mmap_region_t *mm_cursor = mm;

	while (mm_cursor->size != 0U) {
		mmap_add_region_ctx(ctx, mm_cursor);
		mm_cursor++;
	}
}

#if PLAT_XLAT_TABLES_DYNAMIC

int mmap_add_dynamic_region_ctx(xlat_ctx_t *ctx, mmap_region_t *mm)
{
	mmap_region_t *mm_cursor = ctx->mmap;
	const mmap_region_t *mm_last = mm_cursor + ctx->mmap_num;
	unsigned long long end_pa = mm->base_pa + mm->size - 1U;
	uintptr_t end_va = mm->base_va + mm->size - 1U;
	int ret;

	/* Nothing to do */
	if (mm->size == 0U)
		return 0;

	/* Now this region is a dynamic one */
	mm->attr |= MT_DYNAMIC;

	ret = mmap_add_region_check(ctx, mm);
	if (ret != 0)
		return ret;

	/*
	 * Find the adequate entry in the mmap array in the same way done for
	 * static regions in mmap_add_region_ctx().
	 */

	while (((mm_cursor->base_va + mm_cursor->size - 1U) < end_va)
	       && (mm_cursor->size != 0U)) {
		++mm_cursor;
	}

	while (((mm_cursor->base_va + mm_cursor->size - 1U) == end_va) &&
	       (mm_cursor->size != 0U) && (mm_cursor->size < mm->size)) {
		++mm_cursor;
	}

	/* Make room for new region by moving other regions up by one place */
	(void)memmove(mm_cursor + 1U, mm_cursor,
		     (uintptr_t)mm_last - (uintptr_t)mm_cursor);

	/*
	 * Check we haven't lost the empty sentinal from the end of the array.
	 * This shouldn't happen as we have checked in mmap_add_region_check
	 * that there is free space.
	 */
	assert(mm_last->size == 0U);

	*mm_cursor = *mm;

	/*
	 * Update the translation tables if the xlat tables are initialized. If
	 * not, this region will be mapped when they are initialized.
	 */
	if (ctx->initialized) {
		end_va = xlat_tables_map_region(ctx, mm_cursor,
				0U, ctx->base_table, ctx->base_table_entries,
				ctx->base_level);
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
		xlat_clean_dcache_range((uintptr_t)ctx->base_table,
				   ctx->base_table_entries * sizeof(uint64_t));
#endif
		/* Failed to map, remove mmap entry, unmap and return error. */
		if (end_va != (mm_cursor->base_va + mm_cursor->size - 1U)) {
			(void)memmove(mm_cursor, mm_cursor + 1U,
				(uintptr_t)mm_last - (uintptr_t)mm_cursor);

			/*
			 * Check if the mapping function actually managed to map
			 * anything. If not, just return now.
			 */
			if (mm->base_va >= end_va)
				return -ENOMEM;

			/*
			 * Something went wrong after mapping some table
			 * entries, undo every change done up to this point.
			 */
			mmap_region_t unmap_mm = {
					.base_pa = 0U,
					.base_va = mm->base_va,
					.size = end_va - mm->base_va,
					.attr = 0U
			};
			xlat_tables_unmap_region(ctx, &unmap_mm, 0U,
				ctx->base_table, ctx->base_table_entries,
				ctx->base_level);
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
			xlat_clean_dcache_range((uintptr_t)ctx->base_table,
				ctx->base_table_entries * sizeof(uint64_t));
#endif
			return -ENOMEM;
		}

		/*
		 * Make sure that all entries are written to the memory. There
		 * is no need to invalidate entries when mapping dynamic regions
		 * because new table/block/page descriptors only replace old
		 * invalid descriptors, that aren't TLB cached.
		 */
		dsbishst();
	}

	if (end_pa > ctx->max_pa)
		ctx->max_pa = end_pa;
	if (end_va > ctx->max_va)
		ctx->max_va = end_va;

	return 0;
}

/*
 * Removes the region with given base Virtual Address and size from the given
 * context.
 *
 * Returns:
 *        0: Success.
 *   EINVAL: Invalid values were used as arguments (region not found).
 *    EPERM: Tried to remove a static region.
 */
int mmap_remove_dynamic_region_ctx(xlat_ctx_t *ctx, uintptr_t base_va,
				   size_t size)
{
	mmap_region_t *mm = ctx->mmap;
	const mmap_region_t *mm_last = mm + ctx->mmap_num;
	int update_max_va_needed = 0;
	int update_max_pa_needed = 0;

	/* Check sanity of mmap array. */
	assert(mm[ctx->mmap_num].size == 0U);

	while (mm->size != 0U) {
		if ((mm->base_va == base_va) && (mm->size == size))
			break;
		++mm;
	}

	/* Check that the region was found */
	if (mm->size == 0U)
		return -EINVAL;

	/* If the region is static it can't be removed */
	if ((mm->attr & MT_DYNAMIC) == 0U)
		return -EPERM;

	/* Check if this region is using the top VAs or PAs. */
	if ((mm->base_va + mm->size - 1U) == ctx->max_va)
		update_max_va_needed = 1;
	if ((mm->base_pa + mm->size - 1U) == ctx->max_pa)
		update_max_pa_needed = 1;

	/* Update the translation tables if needed */
	if (ctx->initialized) {
		xlat_tables_unmap_region(ctx, mm, 0U, ctx->base_table,
					 ctx->base_table_entries,
					 ctx->base_level);
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
		xlat_clean_dcache_range((uintptr_t)ctx->base_table,
			ctx->base_table_entries * sizeof(uint64_t));
#endif
		xlat_arch_tlbi_va_sync();
	}

	/* Remove this region by moving the rest down by one place. */
	(void)memmove(mm, mm + 1U, (uintptr_t)mm_last - (uintptr_t)mm);

	/* Check if we need to update the max VAs and PAs */
	if (update_max_va_needed == 1) {
		ctx->max_va = 0U;
		mm = ctx->mmap;
		while (mm->size != 0U) {
			if ((mm->base_va + mm->size - 1U) > ctx->max_va)
				ctx->max_va = mm->base_va + mm->size - 1U;
			++mm;
		}
	}

	if (update_max_pa_needed == 1) {
		ctx->max_pa = 0U;
		mm = ctx->mmap;
		while (mm->size != 0U) {
			if ((mm->base_pa + mm->size - 1U) > ctx->max_pa)
				ctx->max_pa = mm->base_pa + mm->size - 1U;
			++mm;
		}
	}

	return 0;
}

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

void init_xlat_tables_ctx(xlat_ctx_t *ctx)
{
	assert(ctx != NULL);
	assert(!ctx->initialized);
	assert((ctx->xlat_regime == EL3_REGIME) ||
	       (ctx->xlat_regime == EL2_REGIME) ||
	       (ctx->xlat_regime == EL1_EL0_REGIME));
	assert(!is_mmu_enabled_ctx(ctx));

	mmap_region_t *mm = ctx->mmap;

	xlat_mmap_print(mm);

	/* All tables must be zeroed before mapping any region. */

	for (unsigned int i = 0U; i < ctx->base_table_entries; i++)
		ctx->base_table[i] = INVALID_DESC;

	for (int j = 0; j < ctx->tables_num; j++) {
#if PLAT_XLAT_TABLES_DYNAMIC
		ctx->tables_mapped_regions[j] = 0;
#endif
		for (unsigned int i = 0U; i < XLAT_TABLE_ENTRIES; i++)
			ctx->tables[j][i] = INVALID_DESC;
	}

	while (mm->size != 0U) {
		uintptr_t end_va = xlat_tables_map_region(ctx, mm, 0U,
				ctx->base_table, ctx->base_table_entries,
				ctx->base_level);
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
		xlat_clean_dcache_range((uintptr_t)ctx->base_table,
				   ctx->base_table_entries * sizeof(uint64_t));
#endif
		if (end_va != (mm->base_va + mm->size - 1U)) {
			ERROR("Not enough memory to map region:\n"
			      " VA:0x%lx  PA:0x%llx  size:0x%zx  attr:0x%x\n",
			      mm->base_va, mm->base_pa, mm->size, mm->attr);
			panic();
		}

		mm++;
	}

	assert(ctx->pa_max_address <= xlat_arch_get_max_supported_pa());
	assert(ctx->max_va <= ctx->va_max_address);
	assert(ctx->max_pa <= ctx->pa_max_address);

	ctx->initialized = true;

	xlat_tables_print(ctx);
}
