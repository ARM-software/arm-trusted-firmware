/*
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl2u_entrypoint)

MEMORY {
    RAM (rwx): ORIGIN = BL2U_BASE, LENGTH = BL2U_LIMIT - BL2U_BASE
}

SECTIONS {
    RAM_REGION_START = ORIGIN(RAM);
    RAM_REGION_LENGTH = LENGTH(RAM);
    . = BL2U_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL2U_BASE address is not aligned on a page boundary.")

#if SEPARATE_CODE_AND_RODATA
    .text . : {
        __TEXT_START__ = .;

        *bl2u_entrypoint.o(.text*)
        *(SORT_BY_ALIGNMENT(.text*))
        *(.vectors)
        __TEXT_END_UNALIGNED__ = .;

        . = ALIGN(PAGE_SIZE);

        __TEXT_END__ = .;
    } >RAM

    /* .ARM.extab and .ARM.exidx are only added because Clang needs them */
    .ARM.extab . : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >RAM

    .ARM.exidx . : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >RAM

    .rodata . : {
        __RODATA_START__ = .;
        *(SORT_BY_ALIGNMENT(.rodata*))

        RODATA_COMMON

        __RODATA_END_UNALIGNED__ = .;
        . = ALIGN(PAGE_SIZE);
        __RODATA_END__ = .;
    } >RAM
#else /* SEPARATE_CODE_AND_RODATA */
    .ro . : {
        __RO_START__ = .;

        *bl2u_entrypoint.o(.text*)
        *(SORT_BY_ALIGNMENT(.text*))
        *(SORT_BY_ALIGNMENT(.rodata*))

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
    STACK_SECTION >RAM
    BSS_SECTION >RAM
    XLAT_TABLE_SECTION >RAM

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored on their own pages and are
     * not mixed with normal data.  This is required to set up the correct
     * memory attributes for the coherent data page tables.
     */
    .coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        __COHERENT_RAM_START__ = .;
        *(.tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as device
         * memory. No other unexpected data must creep in. Ensure the rest of
         * the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __COHERENT_RAM_END__ = .;
    } >RAM
#endif /* USE_COHERENT_MEM */

    __RW_END__ = .;
    __BL2U_END__ = .;

    __BSS_SIZE__ = SIZEOF(.bss);

    ASSERT(. <= BL2U_LIMIT, "BL2U image has exceeded its limit.")
    RAM_REGION_END = .;
}
