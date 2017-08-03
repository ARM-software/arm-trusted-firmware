/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <utils.h>

/*
 * All the regions defined in mem_region_t must have the following properties
 *
 * - Any contiguous regions must be merged into a single entry.
 * - The number of bytes of each region must be greater than zero.
 * - The calculation of the highest address within the region (base + nbytes-1)
 *   doesn't produce an overflow.
 *
 * These conditions must be fulfilled by the caller and they aren't checked
 * at runtime.
 */

/*
 * zero_normalmem all the regions defined in tbl.
 * It assumes that MMU is enabled and the memory is Normal memory.
 * tbl must be a valid pointer to a memory mem_region_t array,
 * nregions is the size of the array.
 */
void clear_mem_regions(mem_region_t *tbl, size_t nregions)
{
	size_t i;

	assert(tbl);
	assert(nregions > 0);

	for (i = 0; i < nregions; i++) {
		assert(tbl->nbytes > 0);
		assert(!check_uptr_overflow(tbl->base, tbl->nbytes-1));
		zero_normalmem((void *) (tbl->base), tbl->nbytes);
		tbl++;
	}
}

/*
 * This function checks that a region (addr + nbytes-1) of memory is totally
 * covered by one of the regions defined in tbl.
 * tbl must be a valid pointer to a memory mem_region_t array, nregions
 * is the size of the array and the region described by addr and nbytes must
 * not generate an overflow.
 * Returns:
 *  -1 means that the region is not covered by any of the regions
 *     described in tbl.
 *   0 the region (addr + nbytes-1) is covered by one of the regions described
 *     in tbl
 */
int mem_region_in_array_chk(mem_region_t *tbl, size_t nregions,
			    uintptr_t addr, size_t nbytes)
{
	uintptr_t region_start, region_end, start, end;
	size_t i;

	assert(tbl);
	assert(nbytes > 0);
	assert(!check_uptr_overflow(addr, nbytes-1));

	region_start = addr;
	region_end = addr + (nbytes - 1);
	for (i = 0; i < nregions; i++) {
		assert(tbl->nbytes > 0);
		assert(!check_uptr_overflow(tbl->base, tbl->nbytes-1));
		start = tbl->base;
		end = start + (tbl->nbytes - 1);
		if (region_start >= start && region_end <= end)
			return 0;
		tbl++;
	}

	return -1;
}
