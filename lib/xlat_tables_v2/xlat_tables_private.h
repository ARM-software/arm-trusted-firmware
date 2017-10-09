/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_PRIVATE_H__
#define __XLAT_TABLES_PRIVATE_H__

#include <platform_def.h>
#include <xlat_tables_defs.h>

#if PLAT_XLAT_TABLES_DYNAMIC
/*
 * Shifts and masks to access fields of an mmap_attr_t
 */
/* Dynamic or static */
#define MT_DYN_SHIFT		30 /* 31 would cause undefined behaviours */

/*
 * Memory mapping private attributes
 *
 * Private attributes not exposed in the mmap_attr_t enum.
 */
typedef enum  {
	/*
	 * Regions mapped before the MMU can't be unmapped dynamically (they are
	 * static) and regions mapped with MMU enabled can be unmapped. This
	 * behaviour can't be overridden.
	 *
	 * Static regions can overlap each other, dynamic regions can't.
	 */
	MT_STATIC	= 0 << MT_DYN_SHIFT,
	MT_DYNAMIC	= 1 << MT_DYN_SHIFT
} mmap_priv_attr_t;

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

/*
 * Invalidate all TLB entries that match the given virtual address. This
 * operation applies to all PEs in the same Inner Shareable domain as the PE
 * that executes this function. This functions must be called for every
 * translation table entry that is modified.
 *
 * xlat_arch_tlbi_va() applies the invalidation to the exception level of the
 * current translation regime, whereas xlat_arch_tlbi_va_regime() applies it to
 * the given translation regime.
 *
 * Note, however, that it is architecturally UNDEFINED to invalidate TLB entries
 * pertaining to a higher exception level, e.g. invalidating EL3 entries from
 * S-EL1.
 */
void xlat_arch_tlbi_va(uintptr_t va);
void xlat_arch_tlbi_va_regime(uintptr_t va, xlat_regime_t xlat_regime);

/*
 * This function has to be called at the end of any code that uses the function
 * xlat_arch_tlbi_va().
 */
void xlat_arch_tlbi_va_sync(void);

/* Print VA, PA, size and attributes of all regions in the mmap array. */
void print_mmap(mmap_region_t *const mmap);

/*
 * Print the current state of the translation tables by reading them from
 * memory.
 */
void xlat_tables_print(xlat_ctx_t *ctx);

/*
 * Architecture-specific initialization code.
 */

/* Returns the current Exception Level. The returned EL must be 1 or higher. */
int xlat_arch_current_el(void);

/*
 * Return the maximum physical address supported by the hardware.
 * This value depends on the execution state (AArch32/AArch64).
 */
unsigned long long xlat_arch_get_max_supported_pa(void);

/* Enable MMU and configure it to use the specified translation tables. */
void enable_mmu_arch(unsigned int flags, uint64_t *base_table,
		unsigned long long pa, uintptr_t max_va);

/*
 * Return 1 if the MMU of the translation regime managed by the given xlat_ctx_t
 * is enabled, 0 otherwise.
 */
int is_mmu_enabled_ctx(const xlat_ctx_t *ctx);

#endif /* __XLAT_TABLES_PRIVATE_H__ */
