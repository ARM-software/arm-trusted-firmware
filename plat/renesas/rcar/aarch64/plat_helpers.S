/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <common/runtime_svc.h>
#include <cortex_a57.h>
#include <platform_def.h>

#include "rcar_def.h"

	.globl	plat_get_my_entrypoint
	.extern	plat_set_my_stack
	.globl	platform_mem_init

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_invalidate_icache
	.globl	plat_report_exception
	.globl	plat_secondary_reset
	.globl	plat_reset_handler
	.globl	plat_my_core_pos
	.extern	rcar_log_init

	.extern console_rcar_init
	.extern console_rcar_putc
	.extern console_rcar_flush

#if IMAGE_BL2
	#define	INT_ID_MASK	(0x3ff)
	.extern bl2_interrupt_error_type
	.extern bl2_interrupt_error_id
	.globl  bl2_enter_bl31
	.extern gicv2_acknowledge_interrupt
	.extern rcar_swdt_exec
#endif

	/* -----------------------------------------------------
	 * void platform_get_core_pos (mpidr)
	 * -----------------------------------------------------
	 */
func platform_get_core_pos
	and     x1, x0, #MPIDR_CPU_MASK
	and     x0, x0, #MPIDR_CLUSTER_MASK
	add     x0, x1, x0, LSR #6
	ret
endfunc platform_get_core_pos

	/* -----------------------------------------------------
	 * void platform_my_core_pos
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs     x0, mpidr_el1
	b	platform_get_core_pos
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

#if defined(IMAGE_BL2)
	/* always cold boot on bl2 */
	mov	x0, #0
	ret	x9
#else
       ldr 	x1, =BOOT_KIND_BASE
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
	bl	platform_get_core_pos
	lsl	x0, x0, #CACHE_WRITEBACK_SHIFT
	ldr	x0, [x10, x0]
	cbz	x0, _panic
exit:
	ret	x9
_panic:
	b	do_panic
#endif

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

	/* ---------------------------------------------
	 * plat_enter_bl31
	 *
	 * ---------------------------------------------
	 */
func bl2_enter_bl31
	mov	x20, x0
        /*
         * MMU needs to be disabled because both BL2 and BL31 execute
         * in EL3, and therefore share the same address space.
         * BL31 will initialize the address space according to its
         * own requirement.
         */
#if RCAR_BL2_DCACHE == 1
	/* Disable mmu and data cache */
	bl	disable_mmu_el3
	/* Data cache clean and invalidate */
	mov	x0, #DCCISW
	bl	dcsw_op_all
	/* TLB invalidate all, EL3 */
	tlbi	alle3
#endif /* RCAR_BL2_DCACHE == 1 */
	bl	disable_mmu_icache_el3
	/* Invalidate instruction cache */
	ic	iallu
	dsb	sy
	isb
	ldp	x0, x1, [x20, #ENTRY_POINT_INFO_PC_OFFSET]
	msr	elr_el3, x0
	msr	spsr_el3, x1
	eret
endfunc bl2_enter_bl31

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
#if !IMAGE_BL2
	ldr	x0, =MBOX_BASE
	mov	w1, #PLATFORM_CORE_COUNT
loop:
	str	xzr, [x0], #CACHE_WRITEBACK_GRANULE
	subs	w1, w1, #1
	b.gt	loop
#endif
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * void plat_report_exception(unsigned int type)
	 * Function to report an unhandled exception
	 * with platform-specific means.
	 * ---------------------------------------------
	 */
func plat_report_exception
	/* Switch to SP_EL0 */
	msr	spsel, #0
#if IMAGE_BL2
	mov	w1, #FIQ_SP_EL0
	cmp	w0, w1
	beq	rep_exec_fiq_elx
	b	rep_exec_panic_type
rep_exec_fiq_elx:
	bl	gicv2_acknowledge_interrupt
	mov	x2, #INT_ID_MASK
	and	x0, x0, x2
	mov	x1, #ARM_IRQ_SEC_WDT
	cmp	x0, x1
	bne	rep_exec_panic_id
	mrs	x0, ELR_EL3
	b	rcar_swdt_exec
rep_exec_panic_type:
	/* x0 is interrupt TYPE */
	b	bl2_interrupt_error_type
rep_exec_panic_id:
	/* x0 is interrupt ID */
	b	bl2_interrupt_error_id
rep_exec_end:
#endif
	ret
endfunc plat_report_exception

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize log area
	 * ---------------------------------------------
	 */
func plat_crash_console_init
#if IMAGE_BL2
	mov	x0, #0
#else
	mov	x1, sp
	mov_imm	x2, RCAR_CRASH_STACK
	mov	sp, x2
	str	x1, [sp, #-16]!
	str	x30, [sp, #-16]!
	bl	console_rcar_init
	ldr	x30, [sp], #16
	ldr	x1, [sp], #16
	mov	sp, x1
#endif
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
	bl	console_rcar_putc
	ldr	x5, [sp], #16
	ldr	x4, [sp], #16
	ldr	x3, [sp], #16
	ldr	x30, [sp], #16
	ldr	x1, [sp], #16
	mov	sp, x1
	ret
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * int plat_crash_console_flush()
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	b	console_rcar_flush
endfunc plat_crash_console_flush

	/* --------------------------------------------------------------------
	 * void plat_reset_handler(void);
	 *
	 * Before adding code in this function, refer to the guidelines in
	 * docs/firmware-design.md to determine whether the code should reside
	 * within the FIRST_RESET_HANDLER_CALL block or not.
	 *
	 * For R-Car H3:
	 * - Set the L2 Tag RAM latency to 2 (i.e. 3 cycles) for Cortex-A57
	 * - Set the L2 Data setup latency to 1 (i.e. 1 cycles) for Cortex-A57
	 * - Set the L2 Data RAM latency to 3 (i.e. 4 cycles) for Cortex-A57
	 * For R-Car M3/M3N:
	 * - Set the L2 Tag RAM latency to 2 (i.e. 3 cycles) for Cortex-A57
	 * - Set the L2 Data setup latency to 0 (i.e. 0 cycles) for Cortex-A57
	 * - Set the L2 Data RAM latency to 3 (i.e. 4 cycles) for Cortex-A57
	 *
	 * --------------------------------------------------------------------
	 */
func plat_reset_handler
	/*
	 * On R-Car H3    :  x2 := 0
	 * On R-Car M3/M3N:  x2 := 1
	 */
	/* read PRR */
	ldr	x0, =0xFFF00044
	ldr	w0, [x0]
	ubfx	w0, w0, 8, 8
	/* H3? */
	cmp	w0, #0x4F
	b.eq	RCARH3
	/* set R-Car M3/M3N */
	mov	x2, #1
	b	CHK_A5x
RCARH3:
	/* set R-Car H3 */
	mov	x2, #0
	/* --------------------------------------------------------------------
	 * Determine whether this code is executed on a Cortex-A53 or on a
	 * Cortex-A57 core.
	 * --------------------------------------------------------------------
	 */
CHK_A5x:
	mrs	x0, midr_el1
	ubfx	x1, x0, MIDR_PN_SHIFT, #12
	cmp     w1, #((CORTEX_A57_MIDR >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	A57
	ret
A57:
	/* Get data from CORTEX_A57_L2CTLR_EL1	*/
	mrs	x0, CORTEX_A57_L2CTLR_EL1
	/*
	 * On R-Car H3/M3/M3N
	 *
	 * L2 Tag RAM latency is bit8-6 of CORTEX_A57_L2CTLR_EL1
	 * L2 Data RAM setup is bit5 of CORTEX_A57_L2CTLR_EL1
	 * L2 Data RAM latency is bit2-0 of CORTEX_A57_L2CTLR_EL1
	 */
	/* clear bit of L2 RAM	*/
	/* ~(0x1e7) -> x1	*/
	mov	x1, #0x1e7
	neg	x1, x1
	/* clear bit of L2 RAM -> x0 */
	and	x0, x0, x1
	/* L2 Tag RAM latency (3 cycles) */
	orr	x0, x0, #0x2 << 6
	/* If M3/M3N then L2 RAM setup is 0 */
	cbnz	x2, M3_L2
	/* L2 Data RAM setup (1 cycle) */
	orr	x0, x0, #0x1 << 5
M3_L2:
	/* L2 Data RAM latency (4 cycles) */
	orr	x0, x0, #0x3
	/* Store data to L2CTLR_EL1 */
	msr	CORTEX_A57_L2CTLR_EL1, x0
apply_l2_ram_latencies:
	ret
endfunc plat_reset_handler

	/* ---------------------------------------------
	 * void plat_invalidate_icache(void)
	 * Instruction Cache Invalidate All to PoU
	 * ---------------------------------------------
	 */
func plat_invalidate_icache
	ic	iallu

	ret
endfunc plat_invalidate_icache
