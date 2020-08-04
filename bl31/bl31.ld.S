/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl31_entrypoint)


MEMORY {
    RAM (rwx): ORIGIN = BL31_BASE, LENGTH = BL31_LIMIT - BL31_BASE
#if SEPARATE_NOBITS_REGION
    NOBITS (rw!a): ORIGIN = BL31_NOBITS_BASE, LENGTH = BL31_NOBITS_LIMIT - BL31_NOBITS_BASE
#else
#define NOBITS RAM
#endif
}

#ifdef PLAT_EXTRA_LD_SCRIPT
#include <plat.ld.S>
#endif

SECTIONS
{
    . = BL31_BASE;
    ASSERT(. == ALIGN(PAGE_SIZE),
           "BL31_BASE address is not aligned on a page boundary.")

    __BL31_START__ = .;

#if SEPARATE_CODE_AND_RODATA
    .text . : {
        __TEXT_START__ = .;
        *bl31_entrypoint.o(.text*)
        *(SORT_BY_ALIGNMENT(SORT(.text*)))
        *(.vectors)
        . = ALIGN(PAGE_SIZE);
        __TEXT_END__ = .;
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
        *bl31_entrypoint.o(.text*)
        *(SORT_BY_ALIGNMENT(.text*))
        *(SORT_BY_ALIGNMENT(.rodata*))

	RODATA_COMMON

        /* Place pubsub sections for events */
        . = ALIGN(8);
#include <lib/el3_runtime/pubsub_events.h>

        *(.vectors)
        __RO_END_UNALIGNED__ = .;
        /*
         * Memory page(s) mapped to this section will be marked as read-only,
         * executable.  No RW data from the next section must creep in.
         * Ensure the rest of the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);
        __RO_END__ = .;
    } >RAM
#endif

    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
           "cpu_ops not defined for this platform.")

#if SPM_MM
#ifndef SPM_SHIM_EXCEPTIONS_VMA
#define SPM_SHIM_EXCEPTIONS_VMA         RAM
#endif

    /*
     * Exception vectors of the SPM shim layer. They must be aligned to a 2K
     * address, but we need to place them in a separate page so that we can set
     * individual permissions to them, so the actual alignment needed is 4K.
     *
     * There's no need to include this into the RO section of BL31 because it
     * doesn't need to be accessed by BL31.
     */
    spm_shim_exceptions : ALIGN(PAGE_SIZE) {
        __SPM_SHIM_EXCEPTIONS_START__ = .;
        *(.spm_shim_exceptions)
        . = ALIGN(PAGE_SIZE);
        __SPM_SHIM_EXCEPTIONS_END__ = .;
    } >SPM_SHIM_EXCEPTIONS_VMA AT>RAM

    PROVIDE(__SPM_SHIM_EXCEPTIONS_LMA__ = LOADADDR(spm_shim_exceptions));
    . = LOADADDR(spm_shim_exceptions) + SIZEOF(spm_shim_exceptions);
#endif

    /*
     * Define a linker symbol to mark start of the RW memory area for this
     * image.
     */
    __RW_START__ = . ;

    DATA_SECTION >RAM
    RELA_SECTION >RAM

#ifdef BL31_PROGBITS_LIMIT
    ASSERT(. <= BL31_PROGBITS_LIMIT, "BL31 progbits has exceeded its limit.")
#endif

#if SEPARATE_NOBITS_REGION
    /*
     * Define a linker symbol to mark end of the RW memory area for this
     * image.
     */
    . = ALIGN(PAGE_SIZE);
    __RW_END__ = .;
    __BL31_END__ = .;

    ASSERT(. <= BL31_LIMIT, "BL31 image has exceeded its limit.")

    . = BL31_NOBITS_BASE;
    ASSERT(. == ALIGN(PAGE_SIZE),
           "BL31 NOBITS base address is not aligned on a page boundary.")

    __NOBITS_START__ = .;
#endif

    STACK_SECTION >NOBITS
    BSS_SECTION >NOBITS
    XLAT_TABLE_SECTION >NOBITS

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
    } >NOBITS
#endif

#if SEPARATE_NOBITS_REGION
    /*
     * Define a linker symbol to mark end of the NOBITS memory area for this
     * image.
     */
    __NOBITS_END__ = .;

    ASSERT(. <= BL31_NOBITS_LIMIT, "BL31 NOBITS region has exceeded its limit.")
#else
    /*
     * Define a linker symbol to mark end of the RW memory area for this
     * image.
     */
    __RW_END__ = .;
    __BL31_END__ = .;

    /DISCARD/ : {
        *(.dynsym .dynstr .hash .gnu.hash)
    }

    ASSERT(. <= BL31_LIMIT, "BL31 image has exceeded its limit.")
#endif
}
