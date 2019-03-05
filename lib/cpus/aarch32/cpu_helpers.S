/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#include <common/bl_common.h>
#include <lib/el3_runtime/cpu_data.h>

#if defined(IMAGE_BL1) || defined(IMAGE_BL32) || (defined(IMAGE_BL2) && BL2_AT_EL3)
	/*
	 * The reset handler common to all platforms.  After a matching
	 * cpu_ops structure entry is found, the correponding reset_handler
	 * in the cpu_ops is invoked. The reset handler is invoked very early
	 * in the boot sequence and it is assumed that we can clobber r0 - r10
	 * without the need to follow AAPCS.
	 * Clobbers: r0 - r10
	 */
	.globl	reset_handler
func reset_handler
	mov	r8, lr

	/* The plat_reset_handler can clobber r0 - r7 */
	bl	plat_reset_handler

	/* Get the matching cpu_ops pointer (clobbers: r0 - r5) */
	bl	get_cpu_ops_ptr

#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif

	/* Get the cpu_ops reset handler */
	ldr	r1, [r0, #CPU_RESET_FUNC]
	cmp	r1, #0
	mov	lr, r8
	bxne	r1
	bx	lr
endfunc reset_handler

#endif

#ifdef IMAGE_BL32 /* The power down core and cluster is needed only in  BL32 */
	/*
	 * void prepare_cpu_pwr_dwn(unsigned int power_level)
	 *
	 * Prepare CPU power down function for all platforms. The function takes
	 * a domain level to be powered down as its parameter. After the cpu_ops
	 * pointer is retrieved from cpu_data, the handler for requested power
	 * level is called.
	 */
	.globl	prepare_cpu_pwr_dwn
func prepare_cpu_pwr_dwn
	/*
	 * If the given power level exceeds CPU_MAX_PWR_DWN_OPS, we call the
	 * power down handler for the last power level
	 */
	mov	r2, #(CPU_MAX_PWR_DWN_OPS - 1)
	cmp	r0, r2
	movhi	r0, r2

	push	{r0, lr}
	bl	_cpu_data
	pop	{r2, lr}

	ldr	r0, [r0, #CPU_DATA_CPU_OPS_PTR]
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif

	/* Get the appropriate power down handler */
	mov	r1, #CPU_PWR_DWN_OPS
	add	r1, r1, r2, lsl #2
	ldr	r1, [r0, r1]
	bx	r1
endfunc prepare_cpu_pwr_dwn

	/*
	 * Initializes the cpu_ops_ptr if not already initialized
	 * in cpu_data. This must only be called after the data cache
	 * is enabled. AAPCS is followed.
	 */
	.globl	init_cpu_ops
func init_cpu_ops
	push	{r4 - r6, lr}
	bl	_cpu_data
	mov	r6, r0
	ldr	r1, [r0, #CPU_DATA_CPU_OPS_PTR]
	cmp	r1, #0
	bne	1f
	bl	get_cpu_ops_ptr
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif
	str	r0, [r6, #CPU_DATA_CPU_OPS_PTR]!
1:
	pop	{r4 - r6, pc}
endfunc init_cpu_ops

#endif /* IMAGE_BL32 */

	/*
	 * The below function returns the cpu_ops structure matching the
	 * midr of the core. It reads the MIDR and finds the matching
	 * entry in cpu_ops entries. Only the implementation and part number
	 * are used to match the entries.
	 * Return :
	 *     r0 - The matching cpu_ops pointer on Success
	 *     r0 - 0 on failure.
	 * Clobbers: r0 - r5
	 */
	.globl	get_cpu_ops_ptr
func get_cpu_ops_ptr
	/* Get the cpu_ops start and end locations */
	ldr	r4, =(__CPU_OPS_START__ + CPU_MIDR)
	ldr	r5, =(__CPU_OPS_END__ + CPU_MIDR)

	/* Initialize the return parameter */
	mov	r0, #0

	/* Read the MIDR_EL1 */
	ldcopr	r2, MIDR
	ldr	r3, =CPU_IMPL_PN_MASK

	/* Retain only the implementation and part number using mask */
	and	r2, r2, r3
1:
	/* Check if we have reached end of list */
	cmp	r4, r5
	bhs	error_exit

	/* load the midr from the cpu_ops */
	ldr	r1, [r4], #CPU_OPS_SIZE
	and	r1, r1, r3

	/* Check if midr matches to midr of this core */
	cmp	r1, r2
	bne	1b

	/* Subtract the increment and offset to get the cpu-ops pointer */
	sub	r0, r4, #(CPU_OPS_SIZE + CPU_MIDR)
error_exit:
	bx	lr
endfunc get_cpu_ops_ptr

/*
 * Extract CPU revision and variant, and combine them into a single numeric for
 * easier comparison.
 */
	.globl	cpu_get_rev_var
func cpu_get_rev_var
	ldcopr	r1, MIDR

	/*
	 * Extract the variant[23:20] and revision[3:0] from r1 and pack it in
	 * r0[0:7] as variant[7:4] and revision[3:0]:
	 *
	 * First extract r1[23:16] to r0[7:0] and zero fill the rest. Then
	 * extract r1[3:0] into r0[3:0] retaining other bits.
	 */
	ubfx	r0, r1, #(MIDR_VAR_SHIFT - MIDR_REV_BITS), #(MIDR_REV_BITS + MIDR_VAR_BITS)
	bfi	r0, r1, #MIDR_REV_SHIFT, #MIDR_REV_BITS
	bx	lr
endfunc cpu_get_rev_var

/*
 * Compare the CPU's revision-variant (r0) with a given value (r1), for errata
 * application purposes. If the revision-variant is less than or same as a given
 * value, indicates that errata applies; otherwise not.
 */
	.globl	cpu_rev_var_ls
func cpu_rev_var_ls
	cmp	r0, r1
	movls	r0, #ERRATA_APPLIES
	movhi	r0, #ERRATA_NOT_APPLIES
	bx	lr
endfunc cpu_rev_var_ls

/*
 * Compare the CPU's revision-variant (r0) with a given value (r1), for errata
 * application purposes. If the revision-variant is higher than or same as a
 * given value, indicates that errata applies; otherwise not.
 */
	.globl	cpu_rev_var_hs
func cpu_rev_var_hs
	cmp	r0, r1
	movge	r0, #ERRATA_APPLIES
	movlt	r0, #ERRATA_NOT_APPLIES
	bx	lr
endfunc cpu_rev_var_hs

#if REPORT_ERRATA
/*
 * void print_errata_status(void);
 *
 * Function to print errata status for CPUs of its class. Must be called only:
 *
 *   - with MMU and data caches are enabled;
 *   - after cpu_ops have been initialized in per-CPU data.
 */
	.globl print_errata_status
func print_errata_status
	/* r12 is pushed only for the sake of 8-byte stack alignment */
	push	{r4, r5, r12, lr}
#ifdef IMAGE_BL1
	/*
	 * BL1 doesn't have per-CPU data. So retrieve the CPU operations
	 * directly.
	 */
	bl	get_cpu_ops_ptr
	ldr	r0, [r0, #CPU_ERRATA_FUNC]
	cmp	r0, #0
	blxne	r0
#else
	/*
	 * Retrieve pointer to cpu_ops, and further, the errata printing
	 * function. If it's non-NULL, jump to the function in turn.
	 */
	bl	_cpu_data
	ldr	r1, [r0, #CPU_DATA_CPU_OPS_PTR]
	ldr	r0, [r1, #CPU_ERRATA_FUNC]
	cmp	r0, #0
	beq	1f

	mov	r4, r0

	/*
	 * Load pointers to errata lock and printed flag. Call
	 * errata_needs_reporting to check whether this CPU needs to report
	 * errata status pertaining to its class.
	 */
	ldr	r0, [r1, #CPU_ERRATA_LOCK]
	ldr	r1, [r1, #CPU_ERRATA_PRINTED]
	bl	errata_needs_reporting
	cmp	r0, #0
	blxne	r4
1:
#endif
	pop	{r4, r5, r12, pc}
endfunc print_errata_status
#endif
