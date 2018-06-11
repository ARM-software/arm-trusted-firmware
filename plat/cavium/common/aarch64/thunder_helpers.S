/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <platform_def.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>

	/* Global functions */
	.globl	plat_my_core_pos
	.globl	plat_is_my_cpu_primary
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	cavium_secure_entrypoint
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	platform_mem_init

	.globl	thunder_calc_core_pos
	.globl	thunder_sec_entry_point

	/* -----------------------------------------------------
	 * unsigned int plat_my_core_pos(void);
	 *
	 * result: CorePos = CoreId + (ClusterId << 4) + (NodeId << 6)
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	thunder_calc_core_pos
endfunc plat_my_core_pos

func thunder_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	orr	x1, x1, x0, LSR #4
	and	x1, x1, #0x3f /* Cluster and Core ID */
	ubfx	x0, x0, 16, 2 /* Node ID */
	ldr	w2, =(PLATFORM_MAX_CLUSTERS_PER_NODE * PLATFORM_MAX_CPUS_PER_CLUSTER)
	umaddl	x0, w0, w2, x1
	ret
endfunc thunder_calc_core_pos

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 *
	 * This function checks if this is the Primary CPU
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK | MPIDR_NODE_MASK)
	cmp	x0, #THUNDER_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between
	 * a cold and warm boot. If the thunder_sec_entry_point for
	 * this CPU is present, then it's a warm boot.
	 *
	 * -----------------------------------------------------
	 */
func plat_get_my_entrypoint
	adr	x1, thunder_sec_entry_point
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
	/* No support for cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

	/* -----------------------------------------------------
	 * void cavium_secure_entrypoint (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a warm reset.
	 * -----------------------------------------------------
	 */
func cavium_secure_entrypoint

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
endfunc cavium_secure_entrypoint

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, 0x87E028000000
	mov_imm	x1, 7273800
	mov_imm	x2, 115200
	b	console_pl011_core_init

endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, 0x87E028000000
	b	console_pl011_core_putc
endfunc plat_crash_console_putc

	/* --------------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Any memory init, relocation to be done before the
	 * platform boots. Called very early in the boot process.
	 * --------------------------------------------------------
	 */
func platform_mem_init
	ret
endfunc platform_mem_init

	.data
	.align 3

	/* --------------------------------------------------
	 * CPU Secure entry point - resume from suspend
	 * --------------------------------------------------
	 */
thunder_sec_entry_point:
	.quad	0
