/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <aem_generic.h>
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>

func aem_generic_core_pwr_dwn
	/* Assert if cache is enabled */
#if ENABLE_ASSERTIONS
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif
	/* ---------------------------------------------
	 * Flush L1 cache to PoU.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
	b	dcsw_op_louis
endfunc aem_generic_core_pwr_dwn


func aem_generic_cluster_pwr_dwn
	/* Assert if cache is enabled */
#if ENABLE_ASSERTIONS
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif
	/* ---------------------------------------------
	 * Flush L1 and L2 caches to PoC.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
	b	dcsw_op_all
endfunc aem_generic_cluster_pwr_dwn

/* cpu_ops for Base AEM FVP */
declare_cpu_ops aem_generic, BASE_AEM_MIDR, CPU_NO_RESET_FUNC, \
	aem_generic_core_pwr_dwn, \
	aem_generic_cluster_pwr_dwn
