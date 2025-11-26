/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/extensions/idte3.h>

void idte3_init_percpu_once_regs(size_t security_state)
{
	assert(security_state < CPU_CONTEXT_NUM);

	percpu_idregs_t * const reg =
		&get_cpu_data(idregs[security_state]);

	reg->id_aa64dfr0_el1 = read_id_aa64dfr0_el1();
	reg->id_aa64dfr1_el1 = read_id_aa64dfr1_el1();

	update_feat_spe_idreg_field(security_state);
	update_feat_brbe_idreg_field(security_state);
	update_feat_trbe_idreg_field(security_state);
	update_feat_trf_idreg_field(security_state);
	update_feat_mtpmu_idreg_field(security_state);
	update_feat_sebep_idreg_field(security_state);
	update_feat_sys_reg_trace_idreg_field(security_state);
	update_feat_debugv8p9_idreg_field(security_state);
	update_feat_ebep_idreg_field(security_state);
}

void idte3_init_cached_idregs_per_world(size_t security_state)
{

	assert(security_state < CPU_CONTEXT_NUM);

	per_world_context_t *per_world_ctx = &per_world_context[security_state];
	perworld_idregs_t *reg = &(per_world_ctx->idregs);

	reg->id_aa64pfr0_el1 = read_id_aa64pfr0_el1();
	reg->id_aa64pfr1_el1 = read_id_aa64pfr1_el1();
	reg->id_aa64pfr2_el1 = read_id_aa64pfr2_el1();
	reg->id_aa64smfr0_el1 = read_id_aa64smfr0_el1();
	reg->id_aa64isar0_el1 = read_id_aa64isar0_el1();
	reg->id_aa64isar1_el1 = read_id_aa64isar1_el1();
	reg->id_aa64isar2_el1 = read_id_aa64isar2_el1();
	reg->id_aa64isar3_el1 = read_id_aa64isar3_el1();
	reg->id_aa64mmfr0_el1 = read_id_aa64mmfr0_el1();
	reg->id_aa64mmfr1_el1 = read_id_aa64mmfr1_el1();
	reg->id_aa64mmfr2_el1 = read_id_aa64mmfr2_el1();
	reg->id_aa64mmfr3_el1 = read_id_aa64mmfr3_el1();
	reg->id_aa64mmfr4_el1 = read_id_aa64mmfr4_el1();

	update_feat_pan_idreg_field(security_state);
	update_feat_vhe_idreg_field(security_state);
	update_feat_ttcnp_idreg_field(security_state);
	update_feat_uao_idreg_field(security_state);
	update_feat_pacqarma3_idreg_field(security_state);
	update_feat_pauth_idreg_field(security_state);
	update_feat_ttst_idreg_field(security_state);
	update_feat_bti_idreg_field(security_state);
	update_feat_mte2_idreg_field(security_state);
	update_feat_ssbs_idreg_field(security_state);
	update_feat_nmi_idreg_field(security_state);
	update_feat_gcs_idreg_field(security_state);
	update_feat_ebep_idreg_field(security_state);
	update_feat_sel2_idreg_field(security_state);
	update_feat_twed_idreg_field(security_state);
	update_feat_fgt_idreg_field(security_state);
	update_feat_ecv_idreg_field(security_state);
	update_feat_rng_idreg_field(security_state);
	update_feat_tcr2_idreg_field(security_state);
	update_feat_s2poe_idreg_field(security_state);
	update_feat_s1poe_idreg_field(security_state);
	update_feat_s2pie_idreg_field(security_state);
	update_feat_s1pie_idreg_field(security_state);
	update_feat_amu_idreg_field(security_state);
	update_feat_mpam_idreg_field(security_state);
	update_feat_hcx_idreg_field(security_state);
	update_feat_rng_trap_idreg_field(security_state);
	update_feat_sb_idreg_field(security_state);
	update_feat_csv2_2_idreg_field(security_state);
	update_feat_sve_idreg_field(security_state);
	update_feat_ras_idreg_field(security_state);
	update_feat_dit_idreg_field(security_state);
	update_feat_trbe_idreg_field(security_state);
	update_feat_sme_idreg_field(security_state);
	update_feat_fgt2_idreg_field(security_state);
	update_feat_the_idreg_field(security_state);
	update_feat_sctlr2_idreg_field(security_state);
	update_feat_d128_idreg_field(security_state);
	update_feat_ls64_accdata_idreg_field(security_state);
	update_feat_fpmr_idreg_field(security_state);
	update_feat_mops_idreg_field(security_state);
	update_feat_fgwte3_idreg_field(security_state);
	update_feat_cpa2_idreg_field(security_state);
	update_feat_idte3_idreg_field(security_state);
	update_feat_uinj_idreg_field(security_state);
}

int handle_idreg_trap(uint64_t esr_el3, cpu_context_t *ctx, u_register_t flags)
{
	uint32_t iss = (uint32_t) ESR_ELx_ISS(esr_el3);
	uint8_t rt = (uint8_t) ISS_SYS64_RT(iss);
	uint8_t op0 = (uint8_t) ISS_SYS64_OP0(iss);
	uint8_t op1 = (uint8_t) ISS_SYS64_OP1(iss);
	uint8_t CRn = (uint8_t) ISS_SYS64_CRN(iss);
	uint8_t CRm = (uint8_t) ISS_SYS64_CRM(iss);
	uint8_t op2 = (uint8_t) ISS_SYS64_OP2(iss);

	u_register_t idreg = esr_el3 & ESR_EL3_SYSREG_MASK;

	u_register_t value = 0ULL;
	size_t security_state = GET_SECURITY_STATE(flags);
	percpu_idregs_t *percpu_reg = &(get_cpu_data(idregs[security_state]));

	per_world_context_t *per_world_ctx =
		&per_world_context[get_cpu_context_index(security_state)];
	perworld_idregs_t *perworld_reg = &(per_world_ctx->idregs);

	switch (idreg) {
	case ESR_EL3_IDREG_ID_AA64PFR0_EL1:
		value = perworld_reg->id_aa64pfr0_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64PFR1_EL1:
		value = perworld_reg->id_aa64pfr1_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64PFR2_EL1:
		value = perworld_reg->id_aa64pfr2_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64SMFR0_EL1:
		value = perworld_reg->id_aa64smfr0_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64ISAR0_EL1:
		value = perworld_reg->id_aa64isar0_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64ISAR1_EL1:
		value = perworld_reg->id_aa64isar1_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64ISAR2_EL1:
		value = perworld_reg->id_aa64isar2_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64ISAR3_EL1:
		value = perworld_reg->id_aa64isar3_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64MMFR0_EL1:
		value = perworld_reg->id_aa64mmfr0_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64MMFR1_EL1:
		value = perworld_reg->id_aa64mmfr1_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64MMFR2_EL1:
		value = perworld_reg->id_aa64mmfr2_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64MMFR3_EL1:
		value = perworld_reg->id_aa64mmfr3_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64MMFR4_EL1:
		value = perworld_reg->id_aa64mmfr4_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64DFR0_EL1:
		value = percpu_reg->id_aa64dfr0_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64DFR1_EL1:
		value = percpu_reg->id_aa64dfr1_el1;
		break;
	case ESR_EL3_IDREG_ID_AA64ZFR0_EL1:
		value = read_id_aa64zfr0_el1();
		break;
	case ESR_EL3_IDREG_ID_AA64FPFR0_EL1:
		value = read_id_aa64fpfr0_el1();
		break;
	case ESR_EL3_IDREG_ID_AA64DFR2_EL1:
		value = read_id_aa64dfr2_el1();
		break;
	case ESR_EL3_IDREG_ID_AA64AFR0_EL1:
		value = read_id_aa64afr0_el1();
		break;
	case ESR_EL3_IDREG_ID_AA64AFR1_EL1:
		value = read_id_aa64afr1_el1();
		break;
	case ESR_EL3_IDREG_GMID_EL1:
		value = read_gmid_el1();
		break;
	case ESR_EL3_IDREG_ID_PFR0_EL1:
		value = read_id_pfr0_el1();
		break;
	case ESR_EL3_IDREG_ID_PFR1_EL1:
		value = read_id_pfr1_el1();
		break;
	case ESR_EL3_IDREG_ID_DFR0_EL1:
		value = read_id_dfr0_el1();
		break;
	case ESR_EL3_IDREG_ID_AFR0_EL1:
		value = read_id_afr0_el1();
		break;
	case ESR_EL3_IDREG_ID_PFR2_EL1:
		value = read_id_pfr2_el1();
		break;
	case ESR_EL3_IDREG_ID_DFR1_EL1:
		value = read_id_dfr1_el1();
		break;
	case ESR_EL3_IDREG_ID_MMFR0_EL1:
		value = read_id_mmfr0_el1();
		break;
	case ESR_EL3_IDREG_ID_MMFR1_EL1:
		value = read_id_mmfr1_el1();
		break;
	case ESR_EL3_IDREG_ID_MMFR2_EL1:
		value = read_id_mmfr2_el1();
		break;
	case ESR_EL3_IDREG_ID_MMFR3_EL1:
		value = read_id_mmfr3_el1();
		break;
	case ESR_EL3_IDREG_ID_MMFR4_EL1:
		value = read_id_mmfr4_el1();
		break;
	case ESR_EL3_IDREG_ID_MMFR5_EL1:
		value = read_id_mmfr5_el1();
		break;
	case ESR_EL3_IDREG_ID_ISAR0_EL1:
		value = read_id_isar0_el1();
		break;
	case ESR_EL3_IDREG_ID_ISAR1_EL1:
		value = read_id_isar1_el1();
		break;
	case ESR_EL3_IDREG_ID_ISAR2_EL1:
		value = read_id_isar2_el1();
		break;
	case ESR_EL3_IDREG_ID_ISAR3_EL1:
		value = read_id_isar3_el1();
		break;
	case ESR_EL3_IDREG_ID_ISAR4_EL1:
		value = read_id_isar4_el1();
		break;
	case ESR_EL3_IDREG_ID_ISAR5_EL1:
		value = read_id_isar5_el1();
		break;
	case ESR_EL3_IDREG_ID_ISAR6_EL1:
		value = read_id_isar6_el1();
		break;
	case ESR_EL3_IDREG_MVFR0_EL1:
		value = read_mvfr0_el1();
		break;
	case ESR_EL3_IDREG_MVFR1_EL1:
		value = read_mvfr1_el1();
		break;
	case ESR_EL3_IDREG_MVFR2_EL1:
		value = read_mvfr2_el1();
		break;

	/*
	 * Any ID register access that falls within the Group 3
	 * ID space (op0 == 3, op1 == 0, CRn == 0, CRm == {2-7}, op2 == {0-7})
	 * but is not explicitly handled here will return 0.
	 * This covers newly introduced ID registers that were previously
	 * reserved or unknown.
	 *
	 * When new ID registers are added in future revisions of
	 * the architecture, they must be explicitly handled in this
	 * switch statement to return their actual value instead of
	 * Res0.
	 */
	default:
		WARN("Unknown ID register: S%u_%u_C%u_C%u_%u is trapped\n",
			op0, op1, CRn, CRm, op2);
		value = 0UL;
	}

	ctx->gpregs_ctx.ctx_regs[rt] = value;
	return TRAP_RET_CONTINUE;
}

void idte3_enable(cpu_context_t *context)
{
	u_register_t reg;
	el3_state_t *state;

	state = get_el3state_ctx(context);

	/*
	 * Setting the TID3 & TID5 bits enables trapping for
	 * group 3 ID registers and group 5
	 * ID register - GMID_EL1.
	 */

	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg |= (SCR_TID3_BIT | SCR_TID5_BIT);
	write_ctx_reg(state, CTX_SCR_EL3, reg);
}
