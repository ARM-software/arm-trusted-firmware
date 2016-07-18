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
#include <platform_def.h>

	.globl	clst_warmboot_data

#define PLL_MODE_SHIFT	(0x8)
#define PLL_NORMAL_MODE	((0x3 << (PLL_MODE_SHIFT + 16)) | \
						 (0x1 << PLL_MODE_SHIFT))
#define MPIDR_CLST_L_BITS 0x0
	/*
	 * For different socs, if we want to speed up warmboot,
	 * we need to config some regs here.
	 * If scu was suspend, we must resume related clk
	 * from slow (24M) mode to normal mode first.
	 * X0: MPIDR_EL1 & MPIDR_CLUSTER_MASK
	 */
.macro	func_rockchip_clst_warmboot
	adr	x4, clst_warmboot_data
	lsr	x5, x0, #6
	ldr	w3, [x4, x5]
	str	wzr, [x4, x5]
	cmp	w3, #PMU_CLST_RET
	b.ne	clst_warmboot_end
	ldr	w6, =(PLL_NORMAL_MODE)
	/*
	 * core_l offset is CRU_BASE + 0xc,
	 * core_b offset is CRU_BASE + 0x2c
	 */
	ldr	x7, =(CRU_BASE + 0xc)
	lsr	x2, x0, #3
	str	w6, [x7, x2]
clst_warmboot_end:
.endm

.macro rockchip_clst_warmboot_data
clst_warmboot_data:
	.rept	PLATFORM_CLUSTER_COUNT
	.word	0
	.endr
.endm
