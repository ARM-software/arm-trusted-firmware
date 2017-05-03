/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ROCKCHIP_PLAT_LD_S__
#define __ROCKCHIP_PLAT_LD_S__

MEMORY {
    SRAM (rwx): ORIGIN = SRAM_BASE, LENGTH = SRAM_SIZE
}

SECTIONS
{
	. = SRAM_BASE;
	ASSERT(. == ALIGN(4096),
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
	.incbin_sram : ALIGN(4096) {
		__sram_incbin_start = .;
		*(.sram.incbin)
		. = ALIGN(4096);
		__sram_incbin_end = .;
	} >SRAM

	.text_sram : ALIGN(4096) {
		__bl31_sram_text_start = .;
		*(.sram.text)
		*(.sram.rodata)
		. = ALIGN(4096);
		__bl31_sram_text_end = .;
	} >SRAM

	.data_sram : ALIGN(4096) {
		__bl31_sram_data_start = .;
		*(.sram.data)
		. = ALIGN(4096);
		__bl31_sram_data_end = .;
	} >SRAM
}

#endif /* __ROCKCHIP_PLAT_LD_S__ */
