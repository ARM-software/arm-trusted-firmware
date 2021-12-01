/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

#include <msm8916_mmap.h>

#define APCS_TCM_START_ADDR	0x10
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
	 * Out: x0 - 1 on success, 0 on error
	 * Clobber list : x0 - x4
	 * -------------------------------------------------
	 */
func plat_crash_console_init
	mov	x1, #BLSP_UART2_BASE

	/*
	 * If the non-secure world has been actively using the UART there might
	 * be still some characters left to be sent in the FIFO. In that case,
	 * resetting the transmitter too early might cause all output to become
	 * corrupted. To avoid that, try to flush (wait until FIFO empty) first.
	 */
	mov	x4, lr
	bl	console_uartdm_core_flush
	mov	lr, x4

	mov	x0, #1
	b	console_uartdm_core_init
endfunc plat_crash_console_init

	/* -------------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Print a character on the crash console.
	 * In : w0 - character to be printed
	 * Out: w0 - printed character on success
	 * Clobber list : x1, x2
	 * -------------------------------------------------
	 */
func plat_crash_console_putc
	mov	x1, #BLSP_UART2_BASE
	b	console_uartdm_core_putc
endfunc plat_crash_console_putc

	/* -------------------------------------------------
	 * void plat_crash_console_flush(void)
	 * Force a write of all buffered data that has not
	 * been output.
	 * Clobber list : x1, x2
	 * -------------------------------------------------
	 */
func plat_crash_console_flush
	mov	x1, #BLSP_UART2_BASE
	b	console_uartdm_core_flush
endfunc plat_crash_console_flush

	/* -------------------------------------------------
	 * void plat_panic_handler(void) __dead
	 * Called when an unrecoverable error occurs.
	 * -------------------------------------------------
	 */
func plat_panic_handler
	/* Try to shutdown/reset */
	mov_imm	x0, MPM_PS_HOLD
	str	wzr, [x0]
1:	b	1b
endfunc plat_panic_handler

	/* -------------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * Out: x0 - index of the calling CPU
	 * -------------------------------------------------
	 */
func plat_my_core_pos
	/* There is just a single cluster so this is very simple */
	mrs	x0, mpidr_el1
	and	x0, x0, #MPIDR_CPU_MASK
	ret
endfunc plat_my_core_pos

	/* -------------------------------------------------
	 * uintptr_t plat_get_my_entrypoint(void)
	 * Distinguish cold and warm boot and return warm boot
	 * entry address if available.
	 * Out: x0 - warm boot entry point or 0 on cold boot
	 * -------------------------------------------------
	 */
func plat_get_my_entrypoint
	ldr	x0, msm8916_entry_point
	ret
endfunc plat_get_my_entrypoint

	/* -------------------------------------------------
	 * void plat_reset_handler(void)
	 * Perform additional initialization after reset.
	 * Clobber list : x0 - x18, x30
	 * -------------------------------------------------
	 */
func plat_reset_handler
	/*
	 * Check if the CPU is running at the correct address.
	 * During cold boot the CPU enters here at the wrong address
	 * using the "boot remapper". (It remaps the BL31_BASE to
	 * the CPU reset address 0x0).
	 */
	mov	x0, #BL31_BASE
	adr	x1, bl31_entrypoint
	cmp	x0, x1
	b.ne	_remapped_cold_boot
	/* Already running at correct address, just return directly */
	ret

_remapped_cold_boot:
	/*
	 * The previous boot stage seems to use the L2 cache as TCM.
	 * Disable the TCM redirect before enabling caches to avoid
	 * strange crashes.
	 */
	mov	x2, #APCS_CFG
	ldr	w3, [x2, #APCS_TCM_START_ADDR]
	and	w3, w3, #~APCS_TCM_REDIRECT_EN_0
	str	w3, [x2, #APCS_TCM_START_ADDR]

	/* Enter BL31 again at the real address */
	br	x0
endfunc plat_reset_handler

	/* -------------------------------------------------
	 * void platform_mem_init(void)
	 * Performs additional memory initialization early
	 * in the boot process.
	 * -------------------------------------------------
	 */
func platform_mem_init
	/* Nothing to do here, all memory is already initialized */
	ret
endfunc platform_mem_init

	.data
	.align	3

	/* -------------------------------------------------
	 * Warm boot entry point for CPU. Set by PSCI code.
	 * -------------------------------------------------
	 */
msm8916_entry_point:
	.quad	0
