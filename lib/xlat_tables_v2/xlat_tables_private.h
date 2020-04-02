/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_TABLES_PRIVATE_H
#define XLAT_TABLES_PRIVATE_H

#include <stdbool.h>

#include <platform_def.h>

#include <lib/xlat_tables/xlat_tables_defs.h>

#if PLAT_XLAT_TABLES_DYNAMIC
/*
 * Private shifts and masks to access fields of an mmap attribute
 */
/* Dynamic or static */
#define MT_DYN_SHIFT		U(31)

/*
 * Memory mapping private attributes
 *
 * Private attributes not exposed in the public header.
 */

/*
 * Regions mapped before the MMU can't be unmapped dynamically (they are
 * static) and regions mapped with MMU enabled can be unmapped. This
 * behaviour can't be overridden.
 *
 * Static regions can overlap each other, dynamic regions can't.
 */
#define MT_STATIC	(U(0) << MT_DYN_SHIFT)
#define MT_DYNAMIC	(U(1) << MT_DYN_SHIFT)

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

extern uint64_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

/*
 * Return the execute-never mask that will prevent instruction fetch at the
 * given translation regime.
 */
uint64_t xlat_arch_regime_get_xn_desc(int xlat_regime);

/*
 * Invalidate all TLB entries that match the given virtual address. This
 * operation applies to all PEs in the same Inner Shareable domain as the PE
 * that executes this function. This functions must be called for every
 * translation table entry that is modified. It only affects the specified
 * translation regime.
 *
 * Note, however, that it is architecturally UNDEFINED to invalidate TLB entries
 * pertaining to a higher exception level, e.g. invalidating EL3 entries from
 * S-EL1.
 */
void xlat_arch_tlbi_va(uintptr_t va, int xlat_regime);

/*
 * This function has to be called at the end of any code that uses the function
 * xlat_arch_tlbi_va().
 */
void xlat_arch_tlbi_va_sync(void);

/* Print VA, PA, size and attributes of all regions in the mmap array. */
void xlat_mmap_print(const mmap_region_t *mmap);

/*
 * Print the current state of the translation tables by reading them from
 * memory.
 */
void xlat_tables_print(xlat_ctx_t *ctx);

/*
 * Returns a block/page table descriptor for the given level and attributes.
 */
uint64_t xlat_desc(const xlat_ctx_t *ctx, uint32_t attr,
		   unsigned long long addr_pa, unsigned int level);

/*
 * Architecture-specific initialization code.
 */

/* Returns the current Exception Level. The returned EL must be 1 or higher. */
unsigned int xlat_arch_current_el(void);

/*
 * Return the maximum physical address supported by the hardware.
 * This value depends on the execution state (AArch32/AArch64).
 */
unsigned long long xlat_arch_get_max_supported_pa(void);

/*
 * Returns true if the MMU of the translation regime managed by the given
 * xlat_ctx_t is enabled, false otherwise.
 */
bool is_mmu_enabled_ctx(const xlat_ctx_t *ctx);

/*
 * Returns minimum virtual address space size supported by the architecture
 */
uintptr_t xlat_get_min_virt_addr_space_size(void);

#endif /* XLAT_TABLES_PRIVATE_H */
