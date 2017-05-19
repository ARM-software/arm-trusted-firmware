/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XLAT_TABLES_ARCH_H__
#define __XLAT_TABLES_ARCH_H__

#ifdef AARCH32
#include "aarch32/xlat_tables_aarch32.h"
#else
#include "aarch64/xlat_tables_aarch64.h"
#endif

/*
 * Evaluates to 1 if the given virtual address space size is valid, or 0 if it's
 * not.
 *
 * A valid size is one that is a power of 2 and is within the architectural
 * limits. Not that these limits are different for AArch32 and AArch64.
 */
#define CHECK_VIRT_ADDR_SPACE_SIZE(size)			\
	(((size) >= MIN_VIRT_ADDR_SPACE_SIZE) &&		\
	((size) <= MAX_VIRT_ADDR_SPACE_SIZE) &&			\
	IS_POWER_OF_TWO(size))

/*
 * Evaluates to 1 if the given physical address space size is a power of 2,
 * or 0 if it's not.
 */
#define CHECK_PHY_ADDR_SPACE_SIZE(size)				\
	(IS_POWER_OF_TWO(size))

/*
 * Compute the number of entries required at the initial lookup level to address
 * the whole virtual address space.
 */
#define GET_NUM_BASE_LEVEL_ENTRIES(addr_space_size)			\
	((addr_space_size) >>						\
		XLAT_ADDR_SHIFT(GET_XLAT_TABLE_LEVEL_BASE(addr_space_size)))

#endif /* __XLAT_TABLES_ARCH_H__ */
