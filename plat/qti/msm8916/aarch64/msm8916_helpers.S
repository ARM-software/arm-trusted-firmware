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
	 * Out: x0 - 1 on success, 0 on error
	 * Clobber list : x0 - x4
	 * -------------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x1, BLSP_UART_BASE
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
	mov_imm	x1, BLSP_UART_BASE
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
	mov_imm	x1, BLSP_UART_BASE
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
	.if PLATFORM_CORE_COUNT > 1
		mrs	x1, mpidr_el1
		and	x0, x1, #MPIDR_CPU_MASK
		.if PLATFORM_CLUSTER_COUNT > 1
			and	x1, x1, #MPIDR_CLUSTER_MASK
			orr	x0, x0, x1, LSR #(MPIDR_AFFINITY_BITS - \
						  PLATFORM_CPU_PER_CLUSTER_SHIFT)
		.endif
	.else
		/* There is just a single core so always 0 */
		mov	x0, #0
	.endif
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
	cbz	x0, 1f
	ret
1:
	/*
	 * Cold boot: Disable TCM redirect to L2 cache as early as
	 * possible to avoid crashes when making use of the cache.
	 */
	mov_imm	x1, APCS_CFG(0)
	ldr	w2, [x1, #APCS_TCM_START_ADDR]
	and	w2, w2, #~APCS_TCM_REDIRECT_EN_0
	str	w2, [x1, #APCS_TCM_START_ADDR]

	/*
	 * After reset the CPU always starts executing at the fixed reset
	 * address (0x0), which does not match the link address of BL31.
	 * The "boot remapper" redirects all memory accesses to the real
	 * physical address in DRAM.
	 *
	 * For warm boots, this is already handled by loading the real
	 * entry point address above.
	 *
	 * For cold boots, check if the CPU is using the boot remapper,
	 * i.e. if bl31_entrypoint appears to be at the reset address (0x0).
	 */
	adr	x1, bl31_entrypoint
	cbnz	x1, 2f

	/*
	 * Add the real BL31_BASE offset to the return address in the link
	 * register so the CPU will continue at the real address after return.
	 */
	mov_imm	x1, BL31_BASE
	add	lr, lr, x1
2:
	ret
endfunc plat_get_my_entrypoint

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
