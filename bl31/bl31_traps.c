/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Dispatch synchronous system register traps from lower ELs.
 */

#include <arch_features.h>
#include <arch_helpers.h>
#include <bl31/sync_handle.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/idte3.h>

static int access_raz_wi(bool is_read, uint8_t rt, cpu_context_t *ctx)
{
	if (is_read && rt != XZR_REG_NUM) {
		ctx->gpregs_ctx.ctx_regs[rt] = 0UL;
	}
	return TRAP_RET_CONTINUE;
}

int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx, u_register_t flags)
{
	uint64_t opcode = EXTRACT(ESR_ISS, esr_el3) & ~(MASK(ISS_SYS64_DIR) | MASK(ISS_SYS64_RT));
	uint8_t rt = EXTRACT(ISS_SYS64_RT, esr_el3);
	bool is_read = EXTRACT(ISS_SYS64_DIR, opcode);

	if (is_feat_idte3_supported() &&
	    ((opcode >= ISS_SYSREG_OPCODE_IDREG_MIN &&
	      opcode <= ISS_SYSREG_OPCODE_IDREG_MAX) ||
	      opcode == ISS_SYSREG_OPCODE_GMID_EL1)) {
		return handle_idreg_trap(rt, opcode, ctx, flags);
	}

	if (is_feat_rng_trap_supported() &&
	    (opcode == ISS_SYSREG_OPCODE_RNDR ||
	     opcode == ISS_SYSREG_OPCODE_RNDRRS)) {
		el3_state_t *state = get_el3state_ctx(ctx);
		u_register_t *data;
		u_register_t new_spsr;
		int ret = TRAP_RET_CONTINUE;

		/* Only expect reads, write shouldn't be possible. */
		assert(EXTRACT(ISS_SYS64_DIR, esr_el3) != 0);

		/* Successful reads should have NZCV == 0 */
		new_spsr = read_ctx_reg(state, CTX_SPSR_EL3);
		new_spsr &= ~SPSR_NZCV;

		/*
		 * Don't generate entropy for XZR accesses as it will be unused.
		 * Report success despite the zero read. Reporting failure would
		 * also be logical but C6.1.4.1 doesn't make this clear and
		 * success is simpler for now.
		 */
		if (rt != ISS_SYSREG_RT_XZR) {
			data = &(ctx->gpregs_ctx.ctx_regs[rt]);

			ret = plat_handle_rng_trap(data, opcode == ISS_SYSREG_OPCODE_RNDRRS);
			if (ret == TRAP_RET_UNHANDLED) {
				/* Failure is signaled with 0 and NZCV = 0b0100. */
				new_spsr |= SPSR_Z_BIT;
				*data = 0;
				ret = TRAP_RET_CONTINUE;
			}

		}

		write_ctx_reg(state, CTX_SPSR_EL3, new_spsr);
		assert(ret == TRAP_RET_CONTINUE);

		return ret;
	}

	if (is_feat_ras_supported() && !FAULT_INJECTION_SUPPORT &&
	    (opcode == ISS_SYSREG_OPCODE_ERXPFGCDN_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXPFGCTL_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXPFGF_EL1)) {
		return access_raz_wi(is_read, rt, ctx);
	}

	if (is_feat_ras_supported() && RAS_TRAP_NS_ERR_REC_ACCESS &&
	    (opcode == ISS_SYSREG_OPCODE_ERRSELR_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXADDR_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXCTLR_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXMISC0_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXMISC1_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXSTATUS_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERRIDR_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXFR_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXMISC2_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXMISC3_EL1 ||
	     opcode == ISS_SYSREG_OPCODE_ERXGSR_EL1)) {
		return access_raz_wi(is_read, rt, ctx);
	}

#if IMPDEF_SYSREG_TRAP
	/* isolate selected bits and check they are all set */
	if (opcode & ISS_SYSREG_OPCODE_IMPDEF_MASK == ISS_SYSREG_OPCODE_IMPDEF_MASK) {
		return plat_handle_impdef_trap(esr_el3, ctx);
	}
#endif

	return TRAP_RET_UNHANDLED;
}

static bool is_tge_enabled(void)
{
	u_register_t hcr_el2 = read_hcr_el2();

	return ((is_feat_vhe_present()) && ((hcr_el2 & HCR_TGE_BIT) != 0U));
}

/*
 * This function is to ensure that undef injection does not happen into
 * non-existent S-EL2. This could happen when trap happens from S-EL{1,0}
 * and non-secure world is running with TGE bit set, considering EL3 does
 * not save/restore EL2 registers if only one world has EL2 enabled.
 * So reading hcr_el2.TGE would give NS world value.
 */
static bool is_secure_trap_without_sel2(u_register_t scr)
{
	return ((scr & (SCR_NS_BIT | SCR_EEL2_BIT)) == 0);
}

static unsigned int target_el(unsigned int from_el, u_register_t scr)
{
	if (from_el > MODE_EL1) {
		return from_el;
	} else if (is_tge_enabled() && !is_secure_trap_without_sel2(scr)) {
		return MODE_EL2;
	} else {
		return MODE_EL1;
	}
}

static u_register_t get_elr_el3(u_register_t spsr_el3, u_register_t vbar, unsigned int target_el)
{
	unsigned int outgoing_el = GET_EL(spsr_el3);
	u_register_t elr_el3 = 0;

	if (outgoing_el == target_el) {
		/*
		 * Target EL is either EL1 or EL2, lsb can tell us the SPsel
		 *  Thread mode  : 0
		 *  Handler mode : 1
		 */
		if ((spsr_el3 & (MODE_SP_MASK << MODE_SP_SHIFT)) == MODE_SP_ELX) {
			elr_el3 = vbar + CURRENT_EL_SPX;
		} else {
			elr_el3 = vbar + CURRENT_EL_SP0;
		}
	} else {
		/* Vector address for Lower EL using Aarch64 */
		elr_el3 = vbar + LOWER_EL_AARCH64;
	}

	return elr_el3;
}

/*******************************************************************************
 * HARDWARE PSEUDOCODE
 *
 * The code here is based on pseudocode described in the Arm ARM (DDI0487). They
 * can also be found in the Arm A-profile A64 Instruction Set Architecture
 * document (DDI0602) available at
 * https://developer.arm.com/documentation/ddi0602/latest/.
 *
 * NOTE: This piece of code must be reviewed every release against the latest
 * sequence to ensure that we keep up with new arch features.
 *
 * The pseudocode is based on revision 2026-03 (replace `latest` in the URL).
 *
 * Next review: TF-A 2.16 release
 *
 * FEAT_NV3 has an impact but is not implemented in EL3 yet.
 * TODO: this should create a BRBE exception record
 ******************************************************************************/

/*
 * Explicitly create all bits of SPSR to get PSTATE at exception return. Based
 * on aarch64.exceptions.takeexception.AArch64_TakeException.
 */
u_register_t create_spsr(u_register_t old_spsr, unsigned int target_el)
{
	u_register_t new_spsr = 0;
	u_register_t sctlr;

	/* Set M bits for target EL in AArch64 mode, also get sctlr */
	if (target_el == MODE_EL2) {
		sctlr = read_sctlr_el2();
		new_spsr |= (SPSR_M_AARCH64 << SPSR_M_SHIFT) | SPSR_M_EL2H;
	} else {
		sctlr = read_sctlr_el1();
		new_spsr |= (SPSR_M_AARCH64 << SPSR_M_SHIFT) | SPSR_M_EL1H;
	}

	/* Mask all exceptions, update DAIF bits */
	new_spsr |= SPSR_DAIF_MASK << SPSR_DAIF_SHIFT;

	/* If FEAT_BTI is present, clear BTYPE bits */
	new_spsr |= old_spsr & (SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	if (is_feat_bti_present()) {
		new_spsr &= ~(SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	}

	/* If SSBS is implemented, take the value from SCTLR.DSSBS */
	new_spsr |= old_spsr & SPSR_SSBS_BIT_AARCH64;
	if (is_feat_ssbs_present()) {
		if ((sctlr & SCTLR_DSSBS_BIT) != 0U) {
			new_spsr |= SPSR_SSBS_BIT_AARCH64;
		} else {
			new_spsr &= ~SPSR_SSBS_BIT_AARCH64;
		}
	}

	/* If FEAT_NMI is implemented, ALLINT = !(SCTLR.SPINTMASK) */
	new_spsr |= old_spsr & SPSR_ALLINT_BIT_AARCH64;
	if (is_feat_nmi_present()) {
		if ((sctlr & SCTLR_SPINTMASK_BIT) != 0U) {
			new_spsr &= ~SPSR_ALLINT_BIT_AARCH64;
		} else {
			new_spsr |= SPSR_ALLINT_BIT_AARCH64;
		}
	}

	/* Clear PSTATE.IL bit explicitly */
	new_spsr &= ~SPSR_IL_BIT;

	/* Clear PSTATE.SS bit explicitly */
	new_spsr &= ~SPSR_SS_BIT;

	/* Update PSTATE.PAN bit */
	new_spsr |= old_spsr & SPSR_PAN_BIT;
	if (is_feat_pan_present() &&
	    ((target_el == MODE_EL1) || ((target_el == MODE_EL2) && is_tge_enabled())) &&
	    ((sctlr & SCTLR_SPAN_BIT) == 0U)) {
	    new_spsr |= SPSR_PAN_BIT;
	}

	/* Clear UAO bit if FEAT_UAO is present */
	new_spsr |= old_spsr & SPSR_UAO_BIT_AARCH64;
	if (is_feat_uao_present()) {
		new_spsr &= ~SPSR_UAO_BIT_AARCH64;
	}

	/* DIT bits are unchanged */
	new_spsr |= old_spsr & SPSR_DIT_BIT;

	/* If FEAT_MTE2 is implemented mask tag faults by setting TCO bit */
	new_spsr |= old_spsr & SPSR_TCO_BIT_AARCH64;
	if (is_feat_mte2_present()) {
		new_spsr |= SPSR_TCO_BIT_AARCH64;
	}

	/* NZCV bits are unchanged */
	new_spsr |= old_spsr & SPSR_NZCV;

	/* UINJ bit is unchanged */
	new_spsr |= old_spsr & SPSR_UINJ_BIT;

	/* If FEAT_EBEP is present set PM bit */
	new_spsr |= old_spsr & SPSR_PM_BIT_AARCH64;
	if (is_feat_ebep_present()) {
		new_spsr |= SPSR_PM_BIT_AARCH64;
	}

	/* If FEAT_SEBEP is present clear PPEND bit */
	new_spsr |= old_spsr & SPSR_PPEND_BIT;
	if (is_feat_sebep_present()) {
		new_spsr &= ~SPSR_PPEND_BIT;
	}

	/* If FEAT_GCS is present, update EXLOCK bit */
	new_spsr |= old_spsr & SPSR_EXLOCK_BIT_AARCH64;
	if (is_feat_gcs_present()) {
		u_register_t gcscr;
		if (target_el == MODE_EL2) {
			gcscr = read_gcscr_el2();
		} else {
			gcscr = read_gcscr_el1();
		}
		new_spsr |= (gcscr & GCSCR_EXLOCK_EN_BIT) ? SPSR_EXLOCK_BIT_AARCH64 : 0;
	}

	/* If FEAT_PAUTH_LR present then zero the PACM bit. */
	new_spsr |= old_spsr & SPSR_PACM_BIT_AARCH64;
	if (is_feat_pauth_lr_present()) {
		new_spsr &= ~SPSR_PACM_BIT_AARCH64;
	}

	return new_spsr;
}

/*******************************************************************************
 * END OF HARDWARE PSEUDOCODE
 ******************************************************************************/

/*
 * Handler for injecting Undefined exception to lower EL which is caused by
 * lower EL accessing system registers of which (old)EL3 firmware is unaware.
 *
 * This is a safety net to avoid EL3 panics caused by system register access
 * that triggers an exception syndrome EC=0x18.
 */
void inject_undef64(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t old_spsr = read_ctx_reg(state, CTX_SPSR_EL3);
	u_register_t scr_el3 = 0U;
	unsigned int to_el = 0U;
	u_register_t esr = 0U;
	u_register_t elr_el3 = 0U;
	u_register_t new_spsr = 0U;

	if (is_feat_uinj_supported()) {
		new_spsr = old_spsr | SPSR_UINJ_BIT;
		write_ctx_reg(state, CTX_SPSR_EL3, new_spsr);
		return;
	}

	scr_el3 = read_ctx_reg(state, CTX_SCR_EL3);
	to_el = target_el(GET_EL(old_spsr), scr_el3);
	esr = (EC_UNKNOWN << ESR_EC_SHIFT) | ESR_IL_BIT;
	elr_el3 = read_ctx_reg(state, CTX_ELR_EL3);

	if (to_el == MODE_EL2) {
		write_elr_el2(elr_el3);
		elr_el3 = get_elr_el3(old_spsr, read_vbar_el2(), to_el);
		write_esr_el2(esr);
		write_spsr_el2(old_spsr);
	} else {
		write_elr_el1(elr_el3);
		elr_el3 = get_elr_el3(old_spsr, read_vbar_el1(), to_el);
		write_esr_el1(esr);
		write_spsr_el1(old_spsr);
	}

	new_spsr = create_spsr(old_spsr, to_el);

	write_ctx_reg(state, CTX_SPSR_EL3, new_spsr);
	write_ctx_reg(state, CTX_ELR_EL3, elr_el3);
}
