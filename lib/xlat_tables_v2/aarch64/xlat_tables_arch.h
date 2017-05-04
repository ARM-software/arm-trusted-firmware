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

#if PAGE_SIZE == (4*1024) /* 4KB */
# define MIN_LVL_BLOCK_DESC 1
#else /* 16KB or 64KB */
# define MIN_LVL_BLOCK_DESC 2
#endif

/*
 * Each platform can define the size of the virtual address space, which is
 * defined in PLAT_VIRT_ADDR_SPACE_SIZE. TCR.TxSZ is calculated as 64 minus the
 * width of said address space. The value of TCR.TxSZ must be in the range 16
 * to 39 [1], which means that the virtual address space width must be in the
 * range 48 to 25 bits.
 *
 * Here we calculate the initial lookup level from the value of
 * PLAT_VIRT_ADDR_SPACE_SIZE. For a 4 KB page size, level 0 supports virtual
 * address spaces of widths 48 to 40 bits, level 1 from 39 to 31, and level 2
 * from 30 to 25. Wider or narrower address spaces are not supported. As a
 * result, level 3 cannot be used as initial lookup level with 4 KB
 * granularity. [2]
 *
 * For example, for a 35-bit address space (i.e. PLAT_VIRT_ADDR_SPACE_SIZE ==
 * 1 << 35), TCR.TxSZ will be programmed to (64 - 35) = 29. According to Table
 * D4-11 in the ARM ARM, the initial lookup level for an address space like
 * that is 1.
 *
 * See the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information:
 * [1] Page 1730: 'Input address size', 'For all translation stages'.
 * [2] Section D4.2.5
 */

#if PLAT_VIRT_ADDR_SPACE_SIZE > (1ULL << (64 - TCR_TxSZ_MIN))

# error "PLAT_VIRT_ADDR_SPACE_SIZE is too big."

#elif PLAT_VIRT_ADDR_SPACE_SIZE > (1ULL << L0_XLAT_ADDRESS_SHIFT)

# define XLAT_TABLE_LEVEL_BASE	0
# define NUM_BASE_LEVEL_ENTRIES	\
		(PLAT_VIRT_ADDR_SPACE_SIZE >> L0_XLAT_ADDRESS_SHIFT)

#elif PLAT_VIRT_ADDR_SPACE_SIZE > (1 << L1_XLAT_ADDRESS_SHIFT)

# define XLAT_TABLE_LEVEL_BASE	1
# define NUM_BASE_LEVEL_ENTRIES	\
		(PLAT_VIRT_ADDR_SPACE_SIZE >> L1_XLAT_ADDRESS_SHIFT)

#elif PLAT_VIRT_ADDR_SPACE_SIZE >= (1 << (64 - TCR_TxSZ_MAX))

# define XLAT_TABLE_LEVEL_BASE	2
# define NUM_BASE_LEVEL_ENTRIES	\
		(PLAT_VIRT_ADDR_SPACE_SIZE >> L2_XLAT_ADDRESS_SHIFT)

#else

# error "PLAT_VIRT_ADDR_SPACE_SIZE is too small."

#endif

#endif /* __XLAT_TABLES_ARCH_H__ */
