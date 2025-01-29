/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RDV3_PER_CPU_S
#define RDV3_PER_CPU_S

#include <asm_macros.S>
#include <lib/per_cpu/per_cpu_defs.h>

.globl plat_per_cpu_dcache_clean
.globl plat_per_cpu_node_base
.globl plat_per_cpu_base

/* ----------------------------------------------------------------------------
 * Returns per cpu section node base
 * Clobbers : x0, x2
 * ----------------------------------------------------------------------------
*/
func plat_per_cpu_node_base
	/* Load the base address of array */
	adr_l	x2, per_cpu_nodes_base

	/* Calculate the address of base[x0] */
	add	x0, x2, x0, lsl #3
	ldr	x0, [x0]
	ret
endfunc plat_per_cpu_node_base

/* ----------------------------------------------------------------------------
 * Calculate per cpu base for particular cpu.
 * Clobbers : x0 - x2
 * return per cpu base address in x0
 * ----------------------------------------------------------------------------
 */
func plat_per_cpu_base
	/* CPU index */
	mov	x1, x0

	/* Bounds check: cpu < (NRD_CHIP_COUNT * PER_CPU_NODE_CORE_COUNT) */
	mov	x0, #NRD_CHIP_COUNT
	mov	x2, #PER_CPU_NODE_CORE_COUNT
	mul	x0, x0, x2
	cmp	x1, x0
	bhs	exit

	/* r = cpu % PER_CPU_NODE_CORE_COUNT -> x2 */
	udiv	x0, x1, x2
	msub	x2, x0, x2, x1

	/* x0 = per_cpu_nodes_base[node] */
	adrp	x1, per_cpu_nodes_base
	add	x1, x1, :lo12:per_cpu_nodes_base
	add	x1, x1, x0, lsl #3
	/* Loading x0 with particular per cpu section base address */
	ldr	x0, [x1]

	/* x0 += r * __PER_CPU_UNIT_SECTION_SIZE__ */
	ldr	x1, =__PER_CPU_UNIT_SECTION_SIZE__
	madd	x0, x2, x1, x0
	ret

exit:
	b	plat_panic_handler

endfunc plat_per_cpu_base

/* ----------------------------------------------------------------------------
 * No-op placeholder for rdv3.
 *
 * rdv3 uses HW_MANAGED_COHERENCY, so multi-CPU accesses to shared
 * regions are kept coherent in hardware; no explicit dcache clean is needed.
 * ----------------------------------------------------------------------------
 */
func plat_per_cpu_dcache_clean
	ret
endfunc plat_per_cpu_dcache_clean

#endif /* RDV3_PER_CPU_S*/
