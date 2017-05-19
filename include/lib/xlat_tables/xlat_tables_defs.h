/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_DEFS_H__
#define __XLAT_TABLES_DEFS_H__

#include <utils_def.h>

/* Miscellaneous MMU related constants */
#define NUM_2MB_IN_GB		(U(1) << 9)
#define NUM_4K_IN_2MB		(U(1) << 9)
#define NUM_GB_IN_4GB		(U(1) << 2)

#define TWO_MB_SHIFT		U(21)
#define ONE_GB_SHIFT		U(30)
#define FOUR_KB_SHIFT		U(12)

#define ONE_GB_INDEX(x)		((x) >> ONE_GB_SHIFT)
#define TWO_MB_INDEX(x)		((x) >> TWO_MB_SHIFT)
#define FOUR_KB_INDEX(x)	((x) >> FOUR_KB_SHIFT)

#define INVALID_DESC		U(0x0)
#define BLOCK_DESC		U(0x1) /* Table levels 0-2 */
#define TABLE_DESC		U(0x3) /* Table levels 0-2 */
#define PAGE_DESC		U(0x3) /* Table level 3 */
#define DESC_MASK		U(0x3)

#define FIRST_LEVEL_DESC_N	ONE_GB_SHIFT
#define SECOND_LEVEL_DESC_N	TWO_MB_SHIFT
#define THIRD_LEVEL_DESC_N	FOUR_KB_SHIFT

/* XN: Translation regimes that support one VA range (EL2 and EL3). */
#define XN			(ULL(1) << 2)
/* UXN, PXN: Translation regimes that support two VA ranges (EL1&0). */
#define UXN			(ULL(1) << 2)
#define PXN			(ULL(1) << 1)
#define CONT_HINT		(ULL(1) << 0)
#define UPPER_ATTRS(x)		(((x) & ULL(0x7)) << 52)

#define NON_GLOBAL		(U(1) << 9)
#define ACCESS_FLAG		(U(1) << 8)
#define NSH			(U(0x0) << 6)
#define OSH			(U(0x2) << 6)
#define ISH			(U(0x3) << 6)

#define TABLE_ADDR_MASK		ULL(0x0000FFFFFFFFF000)

#define PAGE_SIZE_SHIFT		FOUR_KB_SHIFT /* 4, 16 or 64 KB */
#define PAGE_SIZE		(U(1) << PAGE_SIZE_SHIFT)
#define PAGE_SIZE_MASK		(PAGE_SIZE - 1)
#define IS_PAGE_ALIGNED(addr)	(((addr) & PAGE_SIZE_MASK) == 0)

#define XLAT_ENTRY_SIZE_SHIFT	U(3) /* Each MMU table entry is 8 bytes (1 << 3) */
#define XLAT_ENTRY_SIZE		(U(1) << XLAT_ENTRY_SIZE_SHIFT)

#define XLAT_TABLE_SIZE_SHIFT	PAGE_SIZE_SHIFT /* Size of one complete table */
#define XLAT_TABLE_SIZE		(U(1) << XLAT_TABLE_SIZE_SHIFT)

#define XLAT_TABLE_LEVEL_MAX	U(3)

/* Values for number of entries in each MMU translation table */
#define XLAT_TABLE_ENTRIES_SHIFT (XLAT_TABLE_SIZE_SHIFT - XLAT_ENTRY_SIZE_SHIFT)
#define XLAT_TABLE_ENTRIES	(U(1) << XLAT_TABLE_ENTRIES_SHIFT)
#define XLAT_TABLE_ENTRIES_MASK	(XLAT_TABLE_ENTRIES - 1)

/* Values to convert a memory address to an index into a translation table */
#define L3_XLAT_ADDRESS_SHIFT	PAGE_SIZE_SHIFT
#define L2_XLAT_ADDRESS_SHIFT	(L3_XLAT_ADDRESS_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)
#define L1_XLAT_ADDRESS_SHIFT	(L2_XLAT_ADDRESS_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)
#define L0_XLAT_ADDRESS_SHIFT	(L1_XLAT_ADDRESS_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)
#define XLAT_ADDR_SHIFT(level)	(PAGE_SIZE_SHIFT + \
		  ((XLAT_TABLE_LEVEL_MAX - (level)) * XLAT_TABLE_ENTRIES_SHIFT))

#define XLAT_BLOCK_SIZE(level)	((u_register_t)1 << XLAT_ADDR_SHIFT(level))
/* Mask to get the bits used to index inside a block of a certain level */
#define XLAT_BLOCK_MASK(level)	(XLAT_BLOCK_SIZE(level) - 1)
/* Mask to get the address bits common to a block of a certain table level*/
#define XLAT_ADDR_MASK(level)	(~XLAT_BLOCK_MASK(level))

/*
 * AP[1] bit is ignored by hardware and is
 * treated as if it is One in EL2/EL3
 */
#define AP_RO				(U(0x1) << 5)
#define AP_RW				(U(0x0) << 5)

#define NS				(U(0x1) << 3)
#define ATTR_NON_CACHEABLE_INDEX	U(0x2)
#define ATTR_DEVICE_INDEX		U(0x1)
#define ATTR_IWBWA_OWBWA_NTR_INDEX	U(0x0)
#define LOWER_ATTRS(x)			(((x) & U(0xfff)) << 2)
/* Normal Memory, Outer Write-Through non-transient, Inner Non-cacheable */
#define ATTR_NON_CACHEABLE		U(0x44)
/* Device-nGnRE */
#define ATTR_DEVICE			U(0x4)
/* Normal Memory, Outer Write-Back non-transient, Inner Write-Back non-transient */
#define ATTR_IWBWA_OWBWA_NTR		U(0xff)
#define MAIR_ATTR_SET(attr, index)	((attr) << ((index) << 3))
#define ATTR_INDEX_MASK			U(0x3)
#define ATTR_INDEX_GET(attr)		(((attr) >> 2) & ATTR_INDEX_MASK)

/*
 * Flags to override default values used to program system registers while
 * enabling the MMU.
 */
#define DISABLE_DCACHE			(U(1) << 0)

/*
 * This flag marks the translation tables are Non-cacheable for MMU accesses.
 * If the flag is not specified, by default the tables are cacheable.
 */
#define XLAT_TABLE_NC			(U(1) << 1)

#endif /* __XLAT_TABLES_DEFS_H__ */
