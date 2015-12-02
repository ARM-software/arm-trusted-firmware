/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <assert_macros.S>
#include <cpu_macros.S>
#include <cortex_a57.h>
#include <cortex_a53.h>
#include <tegra_def.h>

	/* Global functions */
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	platform_mem_init
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	tegra_secure_entrypoint
	.globl	plat_reset_handler

	/* Global variables */
	.globl	tegra_sec_entry_point
	.globl	ns_image_entrypoint
	.globl	tegra_bl31_phys_base

	/* ---------------------
	 * Common CPU init code
	 * ---------------------
	 */
.macro	cpu_init_common

#if ENABLE_L2_DYNAMIC_RETENTION
	/* ---------------------------
	 * Enable processor retention
	 * ---------------------------
	*/
	mrs	x0, L2ECTLR_EL1
	mov	x1, #RETENTION_ENTRY_TICKS_512 << L2ECTLR_RET_CTRL_SHIFT
	bic	x0, x0, #L2ECTLR_RET_CTRL_MASK
	orr	x0, x0, x1
	msr	L2ECTLR_EL1, x0
	isb
#endif

#if ENABLE_CPU_DYNAMIC_RETENTION
	mrs	x0, CPUECTLR_EL1
	mov	x1, #RETENTION_ENTRY_TICKS_512 << CPUECTLR_CPU_RET_CTRL_SHIFT
	bic	x0, x0, #CPUECTLR_CPU_RET_CTRL_MASK
	orr	x0, x0, x1
	msr	CPUECTLR_EL1, x0
	isb
#endif

#if ENABLE_NS_L2_CPUECTRL_RW_ACCESS
	/* -------------------------------------------------------
	 * Enable L2 and CPU ECTLR RW access from non-secure world
	 * -------------------------------------------------------
	*/
	mov	x0, #ACTLR_EL3_ENABLE_ALL_ACCESS
	msr	actlr_el3, x0
	msr	actlr_el2, x0
	isb
#endif

	/* --------------------------------
	 * Enable the cycle count register
	 * --------------------------------
	 */
	mrs	x0, pmcr_el0
	ubfx	x0, x0, #11, #5		// read PMCR.N field
	mov	x1, #1
	lsl	x0, x1, x0
	sub	x0, x0, #1		// mask of event counters
	orr	x0, x0, #0x80000000	// disable overflow intrs
	msr	pmintenclr_el1, x0
	msr	pmuserenr_el0, x1	// enable user mode access

	/* ----------------------------------------------------------------
	 * Allow non-privileged access to CNTVCT: Set CNTKCTL (Kernel Count
	 * register), bit 1 (EL0VCTEN) to enable access to CNTVCT/CNTFRQ
	 * registers from EL0.
	 * ----------------------------------------------------------------
	 */
	mrs	x0, cntkctl_el1
	orr	x0, x0, #EL0VCTEN_BIT
	msr	cntkctl_el1, x0
.endm

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 *
	 * This function checks if this is the Primary CPU
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #TEGRA_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 * unsigned int plat_my_core_pos(void);
	 *
	 * result: CorePos = CoreId + (ClusterId << 2)
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between
	 * a cold and warm boot. If the tegra_sec_entry_point for
	 * this CPU is present, then it's a warm boot.
	 *
	 * -----------------------------------------------------
	 */
func plat_get_my_entrypoint
	adr	x1, tegra_sec_entry_point
	ldr	x0, [x1]
	ret
endfunc plat_get_my_entrypoint

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

	/* --------------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Any memory init, relocation to be done before the
	 * platform boots. Called very early in the boot process.
	 * --------------------------------------------------------
	 */
func platform_mem_init
	mov	x0, #0
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, TEGRA_BOOT_UART_BASE
	mov_imm	x1, TEGRA_BOOT_UART_CLK_IN_HZ
	mov_imm	x2, TEGRA_CONSOLE_BAUDRATE
	b	console_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, TEGRA_BOOT_UART_BASE
	b	console_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------------
	 * Function to handle a platform reset and store
	 * input parameters passed by BL2.
	 * ---------------------------------------------------
	 */
func plat_reset_handler

	/* -----------------------------------
	 * derive and save the phys_base addr
	 * -----------------------------------
	 */
	adr	x17, tegra_bl31_phys_base
	ldr	x18, [x17]
	cbnz	x18, 1f
	adr	x18, bl31_entrypoint
	str	x18, [x17]

1:	cpu_init_common

	ret
endfunc plat_reset_handler

	/* ----------------------------------------
	 * Secure entrypoint function for CPU boot
	 * ----------------------------------------
	 */
	.align 6
func tegra_secure_entrypoint

#if ERRATA_TEGRA_INVALIDATE_BTB_AT_BOOT

	/* -------------------------------------------------------
	 * Invalidate BTB along with I$ to remove any stale
	 * entries from the branch predictor array.
	 * -------------------------------------------------------
	 */
	mrs	x0, CPUACTLR_EL1
	orr	x0, x0, #1
	msr	CPUACTLR_EL1, x0	/* invalidate BTB and I$ together */
	dsb	sy
	isb
	ic	iallu			/* actual invalidate */
	dsb	sy
	isb

	mrs	x0, CPUACTLR_EL1
	bic	x0, x0, #1
	msr	CPUACTLR_EL1, X0	/* restore original CPUACTLR_EL1 */
	dsb	sy
	isb

	.rept	7
	nop				/* wait */
	.endr

	/* -----------------------------------------------
	 * Extract OSLK bit and check if it is '1'. This
	 * bit remains '0' for A53 on warm-resets. If '1',
	 * turn off regional clock gating and request warm
	 * reset.
	 * -----------------------------------------------
	 */
	mrs	x0, oslsr_el1
	and	x0, x0, #2
	mrs	x1, mpidr_el1
	bics	xzr, x0, x1, lsr #7	/* 0 = slow cluster or warm reset */
	b.eq	restore_oslock
	mov	x0, xzr
	msr	oslar_el1, x0		/* os lock stays 0 across warm reset */
	mov	x3, #3
	movz	x4, #0x8000, lsl #48
	msr	CPUACTLR_EL1, x4	/* turn off RCG */
	isb
	msr	rmr_el3, x3		/* request warm reset */
	isb
	dsb	sy
1:	wfi
	b	1b

	/* --------------------------------------------------
	 * These nops are here so that speculative execution
	 * won't harm us before we are done with warm reset.
	 * --------------------------------------------------
	 */
	.rept	65
	nop
	.endr

	/* --------------------------------------------------
	 * Do not insert instructions here
	 * --------------------------------------------------
	 */
#endif

	/* --------------------------------------------------
	 * Restore OS Lock bit
	 * --------------------------------------------------
	 */
restore_oslock:
	mov	x0, #1
	msr	oslar_el1, x0

	cpu_init_common

	/* ---------------------------------------------------------------------
	 * The initial state of the Architectural feature trap register
	 * (CPTR_EL3) is unknown and it must be set to a known state. All
	 * feature traps are disabled. Some bits in this register are marked as
	 * Reserved and should not be modified.
	 *
	 * CPTR_EL3.TCPAC: This causes a direct access to the CPACR_EL1 from EL1
	 *  or the CPTR_EL2 from EL2 to trap to EL3 unless it is trapped at EL2.
	 * CPTR_EL3.TTA: This causes access to the Trace functionality to trap
	 *  to EL3 when executed from EL0, EL1, EL2, or EL3. If system register
	 *  access to trace functionality is not supported, this bit is RES0.
	 * CPTR_EL3.TFP: This causes instructions that access the registers
	 *  associated with Floating Point and Advanced SIMD execution to trap
	 *  to EL3 when executed from any exception level, unless trapped to EL1
	 *  or EL2.
	 * ---------------------------------------------------------------------
	 */
	mrs	x1, cptr_el3
	bic	w1, w1, #TCPAC_BIT
	bic	w1, w1, #TTA_BIT
	bic	w1, w1, #TFP_BIT
	msr	cptr_el3, x1

	/* --------------------------------------------------
	 * Get secure world's entry point and jump to it
	 * --------------------------------------------------
	 */
	bl	plat_get_my_entrypoint
	br	x0
endfunc tegra_secure_entrypoint

	.data
	.align 3

	/* --------------------------------------------------
	 * CPU Secure entry point - resume from suspend
	 * --------------------------------------------------
	 */
tegra_sec_entry_point:
	.quad	0

	/* --------------------------------------------------
	 * NS world's cold boot entry point
	 * --------------------------------------------------
	 */
ns_image_entrypoint:
	.quad	0

	/* --------------------------------------------------
	 * BL31's physical base address
	 * --------------------------------------------------
	 */
tegra_bl31_phys_base:
	.quad	0
