/*
 * Copyright (c) 2013-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <common/bl_common.h>
#include <el3_common_macros.S>
#include <lib/pmf/aarch64/pmf_asm_macros.S>
#include <lib/runtime_instr.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>

	.globl	bl31_entrypoint
	.globl	bl31_warm_entrypoint

	/* -----------------------------------------------------
	 * bl31_entrypoint() is the cold boot entrypoint,
	 * executed only by the primary cpu.
	 * -----------------------------------------------------
	 */

func bl31_entrypoint
	/* ---------------------------------------------------------------
	 * Stash the previous bootloader arguments x0 - x3 for later use.
	 * ---------------------------------------------------------------
	 */
	mov	x20, x0
	mov	x21, x1
	mov	x22, x2
	mov	x23, x3

#if !RESET_TO_BL31
	/* ---------------------------------------------------------------------
	 * For !RESET_TO_BL31 systems, only the primary CPU ever reaches
	 * bl31_entrypoint() during the cold boot flow, so the cold/warm boot
	 * and primary/secondary CPU logic should not be executed in this case.
	 *
	 * Also, assume that the previous bootloader has already initialised the
	 * SCTLR_EL3, including the endianness, and has initialised the memory.
	 * ---------------------------------------------------------------------
	 */
	el3_entrypoint_common					\
		_init_sctlr=0					\
		_warm_boot_mailbox=0				\
		_secondary_cold_boot=0				\
		_init_memory=0					\
		_init_c_runtime=1				\
		_exception_vectors=runtime_exceptions		\
		_pie_fixup_size=BL31_LIMIT - BL31_BASE
#else

	/* ---------------------------------------------------------------------
	 * For RESET_TO_BL31 systems which have a programmable reset address,
	 * bl31_entrypoint() is executed only on the cold boot path so we can
	 * skip the warm boot mailbox mechanism.
	 * ---------------------------------------------------------------------
	 */
	el3_entrypoint_common					\
		_init_sctlr=1					\
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS	\
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU	\
		_init_memory=1					\
		_init_c_runtime=1				\
		_exception_vectors=runtime_exceptions		\
		_pie_fixup_size=BL31_LIMIT - BL31_BASE

	/* ---------------------------------------------------------------------
	 * For RESET_TO_BL31 systems, BL31 is the first bootloader to run so
	 * there's no argument to relay from a previous bootloader. Zero the
	 * arguments passed to the platform layer to reflect that.
	 * ---------------------------------------------------------------------
	 */
	mov	x20, 0
	mov	x21, 0
	mov	x22, 0
	mov	x23, 0
#endif /* RESET_TO_BL31 */

	/* --------------------------------------------------------------------
	 * Perform BL31 setup
	 * --------------------------------------------------------------------
	 */
	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	mov	x3, x23
	bl	bl31_setup

#if ENABLE_PAUTH
	/* --------------------------------------------------------------------
	 * Program APIAKey_EL1 and enable pointer authentication
	 * --------------------------------------------------------------------
	 */
	bl	pauth_init_enable_el3
#endif /* ENABLE_PAUTH */

	/* --------------------------------------------------------------------
	 * Jump to main function
	 * --------------------------------------------------------------------
	 */
	bl	bl31_main

	/* --------------------------------------------------------------------
	 * Clean the .data & .bss sections to main memory. This ensures
	 * that any global data which was initialised by the primary CPU
	 * is visible to secondary CPUs before they enable their data
	 * caches and participate in coherency.
	 * --------------------------------------------------------------------
	 */
	adrp	x0, __DATA_START__
	add	x0, x0, :lo12:__DATA_START__
	adrp	x1, __DATA_END__
	add	x1, x1, :lo12:__DATA_END__
	sub	x1, x1, x0
	bl	clean_dcache_range

	adrp	x0, __BSS_START__
	add	x0, x0, :lo12:__BSS_START__
	adrp	x1, __BSS_END__
	add	x1, x1, :lo12:__BSS_END__
	sub	x1, x1, x0
	bl	clean_dcache_range

	b	el3_exit
endfunc bl31_entrypoint

	/* --------------------------------------------------------------------
	 * This CPU has been physically powered up. It is either resuming from
	 * suspend or has simply been turned on. In both cases, call the BL31
	 * warmboot entrypoint
	 * --------------------------------------------------------------------
	 */
func bl31_warm_entrypoint
#if ENABLE_RUNTIME_INSTRUMENTATION

	/*
	 * This timestamp update happens with cache off.  The next
	 * timestamp collection will need to do cache maintenance prior
	 * to timestamp update.
	 */
	pmf_calc_timestamp_addr rt_instr_svc, RT_INSTR_EXIT_HW_LOW_PWR
	mrs	x1, cntpct_el0
	str	x1, [x0]
#endif

	/*
	 * On the warm boot path, most of the EL3 initialisations performed by
	 * 'el3_entrypoint_common' must be skipped:
	 *
	 *  - Only when the platform bypasses the BL1/BL31 entrypoint by
	 *    programming the reset address do we need to initialise SCTLR_EL3.
	 *    In other cases, we assume this has been taken care by the
	 *    entrypoint code.
	 *
	 *  - No need to determine the type of boot, we know it is a warm boot.
	 *
	 *  - Do not try to distinguish between primary and secondary CPUs, this
	 *    notion only exists for a cold boot.
	 *
	 *  - No need to initialise the memory or the C runtime environment,
	 *    it has been done once and for all on the cold boot path.
	 */
	el3_entrypoint_common					\
		_init_sctlr=PROGRAMMABLE_RESET_ADDRESS		\
		_warm_boot_mailbox=0				\
		_secondary_cold_boot=0				\
		_init_memory=0					\
		_init_c_runtime=0				\
		_exception_vectors=runtime_exceptions		\
		_pie_fixup_size=0

	/*
	 * We're about to enable MMU and participate in PSCI state coordination.
	 *
	 * The PSCI implementation invokes platform routines that enable CPUs to
	 * participate in coherency. On a system where CPUs are not
	 * cache-coherent without appropriate platform specific programming,
	 * having caches enabled until such time might lead to coherency issues
	 * (resulting from stale data getting speculatively fetched, among
	 * others). Therefore we keep data caches disabled even after enabling
	 * the MMU for such platforms.
	 *
	 * On systems with hardware-assisted coherency, or on single cluster
	 * platforms, such platform specific programming is not required to
	 * enter coherency (as CPUs already are); and there's no reason to have
	 * caches disabled either.
	 */
#if HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY
	mov	x0, xzr
#else
	mov	x0, #DISABLE_DCACHE
#endif
	bl	bl31_plat_enable_mmu

#if ENABLE_RME
	/*
	 * At warm boot GPT data structures have already been initialized in RAM
	 * but the sysregs for this CPU need to be initialized. Note that the GPT
	 * accesses are controlled attributes in GPCCR and do not depend on the
	 * SCR_EL3.C bit.
	 */
	bl	gpt_enable
	cbz	x0, 1f
	no_ret plat_panic_handler
1:
#endif

#if ENABLE_PAUTH
	/* --------------------------------------------------------------------
	 * Program APIAKey_EL1 and enable pointer authentication
	 * --------------------------------------------------------------------
	 */
	bl	pauth_init_enable_el3
#endif /* ENABLE_PAUTH */

	bl	psci_warmboot_entrypoint

#if ENABLE_RUNTIME_INSTRUMENTATION
	pmf_calc_timestamp_addr rt_instr_svc, RT_INSTR_EXIT_PSCI
	mov	x19, x0

	/*
	 * Invalidate before updating timestamp to ensure previous timestamp
	 * updates on the same cache line with caches disabled are properly
	 * seen by the same core. Without the cache invalidate, the core might
	 * write into a stale cache line.
	 */
	mov	x1, #PMF_TS_SIZE
	mov	x20, x30
	bl	inv_dcache_range
	mov	x30, x20

	mrs	x0, cntpct_el0
	str	x0, [x19]
#endif
	b	el3_exit
endfunc bl31_warm_entrypoint
