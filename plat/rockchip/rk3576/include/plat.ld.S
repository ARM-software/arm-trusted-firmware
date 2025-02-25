// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
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
	.text_pmusram : {
		ASSERT(. == ALIGN(64 * 1024),
			".pmusram.entry request 64K aligned.");
		KEEP(*(.pmusram.entry))
		__bl31_pmusram_text_start = .;
		*(.pmusram.text)
		*(.pmusram.rodata)
		. = ALIGN(PAGE_SIZE);
		__bl31_pmusram_text_end = .;
		__bl31_pmusram_data_start = .;
		*(.pmusram.data)
		. = ALIGN(PAGE_SIZE);
		__bl31_pmusram_data_end = .;

		ASSERT(__bl31_pmusram_data_end <= PMUSRAM_BASE + PMUSRAM_RSIZE,
			".pmusram has exceeded its limit.");
	} >PMUSRAM
}
#endif /* ROCKCHIP_PLAT_LD_S */
