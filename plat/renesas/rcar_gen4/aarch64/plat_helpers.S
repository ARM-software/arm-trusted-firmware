/*
 * Copyright (c) 2013-2025, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2015-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <common/runtime_svc.h>
#include <cortex_a55.h>
#include <platform_def.h>

#include "rcar_def.h"

	.globl	plat_get_my_entrypoint
	.extern	plat_set_my_stack
	.globl	platform_mem_init

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_invalidate_icache
	.globl	plat_secondary_reset
	.globl	plat_my_core_pos
	.globl	plat_renesas_calc_core_pos
	.globl	bl31_plat_enable_mmu

	.extern console_rcar_init
	.extern console_rcar_putc
	.extern console_rcar_flush
	.extern rcar_pwrc_code_copy_to_system_ram

	/* -----------------------------------------------------
	 * void platform_get_core_pos (mpidr)
	 * -----------------------------------------------------
	 */
func plat_renesas_calc_core_pos
	lsr	x0, x0, #8
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #7
	ret
endfunc plat_renesas_calc_core_pos

	/* -----------------------------------------------------
	 * void platform_my_core_pos
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_renesas_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * void platform_get_my_entrypoint (unsigned int mpid);
	 *
	 * Main job of this routine is to distinguish between
	 * a cold and warm boot.
	 * On a cold boot the secondaries first wait for the
	 * platform to be initialized after which they are
	 * hotplugged in. The primary proceeds to perform the
	 * platform initialization.
	 * On a warm boot, each cpu jumps to the address in its
	 * mailbox.
	 *
	 * TODO: Not a good idea to save lr in a temp reg
	 * -----------------------------------------------------
	 */
func plat_get_my_entrypoint
	mrs	x0, mpidr_el1
	mov	x9, x30 /* lr */

	ldr	x1, =BOOT_KIND_BASE
	ldr	x21, [x1]

	/* Check the reset info */
	and	x1, x21, #0x000c
	cmp	x1, #0x0008
	beq	el3_panic
	cmp	x1, #0x000c
	beq	el3_panic

	/* Check the boot kind */
	and	x1, x21, #0x0003
	cmp	x1, #0x0002
	beq	el3_panic
	cmp	x1, #0x0003
	beq	el3_panic

	/* warm boot or cold boot */
	and	x1, x21, #1
	cmp	x1, #0
	bne	warm_reset

	/* Cold boot */
	mov	x0, #0
	b	exit

warm_reset:
	/* --------------------------------------------------------------------
	 * A per-cpu mailbox is maintained in the trusted SDRAM. Its flushed out
	 * of the caches after every update using normal memory so its safe to
	 * read it here with SO attributes
	 * ---------------------------------------------------------------------
	 */
	ldr	x10, =MBOX_BASE
	bl	plat_renesas_calc_core_pos
	lsl	x0, x0, #CACHE_WRITEBACK_SHIFT
	ldr	x0, [x10, x0]
	cbz	x0, _panic
exit:
	ret	x9
_panic:
	b	el3_panic

endfunc plat_get_my_entrypoint

	/* ---------------------------------------------
	 * plat_secondary_reset
	 *
	 * ---------------------------------------------
	 */
func plat_secondary_reset
	mrs	x0, sctlr_el3
	bic	x0, x0, #SCTLR_EE_BIT
	msr	sctlr_el3, x0
	isb

	mrs	x0, cptr_el3
	bic	w0, w0, #TCPAC_BIT
	bic	w0, w0, #TTA_BIT
	bic	w0, w0, #TFP_BIT
	msr	cptr_el3, x0

	mov_imm	x0, PARAMS_BASE
	mov_imm	x2, BL31_BASE
	ldr x3, =BOOT_KIND_BASE
	mov x1, #0x1
	str x1, [x3]
	br	x2	/* jump to BL31 */
	nop
	nop
	nop
endfunc plat_secondary_reset

	/* -----------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Zero out the mailbox registers in the shared memory
	 * and set the rcar_boot_kind_flag.
	 * The mmu is turned off right now and only the primary can
	 * ever execute this code. Secondaries will read the
	 * mailboxes using SO accesses.
	 * -----------------------------------------------------
	 */
func platform_mem_init
	ldr	x0, =MBOX_BASE
	mov	w1, #PLATFORM_CORE_COUNT
loop:
	str	xzr, [x0], #CACHE_WRITEBACK_GRANULE
	subs	w1, w1, #1
	b.gt	loop
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize log area
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov	x1, sp
	mov_imm	x2, RCAR_CRASH_STACK
	mov	sp, x2
	str	x1, [sp, #-16]!
	str	x30, [sp, #-16]!
	bl	console_rcar_init
	ldr	x30, [sp], #16
	ldr	x1, [sp], #16
	mov	sp, x1
	ret
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to store a character to log area
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov	x1, sp
	mov_imm	x2, RCAR_CRASH_STACK
	mov	sp, x2
	str	x1, [sp, #-16]!
	str	x30, [sp, #-16]!
	str	x3, [sp, #-16]!
	str	x4, [sp, #-16]!
	str	x5, [sp, #-16]!
	str	x6, [sp, #-16]!
	str	x7, [sp, #-16]!
	bl	console_rcar_putc
	ldr	x7, [sp], #16
	ldr	x6, [sp], #16
	ldr	x5, [sp], #16
	ldr	x4, [sp], #16
	ldr	x3, [sp], #16
	ldr	x30, [sp], #16
	ldr	x1, [sp], #16
	mov	sp, x1
	ret
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * void plat_crash_console_flush()
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	b	console_rcar_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------
	 * void plat_invalidate_icache(void)
	 * Instruction Cache Invalidate All to PoU
	 * ---------------------------------------------
	 */
func plat_invalidate_icache
	ic	iallu
	ret
endfunc plat_invalidate_icache

	/* -----------------------------------------------------
	 * void bl31_plat_enable_mmu(uint32_t flags);
	 *
	 * Enable MMU in BL31.
	 * And copy the code to run on System RAM.
	 * Note: This function call will only be done from Warm boot.
	 * -----------------------------------------------------
	 */
func bl31_plat_enable_mmu
	mov	x28, x30
	bl	enable_mmu_direct_el3
	bl	rcar_pwrc_code_copy_to_system_ram
	mov	x30, x28
	ret
endfunc bl31_plat_enable_mmu
