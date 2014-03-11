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

#include <arch_helpers.h>

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
	.globl	read_ttbr1_el2
	.globl	write_ttbr1
	.globl	write_ttbr1_el1
	.globl	write_ttbr1_el2

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

	.globl	read_scr
	.globl	write_scr

	.globl	read_hcr
	.globl	write_hcr

	.globl	read_midr
	.globl	read_mpidr

	.globl	read_current_el
	.globl	read_id_pfr1_el1
	.globl	read_id_aa64pfr0_el1

#if SUPPORT_VFP
	.globl	enable_vfp
	.globl	read_fpexc
	.globl	write_fpexc
#endif


	.section	.text, "ax"

read_current_el: ; .type read_current_el, %function
	mrs	x0, CurrentEl
	ret


read_id_pfr1_el1: ; .type read_id_pfr1_el1, %function
	mrs	x0, id_pfr1_el1
	ret


read_id_aa64pfr0_el1: ; .type read_id_aa64pfr0_el1, %function
	mrs	x0, id_aa64pfr0_el1
	ret


	/* -----------------------------------------------------
	 * VBAR accessors
	 * -----------------------------------------------------
	 */
read_vbar_el1: ; .type read_vbar_el1, %function
	mrs	x0, vbar_el1
	ret


read_vbar_el2: ; .type read_vbar_el2, %function
	mrs	x0, vbar_el2
	ret


read_vbar_el3: ; .type read_vbar_el3, %function
	mrs	x0, vbar_el3
	ret


write_vbar_el1: ; .type write_vbar_el1, %function
	msr	vbar_el1, x0
	isb
	ret


write_vbar_el2: ; .type write_vbar_el2, %function
	msr	vbar_el2, x0
	isb
	ret


write_vbar_el3: ; .type write_vbar_el3, %function
	msr	vbar_el3, x0
	isb
	ret


	/* -----------------------------------------------------
	 * AFSR0 accessors
	 * -----------------------------------------------------
	 */
read_afsr0_el1: ; .type read_afsr0_el1, %function
	mrs	x0, afsr0_el1
	ret


read_afsr0_el2: ; .type read_afsr0_el2, %function
	mrs	x0, afsr0_el2
	ret


read_afsr0_el3: ; .type read_afsr0_el3, %function
	mrs	x0, afsr0_el3
	ret


write_afsr0_el1: ; .type write_afsr0_el1, %function
	msr	afsr0_el1, x0
	isb
	ret


write_afsr0_el2: ; .type write_afsr0_el2, %function
	msr	afsr0_el2, x0
	isb
	ret


write_afsr0_el3: ; .type write_afsr0_el3, %function
	msr	afsr0_el3, x0
	isb
	ret


	/* -----------------------------------------------------
	 * FAR accessors
	 * -----------------------------------------------------
	 */
read_far_el1: ; .type read_far_el1, %function
	mrs	x0, far_el1
	ret


read_far_el2: ; .type read_far_el2, %function
	mrs	x0, far_el2
	ret


read_far_el3: ; .type read_far_el3, %function
	mrs	x0, far_el3
	ret


write_far_el1: ; .type write_far_el1, %function
	msr	far_el1, x0
	isb
	ret


write_far_el2: ; .type write_far_el2, %function
	msr	far_el2, x0
	isb
	ret


write_far_el3: ; .type write_far_el3, %function
	msr	far_el3, x0
	isb
	ret


	/* -----------------------------------------------------
	 * MAIR accessors
	 * -----------------------------------------------------
	 */
read_mair_el1: ; .type read_mair_el1, %function
	mrs	x0, mair_el1
	ret


read_mair_el2: ; .type read_mair_el2, %function
	mrs	x0, mair_el2
	ret


read_mair_el3: ; .type read_mair_el3, %function
	mrs	x0, mair_el3
	ret


write_mair_el1: ; .type write_mair_el1, %function
	msr	mair_el1, x0
	isb
	ret


write_mair_el2: ; .type write_mair_el2, %function
	msr	mair_el2, x0
	isb
	ret


write_mair_el3: ; .type write_mair_el3, %function
	msr	mair_el3, x0
	isb
	ret


	/* -----------------------------------------------------
	 * AMAIR accessors
	 * -----------------------------------------------------
	 */
read_amair_el1: ; .type read_amair_el1, %function
	mrs	x0, amair_el1
	ret


read_amair_el2: ; .type read_amair_el2, %function
	mrs	x0, amair_el2
	ret


read_amair_el3: ; .type read_amair_el3, %function
	mrs	x0, amair_el3
	ret


write_amair_el1: ; .type write_amair_el1, %function
	msr	amair_el1, x0
	isb
	ret


write_amair_el2: ; .type write_amair_el2, %function
	msr	amair_el2, x0
	isb
	ret


write_amair_el3: ; .type write_amair_el3, %function
	msr	amair_el3, x0
	isb
	ret


	/* -----------------------------------------------------
	 * RVBAR accessors
	 * -----------------------------------------------------
	 */
read_rvbar_el1: ; .type read_rvbar_el1, %function
	mrs	x0, rvbar_el1
	ret


read_rvbar_el2: ; .type read_rvbar_el2, %function
	mrs	x0, rvbar_el2
	ret


read_rvbar_el3: ; .type read_rvbar_el3, %function
	mrs	x0, rvbar_el3
	ret


	/* -----------------------------------------------------
	 * RMR accessors
	 * -----------------------------------------------------
	 */
read_rmr_el1: ; .type read_rmr_el1, %function
	mrs	x0, rmr_el1
	ret


read_rmr_el2: ; .type read_rmr_el2, %function
	mrs	x0, rmr_el2
	ret


read_rmr_el3: ; .type read_rmr_el3, %function
	mrs	x0, rmr_el3
	ret


write_rmr_el1: ; .type write_rmr_el1, %function
	msr	rmr_el1, x0
	isb
	ret


write_rmr_el2: ; .type write_rmr_el2, %function
	msr	rmr_el2, x0
	isb
	ret


write_rmr_el3: ; .type write_rmr_el3, %function
	msr	rmr_el3, x0
	isb
	ret


	/* -----------------------------------------------------
	 * AFSR1 accessors
	 * -----------------------------------------------------
	 */
read_afsr1_el1: ; .type read_afsr1_el1, %function
	mrs	x0, afsr1_el1
	ret


read_afsr1_el2: ; .type read_afsr1_el2, %function
	mrs	x0, afsr1_el2
	ret


read_afsr1_el3: ; .type read_afsr1_el3, %function
	mrs	x0, afsr1_el3
	ret


write_afsr1_el1: ; .type write_afsr1_el1, %function
	msr	afsr1_el1, x0
	isb
	ret


write_afsr1_el2: ; .type write_afsr1_el2, %function
	msr	afsr1_el2, x0
	isb
	ret


write_afsr1_el3: ; .type write_afsr1_el3, %function
	msr	afsr1_el3, x0
	isb
	ret


	/* -----------------------------------------------------
	 * SCTLR accessors
	 * -----------------------------------------------------
	 */
read_sctlr_el1: ; .type read_sctlr_el1, %function
	mrs	x0, sctlr_el1
	ret


read_sctlr_el2: ; .type read_sctlr_el2, %function
	mrs	x0, sctlr_el2
	ret


read_sctlr_el3: ; .type read_sctlr_el3, %function
	mrs	x0, sctlr_el3
	ret


write_sctlr_el1: ; .type write_sctlr_el1, %function
	msr	sctlr_el1, x0
	dsb	sy
	isb
	ret


write_sctlr_el2: ; .type write_sctlr_el2, %function
	msr	sctlr_el2, x0
	dsb	sy
	isb
	ret


write_sctlr_el3: ; .type write_sctlr_el3, %function
	msr	sctlr_el3, x0
	dsb	sy
	isb
	ret


	/* -----------------------------------------------------
	 * ACTLR accessors
	 * -----------------------------------------------------
	 */
read_actlr_el1: ; .type read_actlr_el1, %function
	mrs	x0, actlr_el1
	ret


read_actlr_el2: ; .type read_actlr_el2, %function
	mrs	x0, actlr_el2
	ret


read_actlr_el3: ; .type read_actlr_el3, %function
	mrs	x0, actlr_el3
	ret


write_actlr_el1: ; .type write_actlr_el1, %function
	msr	actlr_el1, x0
	dsb	sy
	isb
	ret


write_actlr_el2: ; .type write_actlr_el2, %function
	msr	actlr_el2, x0
	dsb	sy
	isb
	ret


write_actlr_el3: ; .type write_actlr_el3, %function
	msr	actlr_el3, x0
	dsb	sy
	isb
	ret


	/* -----------------------------------------------------
	 * ESR accessors
	 * -----------------------------------------------------
	 */
read_esr_el1: ; .type read_esr_el1, %function
	mrs	x0, esr_el1
	ret


read_esr_el2: ; .type read_esr_el2, %function
	mrs	x0, esr_el2
	ret


read_esr_el3: ; .type read_esr_el3, %function
	mrs	x0, esr_el3
	ret


write_esr_el1: ; .type write_esr_el1, %function
	msr	esr_el1, x0
	dsb	sy
	isb
	ret


write_esr_el2: ; .type write_esr_el2, %function
	msr	esr_el2, x0
	dsb	sy
	isb
	ret


write_esr_el3: ; .type write_esr_el3, %function
	msr	esr_el3, x0
	dsb	sy
	isb
	ret


	/* -----------------------------------------------------
	 * TCR accessors
	 * -----------------------------------------------------
	 */
read_tcr_el1: ; .type read_tcr_el1, %function
	mrs	x0, tcr_el1
	ret


read_tcr_el2: ; .type read_tcr_el2, %function
	mrs	x0, tcr_el2
	ret


read_tcr_el3: ; .type read_tcr_el3, %function
	mrs	x0, tcr_el3
	ret


write_tcr_el1: ; .type write_tcr_el1, %function
	msr	tcr_el1, x0
	dsb	sy
	isb
	ret


write_tcr_el2: ; .type write_tcr_el2, %function
	msr	tcr_el2, x0
	dsb	sy
	isb
	ret


write_tcr_el3: ; .type write_tcr_el3, %function
	msr	tcr_el3, x0
	dsb	sy
	isb
	ret


	/* -----------------------------------------------------
	 * CPTR accessors
	 * -----------------------------------------------------
	 */
read_cptr_el1: ; .type read_cptr_el1, %function
	b	read_cptr_el1
	ret


read_cptr_el2: ; .type read_cptr_el2, %function
	mrs	x0, cptr_el2
	ret


read_cptr_el3: ; .type read_cptr_el3, %function
	mrs	x0, cptr_el3
	ret


write_cptr_el1: ; .type write_cptr_el1, %function
	b	write_cptr_el1


write_cptr_el2: ; .type write_cptr_el2, %function
	msr	cptr_el2, x0
	dsb	sy
	isb
	ret


write_cptr_el3: ; .type write_cptr_el3, %function
	msr	cptr_el3, x0
	dsb	sy
	isb
	ret


	/* -----------------------------------------------------
	 * TTBR0 accessors
	 * -----------------------------------------------------
	 */
read_ttbr0_el1: ; .type read_ttbr0_el1, %function
	mrs	x0, ttbr0_el1
	ret


read_ttbr0_el2: ; .type read_ttbr0_el2, %function
	mrs	x0, ttbr0_el2
	ret


read_ttbr0_el3: ; .type read_ttbr0_el3, %function
	mrs	x0, ttbr0_el3
	ret


write_ttbr0_el1: ; .type write_ttbr0_el1, %function
	msr	ttbr0_el1, x0
	isb
	ret


write_ttbr0_el2: ; .type write_ttbr0_el2, %function
	msr	ttbr0_el2, x0
	isb
	ret


write_ttbr0_el3: ; .type write_ttbr0_el3, %function
	msr	ttbr0_el3, x0
	isb
	ret


	/* -----------------------------------------------------
	 * TTBR1 accessors
	 * -----------------------------------------------------
	 */
read_ttbr1_el1: ; .type read_ttbr1_el1, %function
	mrs	x0, ttbr1_el1
	ret


read_ttbr1_el2: ; .type read_ttbr1_el2, %function
	b	read_ttbr1_el2


read_ttbr1_el3: ; .type read_ttbr1_el3, %function
	b	read_ttbr1_el3


write_ttbr1_el1: ; .type write_ttbr1_el1, %function
	msr	ttbr1_el1, x0
	isb
	ret


write_ttbr1_el2: ; .type write_ttbr1_el2, %function
	b	write_ttbr1_el2


write_ttbr1_el3: ; .type write_ttbr1_el3, %function
	b	write_ttbr1_el3


read_hcr: ; .type read_hcr, %function
	mrs	x0, hcr_el2
	ret


write_hcr: ; .type write_hcr, %function
	msr	hcr_el2, x0
	dsb	sy
	isb
	ret


read_cpacr: ; .type read_cpacr, %function
	mrs	x0, cpacr_el1
	ret


write_cpacr: ; .type write_cpacr, %function
	msr	cpacr_el1, x0
	ret


read_cntfrq_el0: ; .type read_cntfrq_el0, %function
	mrs	x0, cntfrq_el0
	ret


write_cntfrq_el0: ; .type write_cntfrq_el0, %function
	msr	cntfrq_el0, x0
	ret


read_cpuectlr: ; .type read_cpuectlr, %function
	mrs	x0, CPUECTLR_EL1
	ret


write_cpuectlr: ; .type write_cpuectlr, %function
	msr	CPUECTLR_EL1, x0
	dsb	sy
	isb
	ret


read_cnthctl_el2: ; .type read_cnthctl_el2, %function
	mrs	x0, cnthctl_el2
	ret


write_cnthctl_el2: ; .type write_cnthctl_el2, %function
	msr	cnthctl_el2, x0
	ret


read_cntfrq: ; .type read_cntfrq, %function
	mrs	x0, cntfrq_el0
	ret


write_cntfrq: ; .type write_cntfrq, %function
	msr	cntfrq_el0, x0
	ret


write_scr: ; .type write_scr, %function
	msr	scr_el3, x0
	dsb	sy
	isb
	ret


read_scr: ; .type read_scr, %function
	mrs	x0, scr_el3
	ret


read_midr: ; .type read_midr, %function
	mrs	x0, midr_el1
	ret


read_mpidr: ; .type read_mpidr, %function
	mrs	x0, mpidr_el1
	ret


#if SUPPORT_VFP
enable_vfp: ; .type enable_vfp, %function
	mrs	x0, cpacr_el1
	orr	x0, x0, #CPACR_VFP_BITS
	msr	cpacr_el1, x0
	mrs	x0, cptr_el3
	mov	x1, #AARCH64_CPTR_TFP
	bic	x0, x0, x1
	msr	cptr_el3, x0
	ret


	// int read_fpexc(void)
read_fpexc: ; .type read_fpexc, %function
	b	read_fpexc
	ret


	// void write_fpexc(int fpexc)
write_fpexc: ; .type write_fpexc, %function
	b	write_fpexc
	ret

#endif
