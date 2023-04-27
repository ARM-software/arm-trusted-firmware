/*
 * Copyright (c) 2013-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(tsp_entrypoint)

MEMORY {
    RAM (rwx): ORIGIN = TSP_SEC_MEM_BASE, LENGTH = TSP_SEC_MEM_SIZE
}

SECTIONS {
    RAM_REGION_START = ORIGIN(RAM);
    RAM_REGION_LENGTH = LENGTH(RAM);
    . = BL32_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL32_BASE address is not aligned on a page boundary.")

#if SEPARATE_CODE_AND_RODATA
    .text . : {
        __TEXT_START__ = .;

        *tsp_entrypoint.o(.text*)
        *(.text*)
        *(.vectors)
        __TEXT_END_UNALIGNED__ = .;

        . = ALIGN(PAGE_SIZE);

        __TEXT_END__ = .;
    } >RAM

    .rodata . : {
        __RODATA_START__ = .;

        *(.rodata*)

        RODATA_COMMON

        __RODATA_END_UNALIGNED__ = .;
        . = ALIGN(PAGE_SIZE);

        __RODATA_END__ = .;
    } >RAM
#else /* SEPARATE_CODE_AND_RODATA */
    .ro . : {
        __RO_START__ = .;

        *tsp_entrypoint.o(.text*)
        *(.text*)
        *(.rodata*)

        RODATA_COMMON

        *(.vectors)

        __RO_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as read-only,
         * executable. No RW data from the next section must creep in. Ensure
         * that the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __RO_END__ = .;
    } >RAM
#endif /* SEPARATE_CODE_AND_RODATA */

    __RW_START__ = .;

    DATA_SECTION >RAM
    RELA_SECTION >RAM

#ifdef TSP_PROGBITS_LIMIT
    ASSERT(. <= TSP_PROGBITS_LIMIT, "TSP progbits has exceeded its limit.")
#endif /* TSP_PROGBITS_LIMIT */

    STACK_SECTION >RAM
    BSS_SECTION >RAM
    XLAT_TABLE_SECTION >RAM

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored on their own pages and are
     * not mixed with normal data. This is required to set up the correct memory
     * attributes for the coherent data page tables.
     */
    .coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        __COHERENT_RAM_START__ = .;
        *(.tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as device
         * memory. No other unexpected data must creep in. Ensure that the rest
         * of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __COHERENT_RAM_END__ = .;
    } >RAM
#endif /* USE_COHERENT_MEM */

    __RW_END__ = .;
    __BL32_END__ = .;

    /DISCARD/ : {
        *(.dynsym .dynstr .hash .gnu.hash)
    }

    __BSS_SIZE__ = SIZEOF(.bss);

#if USE_COHERENT_MEM
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif /* USE_COHERENT_MEM */

    ASSERT(. <= BL32_LIMIT, "BL32 image has exceeded its limit.")
    RAM_REGION_END = .;
}
