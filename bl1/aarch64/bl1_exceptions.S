/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <context.h>

/* -----------------------------------------------------------------------------
 * Very simple stackless exception handlers used by BL1.
 * -----------------------------------------------------------------------------
 */
	.globl	bl1_exceptions

vector_base bl1_exceptions

	/* -----------------------------------------------------
	 * Current EL with SP0 : 0x0 - 0x200
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSP0
	mov	x0, #SYNC_EXCEPTION_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SynchronousExceptionSP0

vector_entry IrqSP0
	mov	x0, #IRQ_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry IrqSP0

vector_entry FiqSP0
	mov	x0, #FIQ_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry FiqSP0

vector_entry SErrorSP0
	mov	x0, #SERROR_SP_EL0
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SErrorSP0

	/* -----------------------------------------------------
	 * Current EL with SPx: 0x200 - 0x400
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionSPx
	mov	x0, #SYNC_EXCEPTION_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SynchronousExceptionSPx

vector_entry IrqSPx
	mov	x0, #IRQ_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry IrqSPx

vector_entry FiqSPx
	mov	x0, #FIQ_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry FiqSPx

vector_entry SErrorSPx
	mov	x0, #SERROR_SP_ELX
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SErrorSPx

	/* -----------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA64
	/* Enable the SError interrupt */
	msr	daifclr, #DAIF_ABT_BIT

	str	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	/* Expect only SMC exceptions */
	mrs	x30, esr_el3
	ubfx	x30, x30, #ESR_EC_SHIFT, #ESR_EC_LENGTH
	cmp	x30, #EC_AARCH64_SMC
	b.ne	unexpected_sync_exception

	b	smc_handler64
end_vector_entry SynchronousExceptionA64

vector_entry IrqA64
	mov	x0, #IRQ_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry IrqA64

vector_entry FiqA64
	mov	x0, #FIQ_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry FiqA64

vector_entry SErrorA64
	mov	x0, #SERROR_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SErrorA64

	/* -----------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * -----------------------------------------------------
	 */
vector_entry SynchronousExceptionA32
	mov	x0, #SYNC_EXCEPTION_AARCH32
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SynchronousExceptionA32

vector_entry IrqA32
	mov	x0, #IRQ_AARCH32
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry IrqA32

vector_entry FiqA32
	mov	x0, #FIQ_AARCH32
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry FiqA32

vector_entry SErrorA32
	mov	x0, #SERROR_AARCH32
	bl	plat_report_exception
	no_ret	plat_panic_handler
end_vector_entry SErrorA32


func smc_handler64

	/* ----------------------------------------------
	 * Detect if this is a RUN_IMAGE or other SMC.
	 * ----------------------------------------------
	 */
	mov	x30, #BL1_SMC_RUN_IMAGE
	cmp	x30, x0
	b.ne	smc_handler

	/* ------------------------------------------------
	 * Make sure only Secure world reaches here.
	 * ------------------------------------------------
	 */
	mrs	x30, scr_el3
	tst	x30, #SCR_NS_BIT
	b.ne	unexpected_sync_exception

	/* ----------------------------------------------
	 * Handling RUN_IMAGE SMC. First switch back to
	 * SP_EL0 for the C runtime stack.
	 * ----------------------------------------------
	 */
	ldr	x30, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]
	msr	spsel, #0
	mov	sp, x30

	/* ---------------------------------------------------------------------
	 * Pass EL3 control to next BL image.
	 * Here it expects X1 with the address of a entry_point_info_t
	 * structure describing the next BL image entrypoint.
	 * ---------------------------------------------------------------------
	 */
	mov	x20, x1

	mov	x0, x20
	bl	bl1_print_next_bl_ep_info

	ldp	x0, x1, [x20, #ENTRY_POINT_INFO_PC_OFFSET]
	msr	elr_el3, x0
	msr	spsr_el3, x1
	ubfx	x0, x1, #MODE_EL_SHIFT, #2
	cmp	x0, #MODE_EL3
	b.ne	unexpected_sync_exception

	bl	disable_mmu_icache_el3
	tlbi	alle3
	dsb	ish /* ERET implies ISB, so it is not needed here */

#if SPIN_ON_BL1_EXIT
	bl	print_debug_loop_message
debug_loop:
	b	debug_loop
#endif

	mov	x0, x20
	bl	bl1_plat_prepare_exit

	ldp	x6, x7, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x30)]
	ldp	x4, x5, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x20)]
	ldp	x2, x3, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x10)]
	ldp	x0, x1, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x0)]
	eret
endfunc smc_handler64

unexpected_sync_exception:
	mov	x0, #SYNC_EXCEPTION_AARCH64
	bl	plat_report_exception
	no_ret	plat_panic_handler

	/* -----------------------------------------------------
	 * Save Secure/Normal world context and jump to
	 * BL1 SMC handler.
	 * -----------------------------------------------------
	 */
smc_handler:
	/* -----------------------------------------------------
	 * Save the GP registers x0-x29.
	 * TODO: Revisit to store only SMCCC specified registers.
	 * -----------------------------------------------------
	 */
	bl	save_gp_registers

	/* -----------------------------------------------------
	 * Populate the parameters for the SMC handler. We
	 * already have x0-x4 in place. x5 will point to a
	 * cookie (not used now). x6 will point to the context
	 * structure (SP_EL3) and x7 will contain flags we need
	 * to pass to the handler.
	 * -----------------------------------------------------
	 */
	mov	x5, xzr
	mov	x6, sp

	/* -----------------------------------------------------
	 * Restore the saved C runtime stack value which will
	 * become the new SP_EL0 i.e. EL3 runtime stack. It was
	 * saved in the 'cpu_context' structure prior to the last
	 * ERET from EL3.
	 * -----------------------------------------------------
	 */
	ldr	x12, [x6, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]

	/* ---------------------------------------------
	 * Switch back to SP_EL0 for the C runtime stack.
	 * ---------------------------------------------
	 */
	msr	spsel, #0
	mov	sp, x12

	/* -----------------------------------------------------
	 * Save the SPSR_EL3, ELR_EL3, & SCR_EL3 in case there
	 * is a world switch during SMC handling.
	 * -----------------------------------------------------
	 */
	mrs	x16, spsr_el3
	mrs	x17, elr_el3
	mrs	x18, scr_el3
	stp	x16, x17, [x6, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	str	x18, [x6, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]

	/* Copy SCR_EL3.NS bit to the flag to indicate caller's security */
	bfi	x7, x18, #0, #1

	/* -----------------------------------------------------
	 * Go to BL1 SMC handler.
	 * -----------------------------------------------------
	 */
	bl	bl1_smc_handler

	/* -----------------------------------------------------
	 * Do the transition to next BL image.
	 * -----------------------------------------------------
	 */
	b	el3_exit
