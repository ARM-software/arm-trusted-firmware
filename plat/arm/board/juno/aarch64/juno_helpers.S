/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cortex_a53.h>
#include <cortex_a57.h>
#include <cortex_a72.h>
#include <cpu_macros.S>
#include <css_def.h>
#include <v2m_def.h>
#include "../juno_def.h"


	.globl	plat_reset_handler
	.globl	plat_arm_calc_core_pos
#if JUNO_AARCH32_EL3_RUNTIME
	.globl	plat_get_my_entrypoint
	.globl	juno_reset_to_aarch32_state
#endif

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
	mov	x0, #((CORTEX_A57_L2_DATA_RAM_LATENCY_3_CYCLES << CORTEX_A57_L2CTLR_DATA_RAM_LATENCY_SHIFT) |	\
		      (CORTEX_A57_L2_TAG_RAM_LATENCY_3_CYCLES << CORTEX_A57_L2CTLR_TAG_RAM_LATENCY_SHIFT))
	msr     CORTEX_A57_L2CTLR_EL1, x0
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
	mov	x0, #(CORTEX_A57_L2_DATA_RAM_LATENCY_3_CYCLES << CORTEX_A57_L2CTLR_DATA_RAM_LATENCY_SHIFT)
	msr     CORTEX_A57_L2CTLR_EL1, x0
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
	mov	x0, #((CORTEX_A72_L2_DATA_RAM_LATENCY_3_CYCLES << CORTEX_A72_L2CTLR_DATA_RAM_LATENCY_SHIFT) |	\
		      (CORTEX_A72_L2_TAG_RAM_LATENCY_2_CYCLES << CORTEX_A72_L2CTLR_TAG_RAM_LATENCY_SHIFT))
	msr     CORTEX_A57_L2CTLR_EL1, x0
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
	no_ret	plat_panic_handler

endfunc plat_reset_handler

	/* -----------------------------------------------------
	 *  void juno_do_reset_to_aarch32_state(void);
	 *
	 *  Request warm reset to AArch32 mode.
	 * -----------------------------------------------------
	 */
func juno_do_reset_to_aarch32_state
	mov	x0, #RMR_EL3_RR_BIT
	dsb	sy
	msr	rmr_el3, x0
	isb
	wfi
	b	plat_panic_handler
endfunc juno_do_reset_to_aarch32_state

	/* -----------------------------------------------------
	 *  unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *  Helper function to calculate the core position.
	 * -----------------------------------------------------
	 */
func plat_arm_calc_core_pos
	b	css_calc_core_pos_swap_cluster
endfunc plat_arm_calc_core_pos

#if JUNO_AARCH32_EL3_RUNTIME
	/* ---------------------------------------------------------------------
	 * uintptr_t plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and a warm
	 * boot. On JUNO platform, this distinction is based on the contents of
	 * the Trusted Mailbox. It is initialised to zero by the SCP before the
	 * AP cores are released from reset. Therefore, a zero mailbox means
	 * it's a cold reset. If it is a warm boot then a request to reset to
	 * AArch32 state is issued. This is the only way to reset to AArch32
	 * in EL3 on Juno. A trampoline located at the high vector address
	 * has already been prepared by BL1.
	 *
	 * This functions returns the contents of the mailbox, i.e.:
	 *  - 0 for a cold boot;
	 *  - request warm reset in AArch32 state for warm boot case;
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	mov_imm	x0, PLAT_ARM_TRUSTED_MAILBOX_BASE
	ldr	x0, [x0]
	cbz	x0, return
	b	juno_do_reset_to_aarch32_state
return:
	ret
endfunc plat_get_my_entrypoint

/*
 * Emit a "movw r0, #imm16" which moves the lower
 * 16 bits of `_val` into r0.
 */
.macro emit_movw _reg_d, _val
	mov_imm	\_reg_d, (0xe3000000 | \
			((\_val & 0xfff) | \
			((\_val & 0xf000) << 4)))
.endm

/*
 * Emit a "movt r0, #imm16" which moves the upper
 * 16 bits of `_val` into r0.
 */
.macro emit_movt _reg_d, _val
	mov_imm	\_reg_d, (0xe3400000 | \
			(((\_val & 0x0fff0000) >> 16) | \
			((\_val & 0xf0000000) >> 12)))
.endm

/*
 * This function writes the trampoline code at HI-VEC (0xFFFF0000)
 * address which loads r0 with the entrypoint address for
 * BL32 (a.k.a SP_MIN) when EL3 is in AArch32 mode. A warm reset
 * to AArch32 mode is then requested by writing into RMR_EL3.
 */
func juno_reset_to_aarch32_state
	/*
	 * Invalidate all caches before the warm reset to AArch32 state.
	 * This is required on the Juno AArch32 boot flow because the L2
	 * unified cache may contain code and data from when the processor
	 * was still executing in AArch64 state.  This code only runs on
	 * the primary core, all other cores are powered down.
	 */
	mov	x0, #DCISW
	bl	dcsw_op_all

	emit_movw	w0, BL32_BASE
	emit_movt	w1, BL32_BASE
	/* opcode "bx r0" to branch using r0 in AArch32 mode */
	mov_imm	w2, 0xe12fff10

	/* Write the above opcodes at HI-VECTOR location */
	mov_imm	x3, HI_VECTOR_BASE
	str	w0, [x3], #4
	str	w1, [x3], #4
	str	w2, [x3]

	b	juno_do_reset_to_aarch32_state
endfunc juno_reset_to_aarch32_state

#endif /* JUNO_AARCH32_EL3_RUNTIME */
