/*
 * Copyright (c) 2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Linker script for the Arm Ltd. FPGA boards to generate an ELF file that
 * contains the ROM trampoline, BL31 and the DTB.
 *
 * This allows to pass just one file to the uploader tool, and automatically
 * provides the correct load addresses.
 */

#include <platform_def.h>

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

INPUT(./bl31/bl31.elf)
INPUT(./rom_trampoline.o)

TARGET(binary)
INPUT(./fdts/arm_fpga.dtb)

ENTRY(_start)

SECTIONS
{
	.rom (0x0): {
		*rom_trampoline.o(.text*)
		KEEP(*(.rom))
	}

	.bl31 (BL31_BASE): {
		ASSERT(. == ALIGN(PAGE_SIZE), "BL31_BASE is not page aligned");
		*bl31.elf(.text* .data* .rodata* ro* .bss*)
		*bl31.elf(.stack)
	}

	.dtb (FPGA_PRELOADED_DTB_BASE): {
		ASSERT(. == ALIGN(8), "DTB address is not 8-byte aligned");
		*arm_fpga.dtb
	}

	/DISCARD/ : { *(.debug_*) }
	/DISCARD/ : { *(.note*) }
	/DISCARD/ : { *(.comment*) }
}
