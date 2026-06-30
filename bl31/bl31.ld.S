/*
 * Copyright (c) 2013-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.ld.h>
#include <lib/per_cpu/per_cpu_defs.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <platform_def.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl31_entrypoint)

MEMORY {

    RAM (rwx): ORIGIN = BL31_BASE, \
               LENGTH = BL31_LIMIT - BL31_BASE - (ENABLE_LFA_BL31 * (2 * PAGE_SIZE))

#if ENABLE_LFA_BL31
    LFA (rwx): ORIGIN = BL31_LIMIT - (2 * PAGE_SIZE), LENGTH = (2 * PAGE_SIZE)
#endif

#if SEPARATE_NOBITS_REGION
    NOBITS (rw!a): ORIGIN = BL31_NOBITS_BASE, LENGTH = BL31_NOBITS_LIMIT - BL31_NOBITS_BASE
#else /* SEPARATE_NOBITS_REGION */
#   define NOBITS RAM
#endif /* SEPARATE_NOBITS_REGION */

#if SEPARATE_RWDATA_REGION
    RAM_RW (rw): ORIGIN = BL31_RWDATA_BASE, LENGTH = BL31_RWDATA_LIMIT - BL31_RWDATA_BASE
#else /* SEPARATE_RWDATA_REGION */
#define RAM_RW RAM
#endif /* SEPARATE_RWDATA_REGION */
}

#if PLAT_EXTRA_LD_SCRIPT
#   include <plat.ld.S>
#endif /* PLAT_EXTRA_LD_SCRIPT */

SECTIONS {
    RAM_REGION_START = ORIGIN(RAM);
    RAM_REGION_LENGTH = LENGTH(RAM);
    . = BL31_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL31_BASE address is not aligned on a page boundary.")

    __BL31_START__ = .;

#if SEPARATE_CODE_AND_RODATA
    .text . : {
        ASSERT(. == ALIGN(PAGE_SIZE),
        ".text is not aligned on a page boundary.");

        __TEXT_START__ = .;

        *bl31_entrypoint.o(.text*)

#if ENABLE_LFA_BL31
        /*
         * When LFA for BL31 is enabled we must group these two subsections
         * together so they live in a fixed area of the binary that can be
         * padded to a constant size. This means this section can be replaced
         * on it's own and expand for new workarounds to be added. Otherwise
         * the linker is free to do whatever it likes with them in text and
         * rodata as usual.
         */
        . = ALIGN(PAGE_SIZE);
        __ERRATA_START__ = .;
        *(.text.errata*)
        *(.rodata.errata*)

        ASSERT(PLAT_LFA_ERRATA_SECTION_SIZE >= (. - __ERRATA_START__),
               "Section .errata is too big for PLAT_LFA_ERRATA_SECTION_SIZE!");

        FILL(0x00)
        . += (( PLAT_LFA_ERRATA_SECTION_SIZE > (. - __ERRATA_START__) )
            ?  ( PLAT_LFA_ERRATA_SECTION_SIZE - (. - __ERRATA_START__) )
            :  0);

        . = ALIGN(PAGE_SIZE);
        __ERRATA_END__ = .;
#endif

        *(SORT_BY_ALIGNMENT(SORT(.text*)))
        *(.vectors)
        __TEXT_END_UNALIGNED__ = .;

        . = ALIGN(PAGE_SIZE);

        __TEXT_END__ = .;
    } >RAM

    .rodata . : {
        __RODATA_START__ = .;

        *(SORT_BY_ALIGNMENT(.rodata*))

#   if PLAT_EXTRA_RODATA_INCLUDES
#       include <plat.ld.rodata.inc>
#   endif /* PLAT_EXTRA_RODATA_INCLUDES */

        RODATA_COMMON

        __GPC_GPF_TRAPS_START__ = .;
        KEEP(*(SORT_BY_NAME(.el3_gpf_traps*)))
        __GPC_GPF_TRAPS_END__ = .;

        . = ALIGN(8);

#   include <lib/el3_runtime/pubsub_events.h>
        __RODATA_END_UNALIGNED__ = .;

        . = ALIGN(PAGE_SIZE);

        __RODATA_END__ = .;
    } >RAM
#else /* SEPARATE_CODE_AND_RODATA */
    .ro . : {
        ASSERT(. == ALIGN(PAGE_SIZE),
        ".ro is not aligned on a page boundary.");

        __RO_START__ = .;

        *bl31_entrypoint.o(.text*)

#if ENABLE_LFA_BL31
        /*
         * When LFA for BL31 is enabled we must group these two subsections
         * together so they live in a fixed area of the binary that can be
         * padded to a constant size. This means this section can be replaced
         * on it's own and expand for new workarounds to be added. Otherwise
         * the linker is free to do whatever it likes with them in text and
         * rodata as usual.
         */
        . = ALIGN(PAGE_SIZE);
        __ERRATA_START__ = .;
        *(.text.errata*)
        *(.rodata.errata*)

        ASSERT(PLAT_LFA_ERRATA_SECTION_SIZE >= (. - __ERRATA_START__),
               "Section .errata is too big for PLAT_LFA_ERRATA_SECTION_SIZE!");

        FILL(0x00)
        . += (( PLAT_LFA_ERRATA_SECTION_SIZE > (. - __ERRATA_START__) )
            ?  ( PLAT_LFA_ERRATA_SECTION_SIZE - (. - __ERRATA_START__) )
            :  0);

        . = ALIGN(PAGE_SIZE);
        __ERRATA_END__ = .;
#endif

        *(SORT_BY_ALIGNMENT(.text*))
        *(SORT_BY_ALIGNMENT(.rodata*))

        RODATA_COMMON

        . = ALIGN(8);

#   include <lib/el3_runtime/pubsub_events.h>

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

    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
        "cpu_ops not defined for this platform.")

#if SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP)
#   ifndef SPM_SHIM_EXCEPTIONS_VMA
#       define SPM_SHIM_EXCEPTIONS_VMA RAM
#   endif /* SPM_SHIM_EXCEPTIONS_VMA */

    /*
     * Exception vectors of the SPM shim layer. They must be aligned to a 2K
     * address but we need to place them in a separate page so that we can set
     * individual permissions on them, so the actual alignment needed is the
     * page size.
     *
     * There's no need to include this into the RO section of BL31 because it
     * doesn't need to be accessed by BL31.
     */
    .spm_shim_exceptions : ALIGN(PAGE_SIZE) {
        __SPM_SHIM_EXCEPTIONS_START__ = .;

        *(.spm_shim_exceptions)

        . = ALIGN(PAGE_SIZE);

        __SPM_SHIM_EXCEPTIONS_END__ = .;
    } >SPM_SHIM_EXCEPTIONS_VMA AT>RAM

    PROVIDE(__SPM_SHIM_EXCEPTIONS_LMA__ = LOADADDR(.spm_shim_exceptions));

    . = LOADADDR(.spm_shim_exceptions) + SIZEOF(.spm_shim_exceptions);
#endif /* SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP) */

#if ENABLE_LFA_BL31
    .lfa_relocatable : ALIGN(PAGE_SIZE) {
        __LFA_RELOCATABLE_CODE_START__ = .;
        KEEP(*(.lfa_relocatable_code*))
        __LFA_RELOCATABLE_CODE_END__ = .;
        . = ALIGN(PAGE_SIZE);
        __LFA_RELOCATABLE_DATA_START__ = .;
        KEEP(*(.lfa_relocatable_data*))
        . = ALIGN(PAGE_SIZE);
        __LFA_RELOCATABLE_DATA_END__ = .;
        __LFA_RELOCATABLE_LIMIT__ = .;
    } >LFA AT> RAM

    PROVIDE(__LFA_RELOCATABLE_LMA__ = LOADADDR(.lfa_relocatable));

    . = LOADADDR(.lfa_relocatable) + SIZEOF(.lfa_relocatable);
    ASSERT(__LFA_RELOCATABLE_LIMIT__ <= BL31_LIMIT, \
           "__LFA_RELOCATABLE_END__ has exceeded its limit.")
#endif /* ENABLE_LFA_BL31 */

#if SEPARATE_RWDATA_REGION
    . = BL31_RWDATA_BASE;
    ASSERT(BL31_RWDATA_BASE == ALIGN(PAGE_SIZE),
           "BL31_RWDATA_BASE address is not aligned on a page boundary.")

    /*
     * Define a linker symbol to mark the start of the RW memory area for this
     * image.
     */
    __RW_START__ = . ;

    DATA_SECTION >RAM_RW AT>RAM
    __DATA_RAM_START__ = __DATA_START__;
    __DATA_RAM_END__ = __DATA_END__;
    __DATA_ROM_START__ = LOADADDR(.data);

    . = ALIGN(PAGE_SIZE);
    __RW_END__ = .;

    RELA_SECTION >RAM
#else /* SEPARATE_RWDATA_REGION */
    /*
     * Define a linker symbol to mark the start of the RW memory area for this
     * image.
     */
    __RW_START__ = . ;

    DATA_SECTION >RAM
    RELA_SECTION >RAM
#endif /* SEPARATE_RWDATA_REGION */

#ifdef BL31_PROGBITS_LIMIT
    ASSERT(
        . <= BL31_PROGBITS_LIMIT,
        "BL31 progbits has exceeded its limit. Consider disabling some features."
    )
#endif /* BL31_PROGBITS_LIMIT */

#if SEPARATE_NOBITS_REGION
    . = ALIGN(PAGE_SIZE);

#if !SEPARATE_RWDATA_REGION
    __RW_END__ = .;
#endif /* SEPARATE_RWDATA_REGION */
    __BL31_END__ = .;

    ASSERT(. <= BL31_LIMIT - (ENABLE_LFA_BL31 * (2 * PAGE_SIZE)), \
           "BL31 image has exceeded its limit.")

    . = BL31_NOBITS_BASE;

    ASSERT(. == ALIGN(PAGE_SIZE),
        "BL31 NOBITS base address is not aligned on a page boundary.")

    __NOBITS_START__ = .;
#endif /* SEPARATE_NOBITS_REGION */

    STACK_SECTION >NOBITS
    BSS_SECTION >NOBITS
    PER_CPU >NOBITS
    XLAT_TABLE_SECTION >NOBITS

#if USE_COHERENT_MEM
    /*
     * The base address of the coherent memory section must be page-aligned to
     * guarantee that the coherent data are stored on their own pages and are
     * not mixed with normal data.  This is required to set up the correct
     * memory attributes for the coherent data page tables.
     */
    .coherent_ram (NOLOAD) : ALIGN(PAGE_SIZE) {
        __COHERENT_RAM_START__ = .;

        /*
         * Bakery locks are stored in coherent memory. Each lock's data is
         * contiguous and fully allocated by the compiler.
         */
        *(.bakery_lock)
        *(.tzfw_coherent_mem)

        __COHERENT_RAM_END_UNALIGNED__ = .;

        /*
         * Memory page(s) mapped to this section will be marked as device
         * memory. No other unexpected data must creep in. Ensure the rest of
         * the current memory page is unused.
         */
        . = ALIGN(PAGE_SIZE);

        __COHERENT_RAM_END__ = .;
    } >NOBITS
#endif /* USE_COHERENT_MEM */

#if SEPARATE_NOBITS_REGION
    __NOBITS_END__ = .;

    ASSERT(. <= BL31_NOBITS_LIMIT, "BL31 NOBITS region has exceeded its limit.")
#else /* SEPARATE_NOBITS_REGION */
    /*
     * Define a linker symbol to mark the end of the RW memory area for this
     * image.
     */
#if !SEPARATE_RWDATA_REGION
    __RW_END__ = .;
#endif /* SEPARATE_RWDATA_REGION */
    __BL31_END__ = .;

    ASSERT(. <= BL31_LIMIT, "BL31 image has exceeded its limit.")
#endif /* SEPARATE_NOBITS_REGION */
    RAM_REGION_END = .;

    /DISCARD/ : {
        *(.dynsym .dynstr .hash .gnu.hash)
    }
}
