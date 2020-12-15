/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.global	mtpmu_disable

/* -------------------------------------------------------------
 * The functions in this file are called at entrypoint, before
 * the CPU has decided whether this is a cold or a warm boot.
 * Therefore there are no stack yet to rely on for a C function
 * call.
 * -------------------------------------------------------------
 */

/*
 * bool mtpmu_supported(void)
 *
 * Return a boolean indicating whether FEAT_MTPMU is supported or not.
 *
 * Trash registers: r0.
 */
func mtpmu_supported
	ldcopr	r0, ID_DFR1
	and	r0, r0, #(ID_DFR1_MTPMU_MASK >> ID_DFR1_MTPMU_SHIFT)
	cmp	r0, #ID_DFR1_MTPMU_SUPPORTED
	mov	r0, #0
	addeq	r0, r0, #1
	bx	lr
endfunc mtpmu_supported

/*
 * bool el_implemented(unsigned int el)
 *
 * Return a boolean indicating if the specified EL (2 or 3) is implemented.
 *
 * Trash registers: r0
 */
func el_implemented
	cmp	r0, #3
	ldcopr	r0, ID_PFR1
	lsreq	r0, r0, #ID_PFR1_SEC_SHIFT
	lsrne	r0, r0, #ID_PFR1_VIRTEXT_SHIFT
	/*
	 * ID_PFR1_VIRTEXT_MASK is the same as ID_PFR1_SEC_MASK
	 * so use any one of them
	 */
	and	r0, r0, #ID_PFR1_VIRTEXT_MASK
	cmp	r0, #ID_PFR1_ELx_ENABLED
	mov	r0, #0
	addeq	r0, r0, #1
	bx	lr
endfunc el_implemented

/*
 * void mtpmu_disable(void)
 *
 * Disable mtpmu feature if supported.
 *
 * Trash register: r0, r1, r2
 */
func mtpmu_disable
	mov	r2, lr
	bl	mtpmu_supported
	cmp	r0, #0
	bxeq	r2	/* FEAT_MTPMU not supported */

	/* FEAT_MTMPU Supported */
	mov	r0, #3
	bl	el_implemented
	cmp	r0, #0
	beq	1f

	/* EL3 implemented */
	ldcopr	r0, SDCR
	ldr	r1, =SDCR_MTPME_BIT
	bic	r0, r0, r1
	stcopr	r0, SDCR

	/*
	 * If EL3 is implemented, HDCR.MTPME is implemented as Res0 and
	 * FEAT_MTPMU is controlled only from EL3, so no need to perform
	 * any operations for EL2.
	 */
	isb
	bx	r2
1:
	/* EL3 not implemented */
	mov	r0, #2
	bl	el_implemented
	cmp	r0, #0
	bxeq	r2	/* No EL2 or EL3 implemented */

	/* EL2 implemented */
	ldcopr	r0, HDCR
	ldr	r1, =HDCR_MTPME_BIT
	orr	r0, r0, r1
	stcopr	r0, HDCR
	isb
	bx	r2
endfunc mtpmu_disable
