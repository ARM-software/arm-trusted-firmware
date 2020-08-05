/*
 * Copyright (c) 2015-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#include <cortex_a72.h>
#include <drivers/ti/uart/uart_16550.h>

#include <platform_def.h>

	.globl	plat_reset_handler
	.globl	platform_get_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	platform_mem_init
	.globl	platform_check_mpidr
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_disable_acp
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	platform_is_primary_cpu
	.globl	plat_brcm_calc_core_pos
	.globl	plat_get_my_entrypoint


	/* ------------------------------------------------------------
	 * void plat_l2_init(void);
	 *
	 * BL1 and BL2 run with one core, one cluster
	 * This is safe to disable cluster coherency
	 * to make use of the data cache MMU WB attribute
	 * for the SRAM.
	 *
	 * Set L2 Auxiliary Control Register
	 * --------------------------------------------------------------------
	 */
func plat_l2_init
	mrs x0, CORTEX_A72_L2ACTLR_EL1
#if (IMAGE_BL1 || IMAGE_BL2) || defined(USE_SINGLE_CLUSTER)
	orr x0, x0, #CORTEX_A72_L2ACTLR_DISABLE_ACE_SH_OR_CHI
#else
	bic x0, x0, #CORTEX_A72_L2ACTLR_DISABLE_ACE_SH_OR_CHI
#endif
	msr CORTEX_A72_L2ACTLR_EL1, x0

	/* Set L2 Control Register */
	mrs x0, CORTEX_A72_L2CTLR_EL1
	mov x1, #((CORTEX_A72_L2_DATA_RAM_LATENCY_MASK << \
			CORTEX_A72_L2CTLR_DATA_RAM_LATENCY_SHIFT) | \
			(CORTEX_A72_L2_TAG_RAM_LATENCY_MASK << \
			CORTEX_A72_L2CTLR_TAG_RAM_LATENCY_SHIFT) | \
			(U(0x1) << CORTEX_A72_L2CTLR_TAG_RAM_SETUP_SHIFT) | \
			(U(0x1) << CORTEX_A72_L2CTLR_DATA_RAM_SETUP_SHIFT))
	bic x0, x0, x1
	mov x1, #((CORTEX_A72_L2_DATA_RAM_LATENCY_3_CYCLES << \
			CORTEX_A72_L2CTLR_DATA_RAM_LATENCY_SHIFT) | \
			(U(0x1) << CORTEX_A72_L2CTLR_TAG_RAM_SETUP_SHIFT) | \
			(U(0x1) << CORTEX_A72_L2CTLR_DATA_RAM_SETUP_SHIFT))
	orr x0, x0, x1
	msr CORTEX_A72_L2CTLR_EL1, x0

	isb
	ret
endfunc plat_l2_init

	/* --------------------------------------------------------------------
	 * void plat_reset_handler(void);
	 *
	 * Before adding code in this function, refer to the guidelines in
	 * docs/firmware-design.md.
	 *
	 * --------------------------------------------------------------------
	 */
func plat_reset_handler
	mov	x9, x30
	bl	plat_l2_init
	mov	x30, x9
	ret
endfunc plat_reset_handler

	/* -----------------------------------------------------
	 * void platform_get_entrypoint (unsigned int mpid);
	 *
	 * Main job of this routine is to distinguish between
	 * a cold and warm boot.
	 * On a cold boot the secondaries first wait for the
	 * platform to be initialized after which they are
	 * hotplugged in. The primary proceeds to perform the
	 * platform initialization.
	 * -----------------------------------------------------
	 */
func platform_get_entrypoint
	/*TBD-STINGRAY*/
	mov x0, #0
	ret
endfunc platform_get_entrypoint

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset e.g
	 * mark the cpu's presence, mechanism to place it in a
	 * holding pen etc.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	bl	plat_my_core_pos
	mov_imm	x1, SECONDARY_CPU_SPIN_BASE_ADDR
	add	x0, x1, x0, LSL #3
	mov	x1, #0
	str	x1, [x0]

	/* Wait until the entrypoint gets populated */
poll_mailbox:
	ldr	x1, [x0]
	cbz	x1, 1f
	br	x1
1:
	wfe
	b	poll_mailbox
endfunc plat_secondary_cold_boot_setup


	/* -----------------------------------------------------
	 * void platform_mem_init(void);
	 *
	 * We don't need to carry out any memory initialization
	 * on CSS platforms. The Secure RAM is accessible straight away.
	 * -----------------------------------------------------
	 */
func platform_mem_init
	/*TBD-STINGRAY*/
	ret
endfunc platform_mem_init

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform.
	 * -----------------------------------------------------
	 */
func platform_check_mpidr
	/*TBD-STINGRAY*/
	mov	x0, xzr
	ret
endfunc platform_check_mpidr

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0, x1, x2
	 * ---------------------------------------------
	 */

func plat_crash_console_init
	mov_imm	x0, BRCM_CRASH_CONSOLE_BASE
	mov_imm	x1, BRCM_CRASH_CONSOLE_REFCLK
	mov_imm	x2, BRCM_CRASH_CONSOLE_BAUDRATE
	b	console_16550_core_init
	ret
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(void)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2, x3
	 * ---------------------------------------------
	 */

func plat_crash_console_putc
	mov_imm x1, BRCM_CRASH_CONSOLE_BASE
	b	console_16550_core_putc
	ret
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * void plat_crash_console_flush(void)
	 * Function to flush crash console
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm x0, BRCM_CRASH_CONSOLE_BASE
	b	console_16550_core_flush
	ret
endfunc plat_crash_console_flush

	/* -----------------------------------------------------
	 * Placeholder function which should be redefined by
	 * each platform. This function is allowed to use
	 * registers x0 - x17.
	 * -----------------------------------------------------
	 */

func plat_disable_acp
	/*TBD-STINGRAY*/
	ret
endfunc plat_disable_acp

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu (applicable only after a cold boot)
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	b	platform_is_primary_cpu
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_brcm_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_brcm_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * unsigned int platform_is_primary_cpu (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu (applicable only after a cold boot)
	 * -----------------------------------------------------
	 */
func platform_is_primary_cpu
	mov	x9, x30
	bl	plat_my_core_pos
	cmp	x0, #PRIMARY_CPU
	cset	x0, eq
	ret	x9
endfunc platform_is_primary_cpu

	/* -----------------------------------------------------
	 *  unsigned int plat_brcm_calc_core_pos(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function: CorePos = (ClusterId * 4) +
	 *  				  CoreId
	 * -----------------------------------------------------
	 */
func plat_brcm_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #7
	ret
endfunc plat_brcm_calc_core_pos

func plat_get_my_entrypoint
	mrs	x0, mpidr_el1
	b	platform_get_entrypoint
endfunc plat_get_my_entrypoint
