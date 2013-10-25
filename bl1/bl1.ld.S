/*
 * Copyright (c) 2013, ARM Limited. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <platform.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)

MEMORY {
    /* ROM is read-only and executable */
    ROM (rx): ORIGIN = TZROM_BASE, LENGTH = TZROM_SIZE
    /* RAM is read/write and Initialised */
    RAM (rwx): ORIGIN = TZRAM_BASE, LENGTH = TZRAM_SIZE
}

SECTIONS
{
    FIRMWARE_ROM : {
        *(reset_code)
        *(.text)
        *(.rodata)
    } >ROM

    .bss : {
        __BSS_RAM_START__ = .;
        *(.bss)
        *(COMMON)
        __BSS_RAM_STOP__ = .;
    } >RAM AT>ROM

    .data : {
        __DATA_RAM_START__ = .;
        *(.data)
        __DATA_RAM_STOP__ = .;
     } >RAM AT>ROM

    FIRMWARE_RAM_STACKS ALIGN (PLATFORM_CACHE_LINE_SIZE) : {
        . += 0x1000;
        *(tzfw_normal_stacks)
        . = ALIGN(4096);
    } >RAM AT>ROM

    FIRMWARE_RAM_COHERENT ALIGN (4096): {
        *(tzfw_coherent_mem)
/*      . += 0x1000;*/
/* Do we need to make sure this is at least 4k? */
         . = ALIGN(4096);
    } >RAM

    __FIRMWARE_ROM_START__ = LOADADDR(FIRMWARE_ROM);
    __FIRMWARE_ROM_SIZE__  = SIZEOF(FIRMWARE_ROM);

    __FIRMWARE_DATA_START__ = LOADADDR(.data);
    __FIRMWARE_DATA_SIZE__  = SIZEOF(.data);

    __FIRMWARE_BSS_START__ = LOADADDR(.bss);
    __FIRMWARE_BSS_SIZE__  = SIZEOF(.bss);

    __FIRMWARE_RAM_STACKS_START__ = LOADADDR(FIRMWARE_RAM_STACKS);
    __FIRMWARE_RAM_STACKS_SIZE__  = SIZEOF(FIRMWARE_RAM_STACKS);
    __FIRMWARE_RAM_COHERENT_START__ = LOADADDR(FIRMWARE_RAM_COHERENT);
    __FIRMWARE_RAM_COHERENT_SIZE__  = SIZEOF(FIRMWARE_RAM_COHERENT);
}
