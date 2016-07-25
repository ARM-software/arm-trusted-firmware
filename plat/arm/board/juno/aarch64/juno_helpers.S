/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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
#include <bl_common.h>
#include <cortex_a53.h>
#include <cortex_a57.h>
#include <cortex_a72.h>
#include <v2m_def.h>
#include "../juno_def.h"


	.globl	plat_reset_handler
	.globl	plat_arm_calc_core_pos

#define JUNO_REVISION(rev)	REV_JUNO_R##rev
#define JUNO_HANDLER(rev)	plat_reset_handler_juno_r##rev
#define JUMP_TO_HANDLER_IF_JUNO_R(revision)	\
	jump_to_handler JUNO_REVISION(revision), JUNO_HANDLER(revision)

	/* --------------------------------------------------------------------
	 * Helper macro to jump to the given handler if the board revision
	 * matches.
	 * Expects the Juno board revision in x0.
	 * --------------------------------------------------------------------
	 */
	.macro jump_to_handler _revision, _handler
	cmp	x0, #\_revision
	b.eq	\_handler
	.endm

	/* --------------------------------------------------------------------
	 * Helper macro that reads the part number of the current CPU and jumps
	 * to the given label if it matches the CPU MIDR provided.
	 *
	 * Clobbers x0.
	 * --------------------------------------------------------------------
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label
	mrs	x0, midr_el1
	ubfx	x0, x0, MIDR_PN_SHIFT, #12
	cmp     w0, #((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	\_label
	.endm

	/* --------------------------------------------------------------------
	 * Platform reset handler for Juno R0.
	 *
	 * Juno R0 has the following topology:
	 * - Quad core Cortex-A53 processor cluster;
	 * - Dual core Cortex-A57 processor cluster.
	 *
	 * This handler does the following:
	 * - Implement workaround for defect id 831273 by enabling an event
	 *   stream every 65536 cycles.
	 * - Set the L2 Data RAM latency to 2 (i.e. 3 cycles) for Cortex-A57
	 * - Set the L2 Tag RAM latency to 2 (i.e. 3 cycles) for Cortex-A57
	 * --------------------------------------------------------------------
	 */
func JUNO_HANDLER(0)
	/* --------------------------------------------------------------------
	 * Enable the event stream every 65536 cycles
	 * --------------------------------------------------------------------
	 */
	mov     x0, #(0xf << EVNTI_SHIFT)
	orr     x0, x0, #EVNTEN_BIT
	msr     CNTKCTL_EL1, x0

	/* --------------------------------------------------------------------
	 * Nothing else to do on Cortex-A53.
	 * --------------------------------------------------------------------
	 */
	jump_if_cpu_midr CORTEX_A53_MIDR, 1f

	/* --------------------------------------------------------------------
	 * Cortex-A57 specific settings
	 * --------------------------------------------------------------------
	 */
	mov	x0, #((L2_DATA_RAM_LATENCY_3_CYCLES << L2CTLR_DATA_RAM_LATENCY_SHIFT) |	\
		      (L2_TAG_RAM_LATENCY_3_CYCLES << L2CTLR_TAG_RAM_LATENCY_SHIFT))
	msr     L2CTLR_EL1, x0
1:
	isb
	ret
endfunc JUNO_HANDLER(0)

	/* --------------------------------------------------------------------
	 * Platform reset handler for Juno R1.
	 *
	 * Juno R1 has the following topology:
	 * - Quad core Cortex-A53 processor cluster;
	 * - Dual core Cortex-A57 processor cluster.
	 *
	 * This handler does the following:
	 * - Set the L2 Data RAM latency to 2 (i.e. 3 cycles) for Cortex-A57
	 *
	 * Note that:
	 * - The default value for the L2 Tag RAM latency for Cortex-A57 is
	 *   suitable.
	 * - Defect #831273 doesn't affect Juno R1.
	 * --------------------------------------------------------------------
	 */
func JUNO_HANDLER(1)
	/* --------------------------------------------------------------------
	 * Nothing to do on Cortex-A53.
	 * --------------------------------------------------------------------
	 */
	jump_if_cpu_midr CORTEX_A57_MIDR, A57
	ret

A57:
	/* --------------------------------------------------------------------
	 * Cortex-A57 specific settings
	 * --------------------------------------------------------------------
	 */
	mov	x0, #(L2_DATA_RAM_LATENCY_3_CYCLES << L2CTLR_DATA_RAM_LATENCY_SHIFT)
	msr     L2CTLR_EL1, x0
	isb
	ret
endfunc JUNO_HANDLER(1)

	/* --------------------------------------------------------------------
	 * Platform reset handler for Juno R2.
	 *
	 * Juno R2 has the following topology:
	 * - Quad core Cortex-A53 processor cluster;
	 * - Dual core Cortex-A72 processor cluster.
	 *
	 * This handler does the following:
	 * - Set the L2 Data RAM latency to 2 (i.e. 3 cycles) for Cortex-A72
	 * - Set the L2 Tag RAM latency to 1 (i.e. 2 cycles) for Cortex-A72
	 *
	 * Note that:
	 * - Defect #831273 doesn't affect Juno R2.
	 * --------------------------------------------------------------------
	 */
func JUNO_HANDLER(2)
	/* --------------------------------------------------------------------
	 * Nothing to do on Cortex-A53.
	 * --------------------------------------------------------------------
	 */
	jump_if_cpu_midr CORTEX_A72_MIDR, A72
	ret

A72:
	/* --------------------------------------------------------------------
	 * Cortex-A72 specific settings
	 * --------------------------------------------------------------------
	 */
	mov	x0, #((L2_DATA_RAM_LATENCY_3_CYCLES << L2CTLR_DATA_RAM_LATENCY_SHIFT) |	\
		      (L2_TAG_RAM_LATENCY_2_CYCLES << L2CTLR_TAG_RAM_LATENCY_SHIFT))
	msr     L2CTLR_EL1, x0
	isb
	ret
endfunc JUNO_HANDLER(2)

	/* --------------------------------------------------------------------
	 * void plat_reset_handler(void);
	 *
	 * Determine the Juno board revision and call the appropriate reset
	 * handler.
	 * --------------------------------------------------------------------
	 */
func plat_reset_handler
	/* Read the V2M SYS_ID register */
	mov_imm	x0, (V2M_SYSREGS_BASE + V2M_SYS_ID)
	ldr	w1, [x0]
	/* Extract board revision from the SYS_ID */
	ubfx	x0, x1, #V2M_SYS_ID_REV_SHIFT, #4

	JUMP_TO_HANDLER_IF_JUNO_R(0)
	JUMP_TO_HANDLER_IF_JUNO_R(1)
	JUMP_TO_HANDLER_IF_JUNO_R(2)

	/* Board revision is not supported */
	bl	plat_panic_handler

endfunc plat_reset_handler

	/* -----------------------------------------------------
	 *  unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *  Helper function to calculate the core position.
	 * -----------------------------------------------------
	 */
func plat_arm_calc_core_pos
	b	css_calc_core_pos_swap_cluster
endfunc plat_arm_calc_core_pos
