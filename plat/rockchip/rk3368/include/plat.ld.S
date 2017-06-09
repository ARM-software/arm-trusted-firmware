/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ROCKCHIP_PLAT_LD_S__
#define __ROCKCHIP_PLAT_LD_S__

MEMORY {
    PMUSRAM (rwx): ORIGIN = PMUSRAM_BASE, LENGTH = PMUSRAM_RSIZE
}

SECTIONS
{
	. = PMUSRAM_BASE;

	/*
	 * pmu_cpuson_entrypoint request address
	 * align 64K when resume, so put it in the
	 * start of pmusram
	 */
	.text_pmusram : {
		ASSERT(. == ALIGN(64 * 1024),
			".pmusram.entry request 64K aligned.");
		*(.pmusram.entry)
		__bl31_pmusram_text_start = .;
		*(.pmusram.text)
		*(.pmusram.rodata)
		__bl31_pmusram_text_end = .;
		__bl31_pmusram_data_start = .;
		*(.pmusram.data)
		__bl31_pmusram_data_end = .;

	} >PMUSRAM
}

#endif /* __ROCKCHIP_PLAT_LD_S__ */
