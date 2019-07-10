/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Stub linker script to provide the armstub8.bin header before the actual
 * code. If the GPU firmware finds a magic value at offset 240 in
 * armstub8.bin, it will put the DTB and kernel load address in subsequent
 * words. We can then read those values to find the proper NS entry point
 * and find our DTB more flexibly.
 */

MEMORY {
    PRERAM (rwx): ORIGIN = 0, LENGTH = 4096
}

SECTIONS
{
    .armstub8 . : {
        *armstub8_header.o(.text*)
        KEEP(*(.armstub8))
    } >PRERAM
}
