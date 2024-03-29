/*
 * Copyright (c) 2015-2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <platform_def.h>
#include <cortex_a35.h>

	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_calc_core_pos
	.globl	plat_reset_handler
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	platform_mem_init
	.globl  imx_mailbox_init

	/* --------------------------------------------------------------------
	 * Helper macro that reads the part number of the current CPU and jumps
	 * to the given label if it matches the CPU MIDR provided.
	 *
	 * Clobbers x0.
	 * --------------------------------------------------------------------
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label

	mrs	x0, midr_el1
	ubfx	x0, x0, MIDR_PN_SHIFT, #12
	cmp     w0, #((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	\_label

	.endm

	/* ----------------------------------------------
	 * The mailbox_base is used to distinguish warm/cold
	 * reset. The mailbox_base is in the data section, not
	 * in .bss, this allows function to start using this
	 * variable before the runtime memory is initialized.
	 * ----------------------------------------------
	 */
	.section .data.mailbox_base
	.align 3
	mailbox_base: .quad 0x0

	/* ----------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * This function checks if this is the primary CPU
	 * ----------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CPU_MASK)
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ----------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * This Function uses the plat_calc_core_pos()
	 * to get the index of the calling CPU.
	 * ----------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and 	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

	/*
	 * unsigned int plat_calc_core_pos(uint64_t mpidr)
	 * helper function to calculate the core position.
	 * With this function.
	 */
func plat_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and 	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_calc_core_pos

	/* ----------------------------------------------
	 * function to handle platform specific reset.
	 * ----------------------------------------------
	 */
func plat_reset_handler
#if defined(PLAT_imx8ulp)
	/* enable the 512KB cache by default */
	mov	x0, #IMX_SIM1_BASE
	/*
	 * if the RVBADDR is ROM entry, that means we did
	 * NOT switch the L2 cache to 512KB. default is 256K config,
	 * so skip
	 */
	ldr	w1, [x0, #0x5c]
	cmp	w1, #0x1000
	b.eq	1f
	add	x0, x0, #0x30
	ldr	w1, [x0]
	/* if already 512KB config, skip */
	tbnz	w1, #4, 1f
	ldr	w1, [x0]
	orr	w1, w1, #0x10
	str	w1, [x0]
	orr	w1, w1, #0x10000
	str	w1, [x0]
	b	.
1:	mrs	x0, CORTEX_A35_CPUECTLR_EL1
	orr     x0, x0, #(0x1 << 0)
	orr     x0, x0, #(0x1 << 3)
	msr	CORTEX_A35_CPUECTLR_EL1, x0

	mrs	x0, CORTEX_A35_L2ECTLR_EL1
	orr     x0, x0, #(0x1 << 0)
	msr	CORTEX_A35_L2ECTLR_EL1, x0
	isb
#endif
	/* enable EL2 cpuectlr RW access */
	mov	x0, #0x73
	msr	actlr_el3, x0
	msr	actlr_el2, x0
	isb

	ret
endfunc plat_reset_handler

	/* ---------------------------------------------
	 * function to get the entrypoint.
	 * ---------------------------------------------
	 */
func plat_get_my_entrypoint
	adrp	x1, mailbox_base
	ldr	x0, [x1, :lo12:mailbox_base]
	ret
endfunc	plat_get_my_entrypoint

func imx_mailbox_init
	adrp	x1, mailbox_base
	str	x0, [x1, :lo12:mailbox_base]
	ret
endfunc imx_mailbox_init

func plat_secondary_cold_boot_setup
	b	.
endfunc plat_secondary_cold_boot_setup

func plat_crash_console_init
	mov	x0, #1
	ret
endfunc plat_crash_console_init

func plat_crash_console_putc
	ret
endfunc plat_crash_console_putc

func plat_crash_console_flush
	mov	x0, #0
	ret
endfunc plat_crash_console_flush

func platform_mem_init
	ret
endfunc platform_mem_init
