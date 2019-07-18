/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UTILS_H
#define UTILS_H

/*
 * C code should be put in this part of the header to avoid breaking ASM files
 * or linker scripts including it.
 */
#if !(defined(__LINKER__) || defined(__ASSEMBLER__))

#include <stddef.h>
#include <stdint.h>

typedef struct mem_region {
	uintptr_t base;
	size_t nbytes;
} mem_region_t;

/*
 * zero_normalmem all the regions defined in tbl.
 */
void clear_mem_regions(mem_region_t *tbl, size_t nregions);

/*
 * zero_normalmem all the regions defined in region. It dynamically
 * maps chunks of 'chunk_size' in 'va' virtual address and clears them.
 * For this reason memory regions must be multiple of chunk_size and
 * must be aligned to it as well. chunk_size and va can be selected
 * in a way that they minimize the number of entries used in the
 * translation tables.
 */
void clear_map_dyn_mem_regions(struct mem_region *regions,
			       size_t nregions,
			       uintptr_t va,
			       size_t chunk);

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

/*
 * Utility function to return the address of a symbol. By default, the
 * compiler generates adr/adrp instruction pair to return the reference
 * to the symbol and this utility is used to override this compiler
 * generated to code to use `ldr` instruction.
 *
 * This helps when Position Independent Executable needs to reference a symbol
 * which is constant and does not depend on the execute address of the binary.
 */
#define DEFINE_LOAD_SYM_ADDR(_name)		\
static inline u_register_t load_addr_## _name(void)			\
{									\
	u_register_t v;							\
	__asm__ volatile ("ldr %0, =" #_name : "=r" (v) : "X" (#_name));\
	return v;							\
}

/* Helper to invoke the function defined by DEFINE_LOAD_SYM_ADDR() */
#define LOAD_ADDR_OF(_name)	(typeof(_name) *) load_addr_## _name()

#endif /* !(defined(__LINKER__) || defined(__ASSEMBLER__)) */

#endif /* UTILS_H */
