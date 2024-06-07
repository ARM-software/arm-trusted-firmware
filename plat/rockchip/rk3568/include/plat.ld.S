/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ROCKCHIP_PLAT_LD_S
#define ROCKCHIP_PLAT_LD_S

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
	.pmusram : {
		ASSERT(. == ALIGN(64 * 1024),
			".pmusram.entry request 64K aligned.");
		KEEP(*(.pmusram.entry))

		__bl31_pmusram_text_start = .;
		*(.pmusram.text)
		*(.pmusram.rodata)
		__bl31_pmusram_text_end = .;
		__bl31_pmusram_data_start = .;
		*(.pmusram.data)
		__bl31_pmusram_data_end = .;
	} >PMUSRAM
}
#endif /* ROCKCHIP_PLAT_LD_S */
