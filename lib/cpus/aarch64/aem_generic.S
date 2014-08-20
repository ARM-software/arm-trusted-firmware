/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <aem_generic.h>
#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>

func aem_generic_core_pwr_dwn
	/* ---------------------------------------------
	 * Disable the Data Cache.
	 * ---------------------------------------------
	 */
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb

	mov	x0, #DCCISW

	/* ---------------------------------------------
	 * Flush L1 cache to PoU.
	 * ---------------------------------------------
	 */
	b	dcsw_op_louis


func aem_generic_cluster_pwr_dwn
	/* ---------------------------------------------
	 * Disable the Data Cache.
	 * ---------------------------------------------
	 */
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb

	/* ---------------------------------------------
	 * Flush L1 and L2 caches to PoC.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	b	dcsw_op_all

	/* ---------------------------------------------
	 * This function provides cpu specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
func aem_generic_cpu_reg_dump
	mov	x6, #0 /* no registers to report */
	ret


/* cpu_ops for Base AEM FVP */
declare_cpu_ops aem_generic, BASE_AEM_MIDR, 1

/* cpu_ops for Foundation FVP */
declare_cpu_ops aem_generic, FOUNDATION_AEM_MIDR, 1
