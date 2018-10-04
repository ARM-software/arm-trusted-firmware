/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <platform_def.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(ble_main)

MEMORY {
    RAM (rwx): ORIGIN = BLE_BASE, LENGTH = BLE_LIMIT - BLE_BASE
}

SECTIONS
{
    . = BLE_BASE;

    ro . : {
        __RO_START__ = .;
        *ble_main.o(.entry*)
        *(.text*)
        *(.rodata*)
        __RO_END_UNALIGNED__ = .;
        __RO_END__ = .;
    } >RAM

    /*
     * Define a linker symbol to mark start of the RW memory area for this
     * image.
     */
    __RW_START__ = . ;

    .data . : {
        __DATA_START__ = .;
        *(.data*)
        __DATA_END__ = .;
    } >RAM

    stacks . (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >RAM

    .bss : {
        __BSS_START__ = .;
        *(.bss*)
        __BSS_END__ = .;
    } >RAM

   /*
    * Extend the BLE binary to the maximum size allocated for it in platform
    * definition files and prevent overlapping between BLE BSS section and
    * additional extensions that can follow the BLE in flash image preamble.
    * This situation happens for instance when secure extension is added to
    * the image preamble.
    */
   .fill LOADADDR(.bss) + SIZEOF(.bss) : {
       FILL(0xDEADC0DE);
       . = ORIGIN(RAM) + LENGTH(RAM) - 1;
       BYTE(0x00)
   } >RAM

    /*
     * Define a linker symbol to mark end of the RW memory area for this
     * image.
     */
    __RW_END__ = .;
    __BLE_END__ = .;

    __BSS_SIZE__ = SIZEOF(.bss);
}
