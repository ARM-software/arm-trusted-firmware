/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <wa_cve_2025_0647_cpprctx.h>

	.globl	wa_cve_2025_0647_instruction_patch
	.globl	wa_cve_2025_0647_do_cpp_wa
	.globl	wa_cve_2025_0647_execute_cpp_el3

/*
 * wa_cve_2025_0647_instruction_patch
 *
 * Function to enable EL3 traps for all CPP RCTX instruction calls from lower
 * ELs to address CVE-2025-0647.
 *
 * Argument
 *   x0 - bit[3]   flag to use T32 opcode format
 *        bit[2:0] patch slot to use
 *
 * Clobbers
 *   x0 - x3
 */
func wa_cve_2025_0647_instruction_patch
	/* Check bit 3 to see if we need T32 opcode format. */
	tbnz	x0, #WA_USE_T32_OPCODE_SHIFT, use_t32_opcode
	ldr	x2, =0x00D50B73E0
	ldr	x3, =0x00FFFFFFE0
	b	apply_patch
use_t32_opcode:
	ldr	x2, =0x00EE670DF3
	ldr	x3, =0x00FFFF0FFF

apply_patch:
	and	x1, x0, #WA_PATCH_SLOT_MASK
	msr	WA_CPUPSELR_EL3, x1
	msr	WA_CPUPOR_EL3, x2
	msr	WA_CPUPMR_EL3, x3
	ldr	x1, =0x800002001FF
	msr	WA_CPUPCR_EL3, x1
	isb

	ret
endfunc wa_cve_2025_0647_instruction_patch

/*
 * wa_cve_2025_0647_do_cpp_wa
 *
 * This function is also called by the trap handler when CPP RCTX is trapped
 * from lower ELs and also used by the EL3 API when the workaround is enabled.
 * It performs the core workaround procedure for the CPP RCTX bug. The CPP RCTX
 * instruction usually takes an argument in the form of a register but that is
 * ignored for this workaround.
 *
 * Arguments
 *   x0 - Config flags for the workaround
 *        bit[0] - indicates context is a trap handler and should ERET when done
 *        bit[1] - perform ls rcg alwayson workaround
 *
 * Clobbers
 *   x0 - x5
 *
 * Register Purposes
 *   x0 - Config flags
 *   x1 - Backup SCR_EL3
 *   x2 - Backup CPUACTLR2
 *   x3 - Backup CPUACTLR
 *   x4 - Backup CPUECTLR
 *   x5 - Scratch register
 */
func wa_cve_2025_0647_do_cpp_wa
	/* give the assembler access to the cpp rctx instruction */
	.arch_extension predres
	psb_csync
	tsb_csync
	dsb	osh

	/* Stash SCR_EL3 so we can restore it later. */
	mrs	x1, SCR_EL3

	/*
	 * There is an issue on some cores where disabling hardware prefetch can
	 * result in a deadlock, setting this bit enables LS RCG AlwaysOn which
	 * will prevent this issue, at the expense of increased power consumption
	 * for the duration of this handler.
	 */
	tbz	x0, #WA_LS_RCG_EN_BIT, skip_ls_rcg_alwayson_enable
	mrs	x2, WA_CPUACTLR2_EL1
	orr	x5, x2, #BIT(29)
	msr	WA_CPUACTLR2_EL1, x5
	isb
skip_ls_rcg_alwayson_enable:

	/* Disable branch prediction and stash CPUACTLR_EL1 in x3. */
	mrs	x3, WA_CPUACTLR_EL1
	orr	x5, x3, #BIT(0)
	msr	WA_CPUACTLR_EL1, x5

	/* Disable hardware prefetch and stash CPUECTLR_EL1 in x4. */
	mrs	x4, WA_CPUECTLR_EL1
	orr	x5, x4, #BIT(15)
	msr	WA_CPUECTLR_EL1, x5

	isb

	/*
	 * Execute CPP instruction for EL3 / root state
	 * EL3_rt: 0x000100000b010000 {GVMID,NSE,NS,EL,GASID} = {1,1,0,11,1}
	 */
	movz	x5, #0x0001, LSL #48
	movk	x5, #0x0B01, LSL #16
	cpp	rctx, x5

#if ENABLE_RME
	/*
	 * Execute CPP instructions for realm state
	 * RL-EL2: 0x000100000e010000 {GVMID,NSE,NS,EL,GASID} = {1,1,1,10,1}
	 */
	movk	x5, #0x0E01, LSL #16
	cpp	rctx, x5

	/* RL-EL1: 0x000100000d010000 {GVMID,NSE,NS,EL,GASID} = {1,1,1,01,1} */
	movk	x5, #0x0D01, LSL #16
	cpp	rctx, x5

	/* RL-EL0: 0x000100000c010000 {GVMID,NSE,NS,EL,GASID} = {1,1,1,00,1} */
	movk	x5, #0x0C01, LSL #16
	cpp	rctx, x5
#endif /* ENABLE_RME */

	/*
	 * Execute CPP instructions for non-secure state
	 * EL2_ns: 0x0001000006010000 {GVMID,NSE,NS,EL,GASID} = {1,0,1,10,1}
	 */
	movk	x5, #0x0601, LSL #16
	cpp	rctx, x5

	/* NS-EL1: 0x0001000005010000 {GVMID,NSE,NS,EL,GASID} = {1,0,1,01,1} */
	movk	x5, #0x0501, LSL #16
	cpp	rctx, x5

	/* NS-EL0: 0x0001000004010000 {GVMID,NSE,NS,EL,GASID} = {1,0,1,00,1} */
	movk	x5, #0x0401, LSL #16
	cpp	rctx, x5

	/*
	 * Execute CPP instructions for secure state
	 * EL1_s: 0x0001000001010000 {GVMID,NSE,NS,EL,GASID} = {1,0,0,01,1}
	 */
	movk	x5, #0x0101, LSL #16
	cpp	rctx, x5

	/* S-EL0: 0x0001000000010000 {GVMID,NSE,NS,EL,GASID} = {1,0,0,00,1} */
	movk	x5, #0x0001, LSL #16
	cpp	rctx, x5

	/* Check secure EL2 presence */
	tbz	x1, #SCR_EEL2_SHIFT, el3_handler_skip_sel2_cpp

	/* S-EL2: 0x0001000002010000 {GVMID,NSE,NS,EL,GASID} = {1,0,0,10,1} */
	movk	x5, #0x0201, LSL #16
	cpp	rctx, x5

el3_handler_skip_sel2_cpp:
	dsb	sy

	/* EL3 / root state TLBI */
	tlbi	alle3

#if ENABLE_RME
	/* Realm state TLBI {NSE,NS} = {1,1} */
	orr	x5, x1, #SCR_NS_BIT
	orr	x5, x5, #SCR_NSE_BIT
	msr	SCR_EL3, x5
	isb
	tlbi	alle1
	tlbi	alle2
#endif /* ENABLE_RME */

	/* Non-secure state TLBI {NSE,NS} = {0,1} */
	orr	x5, x1, #SCR_NS_BIT
	bic	x5, x5, #SCR_NSE_BIT
	msr	SCR_EL3, x5
	isb
	tlbi	alle1
	tlbi	alle2

	/* Secure state TLBI {NSE,NS} = {0,0} */
	bic	x5, x5, #SCR_NS_BIT
	msr	SCR_EL3, x5
	isb
	tlbi	alle1

	/* Check if we need to invalidate for S-EL2. */
	tbz	x1, #SCR_EEL2_SHIFT, el3_handler_skip_sel2_tlbi
	tlbi	alle2

el3_handler_skip_sel2_tlbi:
	/* Clean up and restore register values. */
	dsb	sy
	msr	SCR_EL3, x1

	/* Restore ECTLR and ACTLR values. */
	msr	WA_CPUACTLR_EL1, x3
	msr	WA_CPUECTLR_EL1, x4

	isb

	/* Restore ACTLR2 if needed. */
	tbz	x0, #WA_LS_RCG_EN_BIT, skip_ls_rcg_alwayson_disable
	msr	WA_CPUACTLR2_EL1, x2
	isb
skip_ls_rcg_alwayson_disable:

	/* Skip ERET if this is not an exception handler call. */
	tbz	x0, #WA_IS_TRAP_HANDLER_BIT, skip_eret

	/*
	 * Update ELR_EL3 to skip the triggering instruction
	 */
	mrs	x5, ELR_EL3
	add	x5, x5, #4
	msr	ELR_EL3, x5

	/* Restore context and ERET */
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	ldp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	exception_return

skip_eret:
	ret
endfunc wa_cve_2025_0647_do_cpp_wa

/*
 * wa_cve_2025_0647_execute_cpp_el3
 *
 * If a CPP RCTX instruction is needed in EL3 firmware this API can be used. It
 * performs the workaround steps without requiring a trap or exception handling
 * overhead and simplifies the code since we do not generally support nested
 * exceptions in EL3 outside of specific circumstances.
 *
 * Arguments
 *   x0 - CPP RCTX argument to use when the workaround is not needed, this
 *        argument is ignored on systems with the workaround enabled since
 *        the workaround procedure does not use the argument and does CPP RCTX
 *        for all contexts. This is here for compatibility in multi-core
 *        systems where some cores might need this workaround and others do not.
 *
 * Clobbers
 *   x0 - x7
 */
func wa_cve_2025_0647_execute_cpp_el3
	mov	x7, x0
	mov	x6, lr

	/* Get the CPU ops so we can access the trap handler. */
	bl	get_cpu_ops_ptr
	mov	lr, x6
	ldr	x0, [x0, #CPU_E_HANDLER_FUNC]

	/* If no handler exists, skip the workaround as its not enabled. */
	cbz	x0, skip_wa

	/*
	 * The EL3 handler expects x1 to contain EC=0x1F when handling a trap,
	 * so clear x1 so it knows it came from this API instead.
	 */
	mov	x1, #0

	br	x0

skip_wa:
	cpp	rctx, x7
	ret
endfunc wa_cve_2025_0647_execute_cpp_el3
