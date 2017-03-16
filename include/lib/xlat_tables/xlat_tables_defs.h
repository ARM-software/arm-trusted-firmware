/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __XLAT_TABLES_DEFS_H__
#define __XLAT_TABLES_DEFS_H__

#include <utils.h>

/* Miscellaneous MMU related constants */
#define NUM_2MB_IN_GB		(1 << 9)
#define NUM_4K_IN_2MB		(1 << 9)
#define NUM_GB_IN_4GB		(1 << 2)

#define TWO_MB_SHIFT		21
#define ONE_GB_SHIFT		30
#define FOUR_KB_SHIFT		12

#define ONE_GB_INDEX(x)		((x) >> ONE_GB_SHIFT)
#define TWO_MB_INDEX(x)		((x) >> TWO_MB_SHIFT)
#define FOUR_KB_INDEX(x)	((x) >> FOUR_KB_SHIFT)

#define INVALID_DESC		0x0
#define BLOCK_DESC		0x1 /* Table levels 0-2 */
#define TABLE_DESC		0x3 /* Table levels 0-2 */
#define PAGE_DESC		0x3 /* Table level 3 */
#define DESC_MASK		0x3

#define FIRST_LEVEL_DESC_N	ONE_GB_SHIFT
#define SECOND_LEVEL_DESC_N	TWO_MB_SHIFT
#define THIRD_LEVEL_DESC_N	FOUR_KB_SHIFT

#define XN			(ULL(1) << 2)
#define PXN			(ULL(1) << 1)
#define CONT_HINT		(ULL(1) << 0)
#define UPPER_ATTRS(x)		(((x) & ULL(0x7)) << 52)

#define NON_GLOBAL		(1 << 9)
#define ACCESS_FLAG		(1 << 8)
#define NSH			(0x0 << 6)
#define OSH			(0x2 << 6)
#define ISH			(0x3 << 6)

#define TABLE_ADDR_MASK		ULL(0x0000FFFFFFFFF000)

#define PAGE_SIZE_SHIFT		FOUR_KB_SHIFT /* 4, 16 or 64 KB */
#define PAGE_SIZE		(1 << PAGE_SIZE_SHIFT)
#define PAGE_SIZE_MASK		(PAGE_SIZE - 1)
#define IS_PAGE_ALIGNED(addr)	(((addr) & PAGE_SIZE_MASK) == 0)

#define XLAT_ENTRY_SIZE_SHIFT	3 /* Each MMU table entry is 8 bytes (1 << 3) */
#define XLAT_ENTRY_SIZE		(1 << XLAT_ENTRY_SIZE_SHIFT)

#define XLAT_TABLE_SIZE_SHIFT	PAGE_SIZE_SHIFT /* Size of one complete table */
#define XLAT_TABLE_SIZE		(1 << XLAT_TABLE_SIZE_SHIFT)

#ifdef AARCH32
#define XLAT_TABLE_LEVEL_MIN	1
#else
#define XLAT_TABLE_LEVEL_MIN	0
#endif /* AARCH32 */

#define XLAT_TABLE_LEVEL_MAX	3

/* Values for number of entries in each MMU translation table */
#define XLAT_TABLE_ENTRIES_SHIFT (XLAT_TABLE_SIZE_SHIFT - XLAT_ENTRY_SIZE_SHIFT)
#define XLAT_TABLE_ENTRIES	(1 << XLAT_TABLE_ENTRIES_SHIFT)
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
#define AP_RO				(0x1 << 5)
#define AP_RW				(0x0 << 5)

#define NS				(0x1 << 3)
#define ATTR_NON_CACHEABLE_INDEX	0x2
#define ATTR_DEVICE_INDEX		0x1
#define ATTR_IWBWA_OWBWA_NTR_INDEX	0x0
#define LOWER_ATTRS(x)			(((x) & 0xfff) << 2)
/* Normal Memory, Outer Write-Through non-transient, Inner Non-cacheable */
#define ATTR_NON_CACHEABLE		(0x44)
/* Device-nGnRE */
#define ATTR_DEVICE			(0x4)
/* Normal Memory, Outer Write-Back non-transient, Inner Write-Back non-transient */
#define ATTR_IWBWA_OWBWA_NTR		(0xff)
#define MAIR_ATTR_SET(attr, index)	((attr) << ((index) << 3))
#define ATTR_INDEX_MASK			0x3
#define ATTR_INDEX_GET(attr)		(((attr) >> 2) & ATTR_INDEX_MASK)

/*
 * Flags to override default values used to program system registers while
 * enabling the MMU.
 */
#define DISABLE_DCACHE			(1 << 0)

#endif /* __XLAT_TABLES_DEFS_H__ */
