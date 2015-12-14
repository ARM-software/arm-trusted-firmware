/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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
#ifndef __PLAT_MACROS_S__
#define __PLAT_MACROS_S__

#include <arm_macros.S>
#include <v2m_def.h>
#include "../fvp_def.h"

	/* ---------------------------------------------
	 * The below required platform porting macro
	 * prints out relevant GIC registers whenever an
	 * unhandled exception is taken in BL31.
	 * Clobbers: x0 - x10, x16, x17, sp
	 * ---------------------------------------------
	 */
	.macro plat_print_gic_regs
	/*
	 * Detect if we're using the base memory map or
	 * the legacy VE memory map
	 */
	mov_imm	x0, (V2M_SYSREGS_BASE + V2M_SYS_ID)
	ldr	w16, [x0]
	/* Extract BLD (12th - 15th bits) from the SYS_ID */
	ubfx	x16, x16, #V2M_SYS_ID_BLD_SHIFT, #4
	/* Check if VE mmap */
	cmp	w16, #BLD_GIC_VE_MMAP
	b.eq	use_ve_mmap
	/* Assume Base Cortex mmap */
	mov_imm	x17, BASE_GICC_BASE
	mov_imm	x16, BASE_GICD_BASE
	b	print_gic_regs
use_ve_mmap:
	mov_imm	x17, VE_GICC_BASE
	mov_imm	x16, VE_GICD_BASE
print_gic_regs:
	arm_print_gic_regs
	.endm

#endif /* __PLAT_MACROS_S__ */
