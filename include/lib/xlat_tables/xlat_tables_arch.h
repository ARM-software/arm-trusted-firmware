/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef XLAT_TABLES_ARCH_H
#define XLAT_TABLES_ARCH_H

#ifdef AARCH32
#include "aarch32/xlat_tables_aarch32.h"
#else
#include "aarch64/xlat_tables_aarch64.h"
#endif

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

#endif /* XLAT_TABLES_ARCH_H */
