/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <console_macros.S>
#include <drivers/coreboot/cbmem_console.h>

/*
 * This driver implements access to coreboot's in-memory console
 * (CBMEM console). For the original implementation, see
 * <coreboot>/src/lib/cbmem_console.c.
 */

	.globl console_cbmc_register
	.globl console_cbmc_putc
	.globl console_cbmc_flush

	/* -----------------------------------------------
	 * int console_cbmc_register(uintptr_t base,
	 *                           console_cbmc_t *console);
	 * Registers a new CBMEM console instance. Reads
	 * the size field from the buffer header structure
	 * and stores it in our console_cbmc_t struct, so
	 * that we keep the size in secure memory where we
	 * can trust it. A malicious EL1 could manipulate
	 * the console buffer (including the header), so we
	 * must not trust its contents after boot.
	 * In:  x0 - CBMEM console base address
	 *      x1 - pointer to empty console_cbmc_t struct
	 * Out: x0 - 1 to indicate success
	 * Clobber list: x0, x1, x2, x7
	 * -----------------------------------------------
	 */
func console_cbmc_register
	str	x0, [x1, #CONSOLE_T_CBMC_BASE]
	ldr	w2, [x0]
	str	w2, [x1, #CONSOLE_T_CBMC_SIZE]
	mov	x0, x1
	finish_console_register cbmc putc=1, flush=1
endfunc console_cbmc_register

	/* -----------------------------------------------
	 * int console_cbmc_puts(int c, console_cbmc_t *console)
	 * Writes a character to the CBMEM console buffer,
	 * including overflow handling of the cursor field.
	 * The character must be preserved in x0.
	 * In: x0 - character to be stored
	 *     x1 - pointer to console_cbmc_t struct
	 * Clobber list: x1, x2, x16, x17
	 * -----------------------------------------------
	 */
func console_cbmc_putc
	ldr	w2, [x1, #CONSOLE_T_CBMC_SIZE]
	ldr	x1, [x1, #CONSOLE_T_CBMC_BASE]
	add	x1, x1, #8		/* keep address of body in x1 */

	ldr	w16, [x1, #-4]		/* load cursor (one u32 before body) */
	and	w17, w16, #0xf0000000	/* keep flags part in w17 */
	and	w16, w16, #0x0fffffff	/* keep actual cursor part in w16 */

	cmp	w16, w2			/* sanity check that cursor < size */
	b.lo	putc_within_bounds
	mov	w0, #-1			/* cursor >= size must be malicious */
	ret				/* so return error, don't write char */

putc_within_bounds:
	strb	w0, [x1, w16, uxtw]	/* body[cursor] = character */
	add	w16, w16, #1		/* cursor++ */
	cmp	w16, w2			/* if cursor < size... */
	b.lo	putc_write_back		/* ...skip overflow handling */

	mov	w16, #0			/* on overflow, set cursor back to 0 */
	orr	w17, w17, #(1 << 31)	/* and set overflow flag */

putc_write_back:
	orr	w16, w16, w17		/* merge cursor and flags back */
	str	w16, [x1, #-4]		/* write back cursor to memory */
	ret
endfunc	console_cbmc_putc

	/* -----------------------------------------------
	 * int console_cbmc_flush(console_cbmc_t *console)
	 * Flushes the CBMEM console by flushing the
	 * console buffer from the CPU's data cache.
	 * In:  x0 - pointer to console_cbmc_t struct
	 * Out: x0 - 0 for success
	 * Clobber list: x0, x1, x2, x3, x5
	 * -----------------------------------------------
	 */
func console_cbmc_flush
	mov	x5, x30
	ldr	x1, [x0, #CONSOLE_T_CBMC_SIZE]
	ldr	x0, [x0, #CONSOLE_T_CBMC_BASE]
	add	x1, x1, #8		/* add size of console header */
	bl	clean_dcache_range	/* (clobbers x2 and x3) */
	mov	x0, #0
	ret	x5
endfunc console_cbmc_flush
