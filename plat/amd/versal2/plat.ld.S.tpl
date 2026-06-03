/*
 * Copyright (c) 2026, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Versal Gen 2 Platform Linker Script Template
 *
 * Built by plat/amd/common/custom_pkg.mk:
 * - Each custom_pkg.ld.S is preprocessed separately with package-specific macros
 * - A generated wrapper file includes the preprocessed custom package scripts
 * - This template preprocesses with #include of that wrapper file
 * - Result is final plat.ld.S with TF-A core + custom package sections
 */

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <platform_def.h>

MEMORY {
	BL31_OCRAM (rwx): ORIGIN = BL31_BASE, LENGTH = BL31_LIMIT - BL31_BASE
}

SECTIONS {
	. = BL31_BASE;
	ASSERT(. == ALIGN(PAGE_SIZE), "BL31_BASE not page-aligned")

	/* TF-A Core Sections in BL31_OCRAM */
	.text . : {
		__TEXT_START__ = . ;
		*(.text*) *(.vectors)
		. = ALIGN(PAGE_SIZE);
		__TEXT_END__ = . ;
	} >BL31_OCRAM AT >BL31_OCRAM

	.rodata . : {
		__RODATA_START__ = . ;
		*(.rodata*)
		. = ALIGN(PAGE_SIZE);
		__RODATA_END__ = . ;
	} >BL31_OCRAM AT >BL31_OCRAM

	.data . : {
		__DATA_START__ = . ;
		*(.data*)
		. = ALIGN(PAGE_SIZE);
		__DATA_END__ = . ;
	} >BL31_OCRAM AT >BL31_OCRAM

	.bss . : {
		__BSS_START__ = . ;
		*(.bss*)
		. = ALIGN(PAGE_SIZE);
		__BSS_END__ = . ;
	} >BL31_OCRAM AT >BL31_OCRAM
	/* Custom Package Sections: appended inside SECTIONS */
	#include CUSTOM_PKG_LD_WRAPPER
}
