/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl2_entrypoint)

MEMORY {
#if BL2_IN_XIP_MEM
    ROM (rx): ORIGIN = BL2_RO_BASE, LENGTH = BL2_RO_LIMIT - BL2_RO_BASE
    RAM (rwx): ORIGIN = BL2_RW_BASE, LENGTH = BL2_RW_LIMIT - BL2_RW_BASE
#else /* BL2_IN_XIP_MEM */
    RAM (rwx): ORIGIN = BL2_BASE, LENGTH = BL2_LIMIT - BL2_BASE
#endif /* BL2_IN_XIP_MEM */

#if SEPARATE_BL2_NOLOAD_REGION
    RAM_NOLOAD (rw!a): ORIGIN = BL2_NOLOAD_START, LENGTH = BL2_NOLOAD_LIMIT - BL2_NOLOAD_START
#else /* SEPARATE_BL2_NOLOAD_REGION */
#   define RAM_NOLOAD RAM
#endif /* SEPARATE_BL2_NOLOAD_REGION */
}

#if !BL2_IN_XIP_MEM
#   define ROM RAM
#endif /* !BL2_IN_XIP_MEM */

SECTIONS {
    RAM_REGION_START = ORIGIN(RAM);
    RAM_REGION_LENGTH = LENGTH(RAM);
#if BL2_IN_XIP_MEM
    ROM_REGION_START = ORIGIN(ROM);
    ROM_REGION_LENGTH = LENGTH(ROM);

    . = BL2_RO_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL2_RO_BASE address is not aligned on a page boundary.")
#else /* BL2_IN_XIP_MEM */
    . = BL2_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL2_BASE address is not aligned on a page boundary.")
#endif /* BL2_IN_XIP_MEM */

#if SEPARATE_BL2_NOLOAD_REGION
    RAM_NOLOAD_REGION_START = ORIGIN(RAM_NOLOAD);
    RAM_NOLOAD_REGION_LENGTH = LENGTH(RAM_NOLOAD);
#endif

#if SEPARATE_CODE_AND_RODATA
    .text . : {
        __TEXT_START__ = .;
        __TEXT_RESIDENT_START__ = .;

        *bl2_el3_entrypoint.o(.text*)
        *(.text.asm.*)

        __TEXT_RESIDENT_END__ = .;

        *(SORT_BY_ALIGNMENT(.text*))
        *(.vectors)
        __TEXT_END_UNALIGNED__ = .;

        . = ALIGN(PAGE_SIZE);

        __TEXT_END__ = .;
    } >ROM

    .rodata . : {
        __RODATA_START__ = .;

        *(SORT_BY_ALIGNMENT(.rodata*))

        RODATA_COMMON

        __RODATA_END_UNALIGNED__ = .;
        . = ALIGN(PAGE_SIZE);

        __RODATA_END__ = .;
    } >ROM

    ASSERT(__TEXT_RESIDENT_END__ - __TEXT_RESIDENT_START__ <= PAGE_SIZE,
        "Resident part of BL2 has exceeded its limit.")
#else /* SEPARATE_CODE_AND_RODATA */
    .ro . : {
        __RO_START__ = .;
        __TEXT_RESIDENT_START__ = .;

        *bl2_el3_entrypoint.o(.text*)
        *(.text.asm.*)

        __TEXT_RESIDENT_END__ = .;

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
    } >ROM
#endif /* SEPARATE_CODE_AND_RODATA */

    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
        "cpu_ops not defined for this platform.")

#if BL2_IN_XIP_MEM
    ROM_REGION_END = .;
    . = BL2_RW_BASE;

    ASSERT(BL2_RW_BASE == ALIGN(PAGE_SIZE),
           "BL2_RW_BASE address is not aligned on a page boundary.")
#endif /* BL2_IN_XIP_MEM */

    __RW_START__ = .;

    DATA_SECTION >RAM AT>ROM

    __DATA_RAM_START__ = __DATA_START__;
    __DATA_RAM_END__ = __DATA_END__;

    RELA_SECTION >RAM

#if SEPARATE_BL2_NOLOAD_REGION
    SAVED_ADDR = .;

    . = BL2_NOLOAD_START;

    __BL2_NOLOAD_START__ = .;
#endif /* SEPARATE_BL2_NOLOAD_REGION */

    STACK_SECTION >RAM_NOLOAD
    BSS_SECTION >RAM_NOLOAD
    XLAT_TABLE_SECTION >RAM_NOLOAD

#if SEPARATE_BL2_NOLOAD_REGION
    __BL2_NOLOAD_END__ = .;
    RAM_NOLOAD_REGION_END = .;

    . = SAVED_ADDR;
#endif /* SEPARATE_BL2_NOLOAD_REGION */

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
    __BL2_END__ = .;

    /DISCARD/ : {
        *(.dynsym .dynstr .hash .gnu.hash)
    }

#if BL2_IN_XIP_MEM
    __BL2_RAM_START__ = ADDR(.data);
    __BL2_RAM_END__ = .;

    __DATA_ROM_START__ = LOADADDR(.data);
    __DATA_SIZE__ = SIZEOF(.data);

    /*
     * The .data section is the last PROGBITS section so its end marks the end
     * of BL2's RO content in XIP memory.
     */
    __BL2_ROM_END__ =  __DATA_ROM_START__ + __DATA_SIZE__;

    ASSERT(__BL2_ROM_END__ <= BL2_RO_LIMIT,
           "BL2's RO content has exceeded its limit.")
#endif /* BL2_IN_XIP_MEM */

    __BSS_SIZE__ = SIZEOF(.bss);

#if USE_COHERENT_MEM
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif /* USE_COHERENT_MEM */

    RAM_REGION_END = .;
#if BL2_IN_XIP_MEM
    ASSERT(. <= BL2_RW_LIMIT, "BL2's RW content has exceeded its limit.")
#else /* BL2_IN_XIP_MEM */
    ASSERT(. <= BL2_LIMIT, "BL2 image has exceeded its limit.")
#endif /* BL2_IN_XIP_MEM */
}
