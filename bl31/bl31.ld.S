/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

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
        *(SORT_BY_ALIGNMENT(.text*))
        *(.vectors)
        . = ALIGN(PAGE_SIZE);
        __TEXT_END__ = .;
    } >RAM

    .rodata . : {
        __RODATA_START__ = .;
        *(SORT_BY_ALIGNMENT(.rodata*))

        /* Ensure 8-byte alignment for descriptors and ensure inclusion */
        . = ALIGN(8);
        __RT_SVC_DESCS_START__ = .;
        KEEP(*(rt_svc_descs))
        __RT_SVC_DESCS_END__ = .;

#if ENABLE_PMF
        /* Ensure 8-byte alignment for descriptors and ensure inclusion */
        . = ALIGN(8);
        __PMF_SVC_DESCS_START__ = .;
        KEEP(*(pmf_svc_descs))
        __PMF_SVC_DESCS_END__ = .;
#endif /* ENABLE_PMF */

        /*
         * Ensure 8-byte alignment for cpu_ops so that its fields are also
         * aligned. Also ensure cpu_ops inclusion.
         */
        . = ALIGN(8);
        __CPU_OPS_START__ = .;
        KEEP(*(cpu_ops))
        __CPU_OPS_END__ = .;

        /*
         * Keep the .got section in the RO section as it is patched
         * prior to enabling the MMU and having the .got in RO is better for
         * security. GOT is a table of addresses so ensure 8-byte alignment.
         */
        . = ALIGN(8);
        __GOT_START__ = .;
        *(.got)
        __GOT_END__ = .;

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

        /* Ensure 8-byte alignment for descriptors and ensure inclusion */
        . = ALIGN(8);
        __RT_SVC_DESCS_START__ = .;
        KEEP(*(rt_svc_descs))
        __RT_SVC_DESCS_END__ = .;

#if ENABLE_PMF
        /* Ensure 8-byte alignment for descriptors and ensure inclusion */
        . = ALIGN(8);
        __PMF_SVC_DESCS_START__ = .;
        KEEP(*(pmf_svc_descs))
        __PMF_SVC_DESCS_END__ = .;
#endif /* ENABLE_PMF */

        /*
         * Ensure 8-byte alignment for cpu_ops so that its fields are also
         * aligned. Also ensure cpu_ops inclusion.
         */
        . = ALIGN(8);
        __CPU_OPS_START__ = .;
        KEEP(*(cpu_ops))
        __CPU_OPS_END__ = .;

        /*
         * Keep the .got section in the RO section as it is patched
         * prior to enabling the MMU and having the .got in RO is better for
         * security. GOT is a table of addresses so ensure 8-byte alignment.
         */
        . = ALIGN(8);
        __GOT_START__ = .;
        *(.got)
        __GOT_END__ = .;

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

    /*
     * .data must be placed at a lower address than the stacks if the stack
     * protector is enabled. Alternatively, the .data.stack_protector_canary
     * section can be placed independently of the main .data section.
     */
   .data . : {
        __DATA_START__ = .;
        *(SORT_BY_ALIGNMENT(.data*))
        __DATA_END__ = .;
    } >RAM

    /*
     * .rela.dyn needs to come after .data for the read-elf utility to parse
     * this section correctly. Ensure 8-byte alignment so that the fields of
     * RELA data structure are aligned.
     */
    . = ALIGN(8);
    __RELA_START__ = .;
    .rela.dyn . : {
    } >RAM
    __RELA_END__ = .;

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

    stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >NOBITS

    /*
     * The .bss section gets initialised to 0 at runtime.
     * Its base address should be 16-byte aligned for better performance of the
     * zero-initialization code.
     */
    .bss (NOLOAD) : ALIGN(16) {
        __BSS_START__ = .;
        *(SORT_BY_ALIGNMENT(.bss*))
        *(COMMON)
#if !USE_COHERENT_MEM
        /*
         * Bakery locks are stored in normal .bss memory
         *
         * Each lock's data is spread across multiple cache lines, one per CPU,
         * but multiple locks can share the same cache line.
         * The compiler will allocate enough memory for one CPU's bakery locks,
         * the remaining cache lines are allocated by the linker script
         */
        . = ALIGN(CACHE_WRITEBACK_GRANULE);
        __BAKERY_LOCK_START__ = .;
        __PERCPU_BAKERY_LOCK_START__ = .;
        *(bakery_lock)
        . = ALIGN(CACHE_WRITEBACK_GRANULE);
        __PERCPU_BAKERY_LOCK_END__ = .;
        __PERCPU_BAKERY_LOCK_SIZE__ = ABSOLUTE(__PERCPU_BAKERY_LOCK_END__ - __PERCPU_BAKERY_LOCK_START__);
        . = . + (__PERCPU_BAKERY_LOCK_SIZE__ * (PLATFORM_CORE_COUNT - 1));
        __BAKERY_LOCK_END__ = .;

	/*
	 * If BL31 doesn't use any bakery lock then __PERCPU_BAKERY_LOCK_SIZE__
	 * will be zero. For this reason, the only two valid values for
	 * __PERCPU_BAKERY_LOCK_SIZE__ are 0 or the platform defined value
	 * PLAT_PERCPU_BAKERY_LOCK_SIZE.
	 */
#ifdef PLAT_PERCPU_BAKERY_LOCK_SIZE
    ASSERT((__PERCPU_BAKERY_LOCK_SIZE__ == 0) || (__PERCPU_BAKERY_LOCK_SIZE__ == PLAT_PERCPU_BAKERY_LOCK_SIZE),
        "PLAT_PERCPU_BAKERY_LOCK_SIZE does not match bakery lock requirements");
#endif
#endif

#if ENABLE_PMF
        /*
         * Time-stamps are stored in normal .bss memory
         *
         * The compiler will allocate enough memory for one CPU's time-stamps,
         * the remaining memory for other CPUs is allocated by the
         * linker script
         */
        . = ALIGN(CACHE_WRITEBACK_GRANULE);
        __PMF_TIMESTAMP_START__ = .;
        KEEP(*(pmf_timestamp_array))
        . = ALIGN(CACHE_WRITEBACK_GRANULE);
        __PMF_PERCPU_TIMESTAMP_END__ = .;
        __PERCPU_TIMESTAMP_SIZE__ = ABSOLUTE(. - __PMF_TIMESTAMP_START__);
        . = . + (__PERCPU_TIMESTAMP_SIZE__ * (PLATFORM_CORE_COUNT - 1));
        __PMF_TIMESTAMP_END__ = .;
#endif /* ENABLE_PMF */
        __BSS_END__ = .;
    } >NOBITS

    /*
     * The xlat_table section is for full, aligned page tables (4K).
     * Removing them from .bss avoids forcing 4K alignment on
     * the .bss section. The tables are initialized to zero by the translation
     * tables library.
     */
    xlat_table (NOLOAD) : {
        *(xlat_table)
    } >NOBITS

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
