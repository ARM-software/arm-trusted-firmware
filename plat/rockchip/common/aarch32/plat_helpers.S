/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a12.h>
#include <plat_private.h>
#include <plat_pmu_macros.S>

	.globl	cpuson_entry_point
	.globl	cpuson_flags
	.globl	platform_cpu_warmboot
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_report_exception
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_reset_handler
	.globl	plat_panic_handler

	/*
	 * void plat_reset_handler(void);
	 *
	 * Determine the SOC type and call the appropriate reset
	 * handler.
	 *
	 */
func plat_reset_handler
	bx	lr
endfunc plat_reset_handler

func plat_my_core_pos
	ldcopr	r0, MPIDR
	and	r1, r0, #MPIDR_CPU_MASK
#ifdef PLAT_RK_MPIDR_CLUSTER_MASK
	and	r0, r0, #PLAT_RK_MPIDR_CLUSTER_MASK
#else
	and	r0, r0, #MPIDR_CLUSTER_MASK
#endif
	add	r0, r1, r0, LSR #PLAT_RK_CLST_TO_CPUID_SHIFT
	bx	lr
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
	/* rk3288 does not do cold boot for secondary CPU */
cb_panic:
	b	cb_panic
endfunc plat_secondary_cold_boot_setup

func plat_is_my_cpu_primary
	ldcopr	r0, MPIDR
#ifdef PLAT_RK_MPIDR_CLUSTER_MASK
	ldr	r1, =(PLAT_RK_MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
#else
	ldr	r1, =(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
#endif
	and	r0, r1
	cmp	r0, #PLAT_RK_PRIMARY_CPU
	moveq	r0, #1
	movne	r0, #0
	bx	lr
endfunc plat_is_my_cpu_primary

	/* --------------------------------------------------------------------
	 * void plat_panic_handler(void)
	 * Call system reset function on panic. Set up an emergency stack so we
	 * can run C functions (it only needs to last for a few calls until we
	 * reboot anyway).
	 * --------------------------------------------------------------------
	 */
func plat_panic_handler
	bl	plat_set_my_stack
	b	rockchip_soc_soft_reset
endfunc plat_panic_handler

	/* --------------------------------------------------------------------
	 * void platform_cpu_warmboot (void);
	 * cpus online or resume entrypoint
	 * --------------------------------------------------------------------
	 */
func platform_cpu_warmboot _align=16
	push	{ r4 - r7, lr }
	ldcopr	r0, MPIDR
	and	r5, r0, #MPIDR_CPU_MASK
#ifdef PLAT_RK_MPIDR_CLUSTER_MASK
	and	r6, r0, #PLAT_RK_MPIDR_CLUSTER_MASK
#else
	and	r6, r0, #MPIDR_CLUSTER_MASK
#endif
	mov	r0, r6

	func_rockchip_clst_warmboot
	/* --------------------------------------------------------------------
	 * big cluster id is 1
	 * big cores id is from 0-3, little cores id 4-7
	 * --------------------------------------------------------------------
	 */
	add	r7, r5, r6, LSR #PLAT_RK_CLST_TO_CPUID_SHIFT
	/* --------------------------------------------------------------------
	 * get per cpuup flag
         * --------------------------------------------------------------------
	 */
	ldr	r4, =cpuson_flags
	add	r4, r4, r7, lsl #2
	ldr	r1, [r4]
	/* --------------------------------------------------------------------
	 * check cpuon reason
         * --------------------------------------------------------------------
	 */
	cmp	r1, #PMU_CPU_AUTO_PWRDN
	beq	boot_entry
	cmp	r1, #PMU_CPU_HOTPLUG
	beq	boot_entry
	/* --------------------------------------------------------------------
	 * If the boot core cpuson_flags or cpuson_entry_point is not
	 * expection. force the core into wfe.
	 * --------------------------------------------------------------------
	 */
wfe_loop:
	wfe
	b	wfe_loop
boot_entry:
	mov	r1, #0
	str	r1, [r4]
	/* --------------------------------------------------------------------
	 * get per cpuup boot addr
	 * --------------------------------------------------------------------
	 */
	ldr	r5, =cpuson_entry_point
	ldr	r2, [r5, r7, lsl #2] /* ehem. #3 */
	pop	{ r4 - r7, lr }

	bx	r2
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
rockchip_clst_warmboot_data
