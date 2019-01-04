/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <platform_def.h>

MEMORY {
	ROM (rx): ORIGIN = ROMLIB_RO_BASE, LENGTH = ROMLIB_RO_LIMIT - ROMLIB_RO_BASE
	RAM (rwx): ORIGIN = ROMLIB_RW_BASE, LENGTH = ROMLIB_RW_END - ROMLIB_RW_BASE
}

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(jmptbl)

SECTIONS
{
	. = ROMLIB_RO_BASE;
	.text : {
		*jmptbl.o(.text)
		*(.text*)
		*(.rodata*)
	} >ROM

	__DATA_ROM_START__ = LOADADDR(.data);

	.data : {
		__DATA_RAM_START__ = .;
		*(.data*)
		__DATA_RAM_END__ = .;
	} >RAM AT>ROM

	__DATA_SIZE__ = SIZEOF(.data);

	.bss : {
		__BSS_START__ = .;
		*(.bss*)
		__BSS_END__ = .;
	 } >RAM
	__BSS_SIZE__ = SIZEOF(.bss);
}
