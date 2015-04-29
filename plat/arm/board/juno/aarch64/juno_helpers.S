/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
#include <cortex_a57.h>
#include <v2m_def.h>
#include "../juno_def.h"


	.globl	plat_reset_handler


	/* --------------------------------------------------------------------
	 * void plat_reset_handler(void);
	 *
	 * Before adding code in this function, refer to the guidelines in
	 * docs/firmware-design.md to determine whether the code should reside
	 * within the FIRST_RESET_HANDLER_CALL block or not.
	 *
	 * For Juno r0:
	 * - Implement workaround for defect id 831273 by enabling an event
	 *   stream every 65536 cycles.
	 * - Set the L2 Data RAM latency to 2 (i.e. 3 cycles) for Cortex-A57
	 * - Set the L2 Tag RAM latency to 2 (i.e. 3 cycles) for Cortex-A57
	 *
	 * For Juno r1:
	 * - Set the L2 Data RAM latency to 2 (i.e. 3 cycles) for Cortex-A57
	 * Note that:
	 * - The default value for the L2 Tag RAM latency for Cortex-A57 is
	 *   suitable.
	 * - Defect #831273 doesn't affect Juno r1.
	 *
	 * This code is included only when FIRST_RESET_HANDLER_CALL is defined
	 * since it should be executed only during BL1.
	 * --------------------------------------------------------------------
	 */
func plat_reset_handler
#ifdef FIRST_RESET_HANDLER_CALL
	/* --------------------------------------------------------------------
	 * Determine whether this code is running on Juno r0 or Juno r1.
	 * Keep this information in x2.
	 * --------------------------------------------------------------------
	 */
	/* Read the V2M SYS_ID register */
	mov_imm	x0, (V2M_SYSREGS_BASE + V2M_SYS_ID)
	ldr	w1, [x0]
	/* Extract board revision from the SYS_ID */
	ubfx	x1, x1, #V2M_SYS_ID_REV_SHIFT, #4
	/*
	 * On Juno R0:  x2 := REV_JUNO_R0 - 1 = 0
	 * On Juno R1:  x2 := REV_JUNO_R1 - 1 = 1
	 */
	sub	x2, x1, #1

	/* --------------------------------------------------------------------
	 * Determine whether this code is executed on a Cortex-A53 or on a
	 * Cortex-A57 core.
	 * --------------------------------------------------------------------
	 */
	mrs	x0, midr_el1
	ubfx	x1, x0, MIDR_PN_SHIFT, #12
	cmp     w1, #((CORTEX_A57_MIDR >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	A57

	/* Nothing needs to be done for the Cortex-A53 on Juno r1 */
	cbz	x2, apply_831273
	ret

A57:
	/* --------------------------------------------------------------------
	 * Cortex-A57 specific settings
	 * --------------------------------------------------------------------
	 */

	/* Change the L2 Data RAM latency to 3 cycles */
	mov	x0, #L2_DATA_RAM_LATENCY_3_CYCLES
	cbnz	x2, apply_l2_ram_latencies
	/* On Juno r0, also change the L2 Tag RAM latency to 3 cycles */
	orr	x0, x0, #(L2_TAG_RAM_LATENCY_3_CYCLES << 		\
				L2CTLR_TAG_RAM_LATENCY_SHIFT)
apply_l2_ram_latencies:
	msr     L2CTLR_EL1, x0

	/* Juno r1 doesn't suffer from defect #831273 */
	cbnz	x2, ret

apply_831273:
	/* --------------------------------------------------------------------
	 * On Juno r0, enable the event stream every 65536 cycles
	 * --------------------------------------------------------------------
	 */
	mov     x0, #(0xf << EVNTI_SHIFT)
	orr     x0, x0, #EVNTEN_BIT
	msr     CNTKCTL_EL1, x0
ret:
	isb
#endif /* FIRST_RESET_HANDLER_CALL */
	ret
endfunc plat_reset_handler
