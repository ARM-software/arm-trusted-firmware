/*
 * Copyright (c) 2019-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_FEATURES_H
#define ARCH_FEATURES_H

#include <stdbool.h>

#include <arch_helpers.h>
#include <common/feat_detect.h>

#define ISOLATE_FIELD(reg, feat)					\
	((unsigned int)(((reg) >> (feat)) & ID_REG_FIELD_MASK))

#define CREATE_FEATURE_FUNCS_VER(name, read_func, idvalue, guard)	\
static inline bool is_ ## name ## _supported(void)			\
{									\
	if ((guard) == FEAT_STATE_DISABLED) {				\
		return false;						\
	}								\
	if ((guard) == FEAT_STATE_ALWAYS) {				\
		return true;						\
	}								\
	return read_func() >= (idvalue);				\
}

#define CREATE_FEATURE_FUNCS(name, idreg, idfield, guard)		\
static unsigned int read_ ## name ## _id_field(void)			\
{									\
	return ISOLATE_FIELD(read_ ## idreg(), idfield);		\
}									\
CREATE_FEATURE_FUNCS_VER(name, read_ ## name ## _id_field, 1U, guard)

static inline bool is_armv7_gentimer_present(void)
{
	/* The Generic Timer is always present in an ARMv8-A implementation */
	return true;
}

CREATE_FEATURE_FUNCS(feat_pan, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_PAN_SHIFT,
		     ENABLE_FEAT_PAN)
static inline bool is_feat_pan_present(void)
{
	return read_feat_pan_id_field() != 0U;
}

CREATE_FEATURE_FUNCS(feat_vhe, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_VHE_SHIFT,
		     ENABLE_FEAT_VHE)

static inline bool is_armv8_2_ttcnp_present(void)
{
	return ((read_id_aa64mmfr2_el1() >> ID_AA64MMFR2_EL1_CNP_SHIFT) &
		ID_AA64MMFR2_EL1_CNP_MASK) != 0U;
}

static inline bool is_feat_uao_present(void)
{
	return ((read_id_aa64mmfr2_el1() >> ID_AA64MMFR2_EL1_UAO_SHIFT) &
		ID_AA64MMFR2_EL1_UAO_MASK) != 0U;
}

static inline bool is_feat_pacqarma3_present(void)
{
	uint64_t mask_id_aa64isar2 =
			(ID_AA64ISAR2_GPA3_MASK << ID_AA64ISAR2_GPA3_SHIFT) |
			(ID_AA64ISAR2_APA3_MASK << ID_AA64ISAR2_APA3_SHIFT);

	/* If any of the fields is not zero, QARMA3 algorithm is present */
	return (read_id_aa64isar2_el1() & mask_id_aa64isar2) != 0U;
}

static inline bool is_armv8_3_pauth_present(void)
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

static inline bool is_armv8_4_ttst_present(void)
{
	return ((read_id_aa64mmfr2_el1() >> ID_AA64MMFR2_EL1_ST_SHIFT) &
		ID_AA64MMFR2_EL1_ST_MASK) == 1U;
}

static inline bool is_armv8_5_bti_present(void)
{
	return ((read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_BT_SHIFT) &
		ID_AA64PFR1_EL1_BT_MASK) == BTI_IMPLEMENTED;
}

static inline unsigned int get_armv8_5_mte_support(void)
{
	return ((read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_MTE_SHIFT) &
		ID_AA64PFR1_EL1_MTE_MASK);
}
static inline unsigned int is_feat_mte2_present(void)
{
	return get_armv8_5_mte_support() >= MTE_IMPLEMENTED_ELX;
}

static inline bool is_feat_ssbs_present(void)
{
	return ((read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_SSBS_SHIFT) &
		ID_AA64PFR1_EL1_SSBS_MASK) != SSBS_NOT_IMPLEMENTED;
}

static inline bool is_feat_nmi_present(void)
{
	return ((read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_NMI_SHIFT) &
		ID_AA64PFR1_EL1_NMI_MASK) == NMI_IMPLEMENTED;
}

static inline bool is_feat_gcs_present(void)
{
	return ((read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_GCS_SHIFT) &
		ID_AA64PFR1_EL1_GCS_MASK) == GCS_IMPLEMENTED;
}

static inline bool is_feat_ebep_present(void)
{
	return ((read_id_aa64dfr1_el1() >> ID_AA64DFR1_EBEP_SHIFT) &
		ID_AA64DFR1_EBEP_MASK) == EBEP_IMPLEMENTED;
}

static inline bool is_feat_sebep_present(void)
{
	return ((read_id_aa64dfr0_el1() >> ID_AA64DFR0_SEBEP_SHIFT) &
		ID_AA64DFR0_SEBEP_MASK) == SEBEP_IMPLEMENTED;
}

CREATE_FEATURE_FUNCS_VER(feat_mte2, get_armv8_5_mte_support, MTE_IMPLEMENTED_ELX,
			 ENABLE_FEAT_MTE2)
CREATE_FEATURE_FUNCS(feat_sel2, id_aa64pfr0_el1, ID_AA64PFR0_SEL2_SHIFT,
		     ENABLE_FEAT_SEL2)
CREATE_FEATURE_FUNCS(feat_twed, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_TWED_SHIFT,
		     ENABLE_FEAT_TWED)
CREATE_FEATURE_FUNCS(feat_fgt, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_FGT_SHIFT,
		     ENABLE_FEAT_FGT)
CREATE_FEATURE_FUNCS(feat_ecv, id_aa64mmfr0_el1, ID_AA64MMFR0_EL1_ECV_SHIFT,
		     ENABLE_FEAT_ECV)
CREATE_FEATURE_FUNCS_VER(feat_ecv_v2, read_feat_ecv_id_field,
			 ID_AA64MMFR0_EL1_ECV_SELF_SYNCH, ENABLE_FEAT_ECV)

CREATE_FEATURE_FUNCS(feat_rng, id_aa64isar0_el1, ID_AA64ISAR0_RNDR_SHIFT,
		     ENABLE_FEAT_RNG)
CREATE_FEATURE_FUNCS(feat_tcr2, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_TCRX_SHIFT,
		     ENABLE_FEAT_TCR2)

CREATE_FEATURE_FUNCS(feat_s2poe, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S2POE_SHIFT,
		     ENABLE_FEAT_S2POE)
CREATE_FEATURE_FUNCS(feat_s1poe, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S1POE_SHIFT,
		     ENABLE_FEAT_S1POE)
static inline bool is_feat_sxpoe_supported(void)
{
	return is_feat_s1poe_supported() || is_feat_s2poe_supported();
}

CREATE_FEATURE_FUNCS(feat_s2pie, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S2PIE_SHIFT,
		     ENABLE_FEAT_S2PIE)
CREATE_FEATURE_FUNCS(feat_s1pie, id_aa64mmfr3_el1, ID_AA64MMFR3_EL1_S1PIE_SHIFT,
		     ENABLE_FEAT_S1PIE)
static inline bool is_feat_sxpie_supported(void)
{
	return is_feat_s1pie_supported() || is_feat_s2pie_supported();
}

/* FEAT_GCS: Guarded Control Stack */
CREATE_FEATURE_FUNCS(feat_gcs, id_aa64pfr1_el1, ID_AA64PFR1_EL1_GCS_SHIFT,
		     ENABLE_FEAT_GCS)

/* FEAT_AMU: Activity Monitors Extension */
CREATE_FEATURE_FUNCS(feat_amu, id_aa64pfr0_el1, ID_AA64PFR0_AMU_SHIFT,
		     ENABLE_FEAT_AMU)
CREATE_FEATURE_FUNCS_VER(feat_amuv1p1, read_feat_amu_id_field,
			 ID_AA64PFR0_AMU_V1P1, ENABLE_FEAT_AMUv1p1)

/*
 * Return MPAM version:
 *
 * 0x00: None Armv8.0 or later
 * 0x01: v0.1 Armv8.4 or later
 * 0x10: v1.0 Armv8.2 or later
 * 0x11: v1.1 Armv8.4 or later
 *
 */
static inline unsigned int read_feat_mpam_version(void)
{
	return (unsigned int)((((read_id_aa64pfr0_el1() >>
		ID_AA64PFR0_MPAM_SHIFT) & ID_AA64PFR0_MPAM_MASK) << 4) |
				((read_id_aa64pfr1_el1() >>
		ID_AA64PFR1_MPAM_FRAC_SHIFT) & ID_AA64PFR1_MPAM_FRAC_MASK));
}

CREATE_FEATURE_FUNCS_VER(feat_mpam, read_feat_mpam_version, 1U,
			 ENABLE_FEAT_MPAM)

/* FEAT_HCX: Extended Hypervisor Configuration Register */
CREATE_FEATURE_FUNCS(feat_hcx, id_aa64mmfr1_el1, ID_AA64MMFR1_EL1_HCX_SHIFT,
		     ENABLE_FEAT_HCX)

static inline bool is_feat_rng_trap_present(void)
{
	return (((read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_RNDR_TRAP_SHIFT) &
			ID_AA64PFR1_EL1_RNDR_TRAP_MASK)
			== RNG_TRAP_IMPLEMENTED);
}

static inline unsigned int get_armv9_2_feat_rme_support(void)
{
	/*
	 * Return the RME version, zero if not supported.  This function can be
	 * used as both an integer value for the RME version or compared to zero
	 * to detect RME presence.
	 */
	return (unsigned int)(read_id_aa64pfr0_el1() >>
		ID_AA64PFR0_FEAT_RME_SHIFT) & ID_AA64PFR0_FEAT_RME_MASK;
}

/*********************************************************************************
 * Function to identify the presence of FEAT_SB (Speculation Barrier Instruction)
 ********************************************************************************/
static inline unsigned int read_feat_sb_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar1_el1(), ID_AA64ISAR1_SB_SHIFT);
}

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
static inline unsigned int read_feat_csv2_id_field(void)
{
	return (unsigned int)(read_id_aa64pfr0_el1() >>
		ID_AA64PFR0_CSV2_SHIFT) & ID_AA64PFR0_CSV2_MASK;
}

CREATE_FEATURE_FUNCS_VER(feat_csv2_2, read_feat_csv2_id_field,
			 CSV2_2_IMPLEMENTED, ENABLE_FEAT_CSV2_2)
CREATE_FEATURE_FUNCS_VER(feat_csv2_3, read_feat_csv2_id_field,
			 CSV2_3_IMPLEMENTED, ENABLE_FEAT_CSV2_3)

/* FEAT_SPE: Statistical Profiling Extension */
CREATE_FEATURE_FUNCS(feat_spe, id_aa64dfr0_el1, ID_AA64DFR0_PMS_SHIFT,
		     ENABLE_SPE_FOR_NS)

/* FEAT_SVE: Scalable Vector Extension */
CREATE_FEATURE_FUNCS(feat_sve, id_aa64pfr0_el1, ID_AA64PFR0_SVE_SHIFT,
		     ENABLE_SVE_FOR_NS)

/* FEAT_RAS: Reliability, Accessibility, Serviceability */
CREATE_FEATURE_FUNCS(feat_ras, id_aa64pfr0_el1,
		     ID_AA64PFR0_RAS_SHIFT, ENABLE_FEAT_RAS)

/* FEAT_DIT: Data Independent Timing instructions */
CREATE_FEATURE_FUNCS(feat_dit, id_aa64pfr0_el1,
		     ID_AA64PFR0_DIT_SHIFT, ENABLE_FEAT_DIT)

CREATE_FEATURE_FUNCS(feat_sys_reg_trace, id_aa64dfr0_el1,
		     ID_AA64DFR0_TRACEVER_SHIFT, ENABLE_SYS_REG_TRACE_FOR_NS)

/* FEAT_TRF: TraceFilter */
CREATE_FEATURE_FUNCS(feat_trf, id_aa64dfr0_el1, ID_AA64DFR0_TRACEFILT_SHIFT,
		     ENABLE_TRF_FOR_NS)

/* FEAT_NV2: Enhanced Nested Virtualization */
CREATE_FEATURE_FUNCS(feat_nv, id_aa64mmfr2_el1, ID_AA64MMFR2_EL1_NV_SHIFT, 0)
CREATE_FEATURE_FUNCS_VER(feat_nv2, read_feat_nv_id_field,
			 NV2_IMPLEMENTED, CTX_INCLUDE_NEVE_REGS)

/* FEAT_BRBE: Branch Record Buffer Extension */
CREATE_FEATURE_FUNCS(feat_brbe, id_aa64dfr0_el1, ID_AA64DFR0_BRBE_SHIFT,
		     ENABLE_BRBE_FOR_NS)

/* FEAT_TRBE: Trace Buffer Extension */
CREATE_FEATURE_FUNCS(feat_trbe, id_aa64dfr0_el1, ID_AA64DFR0_TRACEBUFFER_SHIFT,
		     ENABLE_TRBE_FOR_NS)

static inline unsigned int read_feat_sme_fa64_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64smfr0_el1(),
			     ID_AA64SMFR0_EL1_SME_FA64_SHIFT);
}
/* FEAT_SMEx: Scalar Matrix Extension */
CREATE_FEATURE_FUNCS(feat_sme, id_aa64pfr1_el1, ID_AA64PFR1_EL1_SME_SHIFT,
		     ENABLE_SME_FOR_NS)
CREATE_FEATURE_FUNCS_VER(feat_sme2, read_feat_sme_id_field,
			 SME2_IMPLEMENTED, ENABLE_SME2_FOR_NS)

/*******************************************************************************
 * Function to get hardware granularity support
 ******************************************************************************/

static inline unsigned int read_id_aa64mmfr0_el0_tgran4_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr0_el1(),
			     ID_AA64MMFR0_EL1_TGRAN4_SHIFT);
}

static inline unsigned int read_id_aa64mmfr0_el0_tgran16_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr0_el1(),
			     ID_AA64MMFR0_EL1_TGRAN16_SHIFT);
}

static inline unsigned int read_id_aa64mmfr0_el0_tgran64_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr0_el1(),
			     ID_AA64MMFR0_EL1_TGRAN64_SHIFT);
}

static inline unsigned int read_feat_pmuv3_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_PMUVER_SHIFT);
}

static inline unsigned int read_feat_mtpmu_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_MTPMU_SHIFT);
}

static inline bool is_feat_mtpmu_supported(void)
{
	if (DISABLE_MTPMU == FEAT_STATE_DISABLED) {
		return false;
	}

	if (DISABLE_MTPMU == FEAT_STATE_ALWAYS) {
		return true;
	}

	unsigned int mtpmu = read_feat_mtpmu_id_field();

	return (mtpmu != 0U) && (mtpmu != MTPMU_NOT_IMPLEMENTED);
}

#endif /* ARCH_FEATURES_H */
