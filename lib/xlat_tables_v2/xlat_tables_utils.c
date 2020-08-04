/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "xlat_tables_private.h"

#if LOG_LEVEL < LOG_LEVEL_VERBOSE

void xlat_mmap_print(__unused const mmap_region_t *mmap)
{
	/* Empty */
}

void xlat_tables_print(__unused xlat_ctx_t *ctx)
{
	/* Empty */
}

#else /* if LOG_LEVEL >= LOG_LEVEL_VERBOSE */

void xlat_mmap_print(const mmap_region_t *mmap)
{
	printf("mmap:\n");
	const mmap_region_t *mm = mmap;

	while (mm->size != 0U) {
		printf(" VA:0x%lx  PA:0x%llx  size:0x%zx  attr:0x%x  granularity:0x%zx\n",
		       mm->base_va, mm->base_pa, mm->size, mm->attr,
		       mm->granularity);
		++mm;
	};
	printf("\n");
}

/* Print the attributes of the specified block descriptor. */
static void xlat_desc_print(const xlat_ctx_t *ctx, uint64_t desc)
{
	uint64_t mem_type_index = ATTR_INDEX_GET(desc);
	int xlat_regime = ctx->xlat_regime;

	if (mem_type_index == ATTR_IWBWA_OWBWA_NTR_INDEX) {
		printf("MEM");
	} else if (mem_type_index == ATTR_NON_CACHEABLE_INDEX) {
		printf("NC");
	} else {
		assert(mem_type_index == ATTR_DEVICE_INDEX);
		printf("DEV");
	}

	if ((xlat_regime == EL3_REGIME) || (xlat_regime == EL2_REGIME)) {
		/* For EL3 and EL2 only check the AP[2] and XN bits. */
		printf(((desc & LOWER_ATTRS(AP_RO)) != 0ULL) ? "-RO" : "-RW");
		printf(((desc & UPPER_ATTRS(XN)) != 0ULL) ? "-XN" : "-EXEC");
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
		printf(((desc & LOWER_ATTRS(AP_RO)) != 0ULL) ? "-RO" : "-RW");
		/* Only check one of PXN and UXN, the other one is the same. */
		printf(((desc & UPPER_ATTRS(PXN)) != 0ULL) ? "-XN" : "-EXEC");
		/*
		 * Privileged regions can only be accessed from EL1, user
		 * regions can be accessed from EL1 and EL0.
		 */
		printf(((desc & LOWER_ATTRS(AP_ACCESS_UNPRIVILEGED)) != 0ULL)
			  ? "-USER" : "-PRIV");
	}

	printf(((LOWER_ATTRS(NS) & desc) != 0ULL) ? "-NS" : "-S");

#ifdef __aarch64__
	/* Check Guarded Page bit */
	if ((desc & GP) != 0ULL) {
		printf("-GP");
	}
#endif
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
static void xlat_tables_print_internal(xlat_ctx_t *ctx, uintptr_t table_base_va,
		const uint64_t *table_base, unsigned int table_entries,
		unsigned int level)
{
	assert(level <= XLAT_TABLE_LEVEL_MAX);

	uint64_t desc;
	uintptr_t table_idx_va = table_base_va;
	unsigned int table_idx = 0U;
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
				printf("%sVA:0x%lx size:0x%zx\n",
				       level_spacers[level],
				       table_idx_va, level_size);
			}
			invalid_row_count++;

		} else {

			if (invalid_row_count > 1) {
				printf(invalid_descriptors_ommited,
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
				printf("%sVA:0x%lx size:0x%zx\n",
				       level_spacers[level],
				       table_idx_va, level_size);

				uintptr_t addr_inner = desc & TABLE_ADDR_MASK;

				xlat_tables_print_internal(ctx, table_idx_va,
					(uint64_t *)addr_inner,
					XLAT_TABLE_ENTRIES, level + 1U);
			} else {
				printf("%sVA:0x%lx PA:0x%llx size:0x%zx ",
				       level_spacers[level], table_idx_va,
				       (uint64_t)(desc & TABLE_ADDR_MASK),
				       level_size);
				xlat_desc_print(ctx, desc);
				printf("\n");
			}
		}

		table_idx++;
		table_idx_va += level_size;
	}

	if (invalid_row_count > 1) {
		printf(invalid_descriptors_ommited,
		       level_spacers[level], invalid_row_count - 1);
	}
}

void xlat_tables_print(xlat_ctx_t *ctx)
{
	const char *xlat_regime_str;
	int used_page_tables;

	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		xlat_regime_str = "1&0";
	} else if (ctx->xlat_regime == EL2_REGIME) {
		xlat_regime_str = "2";
	} else {
		assert(ctx->xlat_regime == EL3_REGIME);
		xlat_regime_str = "3";
	}
	VERBOSE("Translation tables state:\n");
	VERBOSE("  Xlat regime:     EL%s\n", xlat_regime_str);
	VERBOSE("  Max allowed PA:  0x%llx\n", ctx->pa_max_address);
	VERBOSE("  Max allowed VA:  0x%lx\n", ctx->va_max_address);
	VERBOSE("  Max mapped PA:   0x%llx\n", ctx->max_pa);
	VERBOSE("  Max mapped VA:   0x%lx\n", ctx->max_va);

	VERBOSE("  Initial lookup level: %u\n", ctx->base_level);
	VERBOSE("  Entries @initial lookup level: %u\n",
		ctx->base_table_entries);

#if PLAT_XLAT_TABLES_DYNAMIC
	used_page_tables = 0;
	for (int i = 0; i < ctx->tables_num; ++i) {
		if (ctx->tables_mapped_regions[i] != 0)
			++used_page_tables;
	}
#else
	used_page_tables = ctx->next_table;
#endif
	VERBOSE("  Used %d sub-tables out of %d (spare: %d)\n",
		used_page_tables, ctx->tables_num,
		ctx->tables_num - used_page_tables);

	xlat_tables_print_internal(ctx, 0U, ctx->base_table,
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
				       unsigned int xlat_table_base_entries,
				       unsigned long long virt_addr_space_size,
				       unsigned int *out_level)
{
	unsigned int start_level;
	uint64_t *table;
	unsigned int entries;

	start_level = GET_XLAT_TABLE_LEVEL_BASE(virt_addr_space_size);

	table = xlat_table_base;
	entries = xlat_table_base_entries;

	for (unsigned int level = start_level;
	     level <= XLAT_TABLE_LEVEL_MAX;
	     ++level) {
		uint64_t idx, desc, desc_type;

		idx = XLAT_TABLE_IDX(virtual_addr, level);
		if (idx >= entries) {
			WARN("Missing xlat table entry at address 0x%lx\n",
			     virtual_addr);
			return NULL;
		}

		desc = table[idx];
		desc_type = desc & DESC_MASK;

		if (desc_type == INVALID_DESC) {
			VERBOSE("Invalid entry (memory not mapped)\n");
			return NULL;
		}

		if (level == XLAT_TABLE_LEVEL_MAX) {
			/*
			 * Only page descriptors allowed at the final lookup
			 * level.
			 */
			assert(desc_type == PAGE_DESC);
			*out_level = level;
			return &table[idx];
		}

		if (desc_type == BLOCK_DESC) {
			*out_level = level;
			return &table[idx];
		}

		assert(desc_type == TABLE_DESC);
		table = (uint64_t *)(uintptr_t)(desc & TABLE_ADDR_MASK);
		entries = XLAT_TABLE_ENTRIES;
	}

	/*
	 * This shouldn't be reached, the translation table walk should end at
	 * most at level XLAT_TABLE_LEVEL_MAX and return from inside the loop.
	 */
	assert(false);

	return NULL;
}


static int xlat_get_mem_attributes_internal(const xlat_ctx_t *ctx,
		uintptr_t base_va, uint32_t *attributes, uint64_t **table_entry,
		unsigned long long *addr_pa, unsigned int *table_level)
{
	uint64_t *entry;
	uint64_t desc;
	unsigned int level;
	unsigned long long virt_addr_space_size;

	/*
	 * Sanity-check arguments.
	 */
	assert(ctx != NULL);
	assert(ctx->initialized);
	assert((ctx->xlat_regime == EL1_EL0_REGIME) ||
	       (ctx->xlat_regime == EL2_REGIME) ||
	       (ctx->xlat_regime == EL3_REGIME));

	virt_addr_space_size = (unsigned long long)ctx->va_max_address + 1ULL;
	assert(virt_addr_space_size > 0U);

	entry = find_xlat_table_entry(base_va,
				ctx->base_table,
				ctx->base_table_entries,
				virt_addr_space_size,
				&level);
	if (entry == NULL) {
		WARN("Address 0x%lx is not mapped.\n", base_va);
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
	printf("\n");
#endif /* LOG_LEVEL >= LOG_LEVEL_VERBOSE */

	assert(attributes != NULL);
	*attributes = 0U;

	uint64_t attr_index = (desc >> ATTR_INDEX_SHIFT) & ATTR_INDEX_MASK;

	if (attr_index == ATTR_IWBWA_OWBWA_NTR_INDEX) {
		*attributes |= MT_MEMORY;
	} else if (attr_index == ATTR_NON_CACHEABLE_INDEX) {
		*attributes |= MT_NON_CACHEABLE;
	} else {
		assert(attr_index == ATTR_DEVICE_INDEX);
		*attributes |= MT_DEVICE;
	}

	uint64_t ap2_bit = (desc >> AP2_SHIFT) & 1U;

	if (ap2_bit == AP2_RW)
		*attributes |= MT_RW;

	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		uint64_t ap1_bit = (desc >> AP1_SHIFT) & 1U;

		if (ap1_bit == AP1_ACCESS_UNPRIVILEGED)
			*attributes |= MT_USER;
	}

	uint64_t ns_bit = (desc >> NS_SHIFT) & 1U;

	if (ns_bit == 1U)
		*attributes |= MT_NS;

	uint64_t xn_mask = xlat_arch_regime_get_xn_desc(ctx->xlat_regime);

	if ((desc & xn_mask) == xn_mask) {
		*attributes |= MT_EXECUTE_NEVER;
	} else {
		assert((desc & xn_mask) == 0U);
	}

	return 0;
}


int xlat_get_mem_attributes_ctx(const xlat_ctx_t *ctx, uintptr_t base_va,
				uint32_t *attr)
{
	return xlat_get_mem_attributes_internal(ctx, base_va, attr,
				NULL, NULL, NULL);
}


int xlat_change_mem_attributes_ctx(const xlat_ctx_t *ctx, uintptr_t base_va,
				   size_t size, uint32_t attr)
{
	/* Note: This implementation isn't optimized. */

	assert(ctx != NULL);
	assert(ctx->initialized);

	unsigned long long virt_addr_space_size =
		(unsigned long long)ctx->va_max_address + 1U;
	assert(virt_addr_space_size > 0U);

	if (!IS_PAGE_ALIGNED(base_va)) {
		WARN("%s: Address 0x%lx is not aligned on a page boundary.\n",
		     __func__, base_va);
		return -EINVAL;
	}

	if (size == 0U) {
		WARN("%s: Size is 0.\n", __func__);
		return -EINVAL;
	}

	if ((size % PAGE_SIZE) != 0U) {
		WARN("%s: Size 0x%zx is not a multiple of a page size.\n",
		     __func__, size);
		return -EINVAL;
	}

	if (((attr & MT_EXECUTE_NEVER) == 0U) && ((attr & MT_RW) != 0U)) {
		WARN("%s: Mapping memory as read-write and executable not allowed.\n",
		     __func__);
		return -EINVAL;
	}

	size_t pages_count = size / PAGE_SIZE;

	VERBOSE("Changing memory attributes of %zu pages starting from address 0x%lx...\n",
		pages_count, base_va);

	uintptr_t base_va_original = base_va;

	/*
	 * Sanity checks.
	 */
	for (unsigned int i = 0U; i < pages_count; ++i) {
		const uint64_t *entry;
		uint64_t desc, attr_index;
		unsigned int level;

		entry = find_xlat_table_entry(base_va,
					      ctx->base_table,
					      ctx->base_table_entries,
					      virt_addr_space_size,
					      &level);
		if (entry == NULL) {
			WARN("Address 0x%lx is not mapped.\n", base_va);
			return -EINVAL;
		}

		desc = *entry;

		/*
		 * Check that all the required pages are mapped at page
		 * granularity.
		 */
		if (((desc & DESC_MASK) != PAGE_DESC) ||
			(level != XLAT_TABLE_LEVEL_MAX)) {
			WARN("Address 0x%lx is not mapped at the right granularity.\n",
			     base_va);
			WARN("Granularity is 0x%lx, should be 0x%lx.\n",
			     XLAT_BLOCK_SIZE(level), PAGE_SIZE);
			return -EINVAL;
		}

		/*
		 * If the region type is device, it shouldn't be executable.
		 */
		attr_index = (desc >> ATTR_INDEX_SHIFT) & ATTR_INDEX_MASK;
		if (attr_index == ATTR_DEVICE_INDEX) {
			if ((attr & MT_EXECUTE_NEVER) == 0U) {
				WARN("Setting device memory as executable at address 0x%lx.",
				     base_va);
				return -EINVAL;
			}
		}

		base_va += PAGE_SIZE;
	}

	/* Restore original value. */
	base_va = base_va_original;

	for (unsigned int i = 0U; i < pages_count; ++i) {

		uint32_t old_attr = 0U, new_attr;
		uint64_t *entry = NULL;
		unsigned int level = 0U;
		unsigned long long addr_pa = 0ULL;

		(void) xlat_get_mem_attributes_internal(ctx, base_va, &old_attr,
					    &entry, &addr_pa, &level);

		/*
		 * From attr, only MT_RO/MT_RW, MT_EXECUTE/MT_EXECUTE_NEVER and
		 * MT_USER/MT_PRIVILEGED are taken into account. Any other
		 * information is ignored.
		 */

		/* Clean the old attributes so that they can be rebuilt. */
		new_attr = old_attr & ~(MT_RW | MT_EXECUTE_NEVER | MT_USER);

		/*
		 * Update attributes, but filter out the ones this function
		 * isn't allowed to change.
		 */
		new_attr |= attr & (MT_RW | MT_EXECUTE_NEVER | MT_USER);

		/*
		 * The break-before-make sequence requires writing an invalid
		 * descriptor and making sure that the system sees the change
		 * before writing the new descriptor.
		 */
		*entry = INVALID_DESC;
#if !HW_ASSISTED_COHERENCY
		dccvac((uintptr_t)entry);
#endif
		/* Invalidate any cached copy of this mapping in the TLBs. */
		xlat_arch_tlbi_va(base_va, ctx->xlat_regime);

		/* Ensure completion of the invalidation. */
		xlat_arch_tlbi_va_sync();

		/* Write new descriptor */
		*entry = xlat_desc(ctx, new_attr, addr_pa, level);
#if !HW_ASSISTED_COHERENCY
		dccvac((uintptr_t)entry);
#endif
		base_va += PAGE_SIZE;
	}

	/* Ensure that the last descriptor writen is seen by the system. */
	dsbish();

	return 0;
}
