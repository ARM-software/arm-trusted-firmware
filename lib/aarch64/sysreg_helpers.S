/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <asm_macros.S>

	.globl	read_vbar_el1
	.globl	read_vbar_el2
	.globl	read_vbar_el3
	.globl	write_vbar_el1
	.globl	write_vbar_el2
	.globl	write_vbar_el3

	.globl	read_sctlr_el1
	.globl	read_sctlr_el2
	.globl	read_sctlr_el3
	.globl	write_sctlr_el1
	.globl	write_sctlr_el2
	.globl	write_sctlr_el3

	.globl	read_actlr_el1
	.globl	read_actlr_el2
	.globl	read_actlr_el3
	.globl	write_actlr_el1
	.globl	write_actlr_el2
	.globl	write_actlr_el3

	.globl	read_esr_el1
	.globl	read_esr_el2
	.globl	read_esr_el3
	.globl	write_esr_el1
	.globl	write_esr_el2
	.globl	write_esr_el3

	.globl	read_afsr0_el1
	.globl	read_afsr0_el2
	.globl	read_afsr0_el3
	.globl	write_afsr0_el1
	.globl	write_afsr0_el2
	.globl	write_afsr0_el3

	.globl	read_afsr1_el1
	.globl	read_afsr1_el2
	.globl	read_afsr1_el3
	.globl	write_afsr1_el1
	.globl	write_afsr1_el2
	.globl	write_afsr1_el3

	.globl	read_far_el1
	.globl	read_far_el2
	.globl	read_far_el3
	.globl	write_far_el1
	.globl	write_far_el2
	.globl	write_far_el3

	.globl	read_mair_el1
	.globl	read_mair_el2
	.globl	read_mair_el3
	.globl	write_mair_el1
	.globl	write_mair_el2
	.globl	write_mair_el3

	.globl	read_amair_el1
	.globl	read_amair_el2
	.globl	read_amair_el3
	.globl	write_amair_el1
	.globl	write_amair_el2
	.globl	write_amair_el3

	.globl	read_rvbar_el1
	.globl	read_rvbar_el2
	.globl	read_rvbar_el3

	.globl	read_rmr_el1
	.globl	read_rmr_el2
	.globl	read_rmr_el3
	.globl	write_rmr_el1
	.globl	write_rmr_el2
	.globl	write_rmr_el3

	.globl	read_tcr_el1
	.globl	read_tcr_el2
	.globl	read_tcr_el3
	.globl	write_tcr_el1
	.globl	write_tcr_el2
	.globl	write_tcr_el3

	.globl	read_cptr_el2
	.globl	read_cptr_el3
	.globl	write_cptr_el2
	.globl	write_cptr_el3

	.globl	read_ttbr0_el1
	.globl	read_ttbr0_el2
	.globl	read_ttbr0_el3
	.globl	write_ttbr0_el1
	.globl	write_ttbr0_el2
	.globl	write_ttbr0_el3

	.globl	read_ttbr1_el1
	.globl	write_ttbr1_el1

	.globl	read_cpacr
	.globl	write_cpacr

	.globl	read_cntfrq
	.globl	write_cntfrq

	.globl	read_cpuectlr
	.globl	write_cpuectlr

	.globl	read_cnthctl_el2
	.globl	write_cnthctl_el2

	.globl	read_cntfrq_el0
	.globl	write_cntfrq_el0

	.globl	read_cntps_ctl_el1
	.globl	write_cntps_ctl_el1

	.globl	read_cntps_cval_el1
	.globl	write_cntps_cval_el1

	.globl	read_cntps_tval_el1
	.globl	write_cntps_tval_el1

	.globl	read_scr
	.globl	write_scr

	.globl	read_hcr
	.globl	write_hcr

	.globl	read_midr
	.globl	read_mpidr

	.globl	read_cntpct_el0
	.globl	read_current_el
	.globl	read_id_pfr1_el1
	.globl	read_id_aa64pfr0_el1

	.globl	write_tpidr_el3
	.globl	read_tpidr_el3

#if SUPPORT_VFP
	.globl	enable_vfp
#endif


func read_current_el
	mrs	x0, CurrentEl
	ret


func read_id_pfr1_el1
	mrs	x0, id_pfr1_el1
	ret


func read_id_aa64pfr0_el1
	mrs	x0, id_aa64pfr0_el1
	ret


	/* -----------------------------------------------------
	 * VBAR accessors
	 * -----------------------------------------------------
	 */
func read_vbar_el1
	mrs	x0, vbar_el1
	ret


func read_vbar_el2
	mrs	x0, vbar_el2
	ret


func read_vbar_el3
	mrs	x0, vbar_el3
	ret


func write_vbar_el1
	msr	vbar_el1, x0
	ret


func write_vbar_el2
	msr	vbar_el2, x0
	ret


func write_vbar_el3
	msr	vbar_el3, x0
	ret


	/* -----------------------------------------------------
	 * AFSR0 accessors
	 * -----------------------------------------------------
	 */
func read_afsr0_el1
	mrs	x0, afsr0_el1
	ret


func read_afsr0_el2
	mrs	x0, afsr0_el2
	ret


func read_afsr0_el3
	mrs	x0, afsr0_el3
	ret


func write_afsr0_el1
	msr	afsr0_el1, x0
	ret


func write_afsr0_el2
	msr	afsr0_el2, x0
	ret


func write_afsr0_el3
	msr	afsr0_el3, x0
	ret


	/* -----------------------------------------------------
	 * FAR accessors
	 * -----------------------------------------------------
	 */
func read_far_el1
	mrs	x0, far_el1
	ret


func read_far_el2
	mrs	x0, far_el2
	ret


func read_far_el3
	mrs	x0, far_el3
	ret


func write_far_el1
	msr	far_el1, x0
	ret


func write_far_el2
	msr	far_el2, x0
	ret


func write_far_el3
	msr	far_el3, x0
	ret


	/* -----------------------------------------------------
	 * MAIR accessors
	 * -----------------------------------------------------
	 */
func read_mair_el1
	mrs	x0, mair_el1
	ret


func read_mair_el2
	mrs	x0, mair_el2
	ret


func read_mair_el3
	mrs	x0, mair_el3
	ret


func write_mair_el1
	msr	mair_el1, x0
	ret


func write_mair_el2
	msr	mair_el2, x0
	ret


func write_mair_el3
	msr	mair_el3, x0
	ret


	/* -----------------------------------------------------
	 * AMAIR accessors
	 * -----------------------------------------------------
	 */
func read_amair_el1
	mrs	x0, amair_el1
	ret


func read_amair_el2
	mrs	x0, amair_el2
	ret


func read_amair_el3
	mrs	x0, amair_el3
	ret


func write_amair_el1
	msr	amair_el1, x0
	ret


func write_amair_el2
	msr	amair_el2, x0
	ret


func write_amair_el3
	msr	amair_el3, x0
	ret


	/* -----------------------------------------------------
	 * RVBAR accessors
	 * -----------------------------------------------------
	 */
func read_rvbar_el1
	mrs	x0, rvbar_el1
	ret


func read_rvbar_el2
	mrs	x0, rvbar_el2
	ret


func read_rvbar_el3
	mrs	x0, rvbar_el3
	ret


	/* -----------------------------------------------------
	 * RMR accessors
	 * -----------------------------------------------------
	 */
func read_rmr_el1
	mrs	x0, rmr_el1
	ret


func read_rmr_el2
	mrs	x0, rmr_el2
	ret


func read_rmr_el3
	mrs	x0, rmr_el3
	ret


func write_rmr_el1
	msr	rmr_el1, x0
	ret


func write_rmr_el2
	msr	rmr_el2, x0
	ret


func write_rmr_el3
	msr	rmr_el3, x0
	ret


	/* -----------------------------------------------------
	 * AFSR1 accessors
	 * -----------------------------------------------------
	 */
func read_afsr1_el1
	mrs	x0, afsr1_el1
	ret


func read_afsr1_el2
	mrs	x0, afsr1_el2
	ret


func read_afsr1_el3
	mrs	x0, afsr1_el3
	ret


func write_afsr1_el1
	msr	afsr1_el1, x0
	ret


func write_afsr1_el2
	msr	afsr1_el2, x0
	ret


func write_afsr1_el3
	msr	afsr1_el3, x0
	ret


	/* -----------------------------------------------------
	 * SCTLR accessors
	 * -----------------------------------------------------
	 */
func read_sctlr_el1
	mrs	x0, sctlr_el1
	ret


func read_sctlr_el2
	mrs	x0, sctlr_el2
	ret


func read_sctlr_el3
	mrs	x0, sctlr_el3
	ret


func write_sctlr_el1
	msr	sctlr_el1, x0
	ret


func write_sctlr_el2
	msr	sctlr_el2, x0
	ret


func write_sctlr_el3
	msr	sctlr_el3, x0
	ret


	/* -----------------------------------------------------
	 * ACTLR accessors
	 * -----------------------------------------------------
	 */
func read_actlr_el1
	mrs	x0, actlr_el1
	ret


func read_actlr_el2
	mrs	x0, actlr_el2
	ret


func read_actlr_el3
	mrs	x0, actlr_el3
	ret


func write_actlr_el1
	msr	actlr_el1, x0
	ret


func write_actlr_el2
	msr	actlr_el2, x0
	ret


func write_actlr_el3
	msr	actlr_el3, x0
	ret


	/* -----------------------------------------------------
	 * ESR accessors
	 * -----------------------------------------------------
	 */
func read_esr_el1
	mrs	x0, esr_el1
	ret


func read_esr_el2
	mrs	x0, esr_el2
	ret


func read_esr_el3
	mrs	x0, esr_el3
	ret


func write_esr_el1
	msr	esr_el1, x0
	ret


func write_esr_el2
	msr	esr_el2, x0
	ret


func write_esr_el3
	msr	esr_el3, x0
	ret


	/* -----------------------------------------------------
	 * TCR accessors
	 * -----------------------------------------------------
	 */
func read_tcr_el1
	mrs	x0, tcr_el1
	ret


func read_tcr_el2
	mrs	x0, tcr_el2
	ret


func read_tcr_el3
	mrs	x0, tcr_el3
	ret


func write_tcr_el1
	msr	tcr_el1, x0
	ret


func write_tcr_el2
	msr	tcr_el2, x0
	ret


func write_tcr_el3
	msr	tcr_el3, x0
	ret


	/* -----------------------------------------------------
	 * CPTR accessors
	 * -----------------------------------------------------
	 */
func read_cptr_el2
	mrs	x0, cptr_el2
	ret


func read_cptr_el3
	mrs	x0, cptr_el3
	ret


func write_cptr_el2
	msr	cptr_el2, x0
	ret


func write_cptr_el3
	msr	cptr_el3, x0
	ret


	/* -----------------------------------------------------
	 * TTBR0 accessors
	 * -----------------------------------------------------
	 */
func read_ttbr0_el1
	mrs	x0, ttbr0_el1
	ret


func read_ttbr0_el2
	mrs	x0, ttbr0_el2
	ret


func read_ttbr0_el3
	mrs	x0, ttbr0_el3
	ret


func write_ttbr0_el1
	msr	ttbr0_el1, x0
	ret


func write_ttbr0_el2
	msr	ttbr0_el2, x0
	ret


func write_ttbr0_el3
	msr	ttbr0_el3, x0
	ret


	/* -----------------------------------------------------
	 * TTBR1 accessors
	 * -----------------------------------------------------
	 */
func read_ttbr1_el1
	mrs	x0, ttbr1_el1
	ret


func write_ttbr1_el1
	msr	ttbr1_el1, x0
	ret


func read_hcr
	mrs	x0, hcr_el2
	ret


func write_hcr
	msr	hcr_el2, x0
	ret


func read_cpacr
	mrs	x0, cpacr_el1
	ret


func write_cpacr
	msr	cpacr_el1, x0
	ret


func read_cntfrq_el0
	mrs	x0, cntfrq_el0
	ret


func write_cntfrq_el0
	msr	cntfrq_el0, x0
	ret

func read_cntps_ctl_el1
	mrs	x0, cntps_ctl_el1
	ret

func write_cntps_ctl_el1
	msr	cntps_ctl_el1, x0
	ret

func read_cntps_cval_el1
	mrs	x0, cntps_cval_el1
	ret

func write_cntps_cval_el1
	msr	cntps_cval_el1, x0
	ret

func read_cntps_tval_el1
	mrs	x0, cntps_tval_el1
	ret

func write_cntps_tval_el1
	msr	cntps_tval_el1, x0
	ret

func read_cntpct_el0
	mrs	x0, cntpct_el0
	ret

func read_cpuectlr
	mrs	x0, CPUECTLR_EL1
	ret


func write_cpuectlr
	msr	CPUECTLR_EL1, x0
	ret


func read_cnthctl_el2
	mrs	x0, cnthctl_el2
	ret


func write_cnthctl_el2
	msr	cnthctl_el2, x0
	ret


func read_cntfrq
	mrs	x0, cntfrq_el0
	ret


func write_cntfrq
	msr	cntfrq_el0, x0
	ret


func write_scr
	msr	scr_el3, x0
	ret


func read_scr
	mrs	x0, scr_el3
	ret


func read_midr
	mrs	x0, midr_el1
	ret


func read_mpidr
	mrs	x0, mpidr_el1
	ret

func write_tpidr_el3
	msr	tpidr_el3, x0
	ret

func read_tpidr_el3
	mrs	x0, tpidr_el3
	ret

#if SUPPORT_VFP
func enable_vfp
	mrs	x0, cpacr_el1
	orr	x0, x0, #CPACR_VFP_BITS
	msr	cpacr_el1, x0
	mrs	x0, cptr_el3
	mov	x1, #AARCH64_CPTR_TFP
	bic	x0, x0, x1
	msr	cptr_el3, x0
	isb
	ret

#endif
