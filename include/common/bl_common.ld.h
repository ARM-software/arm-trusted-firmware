/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL_COMMON_LD_H
#define BL_COMMON_LD_H

/*
 * The xlat_table section is for full, aligned page tables (4K).
 * Removing them from .bss avoids forcing 4K alignment on
 * the .bss section. The tables are initialized to zero by the translation
 * tables library.
 */
#define XLAT_TABLE_SECTION				\
	xlat_table (NOLOAD) : {				\
		*(xlat_table)				\
	}

#endif /* BL_COMMON_LD_H */
