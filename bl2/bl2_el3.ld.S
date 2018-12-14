/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

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
        *(.text*)
        *(.vectors)
        . = ALIGN(PAGE_SIZE);
        __TEXT_END__ = .;
#if BL2_IN_XIP_MEM
     } >ROM
#else
     } >RAM
#endif

    .rodata . : {
        __RODATA_START__ = .;
        *(.rodata*)

        /* Ensure 8-byte alignment for descriptors and ensure inclusion */
        . = ALIGN(8);
        __PARSER_LIB_DESCS_START__ = .;
        KEEP(*(.img_parser_lib_descs))
        __PARSER_LIB_DESCS_END__ = .;

        /*
         * Ensure 8-byte alignment for cpu_ops so that its fields are also
         * aligned. Also ensure cpu_ops inclusion.
         */
        . = ALIGN(8);
        __CPU_OPS_START__ = .;
        KEEP(*(cpu_ops))
        __CPU_OPS_END__ = .;

        . = ALIGN(PAGE_SIZE);
        __RODATA_END__ = .;
#if BL2_IN_XIP_MEM
    } >ROM
#else
    } >RAM
#endif

    ASSERT(__TEXT_RESIDENT_END__ - __TEXT_RESIDENT_START__ <= PAGE_SIZE,
          "Resident part of BL2 has exceeded its limit.")
#else
    ro . : {
        __RO_START__ = .;
	__TEXT_RESIDENT_START__ = .;
	*bl2_el3_entrypoint.o(.text*)
	*(.text.asm.*)
	__TEXT_RESIDENT_END__ = .;
        *(.text*)
        *(.rodata*)

        /*
         * Ensure 8-byte alignment for cpu_ops so that its fields are also
         * aligned. Also ensure cpu_ops inclusion.
         */
        . = ALIGN(8);
        __CPU_OPS_START__ = .;
        KEEP(*(cpu_ops))
        __CPU_OPS_END__ = .;

        /* Ensure 8-byte alignment for descriptors and ensure inclusion */
        . = ALIGN(8);
        __PARSER_LIB_DESCS_START__ = .;
        KEEP(*(.img_parser_lib_descs))
        __PARSER_LIB_DESCS_END__ = .;

        *(.vectors)
        __RO_END_UNALIGNED__ = .;
        /*
         * Memory page(s) mapped to this section will be marked as
         * read-only, executable.  No RW data from the next section must
         * creep in.  Ensure the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __RO_END__ = .;
#if BL2_IN_XIP_MEM
    } >ROM
#else
    } >RAM
#endif
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

    /*
     * .data must be placed at a lower address than the stacks if the stack
     * protector is enabled. Alternatively, the .data.stack_protector_canary
     * section can be placed independently of the main .data section.
     */
    .data . : {
        __DATA_RAM_START__ = .;
        *(.data*)
        __DATA_RAM_END__ = .;
#if BL2_IN_XIP_MEM
    } >RAM AT>ROM
#else
    } >RAM
#endif

    stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >RAM

    /*
     * The .bss section gets initialised to 0 at runtime.
     * Its base address should be 16-byte aligned for better performance of the
     * zero-initialization code.
     */
    .bss : ALIGN(16) {
        __BSS_START__ = .;
        *(SORT_BY_ALIGNMENT(.bss*))
        *(COMMON)
        __BSS_END__ = .;
    } >RAM

    /*
     * The xlat_table section is for full, aligned page tables (4K).
     * Removing them from .bss avoids forcing 4K alignment on
     * the .bss section. The tables are initialized to zero by the translation
     * tables library.
     */
    xlat_table (NOLOAD) : {
        *(xlat_table)
    } >RAM

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
