/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_DEFS_H__
#define __XLAT_TABLES_DEFS_H__

#include <arch.h>
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
/*
 * A block descriptor points to a region of memory bigger than the granule size
 * (e.g. a 2MB region when the granule size is 4KB).
 */
#define BLOCK_DESC		U(0x1) /* Table levels 0-2 */
/* A table descriptor points to the next level of translation table. */
#define TABLE_DESC		U(0x3) /* Table levels 0-2 */
/*
 * A page descriptor points to a page, i.e. a memory region whose size is the
 * translation granule size (e.g. 4KB).
 */
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

/*
 * The ARMv8-A architecture allows translation granule sizes of 4KB, 16KB or
 * 64KB. However, TF only supports the 4KB case at the moment.
 */
#define PAGE_SIZE_SHIFT		FOUR_KB_SHIFT
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
 * Extract from the given virtual address the index into the given lookup level.
 * This macro assumes the system is using the 4KB translation granule.
 */
#define XLAT_TABLE_IDX(virtual_addr, level)	\
	(((virtual_addr) >> XLAT_ADDR_SHIFT(level)) & ULL(0x1FF))

/*
 * The ARMv8 translation table descriptor format defines AP[2:1] as the Access
 * Permissions bits, and does not define an AP[0] bit.
 *
 * AP[1] is valid only for a stage 1 translation that supports two VA ranges
 * (i.e. in the ARMv8A.0 architecture, that is the S-EL1&0 regime).
 *
 * AP[1] is RES0 for stage 1 translations that support only one VA range
 * (e.g. EL3).
 */
#define AP2_SHIFT			U(0x7)
#define AP2_RO				U(0x1)
#define AP2_RW				U(0x0)

#define AP1_SHIFT			U(0x6)
#define AP1_ACCESS_UNPRIVILEGED		U(0x1)
#define AP1_NO_ACCESS_UNPRIVILEGED	U(0x0)

/*
 * The following definitions must all be passed to the LOWER_ATTRS() macro to
 * get the right bitmask.
 */
#define AP_RO				(AP2_RO << 5)
#define AP_RW				(AP2_RW << 5)
#define AP_ACCESS_UNPRIVILEGED		(AP1_ACCESS_UNPRIVILEGED    << 4)
#define AP_NO_ACCESS_UNPRIVILEGED	(AP1_NO_ACCESS_UNPRIVILEGED << 4)
#define NS				(U(0x1) << 3)
#define ATTR_NON_CACHEABLE_INDEX	U(0x2)
#define ATTR_DEVICE_INDEX		U(0x1)
#define ATTR_IWBWA_OWBWA_NTR_INDEX	U(0x0)
#define LOWER_ATTRS(x)			(((x) & U(0xfff)) << 2)

/* Normal Memory, Outer Write-Through non-transient, Inner Non-cacheable */
#define ATTR_NON_CACHEABLE		MAKE_MAIR_NORMAL_MEMORY(MAIR_NORM_NC, MAIR_NORM_NC)
/* Device-nGnRE */
#define ATTR_DEVICE			MAIR_DEV_nGnRE
/* Normal Memory, Outer Write-Back non-transient, Inner Write-Back non-transient */
#define ATTR_IWBWA_OWBWA_NTR		MAKE_MAIR_NORMAL_MEMORY(MAIR_NORM_WB_NTR_RWA, MAIR_NORM_WB_NTR_RWA)
#define MAIR_ATTR_SET(attr, index)	((attr) << ((index) << 3))
#define ATTR_INDEX_MASK			U(0x3)
#define ATTR_INDEX_GET(attr)		(((attr) >> 2) & ATTR_INDEX_MASK)

/*
 * Shift values for the attributes fields in a block or page descriptor.
 * See section D4.3.3 in the ARMv8-A ARM (issue B.a).
 */

/* Memory attributes index field, AttrIndx[2:0]. */
#define ATTR_INDEX_SHIFT		2
/* Non-secure bit, NS. */
#define NS_SHIFT			5
/* Shareability field, SH[1:0] */
#define SHAREABILITY_SHIFT		8
/* The Access Flag, AF. */
#define ACCESS_FLAG_SHIFT		10
/* The not global bit, nG. */
#define NOT_GLOBAL_SHIFT		11
/* Contiguous hint bit. */
#define CONT_HINT_SHIFT			52
/* Execute-never bits, XN. */
#define PXN_SHIFT			53
#define XN_SHIFT			54
#define UXN_SHIFT			XN_SHIFT

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
