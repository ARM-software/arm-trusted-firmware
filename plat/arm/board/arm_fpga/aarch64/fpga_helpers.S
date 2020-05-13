/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include "../fpga_private.h"

#include <platform_def.h>

	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	platform_mem_init
	.globl	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_fpga_calc_core_pos

/* -----------------------------------------------------------------------
 * Indicate a cold boot for every CPU - warm boot is unsupported for the
 * holding pen PSCI implementation.
 * -----------------------------------------------------------------------
 */
func plat_get_my_entrypoint
	mov	x0, #0
	ret
endfunc plat_get_my_entrypoint

/* -----------------------------------------------------------------------
 * void plat_secondary_cold_boot_setup (void);
 * -----------------------------------------------------------------------
 */
func plat_secondary_cold_boot_setup

	/*
	 * Wait for the primary processor to initialise the .BSS segment
	 * to avoid a race condition that would erase fpga_valid_mpids
	 * if it is populated before the C runtime is ready.
	 *
	 * We cannot use the current spin-lock implementation until the
	 * runtime is up and we should not rely on sevl/wfe instructions as
	 * it is optional whether they are implemented or not, so we use
	 * a global variable as lock and wait for the primary processor to
	 * finish the C runtime bring-up.
	 */

	ldr	w0, =C_RUNTIME_READY_KEY
	adrp	x1, secondary_core_spinlock
	add	x1, x1, :lo12:secondary_core_spinlock
1:
	wfe
	ldr	w2, [x1]
	cmp	w2, w0
	b.ne	1b
	/* Prevent reordering of the store into fpga_valid_mpids below */
	dmb	ish

	mov	x10, x30
	bl	plat_my_core_pos
	mov	x30, x10

	adrp	x4, fpga_valid_mpids
	add	x4, x4, :lo12:fpga_valid_mpids
	mov	x5, #VALID_MPID
	strb	w5, [x4, x0]

	/*
	 * Poll the CPU's hold entry until it indicates to jump
	 * to the entrypoint address.
	 */

	adrp	x1, hold_base
	add	x1, x1, :lo12:hold_base
poll_hold_entry:
	ldr	x3, [x1, x0, LSL #PLAT_FPGA_HOLD_ENTRY_SHIFT]
	cmp	x3, #PLAT_FPGA_HOLD_STATE_GO
	b.ne	1f

	adrp	x2, fpga_sec_entrypoint
	add	x2, x2, :lo12:fpga_sec_entrypoint
	ldr	x3, [x2]
	br	x3
1:
	wfe
	b	poll_hold_entry

endfunc plat_secondary_cold_boot_setup

/* -----------------------------------------------------------------------
 * unsigned int plat_is_my_cpu_primary (void);
 *
 * Find out whether the current cpu is the primary cpu
 * -----------------------------------------------------------------------
 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	mov_imm	x1, MPIDR_AFFINITY_MASK
	and	x0, x0, x1
	cmp	x0, #FPGA_PRIMARY_CPU
	cset	w0, eq
	ret
endfunc plat_is_my_cpu_primary

func platform_mem_init
	ret
endfunc platform_mem_init

func plat_my_core_pos
	ldr	x1, =(MPID_MASK & ~(MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT))
	mrs	x0, mpidr_el1
	and	x0, x0, x1
	b	plat_fpga_calc_core_pos

endfunc plat_my_core_pos

/* -----------------------------------------------------------------------
 * unsigned int plat_fpga_calc_core_pos (uint32_t mpid)
 * Clobber registers: x0 to x5
 * -----------------------------------------------------------------------
 */
func plat_fpga_calc_core_pos
	/*
	 * Check for MT bit in MPIDR, which may be either value for images
	 * running on the FPGA.
	 *
	 * If not set, shift MPIDR to left to make it look as if in a
	 * multi-threaded implementation.
	 *
	 */
	tst	x0, #MPIDR_MT_MASK
	lsl	x3, x0, #MPIDR_AFFINITY_BITS
	csel	x3, x3, x0, eq

	/* Extract individual affinity fields from MPIDR */
	ubfx	x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x2, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS

	mov	x4, #FPGA_MAX_CPUS_PER_CLUSTER
	mov	x5, #FPGA_MAX_PE_PER_CPU

	/* Compute linear position */
	madd	x1, x2, x4, x1
	madd	x0, x1, x5, x0

	ret
endfunc plat_fpga_calc_core_pos

func plat_crash_console_init
	mov_imm	x0, PLAT_FPGA_CRASH_UART_BASE
	b	console_pl011_core_init
endfunc plat_crash_console_init

func plat_crash_console_putc
	mov_imm	x1, PLAT_FPGA_CRASH_UART_BASE
	b	console_pl011_core_putc
endfunc plat_crash_console_putc

func plat_crash_console_flush
	mov_imm	x0, PLAT_FPGA_CRASH_UART_BASE
	b	console_pl011_core_flush
endfunc plat_crash_console_flush
