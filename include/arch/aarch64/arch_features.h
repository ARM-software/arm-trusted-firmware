/*
 * Copyright (c) 2019-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_FEATURES_H
#define ARCH_FEATURES_H

#include <stdbool.h>

#include <arch_helpers.h>
#include <common/feat_detect.h>

#define ISOLATE_FIELD(reg, feat)					\
	((unsigned int)(((reg) >> (feat ## _SHIFT)) & (feat ## _MASK)))

static inline bool is_armv7_gentimer_present(void)
{
	/* The Generic Timer is always present in an ARMv8-A implementation */
	return true;
}

static inline unsigned int read_feat_pan_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr1_el1(), ID_AA64MMFR1_EL1_PAN);
}

static inline bool is_feat_pan_supported(void)
{
	if (ENABLE_FEAT_PAN == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_PAN == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_pan_id_field() != 0U;
}

static inline unsigned int read_feat_vhe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr1_el1(), ID_AA64MMFR1_EL1_VHE);
}

static inline bool is_feat_vhe_supported(void)
{
	if (ENABLE_FEAT_VHE == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_VHE == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_vhe_id_field() != 0U;
}

static inline bool is_armv8_2_ttcnp_present(void)
{
	return ((read_id_aa64mmfr2_el1() >> ID_AA64MMFR2_EL1_CNP_SHIFT) &
		ID_AA64MMFR2_EL1_CNP_MASK) != 0U;
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

static inline unsigned int read_feat_sel2_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_SEL2);
}

static inline bool is_feat_sel2_supported(void)
{
	if (ENABLE_FEAT_SEL2 == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_SEL2 == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_sel2_id_field() != 0U;
}

static inline unsigned int read_feat_twed_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr1_el1(), ID_AA64MMFR1_EL1_TWED);
}

static inline bool is_feat_twed_supported(void)
{
	if (ENABLE_FEAT_TWED == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_TWED == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_twed_id_field() != 0U;
}

static unsigned int read_feat_fgt_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr0_el1(), ID_AA64MMFR0_EL1_FGT);
}

static inline bool is_feat_fgt_supported(void)
{
	if (ENABLE_FEAT_FGT == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_FGT == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_fgt_id_field() != 0U;
}

static unsigned int read_feat_ecv_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr0_el1(), ID_AA64MMFR0_EL1_ECV);
}

static inline bool is_feat_ecv_supported(void)
{
	if (ENABLE_FEAT_ECV == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_ECV == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_ecv_id_field() != 0U;
}

static inline bool is_feat_ecv_v2_supported(void)
{
	if (ENABLE_FEAT_ECV == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_ECV == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_ecv_id_field() >= ID_AA64MMFR0_EL1_ECV_SELF_SYNCH;
}

static unsigned int read_feat_rng_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar0_el1(), ID_AA64ISAR0_RNDR);
}

static inline bool is_feat_rng_supported(void)
{
	if (ENABLE_FEAT_RNG == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_RNG == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_rng_id_field() != 0U;
}

static unsigned int read_feat_tcrx_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_TCRX);
}

static inline bool is_feat_tcr2_supported(void)
{
	if (ENABLE_FEAT_TCR2 == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_TCR2 == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_tcrx_id_field() != 0U;
}

static unsigned int read_feat_s2poe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_S2POE);
}

static inline bool is_feat_s2poe_supported(void)
{
	if (ENABLE_FEAT_S2POE == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_S2POE == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_s2poe_id_field() != 0U;
}

static unsigned int read_feat_s1poe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_S1POE);
}

static inline bool is_feat_s1poe_supported(void)
{
	if (ENABLE_FEAT_S1POE == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_S1POE == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_s1poe_id_field() != 0U;
}

static inline bool is_feat_sxpoe_supported(void)
{
	return is_feat_s1poe_supported() || is_feat_s2poe_supported();
}

static unsigned int read_feat_s2pie_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_S2PIE);
}

static inline bool is_feat_s2pie_supported(void)
{
	if (ENABLE_FEAT_S2PIE == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_S2PIE == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_s2pie_id_field() != 0U;
}

static unsigned int read_feat_s1pie_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_S1PIE);
}

static inline bool is_feat_s1pie_supported(void)
{
	if (ENABLE_FEAT_S1PIE == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_S1PIE == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_s1pie_id_field() != 0U;
}

static inline bool is_feat_sxpie_supported(void)
{
	return is_feat_s1pie_supported() || is_feat_s2pie_supported();
}

static unsigned int read_feat_gcs_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_GCS);
}

static inline bool is_feat_gcs_supported(void)
{
	if (ENABLE_FEAT_GCS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_GCS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_gcs_id_field() != 0U;
}

/*******************************************************************************
 * Functions to identify the presence of the Activity Monitors Extension
 ******************************************************************************/
static unsigned int read_feat_amu_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_AMU);
}

static inline bool is_feat_amu_supported(void)
{
	if (ENABLE_FEAT_AMU == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_AMU == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_amu_id_field() >= ID_AA64PFR0_AMU_V1;
}

static inline bool is_feat_amuv1p1_supported(void)
{
	if (ENABLE_FEAT_AMUv1p1 == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_AMUv1p1 == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_amu_id_field() >= ID_AA64PFR0_AMU_V1P1;
}

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

static inline bool is_feat_mpam_supported(void)
{
	if (ENABLE_MPAM_FOR_LOWER_ELS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_MPAM_FOR_LOWER_ELS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_mpam_version() != 0U;
}

static inline unsigned int read_feat_hcx_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr1_el1(), ID_AA64MMFR1_EL1_HCX);
}

static inline bool is_feat_hcx_supported(void)
{
	if (ENABLE_FEAT_HCX == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_HCX == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_hcx_id_field() != 0U;
}

static inline bool is_feat_rng_trap_present(void)
{
	return (((read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_RNDR_TRAP_SHIFT) &
			ID_AA64PFR1_EL1_RNDR_TRAP_MASK)
			== ID_AA64PFR1_EL1_RNG_TRAP_SUPPORTED);
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
	return ISOLATE_FIELD(read_id_aa64isar1_el1(), ID_AA64ISAR1_SB);
}

/*********************************************************************************
 * Function to identify the presence of FEAT_CSV2_2 (Cache Speculation Variant 2)
 ********************************************************************************/
static inline unsigned int read_feat_csv2_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_CSV2);
}

static inline bool is_feat_csv2_2_supported(void)
{
	if (ENABLE_FEAT_CSV2_2 == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_CSV2_2 == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_csv2_id_field() >= ID_AA64PFR0_CSV2_2_SUPPORTED;
}

/**********************************************************************************
 * Function to identify the presence of FEAT_SPE (Statistical Profiling Extension)
 *********************************************************************************/
static inline unsigned int read_feat_spe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_PMS);
}

static inline bool is_feat_spe_supported(void)
{
	if (ENABLE_SPE_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_SPE_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_spe_id_field() != 0U;
}

/*******************************************************************************
 * Function to identify the presence of FEAT_SVE (Scalable Vector Extension)
 ******************************************************************************/
static inline unsigned int read_feat_sve_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_SVE);
}

static inline bool is_feat_sve_supported(void)
{
	if (ENABLE_SVE_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_SVE_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_sve_id_field() >= ID_AA64PFR0_SVE_SUPPORTED;
}

static unsigned int read_feat_ras_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_RAS);
}

static inline bool is_feat_ras_supported(void)
{
	if (ENABLE_FEAT_RAS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_RAS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_ras_id_field() != 0U;
}

static unsigned int read_feat_dit_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_DIT);
}

static inline bool is_feat_dit_supported(void)
{
	if (ENABLE_FEAT_DIT == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_DIT == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_dit_id_field() != 0U;
}

static inline unsigned int read_feat_tracever_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_TRACEVER);
}

static inline bool is_feat_sys_reg_trace_supported(void)
{
	if (ENABLE_SYS_REG_TRACE_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_SYS_REG_TRACE_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_tracever_id_field() != 0U;
}

/*************************************************************************
 * Function to identify the presence of FEAT_TRF (TraceLift)
 ************************************************************************/
static inline unsigned int read_feat_trf_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_TRACEFILT);
}

static inline bool is_feat_trf_supported(void)
{
	if (ENABLE_TRF_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_TRF_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_trf_id_field() != 0U;
}

/********************************************************************************
 * Function to identify the presence of FEAT_NV2 (Enhanced Nested Virtualization
 * Support)
 *******************************************************************************/
static inline unsigned int read_feat_nv_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr2_el1(), ID_AA64MMFR2_EL1_NV);
}

static inline bool is_feat_nv2_supported(void)
{
	if (CTX_INCLUDE_NEVE_REGS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (CTX_INCLUDE_NEVE_REGS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_nv_id_field() >= ID_AA64MMFR2_EL1_NV2_SUPPORTED;
}

/*******************************************************************************
 * Function to identify the presence of FEAT_BRBE (Branch Record Buffer
 * Extension)
 ******************************************************************************/
static inline unsigned int read_feat_brbe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_BRBE);
}

static inline bool is_feat_brbe_supported(void)
{
	if (ENABLE_BRBE_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_BRBE_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_brbe_id_field() != 0U;
}

/*******************************************************************************
 * Function to identify the presence of FEAT_TRBE (Trace Buffer Extension)
 ******************************************************************************/
static inline unsigned int read_feat_trbe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_TRACEBUFFER);
}

static inline bool is_feat_trbe_supported(void)
{
	if (ENABLE_TRBE_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_TRBE_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_trbe_id_field() != 0U;

}
/*******************************************************************************
 * Function to identify the presence of FEAT_SMEx (Scalar Matrix Extension)
 ******************************************************************************/
static inline unsigned int read_feat_sme_fa64_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64smfr0_el1(), ID_AA64SMFR0_EL1_SME_FA64);
}

static inline unsigned int read_feat_sme_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_SME);
}

static inline bool is_feat_sme_supported(void)
{
	if (ENABLE_SME_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_SME_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_sme_id_field() >= ID_AA64PFR1_EL1_SME_SUPPORTED;
}

static inline bool is_feat_sme2_supported(void)
{
	if (ENABLE_SME2_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_SME2_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_sme_id_field() >= ID_AA64PFR1_EL1_SME2_SUPPORTED;
}

#endif /* ARCH_FEATURES_H */
