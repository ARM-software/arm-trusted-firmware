/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_TABLES_AARCH64_H
#define XLAT_TABLES_AARCH64_H

#include <arch.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#if !defined(PAGE_SIZE)
#error "PAGE_SIZE is not defined."
#endif

/*
 * Encode a Physical Address Space size for its use in TCR_ELx.
 */
unsigned long long tcr_physical_addr_size_bits(unsigned long long max_addr);

/*
 * In AArch64 state, the MMU may support 4 KB, 16 KB and 64 KB page
 * granularity. For 4KB granularity, a level 0 table descriptor doesn't support
 * block translation. For 16KB, the same thing happens to levels 0 and 1. For
 * 64KB, same for level 1. See section D4.3.1 of the ARMv8-A Architecture
 * Reference Manual (DDI 0487A.k) for more information.
 *
 * The define below specifies the first table level that allows block
 * descriptors.
 */
#if PAGE_SIZE == PAGE_SIZE_4KB
# define MIN_LVL_BLOCK_DESC	U(1)
#elif (PAGE_SIZE == PAGE_SIZE_16KB) || (PAGE_SIZE == PAGE_SIZE_64KB)
# define MIN_LVL_BLOCK_DESC	U(2)
#endif

#define XLAT_TABLE_LEVEL_MIN	U(0)

/*
 * Define the architectural limits of the virtual address space in AArch64
 * state.
 *
 * TCR.TxSZ is calculated as 64 minus the width of said address space.
 * The value of TCR.TxSZ must be in the range 16 to 39 [1] or 48 [2],
 * depending on Small Translation Table Support which means that
 * the virtual address space width must be in the range 48 to 25 or 16 bits.
 *
 * [1] See the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information:
 * Page 1730: 'Input address size', 'For all translation stages'.
 * [2] See section 12.2.55 in the ARMv8-A Architecture Reference Manual
 * (DDI 0487D.a)
 */
/* Maximum value of TCR_ELx.T(0,1)SZ is 39 */
#define MIN_VIRT_ADDR_SPACE_SIZE	(ULL(1) << (U(64) - TCR_TxSZ_MAX))

/* Maximum value of TCR_ELx.T(0,1)SZ is 48 */
#define MIN_VIRT_ADDR_SPACE_SIZE_TTST	\
				(ULL(1) << (U(64) - TCR_TxSZ_MAX_TTST))
#define MAX_VIRT_ADDR_SPACE_SIZE	(ULL(1) << (U(64) - TCR_TxSZ_MIN))

/*
 * Here we calculate the initial lookup level from the value of the given
 * virtual address space size. For a 4 KB page size,
 * - level 0 supports virtual address spaces of widths 48 to 40 bits;
 * - level 1 from 39 to 31;
 * - level 2 from 30 to 22.
 * - level 3 from 21 to 16.
 *
 * Small Translation Table (Armv8.4-TTST) support allows the starting level
 * of the translation table from 3 for 4KB granularity. See section 12.2.55 in
 * the ARMv8-A Architecture Reference Manual (DDI 0487D.a). In Armv8.3 and below
 * wider or narrower address spaces are not supported. As a result, level 3
 * cannot be used as initial lookup level with 4 KB granularity. See section
 * D4.2.5 in the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information.
 *
 * For example, for a 35-bit address space (i.e. virt_addr_space_size ==
 * 1 << 35), TCR.TxSZ will be programmed to (64 - 35) = 29. According to Table
 * D4-11 in the ARM ARM, the initial lookup level for an address space like that
 * is 1.
 *
 * Note that this macro assumes that the given virtual address space size is
 * valid.
 */
#define GET_XLAT_TABLE_LEVEL_BASE(_virt_addr_space_sz)		\
	(((_virt_addr_space_sz) > (ULL(1) << L0_XLAT_ADDRESS_SHIFT))	\
	? 0U								\
	: (((_virt_addr_space_sz) > (ULL(1) << L1_XLAT_ADDRESS_SHIFT))	\
	? 1U								\
	: (((_virt_addr_space_sz) > (ULL(1) << L2_XLAT_ADDRESS_SHIFT))	\
	? 2U : 3U)))

#endif /* XLAT_TABLES_AARCH64_H */
