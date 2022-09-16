/*
 * Copyright (c) 2021-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

#include <msm8916_mmap.h>

#if PLATFORM_CORE_COUNT > 1
#define APCS_TCM_START_ADDR	0x10
#else
#define APCS_TCM_START_ADDR	0x34
#endif
#define APCS_TCM_REDIRECT_EN_0	BIT_32(0)

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_panic_handler
	.globl	plat_my_core_pos
	.globl	plat_get_my_entrypoint
	.globl	plat_reset_handler
	.globl	platform_mem_init
	.globl	msm8916_entry_point

	/* -------------------------------------------------
	 * int plat_crash_console_init(void)
	 * Initialize the crash console.
	 * Out: r0 - 1 on success, 0 on error
	 * Clobber list : r0 - r4
	 * -------------------------------------------------
	 */
func plat_crash_console_init
	ldr	r1, =BLSP_UART_BASE
	mov	r0, #1
	b	console_uartdm_core_init
endfunc plat_crash_console_init

	/* -------------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Print a character on the crash console.
	 * In : r0 - character to be printed
	 * Out: r0 - printed character on success
	 * Clobber list : r1, r2
	 * -------------------------------------------------
	 */
func plat_crash_console_putc
	ldr	r1, =BLSP_UART_BASE
	b	console_uartdm_core_putc
endfunc plat_crash_console_putc

	/* -------------------------------------------------
	 * void plat_crash_console_flush(void)
	 * Force a write of all buffered data that has not
	 * been output.
	 * Clobber list : r1, r2
	 * -------------------------------------------------
	 */
func plat_crash_console_flush
	ldr	r1, =BLSP_UART_BASE
	b	console_uartdm_core_flush
endfunc plat_crash_console_flush

	/* -------------------------------------------------
	 * void plat_panic_handler(void) __dead
	 * Called when an unrecoverable error occurs.
	 * -------------------------------------------------
	 */
func plat_panic_handler
	/* Try to shutdown/reset */
	ldr	r0, =MPM_PS_HOLD
	mov	r1, #0
	str	r1, [r0]
1:	b	1b
endfunc plat_panic_handler

	/* -------------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * Out: r0 - index of the calling CPU
	 * -------------------------------------------------
	 */
func plat_my_core_pos
	.if PLATFORM_CORE_COUNT > 1
		ldcopr	r1, MPIDR
		and	r0, r1, #MPIDR_CPU_MASK
		.if PLATFORM_CLUSTER_COUNT > 1
			and	r1, r1, #MPIDR_CLUSTER_MASK
			orr	r0, r0, r1, LSR #(MPIDR_AFFINITY_BITS - \
						  PLATFORM_CPU_PER_CLUSTER_SHIFT)
		.endif
	.else
		/* There is just a single core so always 0 */
		mov r0, #0
	.endif
	bx	lr
endfunc plat_my_core_pos

	/* -------------------------------------------------
	 * uintptr_t plat_get_my_entrypoint(void)
	 * Distinguish cold and warm boot and return warm boot
	 * entry address if available.
	 * Out: r0 - warm boot entry point or 0 on cold boot
	 * -------------------------------------------------
	 */
func plat_get_my_entrypoint
	ldr	r0, =msm8916_entry_point
	ldr	r0, [r0]
	cmp	r0, #0
	bxne	lr

	/*
	 * Cold boot: Disable TCM redirect to L2 cache as early as
	 * possible to avoid crashes when making use of the cache.
	 */
	ldr	r1, =APCS_CFG(0)
	ldr	r2, [r1, #APCS_TCM_START_ADDR]
	and	r2, r2, #~APCS_TCM_REDIRECT_EN_0
	str	r2, [r1, #APCS_TCM_START_ADDR]
	bx	lr
endfunc plat_get_my_entrypoint

	/* -------------------------------------------------
	 * void platform_mem_init(void)
	 * Performs additional memory initialization early
	 * in the boot process.
	 * -------------------------------------------------
	 */
func platform_mem_init
	/* Nothing to do here, all memory is already initialized */
	bx	lr
endfunc platform_mem_init

	.data
	.align	3

	/* -------------------------------------------------
	 * Warm boot entry point for CPU. Set by PSCI code.
	 * -------------------------------------------------
	 */
msm8916_entry_point:
	.word	0
