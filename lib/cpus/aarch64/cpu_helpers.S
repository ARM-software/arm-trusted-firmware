/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <common/bl_common.h>
#include <common/debug.h>
#include <cpu_macros.S>
#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>
#include <lib/el3_runtime/cpu_data.h>

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
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif
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
	 *
	 * If cpu_ops for the MIDR_EL1 cannot be found and
	 * SUPPORT_UNKNOWN_MPID is enabled, it will try to look for a
	 * default cpu_ops with an MIDR value of 0.
	 * (Implementation number 0x0 should be reserved for software use
	 * and therefore no clashes should happen with that default value).
	 *
	 * Return :
	 *     x0 - The matching cpu_ops pointer on Success
	 *     x0 - 0 on failure.
	 * Clobbers : x0 - x5
	 */
	.globl	get_cpu_ops_ptr
func get_cpu_ops_ptr
	/* Read the MIDR_EL1 */
	mrs	x2, midr_el1
	mov_imm	x3, CPU_IMPL_PN_MASK

	/* Retain only the implementation and part number using mask */
	and	w2, w2, w3

	/* Get the cpu_ops end location */
	adr_l	x5, (__CPU_OPS_END__ + CPU_MIDR)

	/* Initialize the return parameter */
	mov	x0, #0
1:
	/* Get the cpu_ops start location */
	adr_l	x4, (__CPU_OPS_START__ + CPU_MIDR)

2:
	/* Check if we have reached end of list */
	cmp	x4, x5
	b.eq	search_def_ptr

	/* load the midr from the cpu_ops */
	ldr	x1, [x4], #CPU_OPS_SIZE
	and	w1, w1, w3

	/* Check if midr matches to midr of this core */
	cmp	w1, w2
	b.ne	2b

	/* Subtract the increment and offset to get the cpu-ops pointer */
	sub	x0, x4, #(CPU_OPS_SIZE + CPU_MIDR)
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif
#ifdef SUPPORT_UNKNOWN_MPID
	cbnz	x2, exit_mpid_found
	/* Mark the unsupported MPID flag */
	adrp	x1, unsupported_mpid_flag
	add	x1, x1, :lo12:unsupported_mpid_flag
	str	w2, [x1]
exit_mpid_found:
#endif
	ret

	/*
	 * Search again for a default pointer (MIDR = 0x0)
	 * or return error if already searched.
	 */
search_def_ptr:
#ifdef SUPPORT_UNKNOWN_MPID
	cbz	x2, error_exit
	mov	x2, #0
	b	1b
error_exit:
#endif
#if ENABLE_ASSERTIONS
	/*
	 * Assert if invalid cpu_ops obtained. If this is not valid, it may
	 * suggest that the proper CPU file hasn't been included.
	 */
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif
	ret
endfunc get_cpu_ops_ptr

	.globl	cpu_get_rev_var
func cpu_get_rev_var
	get_rev_var x0, x1
	ret
endfunc cpu_get_rev_var
