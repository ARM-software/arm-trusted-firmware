/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl31_entrypoint)


MEMORY {
	RAM (rwx): ORIGIN = BL31_BASE, LENGTH = BL31_TZRAM_SIZE
	RAM2 (rwx): ORIGIN = TZRAM2_BASE, LENGTH = TZRAM2_SIZE
}


SECTIONS
{
    . = BL31_BASE;

    ASSERT(. == ALIGN(2048),
           "vector base is not aligned on a 2K boundary.")

    __RO_START__ = .;
    vector . : {
        *(.vectors)
    } >RAM

    ASSERT(. == ALIGN(PAGE_SIZE),
           "BL31_BASE address is not aligned on a page boundary.")

    ro . : {
        *bl31_entrypoint.o(.text*)
        *(.text*)
        *(.rodata*)

	RODATA_COMMON

        __RO_END_UNALIGNED__ = .;
        /*
         * Memory page(s) mapped to this section will be marked as read-only,
         * executable.  No RW data from the next section must creep in.
         * Ensure the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);
        __RO_END__ = .;
    } >RAM

    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
           "cpu_ops not defined for this platform.")

    /*
     * Define a linker symbol to mark start of the RW memory area for this
     * image.
     */
    __RW_START__ = . ;

    DATA_SECTION >RAM

#ifdef BL31_PROGBITS_LIMIT
    ASSERT(. <= BL31_PROGBITS_LIMIT, "BL3-1 progbits has exceeded its limit.")
#endif

    STACK_SECTION >RAM
    BSS_SECTION >RAM
    __RW_END__ = __BSS_END__;

    ASSERT(. <= BL31_LIMIT, "BL3-1 image has exceeded its limit.")

    XLAT_TABLE_SECTION >RAM2

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned (4K)
     * to guarantee that the coherent data are stored on their own pages and
     * are not mixed with normal data.  This is required to set up the correct
     * memory attributes for the coherent data page tables.
     */
    coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        __COHERENT_RAM_START__ = .;
        /*
         * Bakery locks are stored in coherent memory
         *
         * Each lock's data is contiguous and fully allocated by the compiler
         */
        *(bakery_lock)
        *(tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;
        /*
         * Memory page(s) mapped to this section will be marked
         * as device memory.  No other unexpected data must creep in.
         * Ensure the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);
        __COHERENT_RAM_END__ = .;
    } >RAM2
#endif

    /*
     * Define a linker symbol to mark end of the RW memory area for this
     * image.
     */
    __BL31_END__ = .;

    __BSS_SIZE__ = SIZEOF(.bss);
#if USE_COHERENT_MEM
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif

    ASSERT(. <= TZRAM2_LIMIT, "TZRAM2 image has exceeded its limit.")
}
