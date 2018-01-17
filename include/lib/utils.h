/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#if !ERROR_DEPRECATED
#include <utils_def.h>
#endif

/*
 * C code should be put in this part of the header to avoid breaking ASM files
 * or linker scripts including it.
 */
#if !(defined(__LINKER__) || defined(__ASSEMBLY__))

#include <types.h>

typedef struct mem_region {
	uintptr_t base;
	size_t nbytes;
} mem_region_t;

/*
 * zero_normalmem all the regions defined in tbl.
 */
void clear_mem_regions(mem_region_t *tbl, size_t nregions);


/*
 * checks that a region (addr + nbytes-1) of memory is totally covered by
 * one of the regions defined in tbl. Caller must ensure that (addr+nbytes-1)
 * doesn't overflow.
 */
int mem_region_in_array_chk(mem_region_t *tbl, size_t nregions,
			    uintptr_t addr, size_t nbytes);

/*
 * Fill a region of normal memory of size "length" in bytes with zero bytes.
 *
 * WARNING: This function can only operate on normal memory. This means that
 *          the MMU must be enabled when using this function. Otherwise, use
 *          zeromem.
 */
void zero_normalmem(void *mem, u_register_t length);

/*
 * Fill a region of memory of size "length" in bytes with null bytes.
 *
 * Unlike zero_normalmem, this function has no restriction on the type of
 * memory targeted and can be used for any device memory as well as normal
 * memory. This function must be used instead of zero_normalmem when MMU is
 * disabled.
 *
 * NOTE: When data cache and MMU are enabled, prefer zero_normalmem for faster
 *       zeroing.
 */
void zeromem(void *mem, u_register_t length);
#endif /* !(defined(__LINKER__) || defined(__ASSEMBLY__)) */

#endif /* __UTILS_H__ */
