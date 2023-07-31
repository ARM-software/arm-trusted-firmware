/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (C) 2022-2023 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NUVOTON_HELPERS_S
#define NUVOTON_HELPERS_S

#include <asm_macros.S>
#include <cortex_a35.h>
#include <platform_def.h>

	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_calc_core_pos
	.globl	plat_reset_handler
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	platform_mem_init
	.globl  npcm845x_mailbox_init

	/* --------------------------------------------------------------------
	 * Helper macro that reads the part number of the current CPU and jumps
	 * to the given label if it matches the CPU MIDR provided.
	 *
	 * Clobbers x0.
	 * --------------------------------------------------------------------
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label

	mrs	x0, midr_el1
	ubfx	x0, x0, MIDR_PN_SHIFT, #12
	cmp     w0, #((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	\_label

	.endm

	/* ----------------------------------------------
	 * The mailbox_base is used to distinguish warm/cold
	 * reset. The mailbox_base is in the data section, not
	 * in .bss, this allows function to start using this
	 * variable before the runtime memory is initialized.
	 * ----------------------------------------------
	 */
	.section .data.mailbox_base
	.align 3
	mailbox_base: .quad 0x0

	/* ---------------------------------------------
	 * void plat_reset_handler(void);
	 *
	 * To add: Determine the SoC type and call the appropriate
	 * reset handler.
	 *-----------------------------------------------	 */

func plat_reset_handler
	 ret
endfunc plat_reset_handler

	/* ----------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * This function checks if this is the primary CPU
	 * ----------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CPU_MASK)
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ----------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * This Function uses the plat_calc_core_pos()
	 * to get the index of the calling CPU.
	 * ----------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and 	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

	/*
	 * unsigned int plat_calc_core_pos(uint64_t mpidr)
	 * helper function to calculate the core position.
	 * With this function.
	 */
func plat_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and 	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_calc_core_pos

	/* ---------------------------------------------
	 * function to get the entrypoint.
	 * ---------------------------------------------
	 */
	/* ---------------------------------------------------------------------
	 * uintptr_t plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and a warm
	 * boot.
	 *
	 * This functions returns:
	 *  - 0 for a cold boot.
	 *  - Any other value for a warm boot.
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	mov	x1, x30
	bl	plat_is_my_cpu_primary
	/*
	 * Secondaries always cold boot.
	*/
	cbz	w0, 1f
	/*
	 * Primaries warm boot if they are requested
	 * to power off.
	 */
	mov_imm	x0, PLAT_NPCM_TM_HOLD_BASE
	ldr	x0, [x0]
	cmp	x0, PLAT_NPCM_TM_HOLD_STATE_BSP_OFF
	adr	x0, plat_wait_for_warm_boot
	csel	x0, x0, xzr, eq
	ret	x1
1:	mov	x0, #0
	ret	x1
endfunc plat_get_my_entrypoint

func npcm845x_mailbox_init
	adrp	x1, mailbox_base
	str	x0, [x1, :lo12:mailbox_base]
	ret
endfunc npcm845x_mailbox_init

func plat_wait_for_warm_boot
	/*
	 * Calculate address of our hold entry.
	 * As the function will never return, there is no need to save LR.
	 */
	bl	plat_my_core_pos
	lsl	x0, x0, #3
	mov x8, x0
	mov_imm	x2, PLAT_NPCM_TM_HOLD_BASE
	add	x0, x0, x2
	mov_imm	x2, PLAT_NPCM_TRUSTED_NOTIFICATION_BASE
	add	x8, x8, x2
	/*
	 * This code runs way before requesting the warmboot of this core,
	 * so it is possible to clear the mailbox before getting a request
	 * to boot.
	 */
	mov	x1, PLAT_NPCM_TM_HOLD_STATE_WAIT
	str	x1,[x0]

	/* Notify that core is in pending state - do not use x0!, uses x7 and x8! */
	mov	x7, PLAT_NPCM_TM_NOTIFICATION_START
	str	x7,[x8]
	/*
	 * This code runs way before requesting the warmboot of this core,
	 * so it is possible to clear the mailbox before getting a request
	 * to boot.
	 */
	mov	x1, PLAT_NPCM_TM_HOLD_STATE_WAIT
	str	x1,[x0]
	/* Wait until we have a go */
poll_mailbox:
	wfe
	ldr	x1, [x0]
	cmp	x1, PLAT_NPCM_TM_HOLD_STATE_GO
	bne	poll_mailbox

	mov	x7, PLAT_NPCM_TM_NOTIFICATION_BR
	str	x7,[x8]
	/* Jump to the provided entrypoint */
	mov_imm	x0, PLAT_NPCM_TM_ENTRYPOINT
	ldr	x1, [x0]
	br	x1
endfunc plat_wait_for_warm_boot

func plat_secondary_cold_boot_setup
	b	plat_wait_for_warm_boot
endfunc plat_secondary_cold_boot_setup

func plat_crash_console_init
	mov	x0, #1
	ret
endfunc plat_crash_console_init

func plat_crash_console_putc
	ret
endfunc plat_crash_console_putc

func plat_crash_console_flush
	mov	x0, #0
	ret
endfunc plat_crash_console_flush

func platform_mem_init
	ret
endfunc platform_mem_init

#endif /* NUVOTON_HELPERS_S */
