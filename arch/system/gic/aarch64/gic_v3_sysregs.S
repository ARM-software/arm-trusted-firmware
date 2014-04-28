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

#include <asm_macros.S>

	.globl	read_icc_sre_el1
	.globl	read_icc_sre_el2
	.globl	read_icc_sre_el3
	.globl	write_icc_sre_el1
	.globl	write_icc_sre_el2
	.globl	write_icc_sre_el3
	.globl  write_icc_pmr_el1


/*
 * Register definitions used by GCC for GICv3 access.
 * These are defined by ARMCC, so keep them in the GCC specific code for now.
 */
#define ICC_SRE_EL1     S3_0_C12_C12_5
#define ICC_SRE_EL2     S3_4_C12_C9_5
#define ICC_SRE_EL3     S3_6_C12_C12_5
#define ICC_CTLR_EL1    S3_0_C12_C12_4
#define ICC_CTLR_EL3    S3_6_C12_C12_4
#define ICC_PMR_EL1     S3_0_C4_C6_0

func read_icc_sre_el1
	mrs	x0, ICC_SRE_EL1
	ret


func read_icc_sre_el2
	mrs	x0, ICC_SRE_EL2
	ret


func read_icc_sre_el3
	mrs	x0, ICC_SRE_EL3
	ret


func write_icc_sre_el1
	msr	ICC_SRE_EL1, x0
	ret


func write_icc_sre_el2
	msr	ICC_SRE_EL2, x0
	ret


func write_icc_sre_el3
	msr	ICC_SRE_EL3, x0
	ret


func write_icc_pmr_el1
	msr	ICC_PMR_EL1, x0
	ret
