/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ROCKCHIP_PLAT_LD_S
#define ROCKCHIP_PLAT_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>

MEMORY {
    SRAM (rwx): ORIGIN = SRAM_BASE, LENGTH = SRAM_SIZE
    PMUSRAM (rwx): ORIGIN = PMUSRAM_BASE, LENGTH = PMUSRAM_RSIZE
}

SECTIONS
{
	. = SRAM_BASE;
	ASSERT(. == ALIGN(PAGE_SIZE),
		"SRAM_BASE address is not aligned on a page boundary.")

	/*
	 * The SRAM space allocation for RK3399
	 * ----------------
	 * | m0 code bin
	 * ----------------
	 * | sram text
	 * ----------------
	 * | sram data
	 * ----------------
	 */
	.incbin_sram : ALIGN(PAGE_SIZE) {
		__sram_incbin_start = .;
		*(.sram.incbin)
		 __sram_incbin_real_end = .;
		. = ALIGN(PAGE_SIZE);
		__sram_incbin_end = .;
	} >SRAM
	ASSERT((__sram_incbin_real_end - __sram_incbin_start) <=
		SRAM_BIN_LIMIT, ".incbin_sram has exceeded its limit")

	.text_sram : ALIGN(PAGE_SIZE) {
		__bl31_sram_text_start = .;
		*(.sram.text)
		*(.sram.rodata)
		__bl31_sram_text_real_end = .;
		. = ALIGN(PAGE_SIZE);
		__bl31_sram_text_end = .;
	} >SRAM
	ASSERT((__bl31_sram_text_real_end - __bl31_sram_text_start) <=
		SRAM_TEXT_LIMIT, ".text_sram has exceeded its limit")

	.data_sram : ALIGN(PAGE_SIZE) {
		__bl31_sram_data_start = .;
		*(.sram.data)
		__bl31_sram_data_real_end = .;
		. = ALIGN(PAGE_SIZE);
		__bl31_sram_data_end = .;
	} >SRAM
	ASSERT((__bl31_sram_data_real_end - __bl31_sram_data_start) <=
		SRAM_DATA_LIMIT, ".data_sram has exceeded its limit")

	.stack_sram : ALIGN(PAGE_SIZE) {
		__bl31_sram_stack_start = .;
		. += PAGE_SIZE;
		__bl31_sram_stack_end = .;
	} >SRAM

	. = PMUSRAM_BASE;

	/*
	 * pmu_cpuson_entrypoint request address
	 * align 64K when resume, so put it in the
	 * start of pmusram
	 */
	.pmusram : {
		ASSERT(. == ALIGN(64 * 1024),
			".pmusram.entry request 64K aligned.");
		*(.pmusram.entry)

		__bl31_pmusram_text_start = .;
		*(.pmusram.text)
		*(.pmusram.rodata)
		__bl31_pmusram_text_end = .;

		/* M0 start address request 4K align */
		. = ALIGN(4096);
		__pmusram_incbin_start = .;
		*(.pmusram.incbin)
		__pmusram_incbin_end = .;

		__bl31_pmusram_data_start = .;
		*(.pmusram.data)
		__bl31_pmusram_data_end = .;
	} >PMUSRAM
}

#endif /* ROCKCHIP_PLAT_LD_S */
