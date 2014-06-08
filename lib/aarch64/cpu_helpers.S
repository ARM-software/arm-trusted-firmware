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

	.weak	cpu_reset_handler


func cpu_reset_handler
	/* ---------------------------------------------
	 * As a bare minimal enable the SMP bit.
	 * ---------------------------------------------
	 */
	mrs	x0, midr_el1
	lsr     x1, x0, #MIDR_PN_SHIFT
	and     x1, x1, #MIDR_PN_MASK
	cmp	x1, #MIDR_PN_A57
	b.eq	a57_setup_begin
	cmp	x1, #MIDR_PN_A53
	b.eq	smp_setup_begin
	b	smp_setup_end

a57_setup_begin:
	ubfx	x1, x0, #MIDR_VAR_SHIFT, #4
	cmp	x1, #0		// Major Revision 0
	b.ne	smp_setup_begin
	ubfx	x1, x0, #MIDR_REV_SHIFT, #4
	cmp	x1, #0		// Minor Revision 0
	b.ne	smp_setup_begin
	mov	x1, #CPUACTLR_NO_ALLOC_WBWA
	orr	x1, x1, #CPUACTLR_DCC_AS_DCCI
	mrs	x0, CPUACTLR_EL1
	orr	x0, x0, x1
	msr	CPUACTLR_EL1, x0
	mov	x0, #0x082
	msr	s3_1_c11_c0_2, x0

smp_setup_begin:
	/* ---------------------------------------------
	 * Enable the event stream every 65536 cycles
	 * ---------------------------------------------
	 */
	mov     x0, #(0xf << EVNTI_SHIFT)
	orr	x0, x0, #EVNTEN_BIT
	msr     CNTKCTL_EL1, x0

	mrs	x0, CPUECTLR_EL1
	orr	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
	isb

smp_setup_end:
	ret
