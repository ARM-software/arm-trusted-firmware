/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(elf32-littlearm)
OUTPUT_ARCH(arm)
ENTRY(sp_min_vector_table)

MEMORY {
    RAM (rwx): ORIGIN = BL32_BASE, LENGTH = BL32_LIMIT - BL32_BASE
}

#ifdef PLAT_SP_MIN_EXTRA_LD_SCRIPT
#include <plat_sp_min.ld.S>
#endif

SECTIONS
{
    . = BL32_BASE;
    ASSERT(. == ALIGN(PAGE_SIZE),
           "BL32_BASE address is not aligned on a page boundary.")

#if SEPARATE_CODE_AND_RODATA
    .text . : {
        __TEXT_START__ = .;
        *entrypoint.o(.text*)
        *(SORT_BY_ALIGNMENT(.text*))
        *(.vectors)
        . = ALIGN(PAGE_SIZE);
        __TEXT_END__ = .;
    } >RAM

     /* .ARM.extab and .ARM.exidx are only added because Clang need them */
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

        /* Place pubsub sections for events */
        . = ALIGN(8);
#include <lib/el3_runtime/pubsub_events.h>

        . = ALIGN(PAGE_SIZE);
        __RODATA_END__ = .;
    } >RAM
#else
    ro . : {
        __RO_START__ = .;
        *entrypoint.o(.text*)
        *(SORT_BY_ALIGNMENT(.text*))
        *(SORT_BY_ALIGNMENT(.rodata*))

	RODATA_COMMON

        /* Place pubsub sections for events */
        . = ALIGN(8);
#include <lib/el3_runtime/pubsub_events.h>

        *(.vectors)
        __RO_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as
         * read-only, executable.  No RW data from the next section must
         * creep in.  Ensure the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);
        __RO_END__ = .;
    } >RAM
#endif

    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
           "cpu_ops not defined for this platform.")
    /*
     * Define a linker symbol to mark start of the RW memory area for this
     * image.
     */
    __RW_START__ = . ;

    DATA_SECTION >RAM

#ifdef BL32_PROGBITS_LIMIT
    ASSERT(. <= BL32_PROGBITS_LIMIT, "BL32 progbits has exceeded its limit.")
#endif

    STACK_SECTION >RAM
    BSS_SECTION >RAM
    XLAT_TABLE_SECTION >RAM

     __BSS_SIZE__ = SIZEOF(.bss);

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
    } >RAM

    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
#endif

    /*
     * Define a linker symbol to mark the end of the RW memory area for this
     * image.
     */
    __RW_END__ = .;

    __BL32_END__ = .;

    ASSERT(. <= BL32_LIMIT, "BL32 image has exceeded its limit.")
}
