/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_ARCH_H__
#define __XLAT_TABLES_ARCH_H__

#include <arch.h>
#include <platform_def.h>
#include <xlat_tables_defs.h>
#include "../xlat_tables_private.h"

/*
 * In AArch32 state, the MMU only supports 4KB page granularity, which means
 * that the first translation table level is either 1 or 2. Both of them are
 * allowed to have block and table descriptors. See section G4.5.6 of the
 * ARMv8-A Architecture Reference Manual (DDI 0487A.k) for more information.
 *
 * The define below specifies the first table level that allows block
 * descriptors.
 */

#define MIN_LVL_BLOCK_DESC 1

/*
 * Each platform can define the size of the virtual address space, which is
 * defined in PLAT_VIRT_ADDR_SPACE_SIZE. TTBCR.TxSZ is calculated as 32 minus
 * the width of said address space. The value of TTBCR.TxSZ must be in the
 * range 0 to 7 [1], which means that the virtual address space width must be
 * in the range 32 to 25 bits.
 *
 * Here we calculate the initial lookup level from the value of
 * PLAT_VIRT_ADDR_SPACE_SIZE. For a 4 KB page size, level 1 supports virtual
 * address spaces of widths 32 to 31 bits, and level 2 from 30 to 25. Wider or
 * narrower address spaces are not supported. As a result, level 3 cannot be
 * used as initial lookup level with 4 KB granularity [1].
 *
 * For example, for a 31-bit address space (i.e. PLAT_VIRT_ADDR_SPACE_SIZE ==
 * 1 << 31), TTBCR.TxSZ will be programmed to (32 - 31) = 1. According to Table
 * G4-5 in the ARM ARM, the initial lookup level for an address space like that
 * is 1.
 *
 * See the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information:
 * [1] Section G4.6.5
 */

#if PLAT_VIRT_ADDR_SPACE_SIZE > (1ULL << (32 - TTBCR_TxSZ_MIN))

# error "PLAT_VIRT_ADDR_SPACE_SIZE is too big."

#elif PLAT_VIRT_ADDR_SPACE_SIZE > (1ULL << L1_XLAT_ADDRESS_SHIFT)

# define XLAT_TABLE_LEVEL_BASE	1
# define NUM_BASE_LEVEL_ENTRIES	\
		(PLAT_VIRT_ADDR_SPACE_SIZE >> L1_XLAT_ADDRESS_SHIFT)

#elif PLAT_VIRT_ADDR_SPACE_SIZE >= (1ULL << (32 - TTBCR_TxSZ_MAX))

# define XLAT_TABLE_LEVEL_BASE	2
# define NUM_BASE_LEVEL_ENTRIES	\
		(PLAT_VIRT_ADDR_SPACE_SIZE >> L2_XLAT_ADDRESS_SHIFT)

#else

# error "PLAT_VIRT_ADDR_SPACE_SIZE is too small."

#endif

#endif /* __XLAT_TABLES_ARCH_H__ */
