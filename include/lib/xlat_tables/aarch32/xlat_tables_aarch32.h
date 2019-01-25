/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_TABLES_AARCH32_H
#define XLAT_TABLES_AARCH32_H

#include <arch.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#if !defined(PAGE_SIZE)
#error "PAGE_SIZE is not defined."
#endif

/*
 * In AArch32 state, the MMU only supports 4KB page granularity, which means
 * that the first translation table level is either 1 or 2. Both of them are
 * allowed to have block and table descriptors. See section G4.5.6 of the
 * ARMv8-A Architecture Reference Manual (DDI 0487A.k) for more information.
 *
 * The define below specifies the first table level that allows block
 * descriptors.
 */
#if PAGE_SIZE != PAGE_SIZE_4KB
#error "Invalid granule size. AArch32 supports 4KB pages only."
#endif

#define MIN_LVL_BLOCK_DESC	U(1)

#define XLAT_TABLE_LEVEL_MIN	U(1)

/*
 * Define the architectural limits of the virtual address space in AArch32
 * state.
 *
 * TTBCR.TxSZ is calculated as 32 minus the width of said address space. The
 * value of TTBCR.TxSZ must be in the range 0 to 7 [1], which means that the
 * virtual address space width must be in the range 32 to 25 bits.
 *
 * [1] See the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information, Section G4.6.5
 */
#define MIN_VIRT_ADDR_SPACE_SIZE	(ULL(1) << (U(32) - TTBCR_TxSZ_MAX))
#define MAX_VIRT_ADDR_SPACE_SIZE	(ULL(1) << (U(32) - TTBCR_TxSZ_MIN))

/*
 * Here we calculate the initial lookup level from the value of the given
 * virtual address space size. For a 4 KB page size,
 * - level 1 supports virtual address spaces of widths 32 to 31 bits;
 * - level 2 from 30 to 25.
 *
 * Wider or narrower address spaces are not supported. As a result, level 3
 * cannot be used as the initial lookup level with 4 KB granularity.
 * See the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information, Section G4.6.5
 *
 * For example, for a 31-bit address space (i.e. virt_addr_space_size ==
 * 1 << 31), TTBCR.TxSZ will be programmed to (32 - 31) = 1. According to Table
 * G4-5 in the ARM ARM, the initial lookup level for an address space like that
 * is 1.
 *
 * Note that this macro assumes that the given virtual address space size is
 * valid.
 */
#define GET_XLAT_TABLE_LEVEL_BASE(_virt_addr_space_sz)			\
	(((_virt_addr_space_sz) > (ULL(1) << L1_XLAT_ADDRESS_SHIFT)) ?	\
	 U(1) : U(2))

#endif /* XLAT_TABLES_AARCH32_H */
