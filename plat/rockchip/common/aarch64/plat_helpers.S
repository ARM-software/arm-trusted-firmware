/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cortex_a53.h>
#include <cortex_a72.h>
#include <plat_private.h>
#include <platform_def.h>

	.globl	cpuson_entry_point
	.globl	cpuson_flags
	.globl	platform_cpu_warmboot
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_report_exception
	.globl	platform_is_primary_cpu
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_my_core_pos
	.globl	plat_reset_handler


#define RK_REVISION(rev) RK_PLAT_CFG##rev
#define RK_HANDLER(rev) plat_reset_handler_juno_r##rev
#define JUMP_TO_HANDLER_IF_RK_R(revision)	\
	jump_to_handler RK_REVISION(revision), RK_HANDLER(revision)

	/*
	 * Helper macro to jump to the given handler if the board revision
	 * matches.
	 * Expects the Juno board revision in x0.
	 *
	 */
	.macro jump_to_handler _revision, _handler
	cmp	x0, #\_revision
	b.eq	\_handler
	.endm

	/*
	 * Helper macro that reads the part number of the current CPU and jumps
	 * to the given label if it matches the CPU MIDR provided.
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label
	mrs	x0, midr_el1
	ubfx	x0, x0, MIDR_PN_SHIFT, #12
	cmp     w0, #((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	\_label
	.endm

	/*
	 * Platform reset handler for rockchip.
	 * only A53 cores
	 */
func RK_HANDLER(0)
	ret
endfunc RK_HANDLER(0)

	/*
	 * Platform reset handler for rockchip.
	 * - Cortex-A53 processor cluster;
	 * - Cortex-A72 processor cluster.
	 *
	 * This handler does the following:
	 * - Set the L2 Data RAM latency to 2 (i.e. 3 cycles) for Cortex-A72
	 * - Set the L2 Tag RAM latency to 1 (i.e. 2 cycles) for Cortex-A72
	 */
func RK_HANDLER(1)
	/*
	 * Nothing to do on Cortex-A53.
	 *
	 */
	jump_if_cpu_midr CORTEX_A72_MIDR, A72
	ret

A72:
	/* Cortex-A72 specific settings */
	mov	x0, #((2 << L2CTLR_DATA_RAM_LATENCY_SHIFT) |	\
		     (0x1 << 5))
	msr     L2CTLR_EL1, x0
	isb
	ret
endfunc RK_HANDLER(1)

	/*
	 * void plat_reset_handler(void);
	 *
	 * Determine the SOC type and call the appropriate reset
	 * handler.
	 *
	 */
func plat_reset_handler

	mov	x0, RK_PLAT_AARCH_CFG

	JUMP_TO_HANDLER_IF_RK_R(0)
	JUMP_TO_HANDLER_IF_RK_R(1)

	/* SOC type is not supported */
not_supported:
	b	not_supported
endfunc plat_reset_handler

func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

	/* --------------------------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * --------------------------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	/* rk3368 does not do cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

func platform_is_primary_cpu
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_RK_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc platform_is_primary_cpu

	/* --------------------------------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * --------------------------------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, PLAT_RK_UART_BASE
	mov_imm	x1, PLAT_RK_UART_CLOCK
	mov_imm	x2, PLAT_RK_UART_BAUDRATE
	b	console_core_init
endfunc plat_crash_console_init

	/* --------------------------------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * --------------------------------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm x1, PLAT_RK_UART_BASE
	b	console_core_putc
endfunc plat_crash_console_putc

	/* --------------------------------------------------------------------
	 * void platform_cpu_warmboot (void);
	 * cpus online or resume enterpoint
	 * --------------------------------------------------------------------
	 */
	.align	16
func platform_cpu_warmboot
	mrs	x0, MPIDR_EL1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	/* --------------------------------------------------------------------
	 * big cluster id is 1
	 * big cores id is from 0-3, little cores id 4-7
	 * --------------------------------------------------------------------
	 */
	add	x0, x1, x0, lsr #6
	/* --------------------------------------------------------------------
	 * get per cpuup flag
         * --------------------------------------------------------------------
	 */
	adr	x4, cpuson_flags
	add	x4, x4, x0, lsl #2
	ldr	w1, [x4]
	/* --------------------------------------------------------------------
	 * check cpuon reason
         * --------------------------------------------------------------------
	 */
	ldr	w3, =PMU_CPU_AUTO_PWRDN
	cmp	w1, w3
	b.eq	boot_entry
	ldr	w3, =PMU_CPU_HOTPLUG
	cmp	w1, w3
	b.eq	boot_entry
	/* --------------------------------------------------------------------
	 * If the boot core cpuson_flags or cpuson_entry_point is not
	 * expection. force the core into wfe.
         * --------------------------------------------------------------------
	 */
wfe_loop:
	wfe
	b	wfe_loop
boot_entry:
	mov	w1, #0
	str	w1, [x4]
	/* --------------------------------------------------------------------
	 * get per cpuup boot addr
	 * --------------------------------------------------------------------
	 */
	adr	x5, cpuson_entry_point
	ldr	x2, [x5, x0, lsl #3]

	br	x2
endfunc platform_cpu_warmboot

	/* --------------------------------------------------------------------
	 * Per-CPU Secure entry point - resume or power up
	 * --------------------------------------------------------------------
	 */
	.section tzfw_coherent_mem, "a"
	.align  3
cpuson_entry_point:
	.rept	PLATFORM_CORE_COUNT
	.quad	0
	.endr
cpuson_flags:
	.rept	PLATFORM_CORE_COUNT
	.word	0
	.endr
