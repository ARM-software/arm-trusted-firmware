/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_secondary_cold_boot_setup
	.globl	plat_get_my_entrypoint
	.globl	plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset. Right
	 * now this is a stub function.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	mov	x0, #0
	ret
endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between cold and warm boot
	 * For a cold boot, return 0.
	 * For a warm boot, read the mailbox and return the address it contains.
	 * A magic number is placed before entrypoint to avoid mistake caused by
	 * uninitialized mailbox data area.
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	/* Read first word and compare it with magic num */
	mov_imm x0, PLAT_MARVELL_MAILBOX_BASE
	ldr	x1, [x0]
	mov_imm x2, PLAT_MARVELL_MAILBOX_MAGIC_NUM
	cmp	x1, x2
	/* If compare failed, return 0, i.e. cold boot */
	beq	entrypoint
	mov	x0, #0
	ret
entrypoint:
	/* Second word contains the jump address */
	add	x0, x0, #8
	ldr	x0, [x0]
	ret
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #MVEBU_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary
