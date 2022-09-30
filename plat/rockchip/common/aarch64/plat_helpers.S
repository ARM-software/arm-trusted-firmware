/*
 * Copyright (c) 2013-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a53.h>
#include <cortex_a72.h>
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
	mrs x0, midr_el1
	ubfx x0, x0, MIDR_PN_SHIFT, #12
	cmp w0, #((CORTEX_A72_MIDR >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	handler_a72
	b	handler_end
handler_a72:
	/*
	 * This handler does the following:
	 * Set the L2 Data RAM latency for Cortex-A72.
	 * Set the L2 Tag RAM latency to for Cortex-A72.
	 */
	mov x0, #((5 << CORTEX_A72_L2CTLR_DATA_RAM_LATENCY_SHIFT) |	\
			 (0x1 << 5))
	msr	CORTEX_A72_L2CTLR_EL1, x0
	isb
handler_end:
	ret
endfunc plat_reset_handler

func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #PLAT_RK_CLST_TO_CPUID_SHIFT
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

func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_RK_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* --------------------------------------------------------------------
	 * void plat_panic_handler(void)
	 * Call system reset function on panic. Set up an emergency stack so we
	 * can run C functions (it only needs to last for a few calls until we
	 * reboot anyway).
	 * --------------------------------------------------------------------
	 */
func plat_panic_handler
	msr	spsel, #0
	bl	plat_set_my_stack
	b	rockchip_soc_soft_reset
endfunc plat_panic_handler

	/* --------------------------------------------------------------------
	 * void platform_cpu_warmboot (void);
	 * cpus online or resume enterpoint
	 * --------------------------------------------------------------------
	 */
func platform_cpu_warmboot _align=16
	mrs	x0, MPIDR_EL1
	and	x19, x0, #MPIDR_CPU_MASK
	and	x20, x0, #MPIDR_CLUSTER_MASK
	mov	x0, x20
	func_rockchip_clst_warmboot
	/* --------------------------------------------------------------------
	 * big cluster id is 1
	 * big cores id is from 0-3, little cores id 4-7
	 * --------------------------------------------------------------------
	 */
	add	x21, x19, x20, lsr #PLAT_RK_CLST_TO_CPUID_SHIFT
	/* --------------------------------------------------------------------
	 * get per cpuup flag
         * --------------------------------------------------------------------
	 */
	adr	x4, cpuson_flags
	add	x4, x4, x21, lsl #2
	ldr	w1, [x4]
	/* --------------------------------------------------------------------
	 * check cpuon reason
         * --------------------------------------------------------------------
	 */
	cmp	w1, PMU_CPU_AUTO_PWRDN
	b.eq	boot_entry
	cmp	w1, PMU_CPU_HOTPLUG
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
	str	wzr, [x4]
	/* --------------------------------------------------------------------
	 * get per cpuup boot addr
	 * --------------------------------------------------------------------
	 */
	adr	x5, cpuson_entry_point
	ldr	x2, [x5, x21, lsl #3]
	br	x2
endfunc platform_cpu_warmboot

	/* --------------------------------------------------------------------
	 * Per-CPU Secure entry point - resume or power up
	 * --------------------------------------------------------------------
	 */

#if USE_COHERENT_MEM
	.section .tzfw_coherent_mem, "a"
#else
	.data
#endif
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
