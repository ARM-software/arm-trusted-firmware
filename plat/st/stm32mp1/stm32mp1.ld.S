/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_LD_S
#define STM32MP1_LD_S

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <platform_def.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)

ENTRY(__BL2_IMAGE_START__)

MEMORY {
	HEADER (rw) : ORIGIN = 0x00000000, LENGTH = STM32MP_HEADER_RESERVED_SIZE
	RAM (rwx) : ORIGIN = STM32MP_BINARY_BASE, LENGTH = STM32MP_BINARY_SIZE
}

SECTIONS
{
    /*
     * TF mapping must conform to ROM code specification.
     */
    .header : {
        __HEADER_START__ = .;
        KEEP(*(.header))
        . = ALIGN(4);
        __HEADER_END__ = .;
    } >HEADER

    . = STM32MP_BINARY_BASE;
    .data . : {
        . = ALIGN(PAGE_SIZE);
        __DATA_START__ = .;
        *(.data*)

        /*
         * dtb.
         * The strongest and only alignment contraint is MMU 4K page.
         * Indeed as images below will be removed, 4K pages will be re-used.
         */
        . = ( STM32MP_BL2_DTB_BASE - STM32MP_BINARY_BASE );
        __DTB_IMAGE_START__ = .;
        *(.dtb_image*)
        __DTB_IMAGE_END__ = .;

        /*
         * bl2.
         * The strongest and only alignment contraint is MMU 4K page.
         * Indeed as images below will be removed, 4K pages will be re-used.
         */
#if SEPARATE_CODE_AND_RODATA
        . = ( STM32MP_BL2_RO_BASE - STM32MP_BINARY_BASE );
#else
        . = ( STM32MP_BL2_BASE - STM32MP_BINARY_BASE );
#endif
        __BL2_IMAGE_START__ = .;
        *(.bl2_image*)
        __BL2_IMAGE_END__ = .;

        __DATA_END__ = .;
    } >RAM

    __TF_END__ = .;

}
#endif /* STM32MP1_LD_S */
