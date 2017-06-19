/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
	cmp	r0, #\_revision
	beq	\_handler
	.endm

	/* --------------------------------------------------------------------
	 * Helper macro that reads the part number of the current CPU and jumps
	 * to the given label if it matches the CPU MIDR provided.
	 *
	 * Clobbers r0.
	 * --------------------------------------------------------------------
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label
	ldcopr	r0, MIDR
	ubfx	r0, r0, #MIDR_PN_SHIFT, #12
	ldr	r1, =((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	cmp	r0, r1
	beq	\_label
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
	mov	r0, #(0xf << EVNTI_SHIFT)
	orr	r0, r0, #EVNTEN_BIT
	stcopr	r0, CNTKCTL

	/* --------------------------------------------------------------------
	 * Nothing else to do on Cortex-A53.
	 * --------------------------------------------------------------------
	 */
	jump_if_cpu_midr CORTEX_A53_MIDR, 1f

	/* --------------------------------------------------------------------
	 * Cortex-A57 specific settings
	 * --------------------------------------------------------------------
	 */
	mov	r0, #((CORTEX_A57_L2_DATA_RAM_LATENCY_3_CYCLES << CORTEX_A57_L2CTLR_DATA_RAM_LATENCY_SHIFT) |	\
		      (CORTEX_A57_L2_TAG_RAM_LATENCY_3_CYCLES << CORTEX_A57_L2CTLR_TAG_RAM_LATENCY_SHIFT))
	stcopr	r0, CORTEX_A57_L2CTLR
1:
	isb
	bx	lr
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
	bx	lr

A57:
	/* --------------------------------------------------------------------
	 * Cortex-A57 specific settings
	 * --------------------------------------------------------------------
	 */
	mov	r0, #(CORTEX_A57_L2_DATA_RAM_LATENCY_3_CYCLES << CORTEX_A57_L2CTLR_DATA_RAM_LATENCY_SHIFT)
	stcopr	r0, CORTEX_A57_L2CTLR
	isb
	bx	lr
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
	bx	lr

A72:
	/* --------------------------------------------------------------------
	 * Cortex-A72 specific settings
	 * --------------------------------------------------------------------
	 */
	mov	r0, #((CORTEX_A72_L2_DATA_RAM_LATENCY_3_CYCLES << CORTEX_A72_L2CTLR_DATA_RAM_LATENCY_SHIFT) |	\
		      (CORTEX_A72_L2_TAG_RAM_LATENCY_2_CYCLES << CORTEX_A72_L2CTLR_TAG_RAM_LATENCY_SHIFT))
	stcopr	r0, CORTEX_A72_L2CTLR
	isb
	bx	lr
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
	ldr	r0, =(V2M_SYSREGS_BASE + V2M_SYS_ID)
	ldr	r1, [r0]
	/* Extract board revision from the SYS_ID */
	ubfx	r0, r1, #V2M_SYS_ID_REV_SHIFT, #4

	JUMP_TO_HANDLER_IF_JUNO_R(0)
	JUMP_TO_HANDLER_IF_JUNO_R(1)
	JUMP_TO_HANDLER_IF_JUNO_R(2)

	/* Board revision is not supported */
	no_ret	plat_panic_handler

endfunc plat_reset_handler

	/* -----------------------------------------------------
	 *  unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *  Helper function to calculate the core position.
	 * -----------------------------------------------------
	 */
func plat_arm_calc_core_pos
	b	css_calc_core_pos_swap_cluster
endfunc plat_arm_calc_core_pos
