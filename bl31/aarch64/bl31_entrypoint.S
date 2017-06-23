/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <bl_common.h>
#include <el3_common_macros.S>
#include <pmf_asm_macros.S>
#include <runtime_instr.h>
#include <xlat_tables_defs.h>

	.globl	bl31_entrypoint
	.globl	bl31_warm_entrypoint

	/* -----------------------------------------------------
	 * bl31_entrypoint() is the cold boot entrypoint,
	 * executed only by the primary cpu.
	 * -----------------------------------------------------
	 */

func bl31_entrypoint
#if !RESET_TO_BL31
	/* ---------------------------------------------------------------
	 * Preceding bootloader has populated x0 with a pointer to a
	 * 'bl31_params' structure & x1 with a pointer to platform
	 * specific structure
	 * ---------------------------------------------------------------
	 */
	mov	x20, x0
	mov	x21, x1

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
		_exception_vectors=runtime_exceptions

	/* ---------------------------------------------------------------------
	 * Relay the previous bootloader's arguments to the platform layer
	 * ---------------------------------------------------------------------
	 */
	mov	x0, x20
	mov	x1, x21
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
		_exception_vectors=runtime_exceptions

	/* ---------------------------------------------------------------------
	 * For RESET_TO_BL31 systems, BL31 is the first bootloader to run so
	 * there's no argument to relay from a previous bootloader. Zero the
	 * arguments passed to the platform layer to reflect that.
	 * ---------------------------------------------------------------------
	 */
	mov	x0, 0
	mov	x1, 0
#endif /* RESET_TO_BL31 */

	/* ---------------------------------------------
	 * Perform platform specific early arch. setup
	 * ---------------------------------------------
	 */
	bl	bl31_early_platform_setup
	bl	bl31_plat_arch_setup

	/* ---------------------------------------------
	 * Jump to main function.
	 * ---------------------------------------------
	 */
	bl	bl31_main

	/* -------------------------------------------------------------
	 * Clean the .data & .bss sections to main memory. This ensures
	 * that any global data which was initialised by the primary CPU
	 * is visible to secondary CPUs before they enable their data
	 * caches and participate in coherency.
	 * -------------------------------------------------------------
	 */
	adr	x0, __DATA_START__
	adr	x1, __DATA_END__
	sub	x1, x1, x0
	bl	clean_dcache_range

	adr	x0, __BSS_START__
	adr	x1, __BSS_END__
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
	pmf_calc_timestamp_addr rt_instr_svc RT_INSTR_EXIT_HW_LOW_PWR
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
		_exception_vectors=runtime_exceptions

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
	mov	x0, #DISABLE_DCACHE
	bl	bl31_plat_enable_mmu

#if HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY
	mrs	x0, sctlr_el3
	orr	x0, x0, #SCTLR_C_BIT
	msr	sctlr_el3, x0
	isb
#endif

	bl	psci_warmboot_entrypoint

#if ENABLE_RUNTIME_INSTRUMENTATION
	pmf_calc_timestamp_addr rt_instr_svc RT_INSTR_EXIT_PSCI
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
