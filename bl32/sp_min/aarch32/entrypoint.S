/*
 * Copyright (c) 2016-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <el3_common_macros.S>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/pmf/aarch32/pmf_asm_macros.S>
#include <lib/runtime_instr.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <smccc_helpers.h>
#include <smccc_macros.S>

	.globl	sp_min_vector_table
	.globl	sp_min_entrypoint
	.globl	sp_min_warm_entrypoint
	.globl	sp_min_handle_smc
	.globl	sp_min_handle_fiq

#define FIXUP_SIZE	((BL32_LIMIT) - (BL32_BASE))

	.macro route_fiq_to_sp_min reg
		/* -----------------------------------------------------
		 * FIQs are secure interrupts trapped by Monitor and non
		 * secure is not allowed to mask the FIQs.
		 * -----------------------------------------------------
		 */
		ldcopr	\reg, SCR
		orr	\reg, \reg, #SCR_FIQ_BIT
		bic	\reg, \reg, #SCR_FW_BIT
		stcopr	\reg, SCR
	.endm

	.macro clrex_on_monitor_entry
#if (ARM_ARCH_MAJOR == 7)
	/*
	 * ARMv7 architectures need to clear the exclusive access when
	 * entering Monitor mode.
	 */
	clrex
#endif
	.endm

vector_base sp_min_vector_table
	b	sp_min_entrypoint
	b	plat_panic_handler	/* Undef */
	b	sp_min_handle_smc	/* Syscall */
	b	report_prefetch_abort	/* Prefetch abort */
	b	report_data_abort	/* Data abort */
	b	plat_panic_handler	/* Reserved */
	b	plat_panic_handler	/* IRQ */
	b	sp_min_handle_fiq	/* FIQ */


/*
 * The Cold boot/Reset entrypoint for SP_MIN
 */
func sp_min_entrypoint
#if !RESET_TO_SP_MIN
	/* ---------------------------------------------------------------
	 * Preceding bootloader has populated r0 with a pointer to a
	 * 'bl_params_t' structure & r1 with a pointer to platform
	 * specific structure
	 * ---------------------------------------------------------------
	 */
	mov	r9, r0
	mov	r10, r1
	mov	r11, r2
	mov	r12, r3

	/* ---------------------------------------------------------------------
	 * For !RESET_TO_SP_MIN systems, only the primary CPU ever reaches
	 * sp_min_entrypoint() during the cold boot flow, so the cold/warm boot
	 * and primary/secondary CPU logic should not be executed in this case.
	 *
	 * Also, assume that the previous bootloader has already initialised the
	 * SCTLR, including the CPU endianness, and has initialised the memory.
	 * ---------------------------------------------------------------------
	 */
	el3_entrypoint_common					\
		_init_sctlr=0					\
		_warm_boot_mailbox=0				\
		_secondary_cold_boot=0				\
		_init_memory=0					\
		_init_c_runtime=1				\
		_exception_vectors=sp_min_vector_table		\
		_pie_fixup_size=FIXUP_SIZE

	/* ---------------------------------------------------------------------
	 * Relay the previous bootloader's arguments to the platform layer
	 * ---------------------------------------------------------------------
	 */
#else
	/* ---------------------------------------------------------------------
	 * For RESET_TO_SP_MIN systems which have a programmable reset address,
	 * sp_min_entrypoint() is executed only on the cold boot path so we can
	 * skip the warm boot mailbox mechanism.
	 * ---------------------------------------------------------------------
	 */
	el3_entrypoint_common					\
		_init_sctlr=1					\
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS	\
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU	\
		_init_memory=1					\
		_init_c_runtime=1				\
		_exception_vectors=sp_min_vector_table		\
		_pie_fixup_size=FIXUP_SIZE

	/* ---------------------------------------------------------------------
	 * For RESET_TO_SP_MIN systems, BL32 (SP_MIN) is the first bootloader
	 * to run so there's no argument to relay from a previous bootloader.
	 * Zero the arguments passed to the platform layer to reflect that.
	 * ---------------------------------------------------------------------
	 */
	mov	r9, #0
	mov	r10, #0
	mov	r11, #0
	mov	r12, #0

#endif /* RESET_TO_SP_MIN */

#if SP_MIN_WITH_SECURE_FIQ
	route_fiq_to_sp_min r4
#endif

	mov	r0, r9
	mov	r1, r10
	mov	r2, r11
	mov	r3, r12
	bl	sp_min_early_platform_setup2
	bl	sp_min_plat_arch_setup

	/* Jump to the main function */
	bl	sp_min_main

	/* -------------------------------------------------------------
	 * Clean the .data & .bss sections to main memory. This ensures
	 * that any global data which was initialised by the primary CPU
	 * is visible to secondary CPUs before they enable their data
	 * caches and participate in coherency.
	 * -------------------------------------------------------------
	 */
	ldr	r0, =__DATA_START__
	ldr	r1, =__DATA_END__
	sub	r1, r1, r0
	bl	clean_dcache_range

	ldr	r0, =__BSS_START__
	ldr	r1, =__BSS_END__
	sub	r1, r1, r0
	bl	clean_dcache_range

	bl	smc_get_next_ctx

	/* r0 points to `smc_ctx_t` */
	/* The PSCI cpu_context registers have been copied to `smc_ctx_t` */
	b	sp_min_exit
endfunc sp_min_entrypoint


/*
 * SMC handling function for SP_MIN.
 */
func sp_min_handle_smc
	/* On SMC entry, `sp` points to `smc_ctx_t`. Save `lr`. */
	str	lr, [sp, #SMC_CTX_LR_MON]

#if ENABLE_RUNTIME_INSTRUMENTATION
	/*
	 * Read the timestamp value and store it on top of the C runtime stack.
	 * The value will be saved to the per-cpu data once the C stack is
	 * available, as a valid stack is needed to call _cpu_data()
	 */
	strd	r0, r1, [sp, #SMC_CTX_GPREG_R0]
	ldcopr16 r0, r1, CNTPCT_64
	ldr	lr, [sp, #SMC_CTX_SP_MON]
	strd	r0, r1, [lr, #-8]!
	str	lr, [sp, #SMC_CTX_SP_MON]
	ldrd	r0, r1, [sp, #SMC_CTX_GPREG_R0]
#endif

	smccc_save_gp_mode_regs

	clrex_on_monitor_entry

	/*
	 * `sp` still points to `smc_ctx_t`. Save it to a register
	 * and restore the C runtime stack pointer to `sp`.
	 */
	mov	r2, sp				/* handle */
	ldr	sp, [r2, #SMC_CTX_SP_MON]

#if ENABLE_RUNTIME_INSTRUMENTATION
	/* Save handle to a callee saved register */
	mov	r6, r2

	/*
	 * Restore the timestamp value and store it in per-cpu data. The value
	 * will be extracted from per-cpu data by the C level SMC handler and
	 * saved to the PMF timestamp region.
	 */
	ldrd	r4, r5, [sp], #8
	bl	_cpu_data
	strd	r4, r5, [r0, #CPU_DATA_PMF_TS0_OFFSET]

	/* Restore handle */
	mov	r2, r6
#endif

	ldr	r0, [r2, #SMC_CTX_SCR]
	and	r3, r0, #SCR_NS_BIT		/* flags */

	/* Switch to Secure Mode*/
	bic	r0, #SCR_NS_BIT
	stcopr	r0, SCR
	isb

	ldr	r0, [r2, #SMC_CTX_GPREG_R0]	/* smc_fid */
	/* Check whether an SMC64 is issued */
	tst	r0, #(FUNCID_CC_MASK << FUNCID_CC_SHIFT)
	beq	1f
	/* SMC32 is not detected. Return error back to caller */
	mov	r0, #SMC_UNK
	str	r0, [r2, #SMC_CTX_GPREG_R0]
	mov	r0, r2
	b	sp_min_exit
1:
	/* SMC32 is detected */
	mov	r1, #0				/* cookie */
	bl	handle_runtime_svc

	/* `r0` points to `smc_ctx_t` */
	b	sp_min_exit
endfunc sp_min_handle_smc

/*
 * Secure Interrupts handling function for SP_MIN.
 */
func sp_min_handle_fiq
#if !SP_MIN_WITH_SECURE_FIQ
	b plat_panic_handler
#else
	/* FIQ has a +4 offset for lr compared to preferred return address */
	sub	lr, lr, #4
	/* On SMC entry, `sp` points to `smc_ctx_t`. Save `lr`. */
	str	lr, [sp, #SMC_CTX_LR_MON]

	smccc_save_gp_mode_regs

	clrex_on_monitor_entry

	/* load run-time stack */
	mov	r2, sp
	ldr	sp, [r2, #SMC_CTX_SP_MON]

	/* Switch to Secure Mode */
	ldr	r0, [r2, #SMC_CTX_SCR]
	bic	r0, #SCR_NS_BIT
	stcopr	r0, SCR
	isb

	push	{r2, r3}
	bl	sp_min_fiq
	pop	{r0, r3}

	b	sp_min_exit
#endif
endfunc sp_min_handle_fiq

/*
 * The Warm boot entrypoint for SP_MIN.
 */
func sp_min_warm_entrypoint
#if ENABLE_RUNTIME_INSTRUMENTATION
	/*
	 * This timestamp update happens with cache off.  The next
	 * timestamp collection will need to do cache maintenance prior
	 * to timestamp update.
	 */
	pmf_calc_timestamp_addr rt_instr_svc, RT_INSTR_EXIT_HW_LOW_PWR
	ldcopr16 r2, r3, CNTPCT_64
	strd	r2, r3, [r0]
#endif
	/*
	 * On the warm boot path, most of the EL3 initialisations performed by
	 * 'el3_entrypoint_common' must be skipped:
	 *
	 *  - Only when the platform bypasses the BL1/BL32 (SP_MIN) entrypoint by
	 *    programming the reset address do we need to initialied the SCTLR.
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
		_exception_vectors=sp_min_vector_table		\
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
	mov	r0, #0
#else
	mov	r0, #DISABLE_DCACHE
#endif
	bl	bl32_plat_enable_mmu

#if SP_MIN_WITH_SECURE_FIQ
	route_fiq_to_sp_min r0
#endif

	bl	sp_min_warm_boot
	bl	smc_get_next_ctx
	/* r0 points to `smc_ctx_t` */
	/* The PSCI cpu_context registers have been copied to `smc_ctx_t` */

#if ENABLE_RUNTIME_INSTRUMENTATION
	/* Save smc_ctx_t */
	mov	r5, r0

	pmf_calc_timestamp_addr rt_instr_svc, RT_INSTR_EXIT_PSCI
	mov	r4, r0

	/*
	 * Invalidate before updating timestamp to ensure previous timestamp
	 * updates on the same cache line with caches disabled are properly
	 * seen by the same core. Without the cache invalidate, the core might
	 * write into a stale cache line.
	 */
	mov	r1, #PMF_TS_SIZE
	bl	inv_dcache_range

	ldcopr16 r0, r1, CNTPCT_64
	strd	r0, r1, [r4]

	/* Restore smc_ctx_t */
	mov	r0, r5
#endif

	b	sp_min_exit
endfunc sp_min_warm_entrypoint

/*
 * The function to restore the registers from SMC context and return
 * to the mode restored to SPSR.
 *
 * Arguments : r0 must point to the SMC context to restore from.
 */
func sp_min_exit
	monitor_exit
endfunc sp_min_exit
