/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_PLAT_LD_S__
#define SYNQUACER_PLAT_LD_S__

#include <lib/xlat_tables/xlat_tables_defs.h>

#define SPM_SHIM_EXCEPTIONS_VMA		SP_DRAM

MEMORY {
	SP_DRAM (rw): ORIGIN = PLAT_SQ_SP_PRIV_BASE, LENGTH = PLAT_SQ_SP_PRIV_SIZE
}

SECTIONS
{
	/*
	 * Put the page tables in secure DRAM so that the PTW can make cacheable
	 * accesses, as the core SPM code expects. (The SRAM on SynQuacer does
	 * not support inner shareable WBWA mappings so it is mapped normal
	 * non-cacheable)
	 */
	sp_xlat_table (NOLOAD) : ALIGN(PAGE_SIZE) {
		*(sp_xlat_table)
		*(.bss.sp_base_xlat_table)
	} >SP_DRAM
}

#endif /* SYNQUACER_PLAT_LD_S__ */
