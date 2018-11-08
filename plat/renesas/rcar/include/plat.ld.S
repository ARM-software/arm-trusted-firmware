/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef RCAR_PLAT_LD_S
#define RCAR_PLAT_LD_S

#include <platform_def.h>
#include <xlat_tables_defs.h>

MEMORY {
    SRAM (rwx): ORIGIN = BL31_SRAM_BASE, LENGTH = DEVICE_SRAM_SIZE
    PRAM (r): ORIGIN = BL31_LIMIT - DEVICE_SRAM_SIZE, LENGTH = DEVICE_SRAM_SIZE
}

SECTIONS
{
	/* SRAM_COPY is in PRAM */
	. = BL31_LIMIT - DEVICE_SRAM_SIZE;
	__SRAM_COPY_START__ = .;

	.system_ram : {
		/* system ram start is in SRAM */
		__system_ram_start__ = .;
		*(.system_ram*)
		*iic_dvfs.o(.rodata)
	        __system_ram_end__ = .;
	} >SRAM AT>PRAM

    ASSERT(__BL31_END__ <= BL31_LIMIT - DEVICE_SRAM_SIZE,
    "BL31 image too large - writing on top of SRAM!")

}

#endif /* RCAR_PLAT_LD_S */
