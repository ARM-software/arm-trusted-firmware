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
    /* RAM is read/write and Initialised */
    RAM (rwx): ORIGIN = TZRAM_BASE, LENGTH = TZRAM_SIZE
}


SECTIONS
{
    . = BL2_BASE;

    BL2_RO NEXT (4096): {
        *(entry_code)
        *(.text .rodata)
    } >RAM

    BL2_STACKS NEXT (4096): {
        *(tzfw_normal_stacks)
    } >RAM

    BL2_COHERENT_RAM NEXT (4096): {
        *(tzfw_coherent_mem)
        /*       . += 0x1000;*/
        /* Do we need to ensure at least 4k here? */
         . = NEXT(4096);
    } >RAM

    __BL2_DATA_START__ = .;
    .bss NEXT (4096): {
        *(SORT_BY_ALIGNMENT(.bss))
        *(COMMON)
    } >RAM

    .data : {
        *(.data)
    } >RAM
    __BL2_DATA_STOP__ = .;


    __BL2_RO_BASE__ = LOADADDR(BL2_RO);
    __BL2_RO_SIZE__ = SIZEOF(BL2_RO);

    __BL2_STACKS_BASE__ = LOADADDR(BL2_STACKS);
    __BL2_STACKS_SIZE__ = SIZEOF(BL2_STACKS);

    __BL2_COHERENT_RAM_BASE__ = LOADADDR(BL2_COHERENT_RAM);
    __BL2_COHERENT_RAM_SIZE__ = SIZEOF(BL2_COHERENT_RAM);

    __BL2_RW_BASE__ = __BL2_DATA_START__;
    __BL2_RW_SIZE__ = __BL2_DATA_STOP__ - __BL2_DATA_START__;
}
