/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <bl1.h>
#include <bl_common.h>

	.globl	bl1_aarch32_smc_handler


func bl1_aarch32_smc_handler
	/* ------------------------------------------------
	 * SMC in BL1 is handled assuming that the MMU is
	 * turned off by BL2.
	 * ------------------------------------------------
	 */

	/* ----------------------------------------------
	 * Only RUN_IMAGE SMC is supported.
	 * ----------------------------------------------
	 */
	mov	r8, #BL1_SMC_RUN_IMAGE
	cmp	r8, r0
	blne	report_exception

	/* ------------------------------------------------
	 * Make sure only Secure world reaches here.
	 * ------------------------------------------------
	 */
	ldcopr  r8, SCR
	tst	r8, #SCR_NS_BIT
	blne	report_exception

	/* ---------------------------------------------------------------------
	 * Pass control to next secure image.
	 * Here it expects r1 to contain the address of a entry_point_info_t
	 * structure describing the BL entrypoint.
	 * ---------------------------------------------------------------------
	 */
	mov	r8, r1
	mov	r0, r1
	bl	bl1_print_next_bl_ep_info

#if SPIN_ON_BL1_EXIT
	bl	print_debug_loop_message
debug_loop:
	b	debug_loop
#endif

	mov	r0, r8
	bl	bl1_plat_prepare_exit

	stcopr	r0, TLBIALL
	dsb	sy
	isb

	/*
	 * Extract PC and SPSR based on struct `entry_point_info_t`
	 * and load it in LR and SPSR registers respectively.
	 */
	ldr	lr, [r8, #ENTRY_POINT_INFO_PC_OFFSET]
	ldr	r1, [r8, #(ENTRY_POINT_INFO_PC_OFFSET + 4)]
	msr	spsr, r1

	add	r8, r8, #ENTRY_POINT_INFO_ARGS_OFFSET
	ldm	r8, {r0, r1, r2, r3}
	eret
endfunc bl1_aarch32_smc_handler
