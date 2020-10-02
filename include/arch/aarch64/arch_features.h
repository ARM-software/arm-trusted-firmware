/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
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

#if ENABLE_RME
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

#define ISOLATE_FIELD(reg, feat, mask)						\
	((unsigned int)(((reg) >> (feat)) & mask))

#define SHOULD_ID_FIELD_DISABLE(guard, enabled_worlds, world)		\
	 (((guard) == 0U) || ((((enabled_worlds) >> (world)) & 1U) == 0U))


#define CREATE_FEATURE_SUPPORTED(name, read_func, guard)			\
__attribute__((always_inline))							\
static inline bool is_ ## name ## _supported(void)				\
{										\
	if ((guard) == FEAT_STATE_DISABLED) {					\
		return false;							\
	}									\
	if ((guard) == FEAT_STATE_ALWAYS) {					\
		return true;							\
	}									\
	return read_func();							\
}

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
 *		perworld_idregs->idreg &=
 *			~((u_register_t)mask << idfield);
 *		perworld_idregs->idreg |=
 *		(((u_register_t)<unimplemented state value> & mask) << idfield);
 *	}
 * }
 */

#if (ENABLE_FEAT_IDTE3 && IMAGE_BL31)
#define CREATE_IDREG_UPDATE(name, idreg, idfield, mask, guard, enabled_worlds)	\
	__attribute__((always_inline))						\
static inline void update_ ## name ## _idreg_field(size_t security_state)	\
{										\
	if (SHOULD_ID_FIELD_DISABLE(guard, enabled_worlds, security_state)) {	\
		per_world_context_t *per_world_ctx =				\
				&per_world_context[security_state];		\
		perworld_idregs_t *perworld_idregs = &(per_world_ctx->idregs);	\
		perworld_idregs->idreg &= ~((u_register_t)mask << idfield);	\
	}									\
}
#define CREATE_PERCPU_IDREG_UPDATE(name, idreg, idfield, mask, guard,		\
					enabled_worlds)				\
	__attribute__((always_inline))						\
static inline void update_ ## name ## _idreg_field(size_t security_state)	\
{										\
	if (SHOULD_ID_FIELD_DISABLE(guard, enabled_worlds, security_state)) {	\
		percpu_idregs_t *percpu_idregs =				\
					&(get_cpu_data(idregs[security_state]));\
		percpu_idregs->idreg &= ~((u_register_t)mask << idfield);	\
	}									\
}
#else
#define CREATE_IDREG_UPDATE(name, idreg, idfield, mask, guard, enabled_worlds)
#define CREATE_PERCPU_IDREG_UPDATE(name, idreg, idfield, mask, guard,		\
					enabled_worlds)
#endif

#define _CREATE_FEATURE_PRESENT(name, idreg, idfield, mask, idval)		\
__attribute__((always_inline))							\
static inline bool is_ ## name ## _present(void)				\
{										\
	return (ISOLATE_FIELD(read_ ## idreg(), idfield, mask) >= idval) 	\
		? true : false; 						\
}

#define CREATE_FEATURE_PRESENT(name, idreg, idfield, mask, idval,		\
				enabled_worlds)					\
	_CREATE_FEATURE_PRESENT(name, idreg, idfield, mask, idval)		\
	CREATE_IDREG_UPDATE(name, idreg, idfield, mask, 1U, enabled_worlds)

#define CREATE_PERCPU_FEATURE_PRESENT(name, idreg, idfield, mask, idval,	\
					enabled_worlds)				\
	_CREATE_FEATURE_PRESENT(name, idreg, idfield, mask, idval)		\
	CREATE_PERCPU_IDREG_UPDATE(name, idreg, idfield, mask, 1U,		\
					enabled_worlds)

#define CREATE_FEATURE_FUNCS(name, idreg, idfield, mask, idval, guard,		\
			     enabled_worlds)					\
	CREATE_FEATURE_PRESENT(name, idreg, idfield, mask, idval,		\
				enabled_worlds)					\
	CREATE_FEATURE_SUPPORTED(name, is_ ## name ## _present, guard)

#define CREATE_PERCPU_FEATURE_FUNCS(name, idreg, idfield, mask, idval, guard,	\
				enabled_worlds)					\
	CREATE_PERCPU_FEATURE_PRESENT(name, idreg, idfield, mask, idval,	\
				enabled_worlds)					\
	CREATE_FEATURE_SUPPORTED(name, is_ ## name ## _present, guard)

/* +----------------------------+
 * |	Features supported	|
 * +----------------------------+
 * |	GENTIMER		|
 * +----------------------------+
 * |	FEAT_PAN		|
 * +----------------------------+
 * |	FEAT_VHE		|
 * +----------------------------+
 * |	FEAT_TTCNP		|
 * +----------------------------+
 * |	FEAT_UAO		|
 * +----------------------------+
 * |	FEAT_PACQARMA3		|
 * +----------------------------+
 * |	FEAT_PAUTH		|
 * +----------------------------+
 * |	FEAT_TTST		|
 * +----------------------------+
 * |	FEAT_BTI		|
 * +----------------------------+
 * |	FEAT_MTE2		|
 * +----------------------------+
 * |	FEAT_SSBS		|
 * +----------------------------+
 * |	FEAT_NMI		|
 * +----------------------------+
 * |	FEAT_GCS		|
 * +----------------------------+
 * |	FEAT_EBEP		|
 * +----------------------------+
 * |	FEAT_SEBEP		|
 * +----------------------------+
 * |	FEAT_SEL2		|
 * +----------------------------+
 * |	FEAT_TWED		|
 * +----------------------------+
 * |	FEAT_FGT		|
 * +----------------------------+
 * |	FEAT_EC/ECV2		|
 * +----------------------------+
 * |	FEAT_RNG		|
 * +----------------------------+
 * |	FEAT_TCR2		|
 * +----------------------------+
 * |	FEAT_S2POE		|
 * +----------------------------+
 * |	FEAT_S1POE		|
 * +----------------------------+
 * |	FEAT_S2PIE		|
 * +----------------------------+
 * |	FEAT_S1PIE		|
 * +----------------------------+
 * |	FEAT_AMU/AMUV1P1	|
 * +----------------------------+
 * |	FEAT_MPAM		|
 * +----------------------------+
 * |	FEAT_HCX		|
 * +----------------------------+
 * |	FEAT_RNG_TRAP		|
 * +----------------------------+
 * |	FEAT_RME		|
 * +----------------------------+
 * |	FEAT_SB			|
 * +----------------------------+
 * |	FEAT_CSV2_2/CSV2_3	|
 * +----------------------------+
 * |	FEAT_SPE		|
 * +----------------------------+
 * |	FEAT_SVE		|
 * +----------------------------+
 * |	FEAT_RAS		|
 * +----------------------------+
 * |	FEAT_DIT		|
 * +----------------------------+
 * |	FEAT_SYS_REG_TRACE	|
 * +----------------------------+
 * |	FEAT_TRF		|
 * +----------------------------+
 * |	FEAT_NV2		|
 * +----------------------------+
 * |	FEAT_BRBE		|
 * +----------------------------+
 * |	FEAT_TRBE		|
 * +----------------------------+
 * |	FEAT_SME/SME2		|
 * +----------------------------+
 * |	FEAT_PMUV3		|
 * +----------------------------+
 * |	FEAT_MTPMU		|
 * +----------------------------+
 * |	FEAT_FGT2		|
 * +----------------------------+
 * |	FEAT_THE		|
 * +----------------------------+
 * |	FEAT_SCTLR2		|
 * +----------------------------+
 * |	FEAT_D128		|
 * +----------------------------+
 * |	FEAT_LS64_ACCDATA	|
 * +----------------------------+
 * |	FEAT_FPMR		|
 * +----------------------------+
 * |	FEAT_MOPS		|
 * +----------------------------+
 * |	FEAT_PAUTH_LR		|
 * +----------------------------+
 * |	FEAT_FGWTE3		|
 * +----------------------------+
 * |	FEAT_MPAM_PE_BW_CTRL	|
 * +----------------------------+
 * |	FEAT_CPA2		|
 * +----------------------------+
 * |	FEAT_AIE		|
 * +----------------------------+
 * |	FEAT_PFAR		|
 * +----------------------------+
 * |	FEAT_RME_GPC2		|
 * +----------------------------+
 * |	FEAT_RME_GDI		|
 * +----------------------------+
 * |    FEAT_IDTE3              |
 * +----------------------------+
 * |    FEAT_UINJ               |
 * +----------------------------+
 * |    FEAT_LSE                |
 * +----------------------------+
 * |	FEAT_MORELLO		|
 * +----------------------------+
 */

__attribute__((always_inline))
static inline bool is_armv7_gentimer_present(void)
{
	/* The Generic Timer is always present in an ARMv8-A implementation */
	return true;
}

/* FEAT_PAN: Privileged access never */
CREATE_FEATURE_FUNCS(feat_pan, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_PAN_SHIFT,
		     ID_AA64MMFR1_EL1_PAN_MASK, 1U, ENABLE_FEAT_PAN,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_VHE: Virtualization Host Extensions */
CREATE_FEATURE_FUNCS(feat_vhe, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_VHE_SHIFT,
		     ID_AA64MMFR1_EL1_VHE_MASK, 1U, ENABLE_FEAT_VHE,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_TTCNP: Translation table common not private */
CREATE_FEATURE_PRESENT(feat_ttcnp, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_CNP_SHIFT,
			ID_AA64MMFR2_EL1_CNP_MASK, 1U,
			FEAT_ENABLE_ALL_WORLDS)

/* FEAT_UAO: User access override */
CREATE_FEATURE_PRESENT(feat_uao, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_UAO_SHIFT,
			ID_AA64MMFR2_EL1_UAO_MASK, 1U,
			FEAT_ENABLE_ALL_WORLDS)

/* If any of the fields is not zero, QARMA3 algorithm is present */
CREATE_FEATURE_PRESENT(feat_pacqarma3, id_aa64isar2_el1, 0,
			((ID_AA64ISAR2_GPA3_MASK << ID_AA64ISAR2_GPA3_SHIFT) |
			(ID_AA64ISAR2_APA3_MASK << ID_AA64ISAR2_APA3_SHIFT)), 1U,
			FEAT_ENABLE_ALL_WORLDS)

/* FEAT_PAUTH: Pointer Authentication */
__attribute__((always_inline))
static inline bool is_feat_pauth_present(void)
{
	uint64_t mask_id_aa64isar1 =
		(ID_AA64ISAR1_GPI_MASK << ID_AA64ISAR1_GPI_SHIFT) |
		(ID_AA64ISAR1_GPA_MASK << ID_AA64ISAR1_GPA_SHIFT) |
		(ID_AA64ISAR1_API_MASK << ID_AA64ISAR1_API_SHIFT) |
		(ID_AA64ISAR1_APA_MASK << ID_AA64ISAR1_APA_SHIFT);

	/*
	 * If any of the fields is not zero or QARMA3 is present,
	 * PAuth is present
	 */
	return ((read_id_aa64isar1_el1() & mask_id_aa64isar1) != 0U ||
		is_feat_pacqarma3_present());
}
CREATE_FEATURE_SUPPORTED(feat_pauth, is_feat_pauth_present, ENABLE_PAUTH)
CREATE_FEATURE_SUPPORTED(ctx_pauth, is_feat_pauth_present, CTX_INCLUDE_PAUTH_REGS)

#if (ENABLE_FEAT_IDTE3 && IMAGE_BL31)
__attribute__((always_inline))
static inline void update_feat_pauth_idreg_field(size_t security_state)
{
	uint64_t mask_id_aa64isar1 =
		(ID_AA64ISAR1_GPI_MASK << ID_AA64ISAR1_GPI_SHIFT) |
		(ID_AA64ISAR1_GPA_MASK << ID_AA64ISAR1_GPA_SHIFT) |
		(ID_AA64ISAR1_API_MASK << ID_AA64ISAR1_API_SHIFT) |
		(ID_AA64ISAR1_APA_MASK << ID_AA64ISAR1_APA_SHIFT);

	uint64_t mask_id_aa64isar2 =
		(ID_AA64ISAR2_APA3_MASK << ID_AA64ISAR2_APA3_MASK) |
		(ID_AA64ISAR2_GPA3_MASK << ID_AA64ISAR2_GPA3_MASK);

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
 * FEAT_PAUTH_LR
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
	if (ISOLATE_FIELD(read_id_aa64isar1_el1(), ID_AA64ISAR1_API_SHIFT, ID_AA64ISAR1_API_MASK) == 0b0110) {
		return true;
	}
	if (ISOLATE_FIELD(read_id_aa64isar1_el1(), ID_AA64ISAR1_APA_SHIFT, ID_AA64ISAR1_APA_MASK) == 0b0110) {
		return true;
	}
	if (ISOLATE_FIELD(read_id_aa64isar2_el1(), ID_AA64ISAR2_APA3_SHIFT, ID_AA64ISAR2_APA3_MASK) == 0b0110) {
		return true;
	}
	return false;
}
CREATE_FEATURE_SUPPORTED(feat_pauth_lr, is_feat_pauth_lr_present, ENABLE_FEAT_PAUTH_LR)

/* FEAT_TTST: Small translation tables */
CREATE_FEATURE_PRESENT(feat_ttst, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_ST_SHIFT,
			ID_AA64MMFR2_EL1_ST_MASK, 1U,
			FEAT_ENABLE_ALL_WORLDS)

/* FEAT_BTI: Branch target identification */
CREATE_FEATURE_FUNCS(feat_bti, id_aa64pfr1_el1, ID_AA64PFR1_EL1_BT_SHIFT,
			ID_AA64PFR1_EL1_BT_MASK, BTI_IMPLEMENTED, ENABLE_BTI,
			FEAT_ENABLE_ALL_WORLDS)

/* FEAT_MTE2: Memory tagging extension */
CREATE_FEATURE_FUNCS(feat_mte2, id_aa64pfr1_el1, ID_AA64PFR1_EL1_MTE_SHIFT,
		     ID_AA64PFR1_EL1_MTE_MASK, MTE_IMPLEMENTED_ELX, ENABLE_FEAT_MTE2,
		     FEAT_ENABLE_SECURE | FEAT_ENABLE_NS)

/* FEAT_SSBS: Speculative store bypass safe */
CREATE_FEATURE_PRESENT(feat_ssbs, id_aa64pfr1_el1, ID_AA64PFR1_EL1_SSBS_SHIFT,
			ID_AA64PFR1_EL1_SSBS_MASK, 1U,
			FEAT_ENABLE_ALL_WORLDS)

/* FEAT_NMI: Non-maskable interrupts */
CREATE_FEATURE_PRESENT(feat_nmi, id_aa64pfr1_el1, ID_AA64PFR1_EL1_NMI_SHIFT,
			ID_AA64PFR1_EL1_NMI_MASK, NMI_IMPLEMENTED,
			FEAT_ENABLE_ALL_WORLDS)

/* FEAT_EBEP */
CREATE_PERCPU_FEATURE_FUNCS(feat_ebep, id_aa64dfr1_el1, ID_AA64DFR1_EBEP_SHIFT,
		     ID_AA64DFR1_EBEP_MASK, 1U,  ENABLE_FEAT_EBEP,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_SEBEP */
CREATE_PERCPU_FEATURE_PRESENT(feat_sebep, id_aa64dfr0_el1, ID_AA64DFR0_SEBEP_SHIFT,
			ID_AA64DFR0_SEBEP_MASK, SEBEP_IMPLEMENTED,
			FEAT_ENABLE_ALL_WORLDS)

/* FEAT_SEL2: Secure EL2 */
CREATE_FEATURE_FUNCS(feat_sel2, id_aa64pfr0_el1, ID_AA64PFR0_SEL2_SHIFT,
		     ID_AA64PFR0_SEL2_MASK, 1U, ENABLE_FEAT_SEL2,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_TWED: Delayed trapping of WFE */
CREATE_FEATURE_FUNCS(feat_twed, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_TWED_SHIFT,
		     ID_AA64MMFR1_EL1_TWED_MASK, 1U, ENABLE_FEAT_TWED,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_FGT: Fine-grained traps */
CREATE_FEATURE_FUNCS(feat_fgt, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_FGT_SHIFT,
		     ID_AA64MMFR0_EL1_FGT_MASK, 1U, ENABLE_FEAT_FGT,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_FGT2: Fine-grained traps extended */
CREATE_FEATURE_FUNCS(feat_fgt2, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_FGT_SHIFT,
		     ID_AA64MMFR0_EL1_FGT_MASK, FGT2_IMPLEMENTED, ENABLE_FEAT_FGT2,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_FGWTE3: Fine-grained write traps EL3 */
CREATE_FEATURE_FUNCS(feat_fgwte3, id_aa64mmfr4_el1, ID_AA64MMFR4_EL1_FGWTE3_SHIFT,
		     ID_AA64MMFR4_EL1_FGWTE3_MASK, FGWTE3_IMPLEMENTED,
		     ENABLE_FEAT_FGWTE3, FEAT_ENABLE_ALL_WORLDS)

/* FEAT_ECV: Enhanced Counter Virtualization */
CREATE_FEATURE_FUNCS(feat_ecv, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_ECV_SHIFT,
		     ID_AA64MMFR0_EL1_ECV_MASK, 1U, ENABLE_FEAT_ECV,
		     FEAT_ENABLE_ALL_WORLDS)
CREATE_FEATURE_FUNCS(feat_ecv_v2, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_ECV_SHIFT,
		     ID_AA64MMFR0_EL1_ECV_MASK, ID_AA64MMFR0_EL1_ECV_SELF_SYNCH,
		     ENABLE_FEAT_ECV, FEAT_ENABLE_ALL_WORLDS)

/* FEAT_RNG: Random number generator */
CREATE_FEATURE_FUNCS(feat_rng, id_aa64isar0_el1, ID_AA64ISAR0_RNDR_SHIFT,
		     ID_AA64ISAR0_RNDR_MASK, 1U, ENABLE_FEAT_RNG,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_TCR2: Support TCR2_ELx regs */
CREATE_FEATURE_FUNCS(feat_tcr2, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_TCRX_SHIFT,
		     ID_AA64MMFR3_EL1_TCRX_MASK, 1U, ENABLE_FEAT_TCR2,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_S2POE */
CREATE_FEATURE_FUNCS(feat_s2poe, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S2POE_SHIFT,
		     ID_AA64MMFR3_EL1_S2POE_MASK, 1U, ENABLE_FEAT_S2POE,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_S1POE */
CREATE_FEATURE_FUNCS(feat_s1poe, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S1POE_SHIFT,
		     ID_AA64MMFR3_EL1_S1POE_MASK, 1U, ENABLE_FEAT_S1POE,
		     FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline bool is_feat_sxpoe_supported(void)
{
	return is_feat_s1poe_supported() || is_feat_s2poe_supported();
}

/* FEAT_S2PIE */
CREATE_FEATURE_FUNCS(feat_s2pie, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S2PIE_SHIFT,
		     ID_AA64MMFR3_EL1_S2PIE_MASK, 1U, ENABLE_FEAT_S2PIE,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_S1PIE */
CREATE_FEATURE_FUNCS(feat_s1pie, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S1PIE_SHIFT,
		     ID_AA64MMFR3_EL1_S1PIE_MASK, 1U, ENABLE_FEAT_S1PIE,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_THE: Translation Hardening Extension */
CREATE_FEATURE_FUNCS(feat_the, id_aa64pfr1_el1, ID_AA64PFR1_EL1_THE_SHIFT,
		     ID_AA64PFR1_EL1_THE_MASK, THE_IMPLEMENTED, ENABLE_FEAT_THE,
		     FEAT_ENABLE_NS)

/* FEAT_SCTLR2 */
CREATE_FEATURE_FUNCS(feat_sctlr2, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_SCTLR2_SHIFT,
		     ID_AA64MMFR3_EL1_SCTLR2_MASK, SCTLR2_IMPLEMENTED,
		     ENABLE_FEAT_SCTLR2,
		     FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

/* FEAT_D128 */
CREATE_FEATURE_FUNCS(feat_d128, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_D128_SHIFT,
		     ID_AA64MMFR3_EL1_D128_MASK, D128_IMPLEMENTED,
		     ENABLE_FEAT_D128, FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

/* FEAT_RME_GPC2 */
_CREATE_FEATURE_PRESENT(feat_rme_gpc2, id_aa64pfr0_el1,
		       ID_AA64PFR0_FEAT_RME_SHIFT, ID_AA64PFR0_FEAT_RME_MASK,
		       RME_GPC2_IMPLEMENTED)

/* FEAT_RME_GDI */
CREATE_FEATURE_FUNCS(feat_rme_gdi, id_aa64mmfr4_el1,
		     ID_AA64MMFR4_EL1_RME_GDI_SHIFT,
		     ID_AA64MMFR4_EL1_RME_GDI_MASK, RME_GDI_IMPLEMENTED,
		     ENABLE_FEAT_RME_GDI, FEAT_ENABLE_ALL_WORLDS)

/* FEAT_FPMR */
CREATE_FEATURE_FUNCS(feat_fpmr, id_aa64pfr2_el1, ID_AA64PFR2_EL1_FPMR_SHIFT,
		     ID_AA64PFR2_EL1_FPMR_MASK, FPMR_IMPLEMENTED,
		     ENABLE_FEAT_FPMR, FEAT_ENABLE_NS)
/* FEAT_MOPS */
CREATE_FEATURE_FUNCS(feat_mops, id_aa64isar2_el1, ID_AA64ISAR2_EL1_MOPS_SHIFT,
		     ID_AA64ISAR2_EL1_MOPS_MASK, MOPS_IMPLEMENTED,
		     ENABLE_FEAT_MOPS, FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline bool is_feat_sxpie_supported(void)
{
	return is_feat_s1pie_supported() || is_feat_s2pie_supported();
}

/* FEAT_GCS: Guarded Control Stack */
CREATE_FEATURE_FUNCS(feat_gcs, id_aa64pfr1_el1, ID_AA64PFR1_EL1_GCS_SHIFT,
		     ID_AA64PFR1_EL1_GCS_MASK, 1U, ENABLE_FEAT_GCS,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_AMU: Activity Monitors Extension */
CREATE_FEATURE_FUNCS(feat_amu, id_aa64pfr0_el1, ID_AA64PFR0_AMU_SHIFT,
		     ID_AA64PFR0_AMU_MASK, 1U, ENABLE_FEAT_AMU,
		     FEAT_ENABLE_NS)

/* Auxiliary counters for FEAT_AMU */
_CREATE_FEATURE_PRESENT(feat_amu_aux, amcfgr_el0,
		       AMCFGR_EL0_NCG_SHIFT, AMCFGR_EL0_NCG_MASK, 1U)

CREATE_FEATURE_SUPPORTED(feat_amu_aux, is_feat_amu_aux_present,
			 ENABLE_AMU_AUXILIARY_COUNTERS)

/* FEAT_AMUV1P1: AMU Extension v1.1 */
CREATE_FEATURE_FUNCS(feat_amuv1p1, id_aa64pfr0_el1, ID_AA64PFR0_AMU_SHIFT,
		     ID_AA64PFR0_AMU_MASK, ID_AA64PFR0_AMU_V1P1, ENABLE_FEAT_AMUv1p1,
		     FEAT_ENABLE_NS)

/*
 * Return MPAM version:
 *
 * 0x00: None Armv8.0 or later
 * 0x01: v0.1 Armv8.4 or later
 * 0x10: v1.0 Armv8.2 or later
 * 0x11: v1.1 Armv8.4 or later
 *
 */
__attribute__((always_inline))
static inline bool is_feat_mpam_present(void)
{
	unsigned int ret = (unsigned int)((((read_id_aa64pfr0_el1() >>
		ID_AA64PFR0_MPAM_SHIFT) & ID_AA64PFR0_MPAM_MASK) << 4) |
		((read_id_aa64pfr1_el1() >> ID_AA64PFR1_MPAM_FRAC_SHIFT)
			& ID_AA64PFR1_MPAM_FRAC_MASK));
	return ret;
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
			~((u_register_t)ID_AA64PFR0_MPAM_MASK
					<< ID_AA64PFR0_MPAM_SHIFT);

		perworld_idregs->id_aa64pfr1_el1 &=
			~((u_register_t)ID_AA64PFR1_MPAM_FRAC_MASK
					<< ID_AA64PFR1_MPAM_FRAC_SHIFT);
	}
}
#endif

/* FEAT_MPAM_PE_BW_CTRL: MPAM PE-side bandwidth controls */
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

/*
 * FEAT_DebugV8P9: Debug extension. This function checks the field 3:0 of
 * ID_AA64DFR0 Aarch64 Debug Feature Register 0 for the version of
 * Feat_Debug supported. The value of the field determines feature presence
 *
 * 0b0110 - Arm v8.0 debug
 * 0b0111 - Arm v8.0 debug architecture with Virtualization host extensions
 * 0x1000 - FEAT_Debugv8p2 is supported
 * 0x1001 - FEAT_Debugv8p4 is supported
 * 0x1010 - FEAT_Debugv8p8 is supported
 * 0x1011 - FEAT_Debugv8p9 is supported
 *
 */
CREATE_PERCPU_FEATURE_FUNCS(feat_debugv8p9, id_aa64dfr0_el1,
		ID_AA64DFR0_DEBUGVER_SHIFT, ID_AA64DFR0_DEBUGVER_MASK,
		DEBUGVER_V8P9_IMPLEMENTED, ENABLE_FEAT_DEBUGV8P9,
		FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

/* FEAT_HCX: Extended Hypervisor Configuration Register */
CREATE_FEATURE_FUNCS(feat_hcx, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_HCX_SHIFT,
		     ID_AA64MMFR1_EL1_HCX_MASK, 1U, ENABLE_FEAT_HCX,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_RNG_TRAP: Trapping support */
CREATE_FEATURE_FUNCS(feat_rng_trap, id_aa64pfr1_el1, ID_AA64PFR1_EL1_RNDR_TRAP_SHIFT,
		      ID_AA64PFR1_EL1_RNDR_TRAP_MASK, RNG_TRAP_IMPLEMENTED, ENABLE_FEAT_RNG_TRAP,
		      FEAT_ENABLE_ALL_WORLDS)

/* Return the RME version, zero if not supported. */
_CREATE_FEATURE_PRESENT(feat_rme, id_aa64pfr0_el1,
		      ID_AA64PFR0_FEAT_RME_SHIFT, ID_AA64PFR0_FEAT_RME_MASK, 1U)

CREATE_FEATURE_SUPPORTED(feat_rme, is_feat_rme_present, ENABLE_RME)

/* FEAT_SB: Speculation barrier instruction */
CREATE_FEATURE_PRESENT(feat_sb, id_aa64isar1_el1, ID_AA64ISAR1_SB_SHIFT,
		       ID_AA64ISAR1_SB_MASK, 1U,
		       FEAT_ENABLE_ALL_WORLDS)

/* FEAT_MEC: Memory Encryption Contexts */
CREATE_FEATURE_FUNCS(feat_mec, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_MEC_SHIFT,
		ID_AA64MMFR3_EL1_MEC_MASK, 1U, ENABLE_FEAT_MEC,
		FEAT_ENABLE_ALL_WORLDS)

/*
 * FEAT_CSV2: Cache Speculation Variant 2. This checks bit fields[56-59]
 * of id_aa64pfr0_el1 register and can be used to check for below features:
 * FEAT_CSV2_2: Cache Speculation Variant CSV2_2.
 * FEAT_CSV2_3: Cache Speculation Variant CSV2_3.
 * 0b0000 - Feature FEAT_CSV2 is not implemented.
 * 0b0001 - Feature FEAT_CSV2 is implemented, but FEAT_CSV2_2 and FEAT_CSV2_3
 *          are not implemented.
 * 0b0010 - Feature FEAT_CSV2_2 is implemented but FEAT_CSV2_3 is not
 *          implemented.
 * 0b0011 - Feature FEAT_CSV2_3 is implemented.
 */

CREATE_FEATURE_FUNCS(feat_csv2_2, id_aa64pfr0_el1, ID_AA64PFR0_CSV2_SHIFT,
		     ID_AA64PFR0_CSV2_MASK, CSV2_2_IMPLEMENTED, ENABLE_FEAT_CSV2_2,
		     FEAT_ENABLE_NS | FEAT_ENABLE_REALM)
CREATE_FEATURE_FUNCS(feat_csv2_3, id_aa64pfr0_el1, ID_AA64PFR0_CSV2_SHIFT,
		     ID_AA64PFR0_CSV2_MASK, CSV2_3_IMPLEMENTED, ENABLE_FEAT_CSV2_3,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_SPE: Statistical Profiling Extension */
CREATE_PERCPU_FEATURE_FUNCS(feat_spe, id_aa64dfr0_el1, ID_AA64DFR0_PMS_SHIFT,
		     ID_AA64DFR0_PMS_MASK, 1U, ENABLE_SPE_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_SVE: Scalable Vector Extension */
CREATE_FEATURE_FUNCS(feat_sve, id_aa64pfr0_el1, ID_AA64PFR0_SVE_SHIFT,
		     ID_AA64PFR0_SVE_MASK, 1U, ENABLE_SVE_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_RAS: Reliability, Accessibility, Serviceability */
CREATE_FEATURE_FUNCS(feat_ras, id_aa64pfr0_el1, ID_AA64PFR0_RAS_SHIFT,
		     ID_AA64PFR0_RAS_MASK, 1U, ENABLE_FEAT_RAS,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_DIT: Data Independent Timing instructions */
CREATE_FEATURE_FUNCS(feat_dit, id_aa64pfr0_el1, ID_AA64PFR0_DIT_SHIFT,
		     ID_AA64PFR0_DIT_MASK, 1U, ENABLE_FEAT_DIT,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_SYS_REG_TRACE */
CREATE_PERCPU_FEATURE_FUNCS(feat_sys_reg_trace, id_aa64dfr0_el1,
			ID_AA64DFR0_TRACEVER_SHIFT, ID_AA64DFR0_TRACEVER_MASK,
			1U, ENABLE_SYS_REG_TRACE_FOR_NS,
			FEAT_ENABLE_ALL_WORLDS)

/* FEAT_TRF: TraceFilter */
CREATE_PERCPU_FEATURE_FUNCS(feat_trf, id_aa64dfr0_el1, ID_AA64DFR0_TRACEFILT_SHIFT,
		     ID_AA64DFR0_TRACEFILT_MASK, 1U, ENABLE_TRF_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_NV2: Enhanced Nested Virtualization */
CREATE_FEATURE_FUNCS(feat_nv2, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_NV_SHIFT,
		     ID_AA64MMFR2_EL1_NV_MASK, NV2_IMPLEMENTED, CTX_INCLUDE_NEVE_REGS,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_BRBE: Branch Record Buffer Extension */
CREATE_PERCPU_FEATURE_FUNCS(feat_brbe, id_aa64dfr0_el1, ID_AA64DFR0_BRBE_SHIFT,
		     ID_AA64DFR0_BRBE_MASK, 1U, ENABLE_BRBE_FOR_NS,
		     FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

/* FEAT_TRBE: Trace Buffer Extension */
_CREATE_FEATURE_PRESENT(feat_trbe, id_aa64dfr0_el1, ID_AA64DFR0_TRACEBUFFER_SHIFT,
		       ID_AA64DFR0_TRACEBUFFER_MASK, 1U)

CREATE_FEATURE_SUPPORTED(feat_trbe, is_feat_trbe_present, ENABLE_TRBE_FOR_NS)

CREATE_PERCPU_IDREG_UPDATE(feat_trbe, id_aa64dfr0_el1, ID_AA64DFR0_TRACEBUFFER_SHIFT,
			ID_AA64DFR0_TRACEBUFFER_MASK,
			ENABLE_TRBE_FOR_NS && !check_if_trbe_disable_affected_core(),
			FEAT_ENABLE_NS)

/* FEAT_SME_FA64: Full A64 Instruction support in streaming SVE mode */
CREATE_FEATURE_PRESENT(feat_sme_fa64, id_aa64smfr0_el1, ID_AA64SMFR0_EL1_SME_FA64_SHIFT,
		    ID_AA64SMFR0_EL1_SME_FA64_MASK, 1U,
		    FEAT_ENABLE_ALL_WORLDS)

/* FEAT_SMEx: Scalar Matrix Extension */
CREATE_FEATURE_FUNCS(feat_sme, id_aa64pfr1_el1, ID_AA64PFR1_EL1_SME_SHIFT,
		     ID_AA64PFR1_EL1_SME_MASK, 1U, ENABLE_SME_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_sme2, id_aa64pfr1_el1, ID_AA64PFR1_EL1_SME_SHIFT,
		     ID_AA64PFR1_EL1_SME_MASK, SME2_IMPLEMENTED, ENABLE_SME2_FOR_NS,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_LS64_ACCDATA: Support for 64-byte EL0 stores with status */
CREATE_FEATURE_FUNCS(feat_ls64_accdata, id_aa64isar1_el1, ID_AA64ISAR1_LS64_SHIFT,
		     ID_AA64ISAR1_LS64_MASK, LS64_ACCDATA_IMPLEMENTED,
		     ENABLE_FEAT_LS64_ACCDATA, FEAT_ENABLE_ALL_WORLDS)

/* FEAT_AIE: Memory Attribute Index Enhancement */
CREATE_FEATURE_FUNCS(feat_aie, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_AIE_SHIFT,
		     ID_AA64MMFR3_EL1_AIE_MASK, 1U, ENABLE_FEAT_AIE,
		     FEAT_ENABLE_NS)

/* FEAT_PFAR: Physical Fault Address Register Extension */
CREATE_FEATURE_FUNCS(feat_pfar, id_aa64pfr1_el1, ID_AA64PFR1_EL1_PFAR_SHIFT,
		     ID_AA64PFR1_EL1_PFAR_MASK, 1U, ENABLE_FEAT_PFAR,
		     FEAT_ENABLE_NS)

/* FEAT_IDTE3: Trapping lower EL ID Register access to EL3 */
CREATE_FEATURE_FUNCS(feat_idte3, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_IDS_SHIFT,
		     ID_AA64MMFR2_EL1_IDS_MASK, 2U, ENABLE_FEAT_IDTE3,
		     FEAT_ENABLE_ALL_WORLDS)

/* FEAT_LSE: Atomic instructions */
CREATE_FEATURE_FUNCS(feat_lse, id_aa64isar0_el1, ID_AA64ISAR0_ATOMIC_SHIFT,
		     ID_AA64ISAR0_ATOMIC_MASK, 1U, USE_SPINLOCK_CAS,
		     FEAT_ENABLE_ALL_WORLDS)


/*******************************************************************************
 * Function to get hardware granularity support
 ******************************************************************************/

__attribute__((always_inline))
static inline bool is_feat_tgran4K_present(void)
{
	unsigned int tgranx = ISOLATE_FIELD(read_id_aa64mmfr0_el1(),
			     ID_AA64MMFR0_EL1_TGRAN4_SHIFT, ID_REG_FIELD_MASK);
	return (tgranx < 8U);
}

CREATE_FEATURE_PRESENT(feat_tgran16K, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_TGRAN16_SHIFT,
		       ID_AA64MMFR0_EL1_TGRAN16_MASK, TGRAN16_IMPLEMENTED,
		       FEAT_ENABLE_ALL_WORLDS)

__attribute__((always_inline))
static inline bool is_feat_tgran64K_present(void)
{
	unsigned int tgranx = ISOLATE_FIELD(read_id_aa64mmfr0_el1(),
			     ID_AA64MMFR0_EL1_TGRAN64_SHIFT, ID_REG_FIELD_MASK);
	return (tgranx < 8U);
}

/* FEAT_PMUV3 */
_CREATE_FEATURE_PRESENT(feat_pmuv3, id_aa64dfr0_el1, ID_AA64DFR0_PMUVER_SHIFT,
		      ID_AA64DFR0_PMUVER_MASK, 1U)

/* FEAT_MTPMU */
__attribute__((always_inline))
static inline bool is_feat_mtpmu_present(void)
{
	unsigned int mtpmu = ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_MTPMU_SHIFT,
					   ID_AA64DFR0_MTPMU_MASK);
	return (mtpmu != 0U) && (mtpmu != MTPMU_NOT_IMPLEMENTED);
}

CREATE_FEATURE_SUPPORTED(feat_mtpmu, is_feat_mtpmu_present, DISABLE_MTPMU)

CREATE_PERCPU_IDREG_UPDATE(feat_mtpmu, id_aa64dfr0_el1, ID_AA64DFR0_MTPMU_SHIFT,
			   ID_AA64DFR0_MTPMU_MASK, DISABLE_MTPMU,
			   FEAT_ENABLE_ALL_WORLDS)

/*************************************************************************
 * Function to identify the presence of FEAT_GCIE (GICv5 CPU interface
 * extension).
 ************************************************************************/
CREATE_FEATURE_FUNCS(feat_gcie, id_aa64pfr2_el1, ID_AA64PFR2_EL1_GCIE_SHIFT,
		     ID_AA64PFR2_EL1_GCIE_MASK, 1U, ENABLE_FEAT_GCIE,
		     FEAT_ENABLE_ALL_WORLDS)

CREATE_FEATURE_FUNCS(feat_cpa2, id_aa64isar3_el1, ID_AA64ISAR3_EL1_CPA_SHIFT,
		     ID_AA64ISAR3_EL1_CPA_MASK, CPA2_IMPLEMENTED,
		     ENABLE_FEAT_CPA2, FEAT_ENABLE_ALL_WORLDS)

/* FEAT_UINJ: Injection of Undefined Instruction exceptions */
CREATE_FEATURE_FUNCS(feat_uinj, id_aa64pfr2_el1, ID_AA64PFR2_EL1_UINJ_SHIFT,
		     ID_AA64PFR2_EL1_UINJ_MASK, UINJ_IMPLEMENTED,
		     ENABLE_FEAT_UINJ, FEAT_ENABLE_ALL_WORLDS)

/* FEAT_MORELLO_PRESENT */
CREATE_FEATURE_FUNCS(feat_morello, id_aa64pfr1_el1, ID_AA64PFR1_EL1_CE_SHIFT,
		     ID_AA64PFR1_EL1_CE_MASK, MORELLO_EXTENSION_IMPLEMENTED,
			 ENABLE_FEAT_MORELLO, FEAT_ENABLE_ALL_WORLDS)
#endif /* ARCH_FEATURES_H */
