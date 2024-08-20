/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/libc/errno.h>

#include <asm_macros.S>
#include <console_macros.S>
#include <lib/utils_def.h>

#define LDIV_MULTIPLIER		U(16)

#define LINFLEX_LINCR1		(0x0)
#define LINCR1_INIT		BIT_32(0)
#define LINCR1_MME		BIT_32(4)

#define LINFLEX_LINSR		(0x8)
#define LINSR_LINS_INITMODE	(0x00001000)
#define LINSR_LINS_RX_TX_MODE	(0x00008000)
#define LINSR_LINS_MASK		(0x0000F000)

#define LINFLEX_UARTCR		(0x10)
#define UARTCR_ROSE		BIT_32(23)

#define LINFLEX_UARTSR		(0x14)
#define LINFLEX_LINIBRR		(0x28)
#define LINFLEX_LINFBRR		(0x24)
#define LINFLEX_BDRL		(0x38)
#define LINFLEX_UARTPTO		(0x50)

#define UARTCR_UART		BIT_32(0)
#define UARTCR_WL0		BIT_32(1)
#define UARTCR_PC0		BIT_32(3)
#define UARTCR_TXEN		BIT_32(4)
#define UARTCR_RXEN		BIT_32(5)
#define UARTCR_PC1		BIT_32(6)
#define UARTCR_TFBM		BIT_32(8)
#define UARTCR_RFBM		BIT_32(9)
#define UARTCR_OSR_SHIFT	U(24)
#define UARTCR_OSR_WIDTH	U(4)

#define UARTSR_DTF		BIT_32(1)

/*
 * "core" functions are low-level implementations that do not require
 * writable memory and are thus safe to call in BL1 crash context.
 */
.globl console_linflex_core_init
.globl console_linflex_core_putc
.globl console_linflex_core_flush

.globl console_linflex_register
.globl console_linflex_putc
.globl console_linflex_flush

/**
 * uint32_t get_ldiv_mult(uintptr_t baseaddr, uint32_t clock,
 *                        uint32_t baud, console_t *console,);
 *
 * Clobber list : x0 - x6
 * Out x4: LDIV multiplier
 */
func get_ldiv_mult
	ldr	w4, [x0, LINFLEX_UARTCR]
	mov	w5, w4

	/* Prepare choices in w5 and w6 */
	ubfx	x5, x5, #UARTCR_OSR_SHIFT, #UARTCR_OSR_WIDTH
	mov	w6, #LDIV_MULTIPLIER

	and	w4, w4, #UARTCR_ROSE
	cmp	w4, #0x0
	csel	w4, w5, w6, ne
	ret
endfunc get_ldiv_mult

/*
 * void linflex_set_brg(uintptr_t baseaddr, uint32_t clock
 *                      uint32_t baud, console_t *console);
 *
 * Clobber list : x0 - x7, x13
 */
func linflex_set_brg
	mov	x13, x30
	bl	get_ldiv_mult
	mov	x30, x13

	/* (x4) dividr = baudrate * ldiv_mult */
	mul	x4, x4, x2
	/* (x5) divisr = clock rate */
	mov	x5, x1
	/* (x6) ibr = divisr / dividr */
	udiv	x6, x5, x4
	/* (x7) fbr = divisr % dividr */
	msub	x7, x6, x4, x5
	/* fbr *= 16 / dividr */
	lsl	x7, x7, #4
	udiv	x7, x7, x4
	/* fbr &= 0xf */
	and	w7, w7, #0xf
	str	w6, [x0, LINFLEX_LINIBRR]
	str	w7, [x0, LINFLEX_LINFBRR]
	ret
endfunc linflex_set_brg

/**
 * int console_linflex_core_init(uintptr_t baseaddr, uint32_t clock,
 *                               uint32_t baud);
 *
 * In:  x0 - Linflex base address
 *      x1 - clock frequency
 *      x2 - baudrate
 * Out: x0 - 1 on success, 0 on error
 * Clobber list : x0 - x7, x13 - x14
 */
func console_linflex_core_init
	/* Set master mode and init mode */
	mov	w4, #(LINCR1_INIT)
	str	w4, [x0, LINFLEX_LINCR1]
	mov	w4, #(LINCR1_MME | LINCR1_INIT)
	str	w4, [x0, LINFLEX_LINCR1]

	/* wait for init mode entry */
wait_init_entry:
	ldr	w4, [x0, LINFLEX_LINSR]
	and	w4, w4, #LINSR_LINS_MASK
	cmp	w4, #LINSR_LINS_INITMODE
	b.ne	wait_init_entry

	/* Set UART bit */
	mov	w4, #UARTCR_UART
	str	w4, [x0, LINFLEX_UARTCR]

	mov	x14, x30
	bl	linflex_set_brg
	mov	x30, x14

	/* Set preset timeout register value. */
	mov	w4, #0xf
	str	w4, [x0, LINFLEX_UARTPTO]

	/* 8-bit data, no parity, Tx/Rx enabled, UART mode */
	mov	w4, #(UARTCR_PC1 | UARTCR_RXEN | UARTCR_TXEN | UARTCR_PC0 | \
		      UARTCR_WL0 | UARTCR_UART | UARTCR_RFBM | UARTCR_TFBM)
	str	w4, [x0, LINFLEX_UARTCR]

	/* End init mode */
	ldr	w4, [x0, LINFLEX_LINCR1]
	bic	w4, w4, #LINCR1_INIT
	str	w4, [x0, LINFLEX_LINCR1]
	ret
endfunc console_linflex_core_init

/**
 * int console_linflex_register(uintptr_t baseaddr, uint32_t clock,
 *                              uint32_t clock, uint32_t baud);
 *
 * Function to initialize and register the console.
 * The caller needs to pass an empty console_linflex_t
 * structure in which *MUST* be allocated in
 * persistent memory (e.g. a global or static local
 * variable, *NOT* on the stack).
 * In:  x0 - Linflex base address
 *      x1 - clock frequency
 *      x2 - baudrate
 *      x3 - pointer to empty console_t structure
 * Out: x0 - 1 on success, 0 on error
 * Clobber list : x0 - x7, x13 - x15
 */
func console_linflex_register
	mov	x15, x30
	bl	console_linflex_core_init
	mov	x30, x15

	/* Populate the base address */
	str	x0, [x3, #CONSOLE_T_BASE]

	mov	x0, x3
	finish_console_register linflex, putc=1, getc=0, flush=1
endfunc console_linflex_register

/**
 * int console_linflex_core_flush(uintptr_t baseaddr);
 *
 * Loop while the TX fifo is not empty, depending on the selected UART mode.
 *
 * In:  x0 - Linflex base address
 * Clobber list : x0 - x1
 */
func console_linflex_core_flush
wait_rx_tx:
	ldr	w1, [x0, LINFLEX_LINSR]
	and	w1, w1, #LINSR_LINS_MASK
	cmp	w1, #LINSR_LINS_RX_TX_MODE
	b.eq	wait_rx_tx

	mov	x0, #0
	ret
endfunc console_linflex_core_flush

/**
 * int console_linflex_core_putc(int c, uintptr_t baseaddr);

 * Out: w0 - printed character on success, < 0 on error.
 * Clobber list : x0 - x3
 */
func console_linflex_core_putc
	cbz	x1, putc_error

	cmp	w0, #'\n'
	b.ne	print_char

	/* Print '\r\n' for each '\n' */
	mov	x0, #'\r'
	mov	x14, x30
	bl	console_linflex_core_putc
	mov	x30, x14
	mov	x0, #'\n'

print_char:
	ldr	w2, [x1, LINFLEX_UARTCR]
	and	w2, w2, #UARTCR_TFBM
	cmp	w2, #0x0
	b.eq	buffer_mode

fifo_mode:
	/* UART is in FIFO mode */
	ldr	w2, [x1, LINFLEX_UARTSR]
	and	w2, w2, #UARTSR_DTF
	cmp	w2, #0
	b.ne	fifo_mode

	strb	w0, [x1, LINFLEX_BDRL]
	b	no_error

buffer_mode:
	strb	w0, [x1, LINFLEX_BDRL]

buffer_loop:
	ldr	w2, [x1, LINFLEX_UARTSR]
	and	w3, w2, #UARTSR_DTF
	cmp	w3, #0
	b.eq	buffer_loop

	/**
	 * In Buffer Mode the DTFTFF bit of UARTSR register
	 * has to be set in software
	 */
	mov	w2, #UARTSR_DTF
	str	w2, [x1, LINFLEX_UARTSR]

no_error:
	mov	x0, #0
	ret

putc_error:
	mov	x0, #-EINVAL
	ret
endfunc console_linflex_core_putc

/**
 * int console_linflex_putc(int c, console_t *console);
 *
 * Function to output a character over the console. It
 * returns the character printed on success or -EINVAL on error.
 * In : w0 - character to be printed
 *      x1 - pointer to console_t struct
 * Out: w0 - printed character on success, < 0 on error.
 * Clobber list : x0 - x3, x15
 */
func console_linflex_putc
	cbz	x1, putc_error
	ldr	x1, [x1, #CONSOLE_T_BASE]

	b	console_linflex_core_putc
puct_error:
	mov	x0, #-EINVAL
	ret
endfunc console_linflex_putc

/**
 * int console_linflex_flush(console_t *console);
 *
 * Function to wait for the TX FIFO to be cleared.
 * In : x0 - pointer to console_t struct
 * Out: x0 - return -1 on error else return 0.
 * Clobber list : x0 - x1
 */
func console_linflex_flush
	cbz	x0, flush_error
	ldr	x0, [x0, #CONSOLE_T_BASE]

	b	console_linflex_core_flush
flush_error:
	mov	x0, #-EINVAL
	ret
endfunc console_linflex_flush
