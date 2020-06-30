/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
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
#else
    RAM (rwx): ORIGIN = BL2_BASE, LENGTH = BL2_LIMIT - BL2_BASE
#endif
}

#if !BL2_IN_XIP_MEM
#define ROM RAM
#endif

SECTIONS
{
#if BL2_IN_XIP_MEM
    . = BL2_RO_BASE;
    ASSERT(. == ALIGN(PAGE_SIZE),
           "BL2_RO_BASE address is not aligned on a page boundary.")
#else
    . = BL2_BASE;
    ASSERT(. == ALIGN(PAGE_SIZE),
           "BL2_BASE address is not aligned on a page boundary.")
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
        . = ALIGN(PAGE_SIZE);
        __TEXT_END__ = .;
     } >ROM

    .rodata . : {
        __RODATA_START__ = .;
        *(SORT_BY_ALIGNMENT(.rodata*))

	RODATA_COMMON

        . = ALIGN(PAGE_SIZE);
        __RODATA_END__ = .;
    } >ROM

    ASSERT(__TEXT_RESIDENT_END__ - __TEXT_RESIDENT_START__ <= PAGE_SIZE,
          "Resident part of BL2 has exceeded its limit.")
#else
    ro . : {
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
         * Memory page(s) mapped to this section will be marked as
         * read-only, executable.  No RW data from the next section must
         * creep in.  Ensure the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __RO_END__ = .;
    } >ROM
#endif

    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
          "cpu_ops not defined for this platform.")

#if BL2_IN_XIP_MEM
    . = BL2_RW_BASE;
    ASSERT(BL2_RW_BASE == ALIGN(PAGE_SIZE),
           "BL2_RW_BASE address is not aligned on a page boundary.")
#endif

    /*
     * Define a linker symbol to mark start of the RW memory area for this
     * image.
     */
    __RW_START__ = . ;

    DATA_SECTION >RAM AT>ROM
    __DATA_RAM_START__ = __DATA_START__;
    __DATA_RAM_END__ = __DATA_END__;

    RELA_SECTION >RAM
    STACK_SECTION >RAM
    BSS_SECTION >RAM
    XLAT_TABLE_SECTION >RAM

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned (4K)
     * to guarantee that the coherent data are stored on their own pages and
     * are not mixed with normal data.  This is required to set up the correct
     * memory attributes for the coherent data page tables.
     */
    coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        __COHERENT_RAM_START__ = .;
        *(tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;
        /*
         * Memory page(s) mapped to this section will be marked
         * as device memory.  No other unexpected data must creep in.
         * Ensure the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);
        __COHERENT_RAM_END__ = .;
    } >RAM
#endif

    /*
     * Define a linker symbol to mark end of the RW memory area for this
     * image.
     */
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
     * of BL2's RO content in XIP memory..
     */
    __BL2_ROM_END__ =  __DATA_ROM_START__ + __DATA_SIZE__;
    ASSERT(__BL2_ROM_END__ <= BL2_RO_LIMIT,
           "BL2's RO content has exceeded its limit.")
#endif
    __BSS_SIZE__ = SIZEOF(.bss);


#if USE_COHERENT_MEM
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif

#if BL2_IN_XIP_MEM
    ASSERT(. <= BL2_RW_LIMIT, "BL2's RW content has exceeded its limit.")
#else
    ASSERT(. <= BL2_LIMIT, "BL2 image has exceeded its limit.")
#endif
}
