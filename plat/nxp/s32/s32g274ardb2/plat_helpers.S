/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <platform_def.h>

#define S32G_NCORE_CAIU0_BASE_ADDR		UL(0x50400000)
#define S32G_NCORE_CAIUTC_OFF			U(0x0)
#define S32G_NCORE_CAIUTC_ISOLEN_SHIFT		U(1)

.globl	plat_crash_console_flush
.globl	plat_crash_console_init
.globl	plat_crash_console_putc
.globl	plat_is_my_cpu_primary
.globl	plat_my_core_pos
.globl	plat_reset_handler
.globl	plat_secondary_cold_boot_setup
.globl	platform_mem_init
.globl	s32g2_core_pos_by_mpidr

/* int plat_crash_console_init(void); */
func plat_crash_console_init
	mov_imm	x0, UART_BASE
	mov_imm	x1, UART_CLOCK_HZ
	mov_imm	x2, UART_BAUDRATE
	b	console_linflex_core_init
endfunc plat_crash_console_init

/* int plat_crash_console_putc(int); */
func plat_crash_console_putc
	mov_imm	x1, UART_BASE
	b	console_linflex_core_putc
	ret
endfunc plat_crash_console_putc

/* void plat_crash_console_flush(void); */
func plat_crash_console_flush
	ret
endfunc plat_crash_console_flush

/**
 * unsigned int s32g2_core_pos_by_mpidr(u_register_t mpidr);
 *
 * In: x0 -  MPIDR_EL1
 * Out: x0
 * Clobber list: x0, x1
 */
func s32g2_core_pos_by_mpidr
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	lsr	x0, x0, #MPIDR_AFF1_SHIFT
	add	x0, x1, x0, lsl #PLATFORM_MPIDR_CPU_MASK_BITS
	ret
endfunc s32g2_core_pos_by_mpidr

/**
 * unsigned int plat_my_core_pos(void);
 *
 * Out: x0
 * Clobber list: x0, x1, x8
 */
func plat_my_core_pos
	mov	x8, x30
	mrs x0, mpidr_el1
	bl	s32g2_core_pos_by_mpidr
	mov	x30, x8
	ret
endfunc plat_my_core_pos

/**
 * unsigned int plat_is_my_cpu_primary(void);
 *
 * Clobber list: x0, x1, x7, x8
 */
func plat_is_my_cpu_primary
	mov	x7, x30
	bl	plat_my_core_pos
	cmp	x0, #PLATFORM_PRIMARY_CPU
	cset	x0, eq
	mov	x30, x7
	ret
endfunc plat_is_my_cpu_primary


/**
 * void plat_secondary_cold_boot_setup (void);
 */
func plat_secondary_cold_boot_setup
	ret
endfunc plat_secondary_cold_boot_setup

/**
 * void plat_reset_handler(void);
 *
 * Set the CAIUTC[IsolEn] bit for the primary A53 cluster.
 * This is so cache invalidate operations from the early TF-A boot code
 * won't cause Ncore to crash.
 *
 * Clobber list: x0, x1, x2
 */
func plat_reset_handler
	mov	x0, #S32G_NCORE_CAIU0_BASE_ADDR
	ldr	w1, [x0, #S32G_NCORE_CAIUTC_OFF]
	movz	w2, #1
	lsl	w2, w2, #S32G_NCORE_CAIUTC_ISOLEN_SHIFT
	orr	w1, w1, w2
	str	w1, [x0, #S32G_NCORE_CAIUTC_OFF]
	ret
endfunc plat_reset_handler

/* void platform_mem_init(void); */
func platform_mem_init
	mov	x10, x30
	mov	x0, #BL31_BASE
	mov	x1, #(BL31_LIMIT & 0xFFFFU)
	movk	x1, #(BL31_LIMIT >> 16), lsl #16
	sub	x1, x1, x0
	bl	zeromem
	mov	x0, #BL33_BASE
	mov	x1, #(BL33_LIMIT & 0xFFFFU)
	movk	x1, #(BL33_LIMIT >> 16), lsl #16
	sub	x1, x1, x0
	bl	zeromem
	mov	x30, x10
	ret
endfunc platform_mem_init

