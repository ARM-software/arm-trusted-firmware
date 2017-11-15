/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __ARM_COMMON_LD_S__
#define __ARM_COMMON_LD_S__

#include <xlat_tables_defs.h>

MEMORY {
    EL3_SEC_DRAM (rw): ORIGIN = ARM_EL3_TZC_DRAM1_BASE, LENGTH = ARM_EL3_TZC_DRAM1_SIZE
}

SECTIONS
{
	. = ARM_EL3_TZC_DRAM1_BASE;
	ASSERT(. == ALIGN(PAGE_SIZE),
	"ARM_EL3_TZC_DRAM_BASE address is not aligned on a page boundary.")
	el3_tzc_dram (NOLOAD) : ALIGN(PAGE_SIZE) {
	__EL3_SEC_DRAM_START__ = .;
	*(arm_el3_tzc_dram)
	__EL3_SEC_DRAM_UNALIGNED_END__ = .;

	. = NEXT(PAGE_SIZE);
	__EL3_SEC_DRAM_END__ = .;
	} >EL3_SEC_DRAM
}

#endif /* __ARM_COMMON_LD_S__ */
