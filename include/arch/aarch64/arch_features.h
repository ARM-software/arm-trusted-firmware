/*
 * Copyright (c) 2019-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_FEATURES_H
#define ARCH_FEATURES_H

#include <stdbool.h>

#include <arch_helpers.h>
#include <common/feat_detect.h>
#include <lib/cpus/errata.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>

#if ENABLE_RMM
#define FEAT_ENABLE_ALL_WORLDS			\
	((1u << CPU_CONTEXT_SECURE)	|	\
	(1u << CPU_CONTEXT_NS)		|	\
	(1u << CPU_CONTEXT_REALM))
#define FEAT_ENABLE_REALM		(1 << CPU_CONTEXT_REALM)
#else
#define FEAT_ENABLE_ALL_WORLDS			\
	((1u << CPU_CONTEXT_SECURE)	|	\
	(1u << CPU_CONTEXT_NS))
#define FEAT_ENABLE_REALM		U(0)
#endif

#define FEAT_ENABLE_SECURE		(1 << CPU_CONTEXT_SECURE)
#define FEAT_ENABLE_NS			(1 << CPU_CONTEXT_NS)

#define SHOULD_ID_FIELD_DISABLE(guard, enabled_worlds, world)		\
	 (((guard) == 0U) || ((((enabled_worlds) >> (world)) & 1U) == 0U))


/*
 * CREATE_IDREG_UPDATE and CREATE_PERCPU_IDREG_UPDATE are two macros that
 * generate the update_feat_abc_idreg_field() function based on how its
 * corresponding ID register is cached.
 * The function disables ID register fields related to a feature if the build
 * flag for that feature is 0 or if the feature should be disabled for that
 * world. If the particular field has to be disabled, its field in the cached
 * ID register is set to 0.
 *
 * Note: For most ID register fields, a value of 0 represents
 * the Unimplemented state, and hence we use this macro to show features
 * disabled in EL3 as unimplemented to lower ELs. However, certain feature's
 * ID Register fields (like ID_AA64MMFR4_EL1.E2H0) deviate from this convention,
 * where 0 does not represent Unimplemented.
 * For those features, a custom update_feat_abc_idreg_field()
 * needs to be created. This custom function should set the field to the
 * feature's unimplemented state value if the feature is disabled in EL3.
 *
 * For example:
 *
 * __attribute__((always_inline))
 * static inline void update_feat_abc_idreg_field(size_t security_state)
 * {
 *	if (SHOULD_ID_FIELD_DISABLE(guard, enabled_worlds, security_state)) {
 *		per_world_context_t *per_world_ctx =
 *				&per_world_context[security_state];
 *		perworld_idregs_t *perworld_idregs = &(per_world_ctx->idregs);
 *
 *		perworld_idregs->idreg &= ~((u_register_t)MASK(idfield));
 *		perworld_idregs->idreg |=
 *		((u_register_t)<unimplemented state value> << idfield##_SHIFT);
 *	}
 * }
 */

#if (ENABLE_FEAT_IDTE3 && IMAGE_BL31)
#define CREATE_IDREG_UPDATE(name, idreg, idfield, guard, enabled_worlds)	\
	__attribute__((always_inline))						\
static inline void update_ ## name ## _idreg_field(size_t security_state)	\
{										\
	if (SHOULD_ID_FIELD_DISABLE(guard, enabled_worlds, security_state)) {	\
		per_world_context_t *per_world_ctx =				\
				&per_world_context[security_state];		\
		perworld_idregs_t *perworld_idregs = &(per_world_ctx->idregs);	\
		perworld_idregs->idreg &= ~((u_register_t)MASK(idfield));	\
	}									\
}
#define CREATE_PERCPU_IDREG_UPDATE(name, idreg, idfield, guard,		\
					enabled_worlds)				\
	__attribute__((always_inline))						\
static inline void update_ ## name ## _idreg_field(size_t security_state)	\
{										\
	if (SHOULD_ID_FIELD_DISABLE(guard, enabled_worlds, security_state)) {	\
		percpu_idregs_t *percpu_idregs =				\
					&(get_cpu_data(idregs[security_state]));\
		percpu_idregs->idreg &= ~((u_register_t)MASK(idfield));	\
	}									\
}
#else
#define CREATE_IDREG_UPDATE(name, idreg, idfield, guard, enabled_worlds)
#define CREATE_PERCPU_IDREG_UPDATE(name, idreg, idfield, guard,		\
					enabled_worlds)
#endif

#define _CREATE_FEATURE_PRESENT(name, idreg, idfield, idval)			\
__attribute__((always_inline))							\
static inline unsigned int read_ ## name ## _id_field(void)			\
{										\
	return (unsigned int)EXTRACT(idfield, read_ ## idreg());		\
}										\
__attribute__((always_inline))							\
static inline bool is_ ## name ## _present(void)				\
{										\
	return (read_ ## name ## _id_field() >= idval) ? true : false; 		\
}

#define CREATE_FEATURE_PRESENT(name, idreg, idfield, idval,			\
				enabled_worlds)					\
	_CREATE_FEATURE_PRESENT(name, idreg, idfield, idval)			\
	CREATE_IDREG_UPDATE(name, idreg, idfield, 1U, enabled_worlds)

#define CREATE_PERCPU_FEATURE_PRESENT(name, idreg, idfield, idval,		\
					enabled_worlds)				\
	_CREATE_FEATURE_PRESENT(name, idreg, idfield, idval)			\
	CREATE_PERCPU_IDREG_UPDATE(name, idreg, idfield, 1U,			\
					enabled_worlds)

#define CREATE_FEATURE_FUNCS(name, idreg, idfield, idval, guard,		\
			     enabled_worlds)					\
	CREATE_FEATURE_PRESENT(name, idreg, idfield, idval,			\
				enabled_worlds)					\
	CREATE_FEATURE_SUPPORTED(name, is_ ## name ## _present, guard)

#define CREATE_PERCPU_FEATURE_FUNCS(name, idreg, idfield, idval, guard,	\
				enabled_worlds)					\
	CREATE_PERCPU_FEATURE_PRESENT(name, idreg, idfield, idval,		\
				enabled_worlds)					\
	CREATE_FEATURE_SUPPORTED(name, is_ ## name ## _present, guard)

__attribute__((always_inline))
static inline bool is_armv7_gentimer_present(void)
{
	/* The Generic Timer is always present in an ARMv8-A implementation */
	return true;
}

CREATE_FEATURE_FUNCS(feat_pan, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_PAN,
		     1U, ENABLE_FEAT_PAN,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_vhe, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_VHE,
		     1U, ENABLE_FEAT_VHE,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_PRESENT(feat_ttcnp, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_CNP,
			1U,
			FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_PRESENT(feat_uao, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_UAO,
			1U,
			FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline bool is_feat_pacqarma3_present(void)
{
	u_register_t isar2 = read_id_aa64isar2_el1();

	return (EXTRACT(ID_AA64ISAR2_GPA3, isar2) |
		EXTRACT(ID_AA64ISAR2_APA3, isar2)) != 0;
}

__attribute__((always_inline))
static inline bool is_feat_pauth_present(void)
{
	uint64_t mask_id_aa64isar1 =
		MASK(ID_AA64ISAR1_GPI) |
		MASK(ID_AA64ISAR1_GPA) |
		MASK(ID_AA64ISAR1_API) |
		MASK(ID_AA64ISAR1_APA);

	/*
	 * If any of the fields is not zero or QARMA3 is present,
	 * PAuth is present
	 */
	return ((read_id_aa64isar1_el1() & mask_id_aa64isar1) != 0U ||
		is_feat_pacqarma3_present());
}
CREATE_FEATURE_SUPPORTED(feat_pauth, is_feat_pauth_present, ENABLE_PAUTH)
CREATE_FEATURE_SUPPORTED(ctx_pauth, is_feat_pauth_present, CTX_INCLUDE_PAUTH_REGS)

__attribute__((always_inline))
static inline bool is_feat_crypto_present(void)
{
	uint64_t mask_id_aa64isar0 =
		MASK(ID_AA64ISAR0_AES) |
		MASK(ID_AA64ISAR0_SHA1) |
		MASK(ID_AA64ISAR0_SHA2);

	/*
	 * Check if AES, SHA1, SHA2 extension presents.
	*/
	return ((read_id_aa64isar0_el1() & mask_id_aa64isar0) != 0U);
}
CREATE_FEATURE_SUPPORTED(feat_crypto, is_feat_crypto_present, ENABLE_FEAT_CRYPTO)

#if (ENABLE_FEAT_IDTE3 && IMAGE_BL31)
__attribute__((always_inline))
static inline void update_feat_pauth_idreg_field(size_t security_state)
{
	uint64_t mask_id_aa64isar1 =
		MASK(ID_AA64ISAR1_GPI) |
		MASK(ID_AA64ISAR1_GPA) |
		MASK(ID_AA64ISAR1_API) |
		MASK(ID_AA64ISAR1_APA);

	/* FEAT_PACQARMA3 */
	uint64_t mask_id_aa64isar2 =
		MASK(ID_AA64ISAR2_APA3) |
		MASK(ID_AA64ISAR2_GPA3);

	per_world_context_t *per_world_ctx = &per_world_context[security_state];
	perworld_idregs_t *perworld_idregs =
		&(per_world_ctx->idregs);

	if ((SHOULD_ID_FIELD_DISABLE(ENABLE_PAUTH, FEAT_ENABLE_NS,
				       security_state))  &&
	    (SHOULD_ID_FIELD_DISABLE(CTX_INCLUDE_PAUTH_REGS,
				       FEAT_ENABLE_ALL_WORLDS,
				       security_state))) {
		perworld_idregs->id_aa64isar1_el1 &= ~(mask_id_aa64isar1);
		perworld_idregs->id_aa64isar2_el1 &= ~(mask_id_aa64isar2);
	}
}
#endif

/*
 * This feature has a non-standard discovery method so define this function
 * manually then call use the CREATE_FEATURE_SUPPORTED macro with it. This
 * feature is enabled with ENABLE_PAUTH when present.
 */
__attribute__((always_inline))
static inline bool is_feat_pauth_lr_present(void)
{
	/*
	 * FEAT_PAUTH_LR support is indicated by up to 3 fields, if one or more
	 * of these is 0b0110 then the feature is present.
	 *   1) id_aa64isr1_el1.api
	 *   2) id_aa64isr1_el1.apa
	 *   3) id_aa64isr2_el1.apa3
	 */
	if (EXTRACT(ID_AA64ISAR1_API, read_id_aa64isar1_el1()) == 0b0110) {
		return true;
	}
	if (EXTRACT(ID_AA64ISAR1_APA, read_id_aa64isar1_el1()) == 0b0110) {
		return true;
	}
	if (EXTRACT(ID_AA64ISAR2_APA3, read_id_aa64isar2_el1()) == 0b0110) {
		return true;
	}
	return false;
}
CREATE_FEATURE_SUPPORTED(feat_pauth_lr, is_feat_pauth_lr_present, ENABLE_FEAT_PAUTH_LR)

CREATE_FEATURE_PRESENT(feat_ttst, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_ST,
			1U,
			FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_bti, id_aa64pfr1_el1, ID_AA64PFR1_EL1_BT,
			BTI_IMPLEMENTED, ENABLE_BTI,
			FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_mte2, id_aa64pfr1_el1, ID_AA64PFR1_EL1_MTE,
		     MTE_IMPLEMENTED_ELX, ENABLE_FEAT_MTE2,
		     FEAT_ENABLE_SECURE | FEAT_ENABLE_NS)

CREATE_FEATURE_PRESENT(feat_ssbs, id_aa64pfr1_el1, ID_AA64PFR1_EL1_SSBS,
			1U,
			FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_PRESENT(feat_nmi, id_aa64pfr1_el1, ID_AA64PFR1_EL1_NMI,
			NMI_IMPLEMENTED,
			FEAT_ENABLE_ALL_WORLDS)

CREATE_PERCPU_FEATURE_FUNCS(feat_ebep, id_aa64dfr1_el1, ID_AA64DFR1_EBEP,
		     1U,  ENABLE_FEAT_EBEP,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_PERCPU_FEATURE_PRESENT(feat_sebep, id_aa64dfr0_el1, ID_AA64DFR0_SEBEP,
			SEBEP_IMPLEMENTED,
			FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_sel2, id_aa64pfr0_el1, ID_AA64PFR0_SEL2,
		     1U, ENABLE_FEAT_SEL2,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_twed, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_TWED,
		     1U, ENABLE_FEAT_TWED,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_fgt, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_FGT,
		     1U, ENABLE_FEAT_FGT,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_fgt2, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_FGT,
		     FGT2_IMPLEMENTED, ENABLE_FEAT_FGT2,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_fgwte3, id_aa64mmfr4_el1, ID_AA64MMFR4_EL1_FGWTE3,
		     FGWTE3_IMPLEMENTED,
		     ENABLE_FEAT_FGWTE3, FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_ecv, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_ECV,
		     1U, ENABLE_FEAT_ECV,
		     FEAT_ENABLE_ALL_WORLDS)
CREATE_FEATURE_FUNCS(feat_ecv_v2, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_ECV,
		     ID_AA64MMFR0_EL1_ECV_SELF_SYNCH,
		     ENABLE_FEAT_ECV, FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_rng, id_aa64isar0_el1, ID_AA64ISAR0_RNDR,
		     1U, ENABLE_FEAT_RNG,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_tcr2, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_TCRX,
		     1U, ENABLE_FEAT_TCR2,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_s2poe, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S2POE,
		     1U, ENABLE_FEAT_S2POE,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_s1poe, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S1POE,
		     1U, ENABLE_FEAT_S1POE,
		     FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline bool is_feat_sxpoe_supported(void)
{
	return is_feat_s1poe_supported() || is_feat_s2poe_supported();
}

CREATE_FEATURE_FUNCS(feat_s2pie, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S2PIE,
		     1U, ENABLE_FEAT_S2PIE,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_s1pie, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S1PIE,
		     1U, ENABLE_FEAT_S1PIE,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_the, id_aa64pfr1_el1, ID_AA64PFR1_EL1_THE,
		     THE_IMPLEMENTED, ENABLE_FEAT_THE,
		     FEAT_ENABLE_NS)

CREATE_FEATURE_FUNCS(feat_sctlr2, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_SCTLR2,
		     SCTLR2_IMPLEMENTED,
		     ENABLE_FEAT_SCTLR2,
		     FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

CREATE_FEATURE_FUNCS(feat_d128, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_D128,
		     D128_IMPLEMENTED,
		     ENABLE_FEAT_D128, FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

_CREATE_FEATURE_PRESENT(feat_rme_gpc2, id_aa64pfr0_el1,
		       ID_AA64PFR0_FEAT_RME,
		       RME_GPC2_IMPLEMENTED)

CREATE_FEATURE_FUNCS(feat_rme_gdi, id_aa64mmfr4_el1,
		     ID_AA64MMFR4_EL1_RME_GDI,
		     RME_GDI_IMPLEMENTED,
		     ENABLE_FEAT_RME_GDI, FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_fpmr, id_aa64pfr2_el1, ID_AA64PFR2_EL1_FPMR,
		     FPMR_IMPLEMENTED,
		     ENABLE_FEAT_FPMR, FEAT_ENABLE_NS)

CREATE_FEATURE_FUNCS(feat_mops, id_aa64isar2_el1, ID_AA64ISAR2_EL1_MOPS,
		     MOPS_IMPLEMENTED,
		     ENABLE_FEAT_MOPS, FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline bool is_feat_sxpie_supported(void)
{
	return is_feat_s1pie_supported() || is_feat_s2pie_supported();
}

CREATE_FEATURE_FUNCS(feat_gcs, id_aa64pfr1_el1, ID_AA64PFR1_EL1_GCS,
		     1U, ENABLE_FEAT_GCS,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_amu, id_aa64pfr0_el1, ID_AA64PFR0_AMU,
		     1U, ENABLE_FEAT_AMU,
		     FEAT_ENABLE_NS)

_CREATE_FEATURE_PRESENT(feat_amu_aux, amcfgr_el0,
		       AMCFGR_EL0_NCG, 1U)

CREATE_FEATURE_SUPPORTED(feat_amu_aux, is_feat_amu_aux_present,
			 ENABLE_AMU_AUXILIARY_COUNTERS)

CREATE_FEATURE_FUNCS(feat_amuv1p1, id_aa64pfr0_el1, ID_AA64PFR0_AMU,
		     ID_AA64PFR0_AMU_V1P1, ENABLE_FEAT_AMUv1p1,
		     FEAT_ENABLE_NS)

/*
 * 0x00: None Armv8.0 or later
 * 0x01: v0.1 Armv8.4 or later
 * 0x10: v1.0 Armv8.2 or later
 * 0x11: v1.1 Armv8.4 or later
 */
__attribute__((always_inline))
static inline unsigned int read_feat_mpam_id_field(void)
{
	return (EXTRACT(ID_AA64PFR0_MPAM, read_id_aa64pfr0_el1()) << 4) |
	       EXTRACT(ID_AA64PFR1_MPAM_FRAC, read_id_aa64pfr1_el1());
}

__attribute__((always_inline))
static inline bool is_feat_mpam_present(void)
{
	return read_feat_mpam_id_field();
}

CREATE_FEATURE_SUPPORTED(feat_mpam, is_feat_mpam_present, ENABLE_FEAT_MPAM)


#if (ENABLE_FEAT_IDTE3 && IMAGE_BL31)
__attribute__((always_inline))
static inline void update_feat_mpam_idreg_field(size_t security_state)
{
	if (SHOULD_ID_FIELD_DISABLE(ENABLE_FEAT_MPAM,
			FEAT_ENABLE_NS | FEAT_ENABLE_REALM, security_state)) {
		per_world_context_t *per_world_ctx =
			&per_world_context[security_state];
		perworld_idregs_t *perworld_idregs =
			&(per_world_ctx->idregs);

		perworld_idregs->id_aa64pfr0_el1 &=
			~((u_register_t)MASK(ID_AA64PFR0_MPAM));

		perworld_idregs->id_aa64pfr1_el1 &=
			~((u_register_t)MASK(ID_AA64PFR1_MPAM_FRAC));
	}
}
#endif

__attribute__((always_inline))
static inline bool is_feat_mpam_pe_bw_ctrl_present(void)
{
	if (is_feat_mpam_present()) {
		return ((unsigned long long)(read_mpamidr_el1() &
				MPAMIDR_HAS_BW_CTRL_BIT) != 0U);
	}
	return false;
}

CREATE_FEATURE_SUPPORTED(feat_mpam_pe_bw_ctrl, is_feat_mpam_pe_bw_ctrl_present,
		ENABLE_FEAT_MPAM_PE_BW_CTRL)

CREATE_PERCPU_FEATURE_FUNCS(feat_debugv8p9, id_aa64dfr0_el1,
		ID_AA64DFR0_DEBUGVER, DEBUGVER_V8P9_IMPLEMENTED, ENABLE_FEAT_DEBUGV8P9,
		FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

CREATE_FEATURE_FUNCS(feat_hcx, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_HCX,
		     1U, ENABLE_FEAT_HCX,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_rng_trap, id_aa64pfr1_el1, ID_AA64PFR1_EL1_RNDR_TRAP,
		      RNG_TRAP_IMPLEMENTED, ENABLE_FEAT_RNG_TRAP,
		      FEAT_ENABLE_ALL_WORLDS)

_CREATE_FEATURE_PRESENT(feat_rme, id_aa64pfr0_el1,
		      ID_AA64PFR0_FEAT_RME, 1U)

CREATE_FEATURE_SUPPORTED(feat_rme, is_feat_rme_present, ENABLE_FEAT_RME)

CREATE_FEATURE_PRESENT(feat_sb, id_aa64isar1_el1, ID_AA64ISAR1_SB,
		       1U,
		       FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_mec, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_MEC,
		1U, ENABLE_FEAT_MEC,
		FEAT_ENABLE_ALL_WORLDS)

/*
 * 0b0000 - Feature FEAT_CSV2 is not implemented.
 * 0b0001 - Feature FEAT_CSV2 is implemented, but FEAT_CSV2_2 and FEAT_CSV2_3
 *          are not implemented.
 * 0b0010 - Feature FEAT_CSV2_2 is implemented but FEAT_CSV2_3 is not
 *          implemented.
 * 0b0011 - Feature FEAT_CSV2_3 is implemented.
 */
CREATE_FEATURE_FUNCS(feat_csv2_2, id_aa64pfr0_el1, ID_AA64PFR0_CSV2,
		     CSV2_2_IMPLEMENTED, ENABLE_FEAT_CSV2_2,
		     FEAT_ENABLE_NS | FEAT_ENABLE_REALM)
CREATE_FEATURE_FUNCS(feat_csv2_3, id_aa64pfr0_el1, ID_AA64PFR0_CSV2,
		     CSV2_3_IMPLEMENTED, ENABLE_FEAT_CSV2_3,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_clrbhb, id_aa64isar2_el1, ID_AA64ISAR2_CLRBHB,
		     1U, ENABLE_FEAT_CLRBHB,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_PERCPU_FEATURE_FUNCS(feat_spe, id_aa64dfr0_el1, ID_AA64DFR0_PMS,
		     1U, ENABLE_SPE_FOR_NS,
		     FEAT_ENABLE_NS)

CREATE_FEATURE_FUNCS(feat_sve, id_aa64pfr0_el1, ID_AA64PFR0_SVE,
		     1U, ENABLE_SVE_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline unsigned int read_feat_iesb_id_field(void)
{
	return (unsigned int)EXTRACT(ID_AA64MMFR2_EL1_IESB,
				read_id_aa64mmfr2_el1());
}

CREATE_FEATURE_FUNCS(feat_ras, id_aa64pfr0_el1, ID_AA64PFR0_RAS,
		     1U, ENABLE_FEAT_RAS,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_dit, id_aa64pfr0_el1, ID_AA64PFR0_DIT,
		     1U, ENABLE_FEAT_DIT,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_PERCPU_FEATURE_FUNCS(feat_sys_reg_trace, id_aa64dfr0_el1,
			ID_AA64DFR0_TRACEVER, 1U, ENABLE_SYS_REG_TRACE_FOR_NS,
			FEAT_ENABLE_ALL_WORLDS)

CREATE_PERCPU_FEATURE_FUNCS(feat_trf, id_aa64dfr0_el1, ID_AA64DFR0_TRACEFILT,
		     1U, ENABLE_TRF_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_nv2, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_NV,
		     NV2_IMPLEMENTED, CTX_INCLUDE_NEVE_REGS,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_PERCPU_FEATURE_FUNCS(feat_brbe, id_aa64dfr0_el1, ID_AA64DFR0_BRBE,
		     1U, ENABLE_BRBE_FOR_NS,
		     FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

_CREATE_FEATURE_PRESENT(feat_trbe, id_aa64dfr0_el1, ID_AA64DFR0_TRACEBUFFER, 1U)

CREATE_FEATURE_SUPPORTED(feat_trbe, is_feat_trbe_present, ENABLE_TRBE_FOR_NS)

CREATE_PERCPU_IDREG_UPDATE(feat_trbe, id_aa64dfr0_el1, ID_AA64DFR0_TRACEBUFFER,
			ENABLE_TRBE_FOR_NS && !check_if_trbe_disable_affected_core(),
			FEAT_ENABLE_NS)

CREATE_FEATURE_PRESENT(feat_sme_fa64, id_aa64smfr0_el1, ID_AA64SMFR0_EL1_SME_FA64,
		    1U,
		    FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_sme, id_aa64pfr1_el1, ID_AA64PFR1_EL1_SME,
		     1U, ENABLE_SME_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_sme2, id_aa64pfr1_el1, ID_AA64PFR1_EL1_SME,
		     SME2_IMPLEMENTED, ENABLE_SME2_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_ls64_accdata, id_aa64isar1_el1, ID_AA64ISAR1_LS64,
		     LS64_ACCDATA_IMPLEMENTED,
		     ENABLE_FEAT_LS64_ACCDATA, FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_aie, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_AIE,
		     1U, ENABLE_FEAT_AIE,
		     FEAT_ENABLE_NS)

CREATE_FEATURE_FUNCS(feat_pfar, id_aa64pfr1_el1, ID_AA64PFR1_EL1_PFAR,
		     1U, ENABLE_FEAT_PFAR,
		     FEAT_ENABLE_NS)

CREATE_FEATURE_FUNCS(feat_idte3, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_IDS,
		     2U, ENABLE_FEAT_IDTE3,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_lse, id_aa64isar0_el1, ID_AA64ISAR0_ATOMIC,
		     1U, USE_SPINLOCK_CAS,
		     FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline bool is_feat_tgran4K_present(void)
{
	unsigned int tgranx = (unsigned int)EXTRACT(ID_AA64MMFR0_EL1_TGRAN4,
						    read_id_aa64mmfr0_el1());
	return (tgranx < 8U);
}

CREATE_FEATURE_PRESENT(feat_tgran16K, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_TGRAN16,
		       TGRAN16_IMPLEMENTED,
		       FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline bool is_feat_tgran64K_present(void)
{
	unsigned int tgranx = (unsigned int)EXTRACT(ID_AA64MMFR0_EL1_TGRAN64,
						    read_id_aa64mmfr0_el1());
	return (tgranx < 8U);
}

_CREATE_FEATURE_PRESENT(feat_pmuv3, id_aa64dfr0_el1, ID_AA64DFR0_PMUVER, 1U)

CREATE_PERCPU_FEATURE_FUNCS(feat_mtpmu, id_aa64dfr0_el1, ID_AA64DFR0_MTPMU,
			   MTPMU_IMPLEMENTED, DISABLE_MTPMU,
			   FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_gcie, id_aa64pfr2_el1, ID_AA64PFR2_EL1_GCIE,
		     1U, ENABLE_FEAT_GCIE,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_cpa2, id_aa64isar3_el1, ID_AA64ISAR3_EL1_CPA,
		     CPA2_IMPLEMENTED,
		     ENABLE_FEAT_CPA2, FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_uinj, id_aa64pfr2_el1, ID_AA64PFR2_EL1_UINJ,
		     UINJ_IMPLEMENTED,
		     ENABLE_FEAT_UINJ, FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_morello, id_aa64pfr1_el1, ID_AA64PFR1_EL1_CE,
		     MORELLO_EXTENSION_IMPLEMENTED,
		     ENABLE_FEAT_MORELLO, FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_step2, id_aa64dfr2_el1, ID_AA64DFR2_STEP,
		     1U, ENABLE_FEAT_STEP2,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_hdbss, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_HAFDBS,
		     HDBSS_IMPLEMENTED,
		     ENABLE_FEAT_HDBSS, FEAT_ENABLE_NS)

CREATE_FEATURE_FUNCS(feat_hacdbs, id_aa64mmfr4_el1, ID_AA64MMFR4_EL1_HACDBS,
		     HACDBS_IMPLEMENTED,
		     ENABLE_FEAT_HACDBS, FEAT_ENABLE_NS)

#endif /* ARCH_FEATURES_H */
