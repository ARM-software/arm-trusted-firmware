/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>


	.globl	bl2u_vector_table
	.globl	bl2u_entrypoint


vector_base bl2u_vector_table
	b	bl2u_entrypoint
	b	report_exception	/* Undef */
	b	report_exception	/* SVC call */
	b	report_exception	/* Prefetch abort */
	b	report_exception	/* Data abort */
	b	report_exception	/* Reserved */
	b	report_exception	/* IRQ */
	b	report_exception	/* FIQ */


func bl2u_entrypoint
	/*---------------------------------------------
	 * Save from r1 the extents of the trusted ram
	 * available to BL2U for future use.
	 * r0 is not currently used.
	 * ---------------------------------------------
	 */
	mov	r11, r1
	mov	r10, r2

	/* ---------------------------------------------
	 * Set the exception vector to something sane.
	 * ---------------------------------------------
	 */
	ldr	r0, =bl2u_vector_table
	stcopr	r0, VBAR
	isb

	/* -----------------------------------------------------
	 * Enable the instruction cache
	 * -----------------------------------------------------
	 */
	ldcopr	r0, SCTLR
	orr	r0, r0, #SCTLR_I_BIT
	stcopr	r0, SCTLR
	isb

	/* ---------------------------------------------
	 * Since BL2U executes after BL1, it is assumed
	 * here that BL1 has already has done the
	 * necessary register initializations.
	 * ---------------------------------------------
	 */

	/* ---------------------------------------------
	 * Invalidate the RW memory used by the BL2U
	 * image. This includes the data and NOBITS
	 * sections. This is done to safeguard against
	 * possible corruption of this memory by dirty
	 * cache lines in a system cache as a result of
	 * use by an earlier boot loader stage.
	 * ---------------------------------------------
	 */
	ldr	r0, =__RW_START__
	ldr	r1, =__RW_END__
	sub	r1, r1, r0
	bl	inv_dcache_range

	/* ---------------------------------------------
	 * Zero out NOBITS sections. There are 2 of them:
	 *   - the .bss section;
	 *   - the coherent memory section.
	 * ---------------------------------------------
	 */
	ldr	r0, =__BSS_START__
	ldr	r1, =__BSS_SIZE__
	bl	zeromem

	/* --------------------------------------------
	 * Allocate a stack whose memory will be marked
	 * as Normal-IS-WBWA when the MMU is enabled.
	 * There is no risk of reading stale stack
	 * memory after enabling the MMU as only the
	 * primary cpu is running at the moment.
	 * --------------------------------------------
	 */
	bl	plat_set_my_stack

	/* ---------------------------------------------
	 * Initialize the stack protector canary before
	 * any C code is called.
	 * ---------------------------------------------
	 */
#if STACK_PROTECTOR_ENABLED
	bl	update_stack_protector_canary
#endif

	/* ---------------------------------------------
	 * Perform early platform setup & platform
	 * specific early arch. setup e.g. mmu setup
	 * ---------------------------------------------
	 */
	mov	r0, r11
	mov	r1, r10
	bl	bl2u_early_platform_setup
	bl	bl2u_plat_arch_setup

	/* ---------------------------------------------
	 * Jump to main function.
	 * ---------------------------------------------
	 */
	bl	bl2u_main

	/* ---------------------------------------------
	 * Should never reach this point.
	 * ---------------------------------------------
	 */
	no_ret	plat_panic_handler

endfunc bl2u_entrypoint
