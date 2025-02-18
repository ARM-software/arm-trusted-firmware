/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TI_PLAT_LD_S
#define TI_PLAT_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <board_def.h>

MEMORY {
    SRAM (rwx): ORIGIN = DEVICE_WKUP_SRAM_BASE, LENGTH = DEVICE_WKUP_SRAM_CODE_SIZE
    PRAM (r): ORIGIN = BL31_LIMIT - DEVICE_WKUP_SRAM_CODE_SIZE, LENGTH = DEVICE_WKUP_SRAM_CODE_SIZE
}

SECTIONS
{
	/* SRAM_COPY is in PRAM */
	. = BL31_LIMIT - DEVICE_WKUP_SRAM_CODE_SIZE;
	__WKUP_SRAM_COPY_START__ = .;

	.wkupsram_resume_entry : ALIGN(PAGE_SIZE) {
		__wkup_sram_start__ = .;
		KEEP(*( .text.asm.k3_lpm_resume))
	} >SRAM AT>PRAM

	.wkupsram_suspend_entry : ALIGN(PAGE_SIZE){
		/* wkup sram start */
		__wkup_sram_suspend_entry__ = .;
		KEEP(*(.wkupsram.suspend_entry))
	} >SRAM AT>PRAM

	.wkupsram_text : ALIGN(PAGE_SIZE) {
		*(.wkupsram.text)
	} >SRAM AT>PRAM

	.wkupsram_data : ALIGN(PAGE_SIZE) {
		*(.wkupsram.data)
		__wkup_sram_end__ = .;
	} >SRAM AT>PRAM

    ASSERT(__BL31_END__ <= BL31_LIMIT - DEVICE_WKUP_SRAM_CODE_SIZE,
    "BL31 image too large - writing on top of SRAM!")

}
#endif /* TI_PLAT_LD_S */
