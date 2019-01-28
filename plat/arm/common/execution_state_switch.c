/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <string.h>

#include <arch_helpers.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/psci/psci.h>
#include <lib/utils.h>
#include <plat/arm/common/arm_sip_svc.h>
#include <plat/arm/common/plat_arm.h>
#include <smccc_helpers.h>

/*
 * Handle SMC from a lower exception level to switch its execution state
 * (either from AArch64 to AArch32, or vice versa).
 *
 * smc_fid:
 *	SMC function ID - either ARM_SIP_SVC_STATE_SWITCH_64 or
 *	ARM_SIP_SVC_STATE_SWITCH_32.
 * pc_hi, pc_lo:
 *	PC upon re-entry to the calling exception level; width dependent on the
 *	calling exception level.
 * cookie_hi, cookie_lo:
 *	Opaque pointer pairs received from the caller to pass it back, upon
 *	re-entry.
 * handle:
 *	Handle to saved context.
 */
int arm_execution_state_switch(unsigned int smc_fid,
		uint32_t pc_hi,
		uint32_t pc_lo,
		uint32_t cookie_hi,
		uint32_t cookie_lo,
		void *handle)
{
	/* Execution state can be switched only if EL3 is AArch64 */
#ifdef AARCH64
	bool caller_64, thumb = false, from_el2;
	unsigned int el, endianness;
	u_register_t spsr, pc, scr, sctlr;
	entry_point_info_t ep;
	cpu_context_t *ctx = (cpu_context_t *) handle;
	el3_state_t *el3_ctx = get_el3state_ctx(ctx);

	/* That the SMC originated from NS is already validated by the caller */

	/*
	 * Disallow state switch if any of the secondaries have been brought up.
	 */
	if (psci_secondaries_brought_up() != 0)
		goto exec_denied;

	spsr = read_ctx_reg(el3_ctx, CTX_SPSR_EL3);
	caller_64 = (GET_RW(spsr) == MODE_RW_64);

	if (caller_64) {
		/*
		 * If the call originated from AArch64, expect 32-bit pointers when
		 * switching to AArch32.
		 */
		if ((pc_hi != 0U) || (cookie_hi != 0U))
			goto invalid_param;

		pc = pc_lo;

		/* Instruction state when entering AArch32 */
		thumb = (pc & 1U) != 0U;
	} else {
		/* Construct AArch64 PC */
		pc = (((u_register_t) pc_hi) << 32) | pc_lo;
	}

	/* Make sure PC is 4-byte aligned, except for Thumb */
	if (((pc & 0x3U) != 0U) && !thumb)
		goto invalid_param;

	/*
	 * EL3 controls register width of the immediate lower EL only. Expect
	 * this request from EL2/Hyp unless:
	 *
	 * - EL2 is not implemented;
	 * - EL2 is implemented, but was disabled. This can be inferred from
	 *   SCR_EL3.HCE.
	 */
	from_el2 = caller_64 ? (GET_EL(spsr) == MODE_EL2) :
		(GET_M32(spsr) == MODE32_hyp);
	scr = read_ctx_reg(el3_ctx, CTX_SCR_EL3);
	if (!from_el2) {
		/* The call is from NS privilege level other than HYP */

		/*
		 * Disallow switching state if there's a Hypervisor in place;
		 * this request must be taken up with the Hypervisor instead.
		 */
		if ((scr & SCR_HCE_BIT) != 0U)
			goto exec_denied;
	}

	/*
	 * Return to the caller using the same endianness. Extract
	 * endianness bit from the respective system control register
	 * directly.
	 */
	sctlr = from_el2 ? read_sctlr_el2() : read_sctlr_el1();
	endianness = ((sctlr & SCTLR_EE_BIT) != 0U) ? 1U : 0U;

	/* Construct SPSR for the exception state we're about to switch to */
	if (caller_64) {
		unsigned long long impl;

		/*
		 * Switching from AArch64 to AArch32. Ensure this CPU implements
		 * the target EL in AArch32.
		 */
		impl = from_el2 ? el_implemented(2) : el_implemented(1);
		if (impl != EL_IMPL_A64_A32)
			goto exec_denied;

		/* Return to the equivalent AArch32 privilege level */
		el = from_el2 ? MODE32_hyp : MODE32_svc;
		spsr = SPSR_MODE32((u_register_t) el,
				thumb ? SPSR_T_THUMB : SPSR_T_ARM,
				endianness, DISABLE_ALL_EXCEPTIONS);
	} else {
		/*
		 * Switching from AArch32 to AArch64. Since it's not possible to
		 * implement an EL as AArch32-only (from which this call was
		 * raised), it's safe to assume AArch64 is also implemented.
		 */
		el = from_el2 ? MODE_EL2 : MODE_EL1;
		spsr = SPSR_64((u_register_t) el, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);
	}

	/*
	 * Use the context management library to re-initialize the existing
	 * context with the execution state flipped. Since the library takes
	 * entry_point_info_t pointer as the argument, construct a dummy one
	 * with PC, state width, endianness, security etc. appropriately set.
	 * Other entries in the entry point structure are irrelevant for
	 * purpose.
	 */
	zeromem(&ep, sizeof(ep));
	ep.pc = pc;
	ep.spsr = (uint32_t) spsr;
	SET_PARAM_HEAD(&ep, PARAM_EP, VERSION_1,
			((unsigned int) ((endianness != 0U) ? EP_EE_BIG :
				EP_EE_LITTLE)
			 | NON_SECURE | EP_ST_DISABLE));

	/*
	 * Re-initialize the system register context, and exit EL3 as if for the
	 * first time. State switch is effectively a soft reset of the
	 * calling EL.
	 */
	cm_init_my_context(&ep);
	cm_prepare_el3_exit(NON_SECURE);

	/*
	 * State switch success. The caller of SMC wouldn't see the SMC
	 * returning. Instead, execution starts at the supplied entry point,
	 * with context pointers populated in registers 0 and 1.
	 */
	SMC_RET2(handle, cookie_hi, cookie_lo);

invalid_param:
	SMC_RET1(handle, STATE_SW_E_PARAM);

exec_denied:
#endif
	/* State switch denied */
	SMC_RET1(handle, STATE_SW_E_DENIED);
}
