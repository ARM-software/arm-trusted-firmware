/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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
#include <cortex_a57.h>
#include <cpu_macros.S>
#include <platform_def.h>
#include "../juno_def.h"

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_report_exception
	.globl	plat_reset_handler
	.globl	platform_get_core_pos
	.globl	platform_mem_init

	/* Define a crash console for the plaform */
#define JUNO_CRASH_CONSOLE_BASE		PL011_UART0_BASE

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, JUNO_CRASH_CONSOLE_BASE
	mov_imm	x1, PL011_UART0_CLK_IN_HZ
	mov_imm	x2, PL011_BAUDRATE
	b	console_core_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, JUNO_CRASH_CONSOLE_BASE
	b	console_core_putc

	/* ---------------------------------------------
	 * void plat_report_exception(unsigned int type)
	 * Function to report an unhandled exception
	 * with platform-specific means.
	 * On Juno platform, it updates the LEDs
	 * to indicate where we are
	 * ---------------------------------------------
	 */
func plat_report_exception
	mrs	x1, CurrentEl
	lsr	x1, x1, #MODE_EL_SHIFT
	lsl	x1, x1, #SYS_LED_EL_SHIFT
	lsl	x0, x0, #SYS_LED_EC_SHIFT
	mov	x2, #(SECURE << SYS_LED_SS_SHIFT)
	orr	x0, x0, x2
	orr	x0, x0, x1
	mov	x1, #VE_SYSREGS_BASE
	add	x1, x1, #V2M_SYS_LED
	str	w0, [x1]
	ret

	/*
	 * Return 0 to 3 for the A53s and 4 or 5 for the A57s
	 */
func platform_get_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	eor	x0, x0, #(1 << MPIDR_AFFINITY_BITS)  // swap A53/A57 order
	add	x0, x1, x0, LSR #6
	ret


	/* -----------------------------------------------------
	 * void platform_mem_init(void);
	 *
	 * We don't need to carry out any memory initialization
	 * on Juno. The Secure RAM is accessible straight away.
	 * -----------------------------------------------------
	 */
func platform_mem_init
	ret

	/* -----------------------------------------------------
	 * void plat_reset_handler(void);
	 *
	 * Implement workaround for defect id 831273 by enabling
	 * an event stream every 65536 cycles and set the L2 RAM
	 * latencies for Cortex-A57.
	 * -----------------------------------------------------
	 */
func plat_reset_handler
	/* Read the MIDR_EL1 */
	mrs	x0, midr_el1
	ubfx	x1, x0, MIDR_PN_SHIFT, #12
	cmp     w1, #((CORTEX_A57_MIDR >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.ne    1f

	/* Change the L2 Data and Tag Ram latency to 3 cycles */
	mov	x0, #(L2_DATA_RAM_LATENCY_3_CYCLES |    \
			(L2_TAG_RAM_LATENCY_3_CYCLES << \
			 L2CTLR_TAG_RAM_LATENCY_SHIFT))
	msr     L2CTLR_EL1, x0

1:
	/* ---------------------------------------------
	* Enable the event stream every 65536 cycles
	* ---------------------------------------------
	*/
	mov     x0, #(0xf << EVNTI_SHIFT)
	orr     x0, x0, #EVNTEN_BIT
	msr     CNTKCTL_EL1, x0
	isb
	ret
