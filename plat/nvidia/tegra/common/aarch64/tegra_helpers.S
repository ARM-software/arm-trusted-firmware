/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#include <cortex_a53.h>
#include <cortex_a57.h>
#include <platform_def.h>
#include <tegra_def.h>

#define MIDR_PN_CORTEX_A57		0xD07

/*******************************************************************************
 * Implementation defined ACTLR_EL3 bit definitions
 ******************************************************************************/
#define ACTLR_EL3_L2ACTLR_BIT		(1 << 6)
#define ACTLR_EL3_L2ECTLR_BIT		(1 << 5)
#define ACTLR_EL3_L2CTLR_BIT		(1 << 4)
#define ACTLR_EL3_CPUECTLR_BIT		(1 << 1)
#define ACTLR_EL3_CPUACTLR_BIT		(1 << 0)
#define ACTLR_EL3_ENABLE_ALL_ACCESS	(ACTLR_EL3_L2ACTLR_BIT | \
					 ACTLR_EL3_L2ECTLR_BIT | \
					 ACTLR_EL3_L2CTLR_BIT | \
					 ACTLR_EL3_CPUECTLR_BIT | \
					 ACTLR_EL3_CPUACTLR_BIT)

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
	.globl	tegra_console_base
	.globl	tegra_enable_l2_ecc_parity_prot

	/* ---------------------
	 * Common CPU init code
	 * ---------------------
	 */
.macro	cpu_init_common

	/* ------------------------------------------------
	 * We enable procesor retention, L2/CPUECTLR NS
	 * access and ECC/Parity protection for A57 CPUs
	 * ------------------------------------------------
	 */
	mrs	x0, midr_el1
	mov	x1, #(MIDR_PN_MASK << MIDR_PN_SHIFT)
	and	x0, x0, x1
	lsr	x0, x0, #MIDR_PN_SHIFT
	cmp	x0, #MIDR_PN_CORTEX_A57
	b.ne	1f

	/* ---------------------------
	 * Enable processor retention
	 * ---------------------------
	 */
	mrs	x0, CORTEX_A57_L2ECTLR_EL1
	mov	x1, #RETENTION_ENTRY_TICKS_512
	bic	x0, x0, #CORTEX_A57_L2ECTLR_RET_CTRL_MASK
	orr	x0, x0, x1
	msr	CORTEX_A57_L2ECTLR_EL1, x0
	isb

	mrs	x0, CORTEX_A57_ECTLR_EL1
	mov	x1, #RETENTION_ENTRY_TICKS_512
	bic	x0, x0, #CORTEX_A57_ECTLR_CPU_RET_CTRL_MASK
	orr	x0, x0, x1
	msr	CORTEX_A57_ECTLR_EL1, x0
	isb

	/* -------------------------------------------------------
	 * Enable L2 and CPU ECTLR RW access from non-secure world
	 * -------------------------------------------------------
	 */
	mov	x0, #ACTLR_EL3_ENABLE_ALL_ACCESS
	msr	actlr_el3, x0
	msr	actlr_el2, x0
	isb

	/* -------------------------------------------------------
	 * Enable L2 ECC and Parity Protection
	 * -------------------------------------------------------
	 */
	adr	x0, tegra_enable_l2_ecc_parity_prot
	ldr	x0, [x0]
	cbz	x0, 1f
	mrs	x0, CORTEX_A57_L2CTLR_EL1
	and	x1, x0, #CORTEX_A57_L2_ECC_PARITY_PROTECTION_BIT
	cbnz	x1, 1f
	orr	x0, x0, #CORTEX_A57_L2_ECC_PARITY_PROTECTION_BIT
	msr	CORTEX_A57_L2CTLR_EL1, x0
	isb

	/* --------------------------------
	 * Enable the cycle count register
	 * --------------------------------
	 */
1:	mrs	x0, pmcr_el0
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
	 * int platform_get_core_pos(int mpidr);
	 *
	 * With this function: CorePos = (ClusterId * 4) +
	 *                                CoreId
	 * -----------------------------------------------------
	 */
func platform_get_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc platform_get_core_pos

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
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov	x0, #0
	adr	x1, tegra_console_base
	ldr	x1, [x1]
	cbz	x1, 1f
	mov	w0, #1
1:	ret
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	adr	x1, tegra_console_base
	ldr	x1, [x1]
	b	console_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------------
	 * Function to handle a platform reset and store
	 * input parameters passed by BL2.
	 * ---------------------------------------------------
	 */
func plat_reset_handler

	/* ----------------------------------------------------
	 * Verify if we are running from BL31_BASE address
	 * ----------------------------------------------------
	 */
	adr	x18, bl31_entrypoint
	mov	x17, #BL31_BASE
	cmp	x18, x17
	b.eq	1f

	/* ----------------------------------------------------
	 * Copy the entire BL31 code to BL31_BASE if we are not
	 * running from it already
	 * ----------------------------------------------------
	 */
	mov	x0, x17
	mov	x1, x18
	mov	x2, #BL31_SIZE
_loop16:
	cmp	x2, #16
	b.lo	_loop1
	ldp	x3, x4, [x1], #16
	stp	x3, x4, [x0], #16
	sub	x2, x2, #16
	b	_loop16
	/* copy byte per byte */
_loop1:
	cbz	x2, _end
	ldrb	w3, [x1], #1
	strb	w3, [x0], #1
	subs	x2, x2, #1
	b.ne	_loop1

	/* ----------------------------------------------------
	 * Jump to BL31_BASE and start execution again
	 * ----------------------------------------------------
	 */
_end:	mov	x0, x20
	mov	x1, x21
	br	x17
1:

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
func tegra_secure_entrypoint _align=6

#if ERRATA_TEGRA_INVALIDATE_BTB_AT_BOOT

	/* -------------------------------------------------------
	 * Invalidate BTB along with I$ to remove any stale
	 * entries from the branch predictor array.
	 * -------------------------------------------------------
	 */
	mrs	x0, CORTEX_A57_CPUACTLR_EL1
	orr	x0, x0, #1
	msr	CORTEX_A57_CPUACTLR_EL1, x0	/* invalidate BTB and I$ together */
	dsb	sy
	isb
	ic	iallu			/* actual invalidate */
	dsb	sy
	isb

	mrs	x0, CORTEX_A57_CPUACTLR_EL1
	bic	x0, x0, #1
	msr	CORTEX_A57_CPUACTLR_EL1, X0	/* restore original CPUACTLR_EL1 */
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
	msr	CORTEX_A57_CPUACTLR_EL1, x4	/* turn off RCG */
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

	/* --------------------------------------------------
	 * UART controller base for console init
	 * --------------------------------------------------
	 */
tegra_console_base:
	.quad	0

	/* --------------------------------------------------
	 * Enable L2 ECC and Parity Protection
	 * --------------------------------------------------
	 */
tegra_enable_l2_ecc_parity_prot:
	.quad	0
