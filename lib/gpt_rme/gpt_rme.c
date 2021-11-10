/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include "gpt_rme_private.h"
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#if !ENABLE_RME
#error "ENABLE_RME must be enabled to use the GPT library."
#endif

/*
 * Lookup T from PPS
 *
 *   PPS    Size    T
 *   0b000  4GB     32
 *   0b001  64GB    36
 *   0b010  1TB     40
 *   0b011  4TB     42
 *   0b100  16TB    44
 *   0b101  256TB   48
 *   0b110  4PB     52
 *
 * See section 15.1.27 of the RME specification.
 */
static const gpt_t_val_e gpt_t_lookup[] = {PPS_4GB_T, PPS_64GB_T,
					   PPS_1TB_T, PPS_4TB_T,
					   PPS_16TB_T, PPS_256TB_T,
					   PPS_4PB_T};

/*
 * Lookup P from PGS
 *
 *   PGS    Size    P
 *   0b00   4KB     12
 *   0b10   16KB    14
 *   0b01   64KB    16
 *
 * Note that pgs=0b10 is 16KB and pgs=0b01 is 64KB, this is not a typo.
 *
 * See section 15.1.27 of the RME specification.
 */
static const gpt_p_val_e gpt_p_lookup[] = {PGS_4KB_P, PGS_64KB_P, PGS_16KB_P};

/*
 * This structure contains GPT configuration data.
 */
typedef struct {
	uintptr_t plat_gpt_l0_base;
	gpccr_pps_e pps;
	gpt_t_val_e t;
	gpccr_pgs_e pgs;
	gpt_p_val_e p;
} gpt_config_t;

static gpt_config_t gpt_config;

/* These variables are used during initialization of the L1 tables. */
static unsigned int gpt_next_l1_tbl_idx;
static uintptr_t gpt_l1_tbl;

/*
 * This function checks to see if a GPI value is valid.
 *
 * These are valid GPI values.
 *   GPT_GPI_NO_ACCESS   U(0x0)
 *   GPT_GPI_SECURE      U(0x8)
 *   GPT_GPI_NS          U(0x9)
 *   GPT_GPI_ROOT        U(0xA)
 *   GPT_GPI_REALM       U(0xB)
 *   GPT_GPI_ANY         U(0xF)
 *
 * Parameters
 *   gpi		GPI to check for validity.
 *
 * Return
 *   true for a valid GPI, false for an invalid one.
 */
static bool gpt_is_gpi_valid(unsigned int gpi)
{
	if ((gpi == GPT_GPI_NO_ACCESS) || (gpi == GPT_GPI_ANY) ||
	    ((gpi >= GPT_GPI_SECURE) && (gpi <= GPT_GPI_REALM))) {
		return true;
	} else {
		return false;
	}
}

/*
 * This function checks to see if two PAS regions overlap.
 *
 * Parameters
 *   base_1: base address of first PAS
 *   size_1: size of first PAS
 *   base_2: base address of second PAS
 *   size_2: size of second PAS
 *
 * Return
 *   True if PAS regions overlap, false if they do not.
 */
static bool gpt_check_pas_overlap(uintptr_t base_1, size_t size_1,
				  uintptr_t base_2, size_t size_2)
{
	if (((base_1 + size_1) > base_2) && ((base_2 + size_2) > base_1)) {
		return true;
	} else {
		return false;
	}
}

/*
 * This helper function checks to see if a PAS region from index 0 to
 * (pas_idx - 1) occupies the L0 region at index l0_idx in the L0 table.
 *
 * Parameters
 *   l0_idx:      Index of the L0 entry to check
 *   pas_regions: PAS region array
 *   pas_idx:     Upper bound of the PAS array index.
 *
 * Return
 *   True if a PAS region occupies the L0 region in question, false if not.
 */
static bool gpt_does_previous_pas_exist_here(unsigned int l0_idx,
					     pas_region_t *pas_regions,
					     unsigned int pas_idx)
{
	/* Iterate over PAS regions up to pas_idx. */
	for (unsigned int i = 0U; i < pas_idx; i++) {
		if (gpt_check_pas_overlap((GPT_L0GPTSZ_ACTUAL_SIZE * l0_idx),
		    GPT_L0GPTSZ_ACTUAL_SIZE,
		    pas_regions[i].base_pa, pas_regions[i].size)) {
			return true;
		}
	}
	return false;
}

/*
 * This function iterates over all of the PAS regions and checks them to ensure
 * proper alignment of base and size, that the GPI is valid, and that no regions
 * overlap. As a part of the overlap checks, this function checks existing L0
 * mappings against the new PAS regions in the event that gpt_init_pas_l1_tables
 * is called multiple times to place L1 tables in different areas of memory. It
 * also counts the number of L1 tables needed and returns it on success.
 *
 * Parameters
 *   *pas_regions	Pointer to array of PAS region structures.
 *   pas_region_cnt	Total number of PAS regions in the array.
 *
 * Return
 *   Negative Linux error code in the event of a failure, number of L1 regions
 *   required when successful.
 */
static int gpt_validate_pas_mappings(pas_region_t *pas_regions,
				     unsigned int pas_region_cnt)
{
	unsigned int idx;
	unsigned int l1_cnt = 0U;
	unsigned int pas_l1_cnt;
	uint64_t *l0_desc = (uint64_t *)gpt_config.plat_gpt_l0_base;

	assert(pas_regions != NULL);
	assert(pas_region_cnt != 0U);

	for (idx = 0U; idx < pas_region_cnt; idx++) {
		/* Check for arithmetic overflow in region. */
		if ((ULONG_MAX - pas_regions[idx].base_pa) <
		    pas_regions[idx].size) {
			ERROR("[GPT] Address overflow in PAS[%u]!\n", idx);
			return -EOVERFLOW;
		}

		/* Initial checks for PAS validity. */
		if (((pas_regions[idx].base_pa + pas_regions[idx].size) >
		    GPT_PPS_ACTUAL_SIZE(gpt_config.t)) ||
		    !gpt_is_gpi_valid(GPT_PAS_ATTR_GPI(pas_regions[idx].attrs))) {
			ERROR("[GPT] PAS[%u] is invalid!\n", idx);
			return -EFAULT;
		}

		/*
		 * Make sure this PAS does not overlap with another one. We
		 * start from idx + 1 instead of 0 since prior PAS mappings will
		 * have already checked themselves against this one.
		 */
		for (unsigned int i = idx + 1; i < pas_region_cnt; i++) {
			if (gpt_check_pas_overlap(pas_regions[idx].base_pa,
			    pas_regions[idx].size,
			    pas_regions[i].base_pa,
			    pas_regions[i].size)) {
				ERROR("[GPT] PAS[%u] overlaps with PAS[%u]\n",
					i, idx);
				return -EFAULT;
			}
		}

		/*
		 * Since this function can be called multiple times with
		 * separate L1 tables we need to check the existing L0 mapping
		 * to see if this PAS would fall into one that has already been
		 * initialized.
		 */
		for (unsigned int i = GPT_L0_IDX(pas_regions[idx].base_pa);
		     i <= GPT_L0_IDX(pas_regions[idx].base_pa + pas_regions[idx].size - 1);
		     i++) {
			if ((GPT_L0_TYPE(l0_desc[i]) == GPT_L0_TYPE_BLK_DESC) &&
			    (GPT_L0_BLKD_GPI(l0_desc[i]) == GPT_GPI_ANY)) {
				/* This descriptor is unused so continue. */
				continue;
			}

			/*
			 * This descriptor has been initialized in a previous
			 * call to this function so cannot be initialized again.
			 */
			ERROR("[GPT] PAS[%u] overlaps with previous L0[%d]!\n",
			      idx, i);
			return -EFAULT;
		}

		/* Check for block mapping (L0) type. */
		if (GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs) ==
		    GPT_PAS_ATTR_MAP_TYPE_BLOCK) {
			/* Make sure base and size are block-aligned. */
			if (!GPT_IS_L0_ALIGNED(pas_regions[idx].base_pa) ||
			    !GPT_IS_L0_ALIGNED(pas_regions[idx].size)) {
				ERROR("[GPT] PAS[%u] is not block-aligned!\n",
				      idx);
				return -EFAULT;
			}

			continue;
		}

		/* Check for granule mapping (L1) type. */
		if (GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs) ==
		    GPT_PAS_ATTR_MAP_TYPE_GRANULE) {
			/* Make sure base and size are granule-aligned. */
			if (!GPT_IS_L1_ALIGNED(gpt_config.p, pas_regions[idx].base_pa) ||
			    !GPT_IS_L1_ALIGNED(gpt_config.p, pas_regions[idx].size)) {
				ERROR("[GPT] PAS[%u] is not granule-aligned!\n",
				      idx);
				return -EFAULT;
			}

			/* Find how many L1 tables this PAS occupies. */
			pas_l1_cnt = (GPT_L0_IDX(pas_regions[idx].base_pa +
				     pas_regions[idx].size - 1) -
				     GPT_L0_IDX(pas_regions[idx].base_pa) + 1);

			/*
			 * This creates a situation where, if multiple PAS
			 * regions occupy the same table descriptor, we can get
			 * an artificially high total L1 table count. The way we
			 * handle this is by checking each PAS against those
			 * before it in the array, and if they both occupy the
			 * same PAS we subtract from pas_l1_cnt and only the
			 * first PAS in the array gets to count it.
			 */

			/*
			 * If L1 count is greater than 1 we know the start and
			 * end PAs are in different L0 regions so we must check
			 * both for overlap against other PAS.
			 */
			if (pas_l1_cnt > 1) {
				if (gpt_does_previous_pas_exist_here(
				    GPT_L0_IDX(pas_regions[idx].base_pa +
				    pas_regions[idx].size - 1),
				    pas_regions, idx)) {
					pas_l1_cnt = pas_l1_cnt - 1;
				}
			}

			if (gpt_does_previous_pas_exist_here(
			    GPT_L0_IDX(pas_regions[idx].base_pa),
			    pas_regions, idx)) {
				pas_l1_cnt = pas_l1_cnt - 1;
			}

			l1_cnt += pas_l1_cnt;
			continue;
		}

		/* If execution reaches this point, mapping type is invalid. */
		ERROR("[GPT] PAS[%u] has invalid mapping type 0x%x.\n", idx,
		      GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs));
		return -EINVAL;
	}

	return l1_cnt;
}

/*
 * This function validates L0 initialization parameters.
 *
 * Parameters
 *   l0_mem_base	Base address of memory used for L0 tables.
 *   l1_mem_size	Size of memory available for L0 tables.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
static int gpt_validate_l0_params(gpccr_pps_e pps, uintptr_t l0_mem_base,
				  size_t l0_mem_size)
{
	size_t l0_alignment;

	/*
	 * Make sure PPS is valid and then store it since macros need this value
	 * to work.
	 */
	if (pps > GPT_PPS_MAX) {
		ERROR("[GPT] Invalid PPS: 0x%x\n", pps);
		return -EINVAL;
	}
	gpt_config.pps = pps;
	gpt_config.t = gpt_t_lookup[pps];

	/* Alignment must be the greater of 4k or l0 table size. */
	l0_alignment = PAGE_SIZE_4KB;
	if (l0_alignment < GPT_L0_TABLE_SIZE(gpt_config.t)) {
		l0_alignment = GPT_L0_TABLE_SIZE(gpt_config.t);
	}

	/* Check base address. */
	if ((l0_mem_base == 0U) || ((l0_mem_base & (l0_alignment - 1)) != 0U)) {
		ERROR("[GPT] Invalid L0 base address: 0x%lx\n", l0_mem_base);
		return -EFAULT;
	}

	/* Check size. */
	if (l0_mem_size < GPT_L0_TABLE_SIZE(gpt_config.t)) {
		ERROR("[GPT] Inadequate L0 memory: need 0x%lx, have 0x%lx)\n",
		      GPT_L0_TABLE_SIZE(gpt_config.t),
		      l0_mem_size);
		return -ENOMEM;
	}

	return 0;
}

/*
 * In the event that L1 tables are needed, this function validates
 * the L1 table generation parameters.
 *
 * Parameters
 *   l1_mem_base	Base address of memory used for L1 table allocation.
 *   l1_mem_size	Total size of memory available for L1 tables.
 *   l1_gpt_cnt		Number of L1 tables needed.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
static int gpt_validate_l1_params(uintptr_t l1_mem_base, size_t l1_mem_size,
				  unsigned int l1_gpt_cnt)
{
	size_t l1_gpt_mem_sz;

	/* Check if the granularity is supported */
	if (!xlat_arch_is_granule_size_supported(
	    GPT_PGS_ACTUAL_SIZE(gpt_config.p))) {
		return -EPERM;
	}

	/* Make sure L1 tables are aligned to their size. */
	if ((l1_mem_base & (GPT_L1_TABLE_SIZE(gpt_config.p) - 1)) != 0U) {
		ERROR("[GPT] Unaligned L1 GPT base address: 0x%lx\n",
		      l1_mem_base);
		return -EFAULT;
	}

	/* Get total memory needed for L1 tables. */
	l1_gpt_mem_sz = l1_gpt_cnt * GPT_L1_TABLE_SIZE(gpt_config.p);

	/* Check for overflow. */
	if ((l1_gpt_mem_sz / GPT_L1_TABLE_SIZE(gpt_config.p)) != l1_gpt_cnt) {
		ERROR("[GPT] Overflow calculating L1 memory size.\n");
		return -ENOMEM;
	}

	/* Make sure enough space was supplied. */
	if (l1_mem_size < l1_gpt_mem_sz) {
		ERROR("[GPT] Inadequate memory for L1 GPTs. ");
		ERROR("      Expected 0x%lx bytes. Got 0x%lx bytes\n",
		      l1_gpt_mem_sz, l1_mem_size);
		return -ENOMEM;
	}

	VERBOSE("[GPT] Requested 0x%lx bytes for L1 GPTs.\n", l1_gpt_mem_sz);
	return 0;
}

/*
 * This function initializes L0 block descriptors (regions that cannot be
 * transitioned at the granule level) according to the provided PAS.
 *
 * Parameters
 *   *pas		Pointer to the structure defining the PAS region to
 *			initialize.
 */
static void gpt_generate_l0_blk_desc(pas_region_t *pas)
{
	uint64_t gpt_desc;
	unsigned int end_idx;
	unsigned int idx;
	uint64_t *l0_gpt_arr;

	assert(gpt_config.plat_gpt_l0_base != 0U);
	assert(pas != NULL);

	/*
	 * Checking of PAS parameters has already been done in
	 * gpt_validate_pas_mappings so no need to check the same things again.
	 */

	l0_gpt_arr = (uint64_t *)gpt_config.plat_gpt_l0_base;

	/* Create the GPT Block descriptor for this PAS region */
	gpt_desc = GPT_L0_BLK_DESC(GPT_PAS_ATTR_GPI(pas->attrs));

	/* Start index of this region in L0 GPTs */
	idx = pas->base_pa >> GPT_L0_IDX_SHIFT;

	/*
	 * Determine number of L0 GPT descriptors covered by
	 * this PAS region and use the count to populate these
	 * descriptors.
	 */
	end_idx = (pas->base_pa + pas->size) >> GPT_L0_IDX_SHIFT;

	/* Generate the needed block descriptors. */
	for (; idx < end_idx; idx++) {
		l0_gpt_arr[idx] = gpt_desc;
		VERBOSE("[GPT] L0 entry (BLOCK) index %u [%p]: GPI = 0x%" PRIx64 " (0x%" PRIx64 ")\n",
			idx, &l0_gpt_arr[idx],
			(gpt_desc >> GPT_L0_BLK_DESC_GPI_SHIFT) &
			GPT_L0_BLK_DESC_GPI_MASK, l0_gpt_arr[idx]);
	}
}

/*
 * Helper function to determine if the end physical address lies in the same L0
 * region as the current physical address. If true, the end physical address is
 * returned else, the start address of the next region is returned.
 *
 * Parameters
 *   cur_pa		Physical address of the current PA in the loop through
 *			the range.
 *   end_pa		Physical address of the end PA in a PAS range.
 *
 * Return
 *   The PA of the end of the current range.
 */
static uintptr_t gpt_get_l1_end_pa(uintptr_t cur_pa, uintptr_t end_pa)
{
	uintptr_t cur_idx;
	uintptr_t end_idx;

	cur_idx = cur_pa >> GPT_L0_IDX_SHIFT;
	end_idx = end_pa >> GPT_L0_IDX_SHIFT;

	assert(cur_idx <= end_idx);

	if (cur_idx == end_idx) {
		return end_pa;
	}

	return (cur_idx + 1U) << GPT_L0_IDX_SHIFT;
}

/*
 * Helper function to fill out GPI entries in a single L1 table. This function
 * fills out entire L1 descriptors at a time to save memory writes.
 *
 * Parameters
 *   gpi		GPI to set this range to
 *   l1			Pointer to L1 table to fill out
 *   first		Address of first granule in range.
 *   last		Address of last granule in range (inclusive).
 */
static void gpt_fill_l1_tbl(uint64_t gpi, uint64_t *l1, uintptr_t first,
			    uintptr_t last)
{
	uint64_t gpi_field = GPT_BUILD_L1_DESC(gpi);
	uint64_t gpi_mask = 0xFFFFFFFFFFFFFFFF;

	assert(first <= last);
	assert((first & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1)) == 0U);
	assert((last & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1)) == 0U);
	assert(GPT_L0_IDX(first) == GPT_L0_IDX(last));
	assert(l1 != NULL);

	/* Shift the mask if we're starting in the middle of an L1 entry. */
	gpi_mask = gpi_mask << (GPT_L1_GPI_IDX(gpt_config.p, first) << 2);

	/* Fill out each L1 entry for this region. */
	for (unsigned int i = GPT_L1_IDX(gpt_config.p, first);
	     i <= GPT_L1_IDX(gpt_config.p, last); i++) {
		/* Account for stopping in the middle of an L1 entry. */
		if (i == GPT_L1_IDX(gpt_config.p, last)) {
			gpi_mask &= (gpi_mask >> ((15 -
				    GPT_L1_GPI_IDX(gpt_config.p, last)) << 2));
		}

		/* Write GPI values. */
		assert((l1[i] & gpi_mask) ==
		       (GPT_BUILD_L1_DESC(GPT_GPI_ANY) & gpi_mask));
		l1[i] = (l1[i] & ~gpi_mask) | (gpi_mask & gpi_field);

		/* Reset mask. */
		gpi_mask = 0xFFFFFFFFFFFFFFFF;
	}
}

/*
 * This function finds the next available unused L1 table and initializes all
 * granules descriptor entries to GPI_ANY. This ensures that there are no chunks
 * of GPI_NO_ACCESS (0b0000) memory floating around in the system in the
 * event that a PAS region stops midway through an L1 table, thus guaranteeing
 * that all memory not explicitly assigned is GPI_ANY. This function does not
 * check for overflow conditions, that should be done by the caller.
 *
 * Return
 *   Pointer to the next available L1 table.
 */
static uint64_t *gpt_get_new_l1_tbl(void)
{
	/* Retrieve the next L1 table. */
	uint64_t *l1 = (uint64_t *)((uint64_t)(gpt_l1_tbl) +
		       (GPT_L1_TABLE_SIZE(gpt_config.p) *
		       gpt_next_l1_tbl_idx));

	/* Increment L1 counter. */
	gpt_next_l1_tbl_idx++;

	/* Initialize all GPIs to GPT_GPI_ANY */
	for (unsigned int i = 0U; i < GPT_L1_ENTRY_COUNT(gpt_config.p); i++) {
		l1[i] = GPT_BUILD_L1_DESC(GPT_GPI_ANY);
	}

	return l1;
}

/*
 * When L1 tables are needed, this function creates the necessary L0 table
 * descriptors and fills out the L1 table entries according to the supplied
 * PAS range.
 *
 * Parameters
 *   *pas		Pointer to the structure defining the PAS region.
 */
static void gpt_generate_l0_tbl_desc(pas_region_t *pas)
{
	uintptr_t end_pa;
	uintptr_t cur_pa;
	uintptr_t last_gran_pa;
	uint64_t *l0_gpt_base;
	uint64_t *l1_gpt_arr;
	unsigned int l0_idx;

	assert(gpt_config.plat_gpt_l0_base != 0U);
	assert(pas != NULL);

	/*
	 * Checking of PAS parameters has already been done in
	 * gpt_validate_pas_mappings so no need to check the same things again.
	 */

	end_pa = pas->base_pa + pas->size;
	l0_gpt_base = (uint64_t *)gpt_config.plat_gpt_l0_base;

	/* We start working from the granule at base PA */
	cur_pa = pas->base_pa;

	/* Iterate over each L0 region in this memory range. */
	for (l0_idx = GPT_L0_IDX(pas->base_pa);
	     l0_idx <= GPT_L0_IDX(end_pa - 1U);
	     l0_idx++) {

		/*
		 * See if the L0 entry is already a table descriptor or if we
		 * need to create one.
		 */
		if (GPT_L0_TYPE(l0_gpt_base[l0_idx]) == GPT_L0_TYPE_TBL_DESC) {
			/* Get the L1 array from the L0 entry. */
			l1_gpt_arr = GPT_L0_TBLD_ADDR(l0_gpt_base[l0_idx]);
		} else {
			/* Get a new L1 table from the L1 memory space. */
			l1_gpt_arr = gpt_get_new_l1_tbl();

			/* Fill out the L0 descriptor and flush it. */
			l0_gpt_base[l0_idx] = GPT_L0_TBL_DESC(l1_gpt_arr);
		}

		VERBOSE("[GPT] L0 entry (TABLE) index %u [%p] ==> L1 Addr 0x%llx (0x%" PRIx64 ")\n",
			l0_idx, &l0_gpt_base[l0_idx],
			(unsigned long long)(l1_gpt_arr),
			l0_gpt_base[l0_idx]);

		/*
		 * Determine the PA of the last granule in this L0 descriptor.
		 */
		last_gran_pa = gpt_get_l1_end_pa(cur_pa, end_pa) -
			       GPT_PGS_ACTUAL_SIZE(gpt_config.p);

		/*
		 * Fill up L1 GPT entries between these two addresses. This
		 * function needs the addresses of the first granule and last
		 * granule in the range.
		 */
		gpt_fill_l1_tbl(GPT_PAS_ATTR_GPI(pas->attrs), l1_gpt_arr,
				cur_pa, last_gran_pa);

		/* Advance cur_pa to first granule in next L0 region. */
		cur_pa = gpt_get_l1_end_pa(cur_pa, end_pa);
	}
}

/*
 * This function flushes a range of L0 descriptors used by a given PAS region
 * array. There is a chance that some unmodified L0 descriptors would be flushed
 * in the case that there are "holes" in an array of PAS regions but overall
 * this should be faster than individually flushing each modified L0 descriptor
 * as they are created.
 *
 * Parameters
 *   *pas		Pointer to an array of PAS regions.
 *   pas_count		Number of entries in the PAS array.
 */
static void flush_l0_for_pas_array(pas_region_t *pas, unsigned int pas_count)
{
	unsigned int idx;
	unsigned int start_idx;
	unsigned int end_idx;
	uint64_t *l0 = (uint64_t *)gpt_config.plat_gpt_l0_base;

	assert(pas != NULL);
	assert(pas_count > 0);

	/* Initial start and end values. */
	start_idx = GPT_L0_IDX(pas[0].base_pa);
	end_idx = GPT_L0_IDX(pas[0].base_pa + pas[0].size - 1);

	/* Find lowest and highest L0 indices used in this PAS array. */
	for (idx = 1; idx < pas_count; idx++) {
		if (GPT_L0_IDX(pas[idx].base_pa) < start_idx) {
			start_idx = GPT_L0_IDX(pas[idx].base_pa);
		}
		if (GPT_L0_IDX(pas[idx].base_pa + pas[idx].size - 1) > end_idx) {
			end_idx = GPT_L0_IDX(pas[idx].base_pa + pas[idx].size - 1);
		}
	}

	/*
	 * Flush all covered L0 descriptors, add 1 because we need to include
	 * the end index value.
	 */
	flush_dcache_range((uintptr_t)&l0[start_idx],
			   ((end_idx + 1) - start_idx) * sizeof(uint64_t));
}

/*
 * Public API to enable granule protection checks once the tables have all been
 * initialized. This function is called at first initialization and then again
 * later during warm boots of CPU cores.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_enable(void)
{
	u_register_t gpccr_el3;

	/*
	 * Granule tables must be initialised before enabling
	 * granule protection.
	 */
	if (gpt_config.plat_gpt_l0_base == 0U) {
		ERROR("[GPT] Tables have not been initialized!\n");
		return -EPERM;
	}

	/* Invalidate any stale TLB entries */
	tlbipaallos();
	dsb();

	/* Write the base address of the L0 tables into GPTBR */
	write_gptbr_el3(((gpt_config.plat_gpt_l0_base >> GPTBR_BADDR_VAL_SHIFT)
			>> GPTBR_BADDR_SHIFT) & GPTBR_BADDR_MASK);

	/* GPCCR_EL3.PPS */
	gpccr_el3 = SET_GPCCR_PPS(gpt_config.pps);

	/* GPCCR_EL3.PGS */
	gpccr_el3 |= SET_GPCCR_PGS(gpt_config.pgs);

	/*
	 * Since EL3 maps the L1 region as Inner shareable, use the same
	 * shareability attribute for GPC as well so that
	 * GPC fetches are visible to PEs
	 */
	gpccr_el3 |= SET_GPCCR_SH(GPCCR_SH_IS);

	/* Outer and Inner cacheability set to Normal memory, WB, RA, WA. */
	gpccr_el3 |= SET_GPCCR_ORGN(GPCCR_ORGN_WB_RA_WA);
	gpccr_el3 |= SET_GPCCR_IRGN(GPCCR_IRGN_WB_RA_WA);

	/* Enable GPT */
	gpccr_el3 |= GPCCR_GPC_BIT;

	/* TODO: Configure GPCCR_EL3_GPCP for Fault control. */
	write_gpccr_el3(gpccr_el3);
	isb();
	tlbipaallos();
	dsb();
	isb();

	return 0;
}

/*
 * Public API to disable granule protection checks.
 */
void gpt_disable(void)
{
	u_register_t gpccr_el3 = read_gpccr_el3();

	write_gpccr_el3(gpccr_el3 & ~GPCCR_GPC_BIT);
	dsbsy();
	isb();
}

/*
 * Public API that initializes the entire protected space to GPT_GPI_ANY using
 * the L0 tables (block descriptors). Ideally, this function is invoked prior
 * to DDR discovery and initialization. The MMU must be initialized before
 * calling this function.
 *
 * Parameters
 *   pps		PPS value to use for table generation
 *   l0_mem_base	Base address of L0 tables in memory.
 *   l0_mem_size	Total size of memory available for L0 tables.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_init_l0_tables(unsigned int pps, uintptr_t l0_mem_base,
		       size_t l0_mem_size)
{
	int ret;
	uint64_t gpt_desc;

	/* Ensure that MMU and Data caches are enabled. */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0U);

	/* Validate other parameters. */
	ret = gpt_validate_l0_params(pps, l0_mem_base, l0_mem_size);
	if (ret < 0) {
		return ret;
	}

	/* Create the descriptor to initialize L0 entries with. */
	gpt_desc = GPT_L0_BLK_DESC(GPT_GPI_ANY);

	/* Iterate through all L0 entries */
	for (unsigned int i = 0U; i < GPT_L0_REGION_COUNT(gpt_config.t); i++) {
		((uint64_t *)l0_mem_base)[i] = gpt_desc;
	}

	/* Flush updated L0 tables to memory. */
	flush_dcache_range((uintptr_t)l0_mem_base,
			   (size_t)GPT_L0_TABLE_SIZE(gpt_config.t));

	/* Stash the L0 base address once initial setup is complete. */
	gpt_config.plat_gpt_l0_base = l0_mem_base;

	return 0;
}

/*
 * Public API that carves out PAS regions from the L0 tables and builds any L1
 * tables that are needed. This function ideally is run after DDR discovery and
 * initialization. The L0 tables must have already been initialized to GPI_ANY
 * when this function is called.
 *
 * This function can be called multiple times with different L1 memory ranges
 * and PAS regions if it is desirable to place L1 tables in different locations
 * in memory. (ex: you have multiple DDR banks and want to place the L1 tables
 * in the DDR bank that they control)
 *
 * Parameters
 *   pgs		PGS value to use for table generation.
 *   l1_mem_base	Base address of memory used for L1 tables.
 *   l1_mem_size	Total size of memory available for L1 tables.
 *   *pas_regions	Pointer to PAS regions structure array.
 *   pas_count		Total number of PAS regions.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_init_pas_l1_tables(gpccr_pgs_e pgs, uintptr_t l1_mem_base,
			   size_t l1_mem_size, pas_region_t *pas_regions,
			   unsigned int pas_count)
{
	int ret;
	int l1_gpt_cnt;

	/* Ensure that MMU and Data caches are enabled. */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0U);

	/* PGS is needed for gpt_validate_pas_mappings so check it now. */
	if (pgs > GPT_PGS_MAX) {
		ERROR("[GPT] Invalid PGS: 0x%x\n", pgs);
		return -EINVAL;
	}
	gpt_config.pgs = pgs;
	gpt_config.p = gpt_p_lookup[pgs];

	/* Make sure L0 tables have been initialized. */
	if (gpt_config.plat_gpt_l0_base == 0U) {
		ERROR("[GPT] L0 tables must be initialized first!\n");
		return -EPERM;
	}

	/* Check if L1 GPTs are required and how many. */
	l1_gpt_cnt = gpt_validate_pas_mappings(pas_regions, pas_count);
	if (l1_gpt_cnt < 0) {
		return l1_gpt_cnt;
	}

	VERBOSE("[GPT] %u L1 GPTs requested.\n", l1_gpt_cnt);

	/* If L1 tables are needed then validate the L1 parameters. */
	if (l1_gpt_cnt > 0) {
		ret = gpt_validate_l1_params(l1_mem_base, l1_mem_size,
		      l1_gpt_cnt);
		if (ret < 0) {
			return ret;
		}

		/* Set up parameters for L1 table generation. */
		gpt_l1_tbl = l1_mem_base;
		gpt_next_l1_tbl_idx = 0U;
	}

	INFO("[GPT] Boot Configuration\n");
	INFO("  PPS/T:     0x%x/%u\n", gpt_config.pps, gpt_config.t);
	INFO("  PGS/P:     0x%x/%u\n", gpt_config.pgs, gpt_config.p);
	INFO("  L0GPTSZ/S: 0x%x/%u\n", GPT_L0GPTSZ, GPT_S_VAL);
	INFO("  PAS count: 0x%x\n", pas_count);
	INFO("  L0 base:   0x%lx\n", gpt_config.plat_gpt_l0_base);

	/* Generate the tables in memory. */
	for (unsigned int idx = 0U; idx < pas_count; idx++) {
		INFO("[GPT] PAS[%u]: base 0x%lx, size 0x%lx, GPI 0x%x, type 0x%x\n",
		     idx, pas_regions[idx].base_pa, pas_regions[idx].size,
		     GPT_PAS_ATTR_GPI(pas_regions[idx].attrs),
		     GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs));

		/* Check if a block or table descriptor is required */
		if (GPT_PAS_ATTR_MAP_TYPE(pas_regions[idx].attrs) ==
		    GPT_PAS_ATTR_MAP_TYPE_BLOCK) {
			gpt_generate_l0_blk_desc(&pas_regions[idx]);

		} else {
			gpt_generate_l0_tbl_desc(&pas_regions[idx]);
		}
	}

	/* Flush modified L0 tables. */
	flush_l0_for_pas_array(pas_regions, pas_count);

	/* Flush L1 tables if needed. */
	if (l1_gpt_cnt > 0) {
		flush_dcache_range(l1_mem_base,
				   GPT_L1_TABLE_SIZE(gpt_config.p) *
				   l1_gpt_cnt);
	}

	/* Make sure that all the entries are written to the memory. */
	dsbishst();
	tlbipaallos();
	dsb();
	isb();

	return 0;
}

/*
 * Public API to initialize the runtime gpt_config structure based on the values
 * present in the GPTBR_EL3 and GPCCR_EL3 registers. GPT initialization
 * typically happens in a bootloader stage prior to setting up the EL3 runtime
 * environment for the granule transition service so this function detects the
 * initialization from a previous stage. Granule protection checks must be
 * enabled already or this function will return an error.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_runtime_init(void)
{
	u_register_t reg;

	/* Ensure that MMU and Data caches are enabled. */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0U);

	/* Ensure GPC are already enabled. */
	if ((read_gpccr_el3() & GPCCR_GPC_BIT) == 0U) {
		ERROR("[GPT] Granule protection checks are not enabled!\n");
		return -EPERM;
	}

	/*
	 * Read the L0 table address from GPTBR, we don't need the L1 base
	 * address since those are included in the L0 tables as needed.
	 */
	reg = read_gptbr_el3();
	gpt_config.plat_gpt_l0_base = ((reg >> GPTBR_BADDR_SHIFT) &
				      GPTBR_BADDR_MASK) <<
				      GPTBR_BADDR_VAL_SHIFT;

	/* Read GPCCR to get PGS and PPS values. */
	reg = read_gpccr_el3();
	gpt_config.pps = (reg >> GPCCR_PPS_SHIFT) & GPCCR_PPS_MASK;
	gpt_config.t = gpt_t_lookup[gpt_config.pps];
	gpt_config.pgs = (reg >> GPCCR_PGS_SHIFT) & GPCCR_PGS_MASK;
	gpt_config.p = gpt_p_lookup[gpt_config.pgs];

	VERBOSE("[GPT] Runtime Configuration\n");
	VERBOSE("  PPS/T:     0x%x/%u\n", gpt_config.pps, gpt_config.t);
	VERBOSE("  PGS/P:     0x%x/%u\n", gpt_config.pgs, gpt_config.p);
	VERBOSE("  L0GPTSZ/S: 0x%x/%u\n", GPT_L0GPTSZ, GPT_S_VAL);
	VERBOSE("  L0 base:   0x%lx\n", gpt_config.plat_gpt_l0_base);

	return 0;
}

/*
 * The L1 descriptors are protected by a spinlock to ensure that multiple
 * CPUs do not attempt to change the descriptors at once. In the future it
 * would be better to have separate spinlocks for each L1 descriptor.
 */
static spinlock_t gpt_lock;

/*
 * Check if caller is allowed to transition a PAS.
 *
 * - Secure world caller can only request S <-> NS transitions on a
 *   granule that is already in either S or NS PAS.
 *
 * - Realm world caller can only request R <-> NS transitions on a
 *   granule that is already in either R or NS PAS.
 *
 * Parameters
 *   src_sec_state	Security state of the caller.
 *   current_gpi	Current GPI of the granule.
 *   target_gpi		Requested new GPI for the granule.
 *
 * Return
 *   Negative Linux error code in the event of a failure, 0 for success.
 */
static int gpt_check_transition_gpi(unsigned int src_sec_state,
				    unsigned int current_gpi,
				    unsigned int target_gpi)
{
	unsigned int check_gpi;

	/* Cannot transition a granule to the state it is already in. */
	if (current_gpi == target_gpi) {
		return -EINVAL;
	}

	/* Check security state, only secure and realm can transition. */
	if (src_sec_state == SMC_FROM_REALM) {
		check_gpi = GPT_GPI_REALM;
	} else if (src_sec_state == SMC_FROM_SECURE) {
		check_gpi = GPT_GPI_SECURE;
	} else {
		return -EINVAL;
	}

	/* Make sure security state is allowed to make the transition. */
	if ((target_gpi != check_gpi) && (target_gpi != GPT_GPI_NS)) {
		return -EINVAL;
	}
	if ((current_gpi != check_gpi) && (current_gpi != GPT_GPI_NS)) {
		return -EINVAL;
	}

	return 0;
}

/*
 * This function is the core of the granule transition service. When a granule
 * transition request occurs it is routed to this function where the request is
 * validated then fulfilled if possible.
 *
 * TODO: implement support for transitioning multiple granules at once.
 *
 * Parameters
 *   base		Base address of the region to transition, must be
 *			aligned to granule size.
 *   size		Size of region to transition, must be aligned to granule
 *			size.
 *   src_sec_state	Security state of the caller.
 *   target_pas		Target PAS of the specified memory region.
 *
 * Return
 *    Negative Linux error code in the event of a failure, 0 for success.
 */
int gpt_transition_pas(uint64_t base, size_t size, unsigned int src_sec_state,
	unsigned int target_pas)
{
	int idx;
	unsigned int gpi_shift;
	unsigned int gpi;
	uint64_t gpt_l0_desc;
	uint64_t gpt_l1_desc;
	uint64_t *gpt_l1_addr;
	uint64_t *gpt_l0_base;

	/* Ensure that the tables have been set up before taking requests. */
	assert(gpt_config.plat_gpt_l0_base != 0U);

	/* Ensure that MMU and data caches are enabled. */
	assert((read_sctlr_el3() & SCTLR_C_BIT) != 0U);

	/* Check for address range overflow. */
	if ((ULONG_MAX - base) < size) {
		VERBOSE("[GPT] Transition request address overflow!\n");
		VERBOSE("      Base=0x%" PRIx64 "\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	/* Make sure base and size are valid. */
	if (((base & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1)) != 0U) ||
	    ((size & (GPT_PGS_ACTUAL_SIZE(gpt_config.p) - 1)) != 0U) ||
	    (size == 0U) ||
	    ((base + size) >= GPT_PPS_ACTUAL_SIZE(gpt_config.t))) {
		VERBOSE("[GPT] Invalid granule transition address range!\n");
		VERBOSE("      Base=0x%" PRIx64 "\n", base);
		VERBOSE("      Size=0x%lx\n", size);
		return -EINVAL;
	}

	/* See if this is a single granule transition or a range of granules. */
	if (size != GPT_PGS_ACTUAL_SIZE(gpt_config.p)) {
		/*
		 * TODO: Add support for transitioning multiple granules with a
		 * single call to this function.
		 */
		panic();
	}

	/* Get the L0 descriptor and make sure it is for a table. */
	gpt_l0_base = (uint64_t *)gpt_config.plat_gpt_l0_base;
	gpt_l0_desc = gpt_l0_base[GPT_L0_IDX(base)];
	if (GPT_L0_TYPE(gpt_l0_desc) != GPT_L0_TYPE_TBL_DESC) {
		VERBOSE("[GPT] Granule is not covered by a table descriptor!\n");
		VERBOSE("      Base=0x%" PRIx64 "\n", base);
		return -EINVAL;
	}

	/* Get the table index and GPI shift from PA. */
	gpt_l1_addr = GPT_L0_TBLD_ADDR(gpt_l0_desc);
	idx = GPT_L1_IDX(gpt_config.p, base);
	gpi_shift = GPT_L1_GPI_IDX(gpt_config.p, base) << 2;

	/*
	 * Access to L1 tables is controlled by a global lock to ensure
	 * that no more than one CPU is allowed to make changes at any
	 * given time.
	 */
	spin_lock(&gpt_lock);
	gpt_l1_desc = gpt_l1_addr[idx];
	gpi = (gpt_l1_desc >> gpi_shift) & GPT_L1_GRAN_DESC_GPI_MASK;

	/* Make sure caller state and source/target PAS are allowed. */
	if (gpt_check_transition_gpi(src_sec_state, gpi, target_pas) < 0) {
		spin_unlock(&gpt_lock);
			VERBOSE("[GPT] Invalid caller state and PAS combo!\n");
		VERBOSE("      Caller: %u, Current GPI: %u, Target GPI: %u\n",
			src_sec_state, gpi, target_pas);
		return -EPERM;
	}

	/* Clear existing GPI encoding and transition granule. */
	gpt_l1_desc &= ~(GPT_L1_GRAN_DESC_GPI_MASK << gpi_shift);
	gpt_l1_desc |= ((uint64_t)target_pas << gpi_shift);
	gpt_l1_addr[idx] = gpt_l1_desc;

	/* Ensure that the write operation will be observed by GPC */
	dsbishst();

	/* Unlock access to the L1 tables. */
	spin_unlock(&gpt_lock);

	gpt_tlbi_by_pa(base, GPT_PGS_ACTUAL_SIZE(gpt_config.p));
	dsbishst();
	/*
	 * The isb() will be done as part of context
	 * synchronization when returning to lower EL
	 */
	VERBOSE("[GPT] Granule 0x%" PRIx64 ", GPI 0x%x->0x%x\n", base, gpi,
		target_pas);

	return 0;
}
