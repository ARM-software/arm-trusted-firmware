/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#if defined(IMAGE_BL31) || (defined(IMAGE_BL2) && BL2_AT_EL3)
#include <cpu_data.h>
#endif
#include <cpu_macros.S>
#include <debug.h>
#include <errata_report.h>

 /* Reset fn is needed in BL at reset vector */
#if defined(IMAGE_BL1) || defined(IMAGE_BL31) || (defined(IMAGE_BL2) && BL2_AT_EL3)
	/*
	 * The reset handler common to all platforms.  After a matching
	 * cpu_ops structure entry is found, the correponding reset_handler
	 * in the cpu_ops is invoked.
	 * Clobbers: x0 - x19, x30
	 */
	.globl	reset_handler
func reset_handler
	mov	x19, x30

	/* The plat_reset_handler can clobber x0 - x18, x30 */
	bl	plat_reset_handler

	/* Get the matching cpu_ops pointer */
	bl	get_cpu_ops_ptr
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif

	/* Get the cpu_ops reset handler */
	ldr	x2, [x0, #CPU_RESET_FUNC]
	mov	x30, x19
	cbz	x2, 1f

	/* The cpu_ops reset handler can clobber x0 - x19, x30 */
	br	x2
1:
	ret
endfunc reset_handler

#endif

#ifdef IMAGE_BL31 /* The power down core and cluster is needed only in  BL31 */
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
	mov_imm	x2, (CPU_MAX_PWR_DWN_OPS - 1)
	cmp	x0, x2
	csel	x2, x2, x0, hi

	mrs	x1, tpidr_el3
	ldr	x0, [x1, #CPU_DATA_CPU_OPS_PTR]
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif

	/* Get the appropriate power down handler */
	mov	x1, #CPU_PWR_DWN_OPS
	add	x1, x1, x2, lsl #3
	ldr	x1, [x0, x1]
	br	x1
endfunc prepare_cpu_pwr_dwn


	/*
	 * Initializes the cpu_ops_ptr if not already initialized
	 * in cpu_data. This can be called without a runtime stack, but may
	 * only be called after the MMU is enabled.
	 * clobbers: x0 - x6, x10
	 */
	.globl	init_cpu_ops
func init_cpu_ops
	mrs	x6, tpidr_el3
	ldr	x0, [x6, #CPU_DATA_CPU_OPS_PTR]
	cbnz	x0, 1f
	mov	x10, x30
	bl	get_cpu_ops_ptr
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif
	str	x0, [x6, #CPU_DATA_CPU_OPS_PTR]!
	mov x30, x10
1:
	ret
endfunc init_cpu_ops
#endif /* IMAGE_BL31 */

#if defined(IMAGE_BL31) && CRASH_REPORTING
	/*
	 * The cpu specific registers which need to be reported in a crash
	 * are reported via cpu_ops cpu_reg_dump function. After a matching
	 * cpu_ops structure entry is found, the correponding cpu_reg_dump
	 * in the cpu_ops is invoked.
	 */
	.globl	do_cpu_reg_dump
func do_cpu_reg_dump
	mov	x16, x30

	/* Get the matching cpu_ops pointer */
	bl	get_cpu_ops_ptr
	cbz	x0, 1f

	/* Get the cpu_ops cpu_reg_dump */
	ldr	x2, [x0, #CPU_REG_DUMP]
	cbz	x2, 1f
	blr	x2
1:
	mov	x30, x16
	ret
endfunc do_cpu_reg_dump
#endif

	/*
	 * The below function returns the cpu_ops structure matching the
	 * midr of the core. It reads the MIDR_EL1 and finds the matching
	 * entry in cpu_ops entries. Only the implementation and part number
	 * are used to match the entries.
	 * Return :
	 *     x0 - The matching cpu_ops pointer on Success
	 *     x0 - 0 on failure.
	 * Clobbers : x0 - x5
	 */
	.globl	get_cpu_ops_ptr
func get_cpu_ops_ptr
	/* Get the cpu_ops start and end locations */
	adr	x4, (__CPU_OPS_START__ + CPU_MIDR)
	adr	x5, (__CPU_OPS_END__ + CPU_MIDR)

	/* Initialize the return parameter */
	mov	x0, #0

	/* Read the MIDR_EL1 */
	mrs	x2, midr_el1
	mov_imm	x3, CPU_IMPL_PN_MASK

	/* Retain only the implementation and part number using mask */
	and	w2, w2, w3
1:
	/* Check if we have reached end of list */
	cmp	x4, x5
	b.eq	error_exit

	/* load the midr from the cpu_ops */
	ldr	x1, [x4], #CPU_OPS_SIZE
	and	w1, w1, w3

	/* Check if midr matches to midr of this core */
	cmp	w1, w2
	b.ne	1b

	/* Subtract the increment and offset to get the cpu-ops pointer */
	sub	x0, x4, #(CPU_OPS_SIZE + CPU_MIDR)
error_exit:
	ret
endfunc get_cpu_ops_ptr

/*
 * Extract CPU revision and variant, and combine them into a single numeric for
 * easier comparison.
 */
	.globl	cpu_get_rev_var
func cpu_get_rev_var
	mrs	x1, midr_el1

	/*
	 * Extract the variant[23:20] and revision[3:0] from MIDR, and pack them
	 * as variant[7:4] and revision[3:0] of x0.
	 *
	 * First extract x1[23:16] to x0[7:0] and zero fill the rest. Then
	 * extract x1[3:0] into x0[3:0] retaining other bits.
	 */
	ubfx	x0, x1, #(MIDR_VAR_SHIFT - MIDR_REV_BITS), #(MIDR_REV_BITS + MIDR_VAR_BITS)
	bfxil	x0, x1, #MIDR_REV_SHIFT, #MIDR_REV_BITS
	ret
endfunc cpu_get_rev_var

/*
 * Compare the CPU's revision-variant (x0) with a given value (x1), for errata
 * application purposes. If the revision-variant is less than or same as a given
 * value, indicates that errata applies; otherwise not.
 */
	.globl	cpu_rev_var_ls
func cpu_rev_var_ls
	mov	x2, #ERRATA_APPLIES
	mov	x3, #ERRATA_NOT_APPLIES
	cmp	x0, x1
	csel	x0, x2, x3, ls
	ret
endfunc cpu_rev_var_ls

/*
 * Compare the CPU's revision-variant (x0) with a given value (x1), for errata
 * application purposes. If the revision-variant is higher than or same as a
 * given value, indicates that errata applies; otherwise not.
 */
	.globl	cpu_rev_var_hs
func cpu_rev_var_hs
	mov	x2, #ERRATA_APPLIES
	mov	x3, #ERRATA_NOT_APPLIES
	cmp	x0, x1
	csel	x0, x2, x3, hs
	ret
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
#ifdef IMAGE_BL1
	/*
	 * BL1 doesn't have per-CPU data. So retrieve the CPU operations
	 * directly.
	 */
	stp	xzr, x30, [sp, #-16]!
	bl	get_cpu_ops_ptr
	ldp	xzr, x30, [sp], #16
	ldr	x1, [x0, #CPU_ERRATA_FUNC]
	cbnz	x1, .Lprint
#else
	/*
	 * Retrieve pointer to cpu_ops from per-CPU data, and further, the
	 * errata printing function. If it's non-NULL, jump to the function in
	 * turn.
	 */
	mrs	x0, tpidr_el3
	ldr	x1, [x0, #CPU_DATA_CPU_OPS_PTR]
	ldr	x0, [x1, #CPU_ERRATA_FUNC]
	cbz	x0, .Lnoprint

	/*
	 * Printing errata status requires atomically testing the printed flag.
	 */
	stp	x19, x30, [sp, #-16]!
	mov	x19, x0

	/*
	 * Load pointers to errata lock and printed flag. Call
	 * errata_needs_reporting to check whether this CPU needs to report
	 * errata status pertaining to its class.
	 */
	ldr	x0, [x1, #CPU_ERRATA_LOCK]
	ldr	x1, [x1, #CPU_ERRATA_PRINTED]
	bl	errata_needs_reporting
	mov	x1, x19
	ldp	x19, x30, [sp], #16
	cbnz	x0, .Lprint
#endif
.Lnoprint:
	ret
.Lprint:
	/* Jump to errata reporting function for this CPU */
	br	x1
endfunc print_errata_status
#endif
