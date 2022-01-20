/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <asm_macros.S>
#include <cortex_a53.h>
#include <drivers/console.h>
#include <lib/cpus/aarch64/cortex_a72.h>

#include <platform_def.h>


	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl  plat_core_pos
	.globl  plat_my_core_pos
	.globl  plat_core_mask
	.globl  plat_my_core_mask
	.globl  plat_core_pos_by_mpidr
	.globl _disable_ldstr_pfetch_A53
	.globl _disable_ldstr_pfetch_A72
	.global	_set_smmu_pagesz_64

	/* int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 */

	/* int plat_crash_console_init(void)
	 * Use normal console by default. Switch it to crash
	 * mode so serial consoles become active again.
	 * NOTE: This default implementation will only work for
	 * crashes that occur after a normal console (marked
	 * valid for the crash state) has been registered with
	 * the console framework. To debug crashes that occur
	 * earlier, the platform has to override these functions
	 * with an implementation that initializes a console
	 * driver with hardcoded parameters. See
	 * docs/porting-guide.rst for more information.
	 */
func plat_crash_console_init
	mov	x3, x30
	mov	x0, #CONSOLE_FLAG_CRASH
	bl	console_switch_state
	mov	x0, #1
	ret	x3
endfunc plat_crash_console_init

	/* void plat_crash_console_putc(int character)
	 * Output through the normal console by default.
	 */
func plat_crash_console_putc
	b	console_putc
endfunc plat_crash_console_putc

	/* void plat_crash_console_flush(void)
	 * Flush normal console by default.
	 */
func plat_crash_console_flush
	b	console_flush
endfunc plat_crash_console_flush

/* This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 */
func plat_core_pos_by_mpidr

	b	plat_core_pos

endfunc plat_core_pos_by_mpidr

#if (SYMMETRICAL_CLUSTERS)
/* unsigned int plat_my_core_mask(void)
 *  generate a mask bit for this core
 */
func plat_my_core_mask
	mrs	x0, MPIDR_EL1
	b	plat_core_mask
endfunc plat_my_core_mask

/* unsigned int plat_core_mask(u_register_t mpidr)
 * generate a lsb-based mask bit for the core specified by mpidr in x0.
 *
 * SoC core = ((cluster * cpu_per_cluster) + core)
 * mask = (1 << SoC core)
 */
func plat_core_mask
	mov	w1, wzr
	mov	w2, wzr

	/* extract cluster */
	bfxil	w1, w0, #8, #8
	/* extract cpu # */
	bfxil	w2, w0, #0, #8

	mov	w0, wzr

	/* error checking */
	cmp	w1, #NUMBER_OF_CLUSTERS
	b.ge	1f
	cmp	w2, #CORES_PER_CLUSTER
	b.ge	1f

	mov	w0, #CORES_PER_CLUSTER
	mul	w1, w1, w0
	add	w1, w1, w2
	mov	w2, #0x1
	lsl	w0, w2, w1
1:
	ret
endfunc plat_core_mask

/*
 * unsigned int plat_my_core_pos(void)
 *  generate a linear core number for this core
 */
func plat_my_core_pos
	mrs	x0, MPIDR_EL1
	b	plat_core_pos
endfunc plat_my_core_pos

/*
 * unsigned int plat_core_pos(u_register_t mpidr)
 * Generate a linear core number for the core specified by mpidr.
 *
 * SoC core = ((cluster * cpu_per_cluster) + core)
 * Returns -1 if mpidr invalid
 */
func plat_core_pos
	mov	w1, wzr
	mov	w2, wzr
	bfxil	w1, w0, #8, #8	/* extract cluster */
	bfxil	w2, w0, #0, #8	/* extract cpu #   */

	mov	w0, #-1

	/* error checking */
	cmp	w1, #NUMBER_OF_CLUSTERS
	b.ge	1f
	cmp	w2, #CORES_PER_CLUSTER
	b.ge	1f

	mov	w0, #CORES_PER_CLUSTER
	mul	w1, w1, w0
	add	w0, w1, w2
1:
	ret
endfunc plat_core_pos

#endif

/* this function disables the load-store prefetch of the calling core
 * Note: this function is for A53 cores ONLY
 * in:  none
 * out: none
 * uses x0
 */
func _disable_ldstr_pfetch_A53
	mrs	x0, CORTEX_A53_CPUACTLR_EL1
	tst	x0, #CORTEX_A53_CPUACTLR_EL1_L1PCTL
	b.ne	1f
	b	2f

.align 6
1:
	dsb	sy
	isb
	bic	x0, x0, #CORTEX_A53_CPUACTLR_EL1_L1PCTL
	msr	CORTEX_A53_CPUACTLR_EL1, x0
	isb

2:
	ret
endfunc _disable_ldstr_pfetch_A53


/* this function disables the load-store prefetch of the calling core
 * Note: this function is for A72 cores ONLY
 * in:  none
 * out: none
 * uses x0
 */
func _disable_ldstr_pfetch_A72

	mrs	x0, CORTEX_A72_CPUACTLR_EL1
	tst	x0, #CORTEX_A72_CPUACTLR_EL1_DISABLE_L1_DCACHE_HW_PFTCH
	b.eq	1f
	b	2f

.align 6
1:
	dsb	sy
	isb
	orr	x0, x0, #CORTEX_A72_CPUACTLR_EL1_DISABLE_L1_DCACHE_HW_PFTCH
	msr	CORTEX_A72_CPUACTLR_EL1, x0
	isb

2:
	ret
endfunc _disable_ldstr_pfetch_A72

/*
 * Function sets the SACR pagesize to 64k
 */
func _set_smmu_pagesz_64

	ldr	x1, =NXP_SMMU_ADDR
	ldr	w0, [x1, #0x10]
	orr	w0, w0, #1 << 16	/* setting to 64K page */
	str	w0, [x1, #0x10]

	ret
endfunc _set_smmu_pagesz_64
