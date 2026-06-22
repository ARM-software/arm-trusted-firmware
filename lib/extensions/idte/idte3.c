/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
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

#define _CHECK_FEATURE_MASKING(guard, worlds)					\
	(guard == FEAT_STATE_DISABLED || (((worlds) >> security_state) & 1U) == 0U)

/*
 * Downgrade the feature to one less than the minimum known version if a higher
 * version is present. This allows for features sharing an ID field to be
 * gradually disabled. Won't work with signed or non-zero disabled ID fields.
 */
#define _MASK_FEATURE(idreg, guard, field, min, worlds)				\
	do {									\
		if (_CHECK_FEATURE_MASKING(guard, worlds)) {			\
			assert(min != 0);					\
			if (EXTRACT(field, idreg) > min - 1) {			\
				UPDATE_REG_FIELD(field, idreg, min - 1);	\
			}							\
		}								\
	} while (0);

#define MASK_PERCPU_FEATURES(name, idreg, guard, field, min, max, worlds)	\
	_MASK_FEATURE(ctx->idreg, guard, field, min, worlds)

#define MASK_FEATURES(name, idreg, guard, field, min, max, worlds)		\
	_MASK_FEATURE(ctx->idreg, guard, field, min, worlds)

#define MASK_NONSTANDARD_FEATURE(name, idreg, guard, field, min, max, worlds)	\
	_CHECK_FEATURE_MASKING(guard, worlds)

void idte3_init_percpu_once_regs(size_t security_state)
{
	assert(security_state < CPU_CONTEXT_NUM);

	percpu_idregs_t * const ctx = &get_cpu_data(idregs[security_state]);

	ctx->id_aa64dfr0_el1 = read_id_aa64dfr0_el1();
	ctx->id_aa64dfr1_el1 = read_id_aa64dfr1_el1();

	CPUFEAT_PERCPU_LIST(MASK_PERCPU_FEATURES)
}

void idte3_init_cached_idregs_per_world(size_t security_state)
{

	assert(security_state < CPU_CONTEXT_NUM);

	perworld_idregs_t *ctx = &(per_world_context[security_state].idregs);

	ctx->id_aa64pfr0_el1 = read_id_aa64pfr0_el1();
	ctx->id_aa64pfr1_el1 = read_id_aa64pfr1_el1();
	ctx->id_aa64pfr2_el1 = read_id_aa64pfr2_el1();
	ctx->id_aa64smfr0_el1 = read_id_aa64smfr0_el1();
	ctx->id_aa64isar0_el1 = read_id_aa64isar0_el1();
	ctx->id_aa64isar1_el1 = read_id_aa64isar1_el1();
	ctx->id_aa64isar2_el1 = read_id_aa64isar2_el1();
	ctx->id_aa64isar3_el1 = read_id_aa64isar3_el1();
	ctx->id_aa64mmfr0_el1 = read_id_aa64mmfr0_el1();
	ctx->id_aa64mmfr1_el1 = read_id_aa64mmfr1_el1();
	ctx->id_aa64mmfr2_el1 = read_id_aa64mmfr2_el1();
	ctx->id_aa64mmfr3_el1 = read_id_aa64mmfr3_el1();
	ctx->id_aa64mmfr4_el1 = read_id_aa64mmfr4_el1();
	ctx->id_aa64dfr2_el1  = read_id_aa64dfr2_el1();

	CPUFEAT_LIST(MASK_FEATURES)

	/*
	 * Features that do not conform to the standard ID register discovery
	 * mechanism.
	 *
	 * FEAT_PAuth_LR is always accesible to lower Els, the flag only enables
	 * usage at EL3. Exclude from masking.
	 *
	 * FEAT_PAuth is always accessible to NS regardless of flags. It needs
	 * contexting to be usable from S/RL
	 */
	if (CTX_INCLUDE_PAUTH_REGS == FEAT_STATE_DISABLED &&
	    security_state != CPU_CONTEXT_NS) {
		ctx->id_aa64isar1_el1 &= ~(MASK(ID_AA64ISAR1_GPI) |
					   MASK(ID_AA64ISAR1_GPA) |
					   MASK(ID_AA64ISAR1_API) |
					   MASK(ID_AA64ISAR1_APA));
		ctx->id_aa64isar2_el1 &= ~(MASK(ID_AA64ISAR2_APA3) |
					   MASK(ID_AA64ISAR2_GPA3));
	}

	if (FEAT_MPAM(MASK_NONSTANDARD_FEATURE)) {
		ctx->id_aa64pfr0_el1 &= ~MASK(ID_AA64PFR0_MPAM);
		ctx->id_aa64pfr1_el1 &= ~MASK(ID_AA64PFR1_MPAM_FRAC);
	}
}

int handle_idreg_trap(uint8_t rt, uint64_t idreg, cpu_context_t *ctx, u_register_t flags)
{
	u_register_t value = 0ULL;
	size_t security_state = GET_SECURITY_STATE(flags);
	percpu_idregs_t *percpu_reg = &(get_cpu_data(idregs[security_state]));

	per_world_context_t *per_world_ctx =
		&per_world_context[get_cpu_context_index(security_state)];
	perworld_idregs_t *perworld_reg = &(per_world_ctx->idregs);

	switch (idreg) {
	case ISS_SYSREG_OPCODE_ID_AA64PFR0_EL1:
		value = perworld_reg->id_aa64pfr0_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64PFR1_EL1:
		value = perworld_reg->id_aa64pfr1_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64PFR2_EL1:
		value = perworld_reg->id_aa64pfr2_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64SMFR0_EL1:
		value = perworld_reg->id_aa64smfr0_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64ISAR0_EL1:
		value = perworld_reg->id_aa64isar0_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64ISAR1_EL1:
		value = perworld_reg->id_aa64isar1_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64ISAR2_EL1:
		value = perworld_reg->id_aa64isar2_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64ISAR3_EL1:
		value = perworld_reg->id_aa64isar3_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64MMFR0_EL1:
		value = perworld_reg->id_aa64mmfr0_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64MMFR1_EL1:
		value = perworld_reg->id_aa64mmfr1_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64MMFR2_EL1:
		value = perworld_reg->id_aa64mmfr2_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64MMFR3_EL1:
		value = perworld_reg->id_aa64mmfr3_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64MMFR4_EL1:
		value = perworld_reg->id_aa64mmfr4_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64DFR0_EL1:
		value = percpu_reg->id_aa64dfr0_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64DFR1_EL1:
		value = percpu_reg->id_aa64dfr1_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64ZFR0_EL1:
		value = read_id_aa64zfr0_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_AA64FPFR0_EL1:
		value = read_id_aa64fpfr0_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_AA64DFR2_EL1:
		value = perworld_reg->id_aa64dfr2_el1;
		break;
	case ISS_SYSREG_OPCODE_ID_AA64AFR0_EL1:
		value = read_id_aa64afr0_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_AA64AFR1_EL1:
		value = read_id_aa64afr1_el1();
		break;
	case ISS_SYSREG_OPCODE_GMID_EL1:
		value = read_gmid_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_PFR0_EL1:
		value = read_id_pfr0_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_PFR1_EL1:
		value = read_id_pfr1_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_DFR0_EL1:
		value = read_id_dfr0_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_AFR0_EL1:
		value = read_id_afr0_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_PFR2_EL1:
		value = read_id_pfr2_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_DFR1_EL1:
		value = read_id_dfr1_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_MMFR0_EL1:
		value = read_id_mmfr0_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_MMFR1_EL1:
		value = read_id_mmfr1_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_MMFR2_EL1:
		value = read_id_mmfr2_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_MMFR3_EL1:
		value = read_id_mmfr3_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_MMFR4_EL1:
		value = read_id_mmfr4_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_MMFR5_EL1:
		value = read_id_mmfr5_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_ISAR0_EL1:
		value = read_id_isar0_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_ISAR1_EL1:
		value = read_id_isar1_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_ISAR2_EL1:
		value = read_id_isar2_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_ISAR3_EL1:
		value = read_id_isar3_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_ISAR4_EL1:
		value = read_id_isar4_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_ISAR5_EL1:
		value = read_id_isar5_el1();
		break;
	case ISS_SYSREG_OPCODE_ID_ISAR6_EL1:
		value = read_id_isar6_el1();
		break;
	case ISS_SYSREG_OPCODE_MVFR0_EL1:
		value = read_mvfr0_el1();
		break;
	case ISS_SYSREG_OPCODE_MVFR1_EL1:
		value = read_mvfr1_el1();
		break;
	case ISS_SYSREG_OPCODE_MVFR2_EL1:
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
		     (uint8_t) EXTRACT(ISS_SYS64_OP0, idreg),
		     (uint8_t) EXTRACT(ISS_SYS64_OP1, idreg),
		     (uint8_t) EXTRACT(ISS_SYS64_CRN, idreg),
		     (uint8_t) EXTRACT(ISS_SYS64_CRM, idreg),
		     (uint8_t) EXTRACT(ISS_SYS64_OP2, idreg));
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
