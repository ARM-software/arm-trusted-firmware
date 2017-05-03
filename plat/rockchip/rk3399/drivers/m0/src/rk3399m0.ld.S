/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <m0_param.h>

OUTPUT_FORMAT("elf32-littlearm")

SECTIONS {
	.m0_bin 0 : {
		KEEP(*(.isr_vector))
		ASSERT(. == 0xc0, "ISR vector has the wrong size.");
		ASSERT(. == PARAM_ADDR, "M0 params should go right behind ISR table.");
		. += PARAM_M0_SIZE;
		*(.text*)
		*(.rodata*)
		*(.data*)
		*(.bss*)
		. = ALIGN(8);
		*(.co_stack*)
	}

	/DISCARD/ : { *(.comment) *(.note*) }
}
