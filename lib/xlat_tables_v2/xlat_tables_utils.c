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
#include <types.h>
#include <utils_def.h>
#include <xlat_tables_arch_private.h>
#include <xlat_tables_defs.h>
#include <xlat_tables_v2.h>

#include "xlat_tables_private.h"

#if LOG_LEVEL < LOG_LEVEL_VERBOSE

void xlat_mmap_print(__unused mmap_region_t *const mmap)
{
	/* Empty */
}

void xlat_tables_print(__unused xlat_ctx_t *ctx)
{
	/* Empty */
}

#else /* if LOG_LEVEL >= LOG_LEVEL_VERBOSE */

void xlat_mmap_print(mmap_region_t *const mmap)
{
	tf_printf("mmap:\n");
	const mmap_region_t *mm = mmap;

	while (mm->size != 0U) {
		tf_printf(" VA:0x%lx  PA:0x%llx  size:0x%zx  attr:0x%x "
			  "granularity:0x%zx\n", mm->base_va, mm->base_pa,
			  mm->size, mm->attr, mm->granularity);
		++mm;
	};
	tf_printf("\n");
}

/* Print the attributes of the specified block descriptor. */
static void xlat_desc_print(const xlat_ctx_t *ctx, uint64_t desc)
{
	int mem_type_index = ATTR_INDEX_GET(desc);
	int xlat_regime = ctx->xlat_regime;

	if (mem_type_index == ATTR_IWBWA_OWBWA_NTR_INDEX) {
		tf_printf("MEM");
	} else if (mem_type_index == ATTR_NON_CACHEABLE_INDEX) {
		tf_printf("NC");
	} else {
		assert(mem_type_index == ATTR_DEVICE_INDEX);
		tf_printf("DEV");
	}

	if (xlat_regime == EL3_REGIME) {
		/* For EL3 only check the AP[2] and XN bits. */
		tf_printf((desc & LOWER_ATTRS(AP_RO)) ? "-RO" : "-RW");
		tf_printf((desc & UPPER_ATTRS(XN)) ? "-XN" : "-EXEC");
	} else {
		assert(xlat_regime == EL1_EL0_REGIME);
		/*
		 * For EL0 and EL1:
		 * - In AArch64 PXN and UXN can be set independently but in
		 *   AArch32 there is no UXN (XN affects both privilege levels).
		 *   For consistency, we set them simultaneously in both cases.
		 * - RO and RW permissions must be the same in EL1 and EL0. If
		 *   EL0 can access that memory region, so can EL1, with the
		 *   same permissions.
		 */
#if ENABLE_ASSERTIONS
		uint64_t xn_mask = xlat_arch_regime_get_xn_desc(EL1_EL0_REGIME);
		uint64_t xn_perm = desc & xn_mask;

		assert((xn_perm == xn_mask) || (xn_perm == 0ULL));
#endif
		tf_printf((desc & LOWER_ATTRS(AP_RO)) ? "-RO" : "-RW");
		/* Only check one of PXN and UXN, the other one is the same. */
		tf_printf((desc & UPPER_ATTRS(PXN)) ? "-XN" : "-EXEC");
		/*
		 * Privileged regions can only be accessed from EL1, user
		 * regions can be accessed from EL1 and EL0.
		 */
		tf_printf((desc & LOWER_ATTRS(AP_ACCESS_UNPRIVILEGED))
			  ? "-USER" : "-PRIV");
	}

	tf_printf(LOWER_ATTRS(NS) & desc ? "-NS" : "-S");
}

static const char * const level_spacers[] = {
	"[LV0] ",
	"  [LV1] ",
	"    [LV2] ",
	"      [LV3] "
};

static const char *invalid_descriptors_ommited =
		"%s(%d invalid descriptors omitted)\n";

/*
 * Recursive function that reads the translation tables passed as an argument
 * and prints their status.
 */
static void xlat_tables_print_internal(xlat_ctx_t *ctx,
		const uintptr_t table_base_va,
		uint64_t *const table_base, const int table_entries,
		const unsigned int level)
{
	assert(level <= XLAT_TABLE_LEVEL_MAX);

	uint64_t desc;
	uintptr_t table_idx_va = table_base_va;
	int table_idx = 0;

	size_t level_size = XLAT_BLOCK_SIZE(level);

	/*
	 * Keep track of how many invalid descriptors are counted in a row.
	 * Whenever multiple invalid descriptors are found, only the first one
	 * is printed, and a line is added to inform about how many descriptors
	 * have been omitted.
	 */
	int invalid_row_count = 0;

	while (table_idx < table_entries) {

		desc = table_base[table_idx];

		if ((desc & DESC_MASK) == INVALID_DESC) {

			if (invalid_row_count == 0) {
				tf_printf("%sVA:%p size:0x%zx\n",
					  level_spacers[level],
					  (void *)table_idx_va, level_size);
			}
			invalid_row_count++;

		} else {

			if (invalid_row_count > 1) {
				tf_printf(invalid_descriptors_ommited,
					  level_spacers[level],
					  invalid_row_count - 1);
			}
			invalid_row_count = 0;

			/*
			 * Check if this is a table or a block. Tables are only
			 * allowed in levels other than 3, but DESC_PAGE has the
			 * same value as DESC_TABLE, so we need to check.
			 */
			if (((desc & DESC_MASK) == TABLE_DESC) &&
					(level < XLAT_TABLE_LEVEL_MAX)) {
				/*
				 * Do not print any PA for a table descriptor,
				 * as it doesn't directly map physical memory
				 * but instead points to the next translation
				 * table in the translation table walk.
				 */
				tf_printf("%sVA:%p size:0x%zx\n",
					  level_spacers[level],
					  (void *)table_idx_va, level_size);

				uintptr_t addr_inner = desc & TABLE_ADDR_MASK;

				xlat_tables_print_internal(ctx, table_idx_va,
					(uint64_t *)addr_inner,
					XLAT_TABLE_ENTRIES, level + 1);
			} else {
				tf_printf("%sVA:%p PA:0x%llx size:0x%zx ",
					  level_spacers[level],
					  (void *)table_idx_va,
					  (unsigned long long)(desc & TABLE_ADDR_MASK),
					  level_size);
				xlat_desc_print(ctx, desc);
				tf_printf("\n");
			}
		}

		table_idx++;
		table_idx_va += level_size;
	}

	if (invalid_row_count > 1) {
		tf_printf(invalid_descriptors_ommited,
			  level_spacers[level], invalid_row_count - 1);
	}
}

void xlat_tables_print(xlat_ctx_t *ctx)
{
	const char *xlat_regime_str;
	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		xlat_regime_str = "1&0";
	} else {
		assert(ctx->xlat_regime == EL3_REGIME);
		xlat_regime_str = "3";
	}
	VERBOSE("Translation tables state:\n");
	VERBOSE("  Xlat regime:     EL%s\n", xlat_regime_str);
	VERBOSE("  Max allowed PA:  0x%llx\n", ctx->pa_max_address);
	VERBOSE("  Max allowed VA:  %p\n", (void *) ctx->va_max_address);
	VERBOSE("  Max mapped PA:   0x%llx\n", ctx->max_pa);
	VERBOSE("  Max mapped VA:   %p\n", (void *) ctx->max_va);

	VERBOSE("  Initial lookup level: %i\n", ctx->base_level);
	VERBOSE("  Entries @initial lookup level: %i\n",
		ctx->base_table_entries);

	int used_page_tables;
#if PLAT_XLAT_TABLES_DYNAMIC
	used_page_tables = 0;
	for (unsigned int i = 0; i < ctx->tables_num; ++i) {
		if (ctx->tables_mapped_regions[i] != 0)
			++used_page_tables;
	}
#else
	used_page_tables = ctx->next_table;
#endif
	VERBOSE("  Used %i sub-tables out of %i (spare: %i)\n",
		used_page_tables, ctx->tables_num,
		ctx->tables_num - used_page_tables);

	xlat_tables_print_internal(ctx, 0, ctx->base_table,
				   ctx->base_table_entries, ctx->base_level);
}

#endif /* LOG_LEVEL >= LOG_LEVEL_VERBOSE */

/*
 * Do a translation table walk to find the block or page descriptor that maps
 * virtual_addr.
 *
 * On success, return the address of the descriptor within the translation
 * table. Its lookup level is stored in '*out_level'.
 * On error, return NULL.
 *
 * xlat_table_base
 *   Base address for the initial lookup level.
 * xlat_table_base_entries
 *   Number of entries in the translation table for the initial lookup level.
 * virt_addr_space_size
 *   Size in bytes of the virtual address space.
 */
static uint64_t *find_xlat_table_entry(uintptr_t virtual_addr,
				       void *xlat_table_base,
				       int xlat_table_base_entries,
				       unsigned long long virt_addr_space_size,
				       int *out_level)
{
	unsigned int start_level;
	uint64_t *table;
	int entries;

	VERBOSE("%s(%p)\n", __func__, (void *)virtual_addr);

	start_level = GET_XLAT_TABLE_LEVEL_BASE(virt_addr_space_size);
	VERBOSE("Starting translation table walk from level %i\n", start_level);

	table = xlat_table_base;
	entries = xlat_table_base_entries;

	for (unsigned int level = start_level;
	     level <= XLAT_TABLE_LEVEL_MAX;
	     ++level) {
		int idx;
		uint64_t desc;
		uint64_t desc_type;

		VERBOSE("Table address: %p\n", (void *)table);

		idx = XLAT_TABLE_IDX(virtual_addr, level);
		VERBOSE("Index into level %i table: %i\n", level, idx);
		if (idx >= entries) {
			VERBOSE("Invalid address\n");
			return NULL;
		}

		desc = table[idx];
		desc_type = desc & DESC_MASK;
		VERBOSE("Descriptor at level %i: 0x%llx\n", level,
				(unsigned long long)desc);

		if (desc_type == INVALID_DESC) {
			VERBOSE("Invalid entry (memory not mapped)\n");
			return NULL;
		}

		if (level == XLAT_TABLE_LEVEL_MAX) {
			/*
			 * There can't be table entries at the final lookup
			 * level.
			 */
			assert(desc_type == PAGE_DESC);
			VERBOSE("Descriptor mapping a memory page (size: 0x%llx)\n",
				(unsigned long long)XLAT_BLOCK_SIZE(XLAT_TABLE_LEVEL_MAX));
			*out_level = level;
			return &table[idx];
		}

		if (desc_type == BLOCK_DESC) {
			VERBOSE("Descriptor mapping a memory block (size: 0x%llx)\n",
				(unsigned long long)XLAT_BLOCK_SIZE(level));
			*out_level = level;
			return &table[idx];
		}

		assert(desc_type == TABLE_DESC);
		VERBOSE("Table descriptor, continuing xlat table walk...\n");
		table = (uint64_t *)(uintptr_t)(desc & TABLE_ADDR_MASK);
		entries = XLAT_TABLE_ENTRIES;
	}

	/*
	 * This shouldn't be reached, the translation table walk should end at
	 * most at level XLAT_TABLE_LEVEL_MAX and return from inside the loop.
	 */
	assert(0);

	return NULL;
}


static int get_mem_attributes_internal(const xlat_ctx_t *ctx, uintptr_t base_va,
		uint32_t *attributes, uint64_t **table_entry,
		unsigned long long *addr_pa, int *table_level)
{
	uint64_t *entry;
	uint64_t desc;
	int level;
	unsigned long long virt_addr_space_size;

	/*
	 * Sanity-check arguments.
	 */
	assert(ctx != NULL);
	assert(ctx->initialized);
	assert(ctx->xlat_regime == EL1_EL0_REGIME || ctx->xlat_regime == EL3_REGIME);

	virt_addr_space_size = (unsigned long long)ctx->va_max_address + 1;
	assert(virt_addr_space_size > 0);

	entry = find_xlat_table_entry(base_va,
				ctx->base_table,
				ctx->base_table_entries,
				virt_addr_space_size,
				&level);
	if (entry == NULL) {
		WARN("Address %p is not mapped.\n", (void *)base_va);
		return -EINVAL;
	}

	if (addr_pa != NULL) {
		*addr_pa = *entry & TABLE_ADDR_MASK;
	}

	if (table_entry != NULL) {
		*table_entry = entry;
	}

	if (table_level != NULL) {
		*table_level = level;
	}

	desc = *entry;

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	VERBOSE("Attributes: ");
	xlat_desc_print(ctx, desc);
	tf_printf("\n");
#endif /* LOG_LEVEL >= LOG_LEVEL_VERBOSE */

	assert(attributes != NULL);
	*attributes = 0;

	int attr_index = (desc >> ATTR_INDEX_SHIFT) & ATTR_INDEX_MASK;

	if (attr_index == ATTR_IWBWA_OWBWA_NTR_INDEX) {
		*attributes |= MT_MEMORY;
	} else if (attr_index == ATTR_NON_CACHEABLE_INDEX) {
		*attributes |= MT_NON_CACHEABLE;
	} else {
		assert(attr_index == ATTR_DEVICE_INDEX);
		*attributes |= MT_DEVICE;
	}

	int ap2_bit = (desc >> AP2_SHIFT) & 1;

	if (ap2_bit == AP2_RW)
		*attributes |= MT_RW;

	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		int ap1_bit = (desc >> AP1_SHIFT) & 1;
		if (ap1_bit == AP1_ACCESS_UNPRIVILEGED)
			*attributes |= MT_USER;
	}

	int ns_bit = (desc >> NS_SHIFT) & 1;

	if (ns_bit == 1)
		*attributes |= MT_NS;

	uint64_t xn_mask = xlat_arch_regime_get_xn_desc(ctx->xlat_regime);

	if ((desc & xn_mask) == xn_mask) {
		*attributes |= MT_EXECUTE_NEVER;
	} else {
		assert((desc & xn_mask) == 0);
	}

	return 0;
}


int get_mem_attributes(const xlat_ctx_t *ctx, uintptr_t base_va,
		       uint32_t *attributes)
{
	return get_mem_attributes_internal(ctx, base_va, attributes,
					   NULL, NULL, NULL);
}


int change_mem_attributes(xlat_ctx_t *ctx,
			uintptr_t base_va,
			size_t size,
			uint32_t attr)
{
	/* Note: This implementation isn't optimized. */

	assert(ctx != NULL);
	assert(ctx->initialized);

	unsigned long long virt_addr_space_size =
		(unsigned long long)ctx->va_max_address + 1;
	assert(virt_addr_space_size > 0);

	if (!IS_PAGE_ALIGNED(base_va)) {
		WARN("%s: Address %p is not aligned on a page boundary.\n",
		     __func__, (void *)base_va);
		return -EINVAL;
	}

	if (size == 0) {
		WARN("%s: Size is 0.\n", __func__);
		return -EINVAL;
	}

	if ((size % PAGE_SIZE) != 0) {
		WARN("%s: Size 0x%zx is not a multiple of a page size.\n",
		     __func__, size);
		return -EINVAL;
	}

	if (((attr & MT_EXECUTE_NEVER) == 0) && ((attr & MT_RW) != 0)) {
		WARN("%s() doesn't allow to remap memory as read-write and executable.\n",
		     __func__);
		return -EINVAL;
	}

	int pages_count = size / PAGE_SIZE;

	VERBOSE("Changing memory attributes of %i pages starting from address %p...\n",
		pages_count, (void *)base_va);

	uintptr_t base_va_original = base_va;

	/*
	 * Sanity checks.
	 */
	for (int i = 0; i < pages_count; ++i) {
		uint64_t *entry;
		uint64_t desc;
		int level;

		entry = find_xlat_table_entry(base_va,
					      ctx->base_table,
					      ctx->base_table_entries,
					      virt_addr_space_size,
					      &level);
		if (entry == NULL) {
			WARN("Address %p is not mapped.\n", (void *)base_va);
			return -EINVAL;
		}

		desc = *entry;

		/*
		 * Check that all the required pages are mapped at page
		 * granularity.
		 */
		if (((desc & DESC_MASK) != PAGE_DESC) ||
			(level != XLAT_TABLE_LEVEL_MAX)) {
			WARN("Address %p is not mapped at the right granularity.\n",
			     (void *)base_va);
			WARN("Granularity is 0x%llx, should be 0x%x.\n",
			     (unsigned long long)XLAT_BLOCK_SIZE(level), PAGE_SIZE);
			return -EINVAL;
		}

		/*
		 * If the region type is device, it shouldn't be executable.
		 */
		int attr_index = (desc >> ATTR_INDEX_SHIFT) & ATTR_INDEX_MASK;
		if (attr_index == ATTR_DEVICE_INDEX) {
			if ((attr & MT_EXECUTE_NEVER) == 0) {
				WARN("Setting device memory as executable at address %p.",
				     (void *)base_va);
				return -EINVAL;
			}
		}

		base_va += PAGE_SIZE;
	}

	/* Restore original value. */
	base_va = base_va_original;

	VERBOSE("%s: All pages are mapped, now changing their attributes...\n",
		__func__);

	for (int i = 0; i < pages_count; ++i) {

		uint32_t old_attr, new_attr;
		uint64_t *entry;
		int level;
		unsigned long long addr_pa;

		get_mem_attributes_internal(ctx, base_va, &old_attr,
					    &entry, &addr_pa, &level);

		VERBOSE("Old attributes: 0x%x\n", old_attr);

		/*
		 * From attr, only MT_RO/MT_RW, MT_EXECUTE/MT_EXECUTE_NEVER and
		 * MT_USER/MT_PRIVILEGED are taken into account. Any other
		 * information is ignored.
		 */

		/* Clean the old attributes so that they can be rebuilt. */
		new_attr = old_attr & ~(MT_RW|MT_EXECUTE_NEVER|MT_USER);

		/*
		 * Update attributes, but filter out the ones this function
		 * isn't allowed to change.
		 */
		new_attr |= attr & (MT_RW|MT_EXECUTE_NEVER|MT_USER);

		VERBOSE("New attributes: 0x%x\n", new_attr);

		/*
		 * The break-before-make sequence requires writing an invalid
		 * descriptor and making sure that the system sees the change
		 * before writing the new descriptor.
		 */
		*entry = INVALID_DESC;

		/* Invalidate any cached copy of this mapping in the TLBs. */
		xlat_arch_tlbi_va_regime(base_va, ctx->xlat_regime);

		/* Ensure completion of the invalidation. */
		xlat_arch_tlbi_va_sync();

		/* Write new descriptor */
		*entry = xlat_desc(ctx, new_attr, addr_pa, level);

		base_va += PAGE_SIZE;
	}

	/* Ensure that the last descriptor writen is seen by the system. */
	dsbish();

	return 0;
}
