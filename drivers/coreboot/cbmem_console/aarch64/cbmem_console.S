/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <console_macros.S>

/*
 * This driver implements access to coreboot's in-memory console
 * (CBMEM console). For the original implementation, see
 * <coreboot>/src/lib/cbmem_console.c.
 */

declare_console cbmc _runtime=1

.section .data.cbmc_size ; .align 2
	cbmc_size: .word 0x0

	/* -----------------------------------------------
	 * int console_cbmc_init(uintptr_t base)
	 * Reads the size field from the structure and
	 * stores it in the data section. This is so that
	 * we have a size we can trust later on, since a
	 * malicious EL1 could manipulate the console
	 * buffer (including the header) at runtime.
	 * In: x0 - CBMEM console base address
	 * Out: returns 1 to indicate success
	 * Clobber list: x0, x2
	 * -----------------------------------------------
	 */
func_console_init cbmc
	ldr	w2, [x0]
	adrp	x0, cbmc_size
	str	w2, [x0, :lo12:cbmc_size]
	mov	x0, #1
	ret
endfunc console_cbmc_init

	/* -----------------------------------------------
	 * int console_cbmc_puts(int c, uintptr_t base)
	 * Writes a character to the CBMEM console buffer,
	 * including overflow handling of the cursor field.
	 * The character must be preserved in x0.
	 * In: x0 - character to be stored
	 *     x1 - CBMEM console base address
	 * Clobber list: x1, x2, x16, x17
	 * -----------------------------------------------
	 */
func_console_putc cbmc
	add	x1, x1, #8		/* keep address of body in X1 */
	adrp	x2, cbmc_size		/* keep known-good size in X2 */
	ldr	w2, [x2, :lo12:cbmc_size]

	ldr	w16, [x1, #-4]		/* load cursor (one u32 before body) */
	and	w17, w16, #0xf0000000	/* keep flags part in W17 */
	and	w16, w16, #0x0fffffff	/* keep actual cursor part in W16 */

	cmp	w16, w2			/* if cursor >= size... */
	b.hs	putc_return		/* ...return (must be malicious!) */

	strb	w0, [x1, w16, uxtw]	/* body[cursor] = character */
	add	w16, w16, #1		/* cursor++ */
	cmp	w16, w2			/* if cursor < size... */
	b.lt	putc_write_back		/* skip overflow handling */

	mov	w16, #0			/* on overflow, set cursor back to 0 */
	orr	w17, w17, #(1 << 31)	/* and set overflow flag */

putc_write_back:
	orr	w16, w16, w17		/* merge cursor and flags back */
	str	w16, [x1, #-4]		/* write back cursor to memory */

putc_return:
	ret
endfunc	console_cbmc_putc

	/* -----------------------------------------------
	 * int console_cbmc_flush(uintptr_t base)
	 * Flushes the CBMEM console by flushing the
	 * console buffer from the cache.
	 * In: x0 - CBMEM console base address
	 * Clobber list: x0, x1, x2, x3
	 * -----------------------------------------------
	 */
func coreboot_cbmc_flush
	adrp	x1, cbmc_size		/* load size of console body */
	ldr	w1, [x1, :lo12:cbmc_size]
	add	x1, x1, #8		/* add size of console header */
	b	clean_dcache_range	/* (clobbers x2 and x3) */
endfunc coreboot_cbmc_flush
