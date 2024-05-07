/*
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <common/debug.h>
#include <common/feat_detect.h>

static bool tainted;

/*******************************************************************************
 * This section lists the wrapper modules for each feature to evaluate the
 * feature states (FEAT_STATE_ALWAYS and FEAT_STATE_CHECK) and perform
 * necessary action as below:
 *
 * It verifies whether the FEAT_XXX (eg: FEAT_SB) is supported by the PE or not.
 * Without this check an exception would occur during context save/restore
 * routines, if the feature is enabled but not supported by PE.
 ******************************************************************************/

#define feat_detect_panic(a, b)		((a) ? (void)0 : feature_panic(b))

/*******************************************************************************
 * Function : feature_panic
 * Customised panic function with error logging mechanism to list the feature
 * not supported by the PE.
 ******************************************************************************/
static inline void feature_panic(char *feat_name)
{
	ERROR("FEAT_%s not supported by the PE\n", feat_name);
	panic();
}

/*******************************************************************************
 * Function : check_feature
 * Check for a valid combination of build time flags (ENABLE_FEAT_xxx) and
 * feature availability on the hardware. <min> is the smallest feature
 * ID field value that is required for that feature.
 * Triggers a panic later if a feature is forcefully enabled, but not
 * available on the PE. Also will panic if the hardware feature ID field
 * is larger than the maximum known and supported number, specified by <max>.
 *
 * We force inlining here to let the compiler optimise away the whole check
 * if the feature is disabled at build time (FEAT_STATE_DISABLED).
 ******************************************************************************/
static inline void __attribute((__always_inline__))
check_feature(int state, unsigned long field, const char *feat_name,
	      unsigned int min, unsigned int max)
{
	if (state == FEAT_STATE_ALWAYS && field < min) {
		ERROR("FEAT_%s not supported by the PE\n", feat_name);
		tainted = true;
	}
	if (state >= FEAT_STATE_ALWAYS && field > max) {
		ERROR("FEAT_%s is version %ld, but is only known up to version %d\n",
		      feat_name, field, max);
		tainted = true;
	}
}

/************************************************
 * Feature : FEAT_PAUTH (Pointer Authentication)
 ***********************************************/
static void read_feat_pauth(void)
{
#if (ENABLE_PAUTH == FEAT_STATE_ALWAYS) || (CTX_INCLUDE_PAUTH_REGS == FEAT_STATE_ALWAYS)
	feat_detect_panic(is_armv8_3_pauth_present(), "PAUTH");
#endif
}

static unsigned int read_feat_rng_trap_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_RNDR_TRAP_SHIFT,
			     ID_AA64PFR1_EL1_RNDR_TRAP_MASK);
}

static unsigned int read_feat_bti_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_BT_SHIFT,
			     ID_AA64PFR1_EL1_BT_MASK);
}

static unsigned int read_feat_sb_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar1_el1(), ID_AA64ISAR1_SB_SHIFT,
			     ID_AA64ISAR1_SB_MASK);
}

static unsigned int read_feat_csv2_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_CSV2_SHIFT,
			     ID_AA64PFR0_CSV2_MASK);
}

static unsigned int read_feat_pmuv3_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_PMUVER_SHIFT,
			     ID_AA64DFR0_PMUVER_MASK);
}

static unsigned int read_feat_vhe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr1_el1(), ID_AA64MMFR1_EL1_VHE_SHIFT,
			     ID_AA64MMFR1_EL1_VHE_MASK);
}

static unsigned int read_feat_sve_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_SVE_SHIFT,
			     ID_AA64PFR0_SVE_MASK);
}

static unsigned int read_feat_ras_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_RAS_SHIFT,
			     ID_AA64PFR0_RAS_MASK);
}

static unsigned int read_feat_dit_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_DIT_SHIFT,
			     ID_AA64PFR0_DIT_MASK);
}

static unsigned int  read_feat_amu_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_AMU_SHIFT,
			     ID_AA64PFR0_AMU_MASK);
}

static unsigned int read_feat_mpam_version(void)
{
	return (unsigned int)((((read_id_aa64pfr0_el1() >>
		ID_AA64PFR0_MPAM_SHIFT) & ID_AA64PFR0_MPAM_MASK) << 4) |
			((read_id_aa64pfr1_el1() >>
		ID_AA64PFR1_MPAM_FRAC_SHIFT) & ID_AA64PFR1_MPAM_FRAC_MASK));
}

static unsigned int read_feat_nv_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr2_el1(), ID_AA64MMFR2_EL1_NV_SHIFT,
			     ID_AA64MMFR2_EL1_NV_MASK);
}

static unsigned int read_feat_sel2_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_SEL2_SHIFT,
			     ID_AA64PFR0_SEL2_MASK);
}

static unsigned int read_feat_trf_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_TRACEFILT_SHIFT,
			     ID_AA64DFR0_TRACEFILT_MASK);
}
static unsigned int get_armv8_5_mte_support(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_MTE_SHIFT,
			     ID_AA64PFR1_EL1_MTE_MASK);
}
static unsigned int read_feat_rng_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar0_el1(), ID_AA64ISAR0_RNDR_SHIFT,
			     ID_AA64ISAR0_RNDR_MASK);
}
static unsigned int read_feat_fgt_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr0_el1(), ID_AA64MMFR0_EL1_FGT_SHIFT,
			     ID_AA64MMFR0_EL1_FGT_MASK);
}
static unsigned int read_feat_ecv_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr0_el1(), ID_AA64MMFR0_EL1_ECV_SHIFT,
			     ID_AA64MMFR0_EL1_ECV_MASK);
}
static unsigned int read_feat_twed_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr1_el1(), ID_AA64MMFR1_EL1_TWED_SHIFT,
			     ID_AA64MMFR1_EL1_TWED_MASK);
}

static unsigned int read_feat_hcx_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr1_el1(), ID_AA64MMFR1_EL1_HCX_SHIFT,
			     ID_AA64MMFR1_EL1_HCX_MASK);
}
static unsigned int read_feat_tcr2_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_TCRX_SHIFT,
			     ID_AA64MMFR3_EL1_TCRX_MASK);
}
static unsigned int read_feat_s2pie_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_S2PIE_SHIFT,
			     ID_AA64MMFR3_EL1_S2PIE_MASK);
}
static unsigned int read_feat_s1pie_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_S1PIE_SHIFT,
			     ID_AA64MMFR3_EL1_S1PIE_MASK);
}
static unsigned int read_feat_s2poe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_S2POE_SHIFT,
			     ID_AA64MMFR3_EL1_S2POE_MASK);
}
static unsigned int read_feat_s1poe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_S1POE_SHIFT,
			     ID_AA64MMFR3_EL1_S1POE_MASK);
}
static unsigned int read_feat_brbe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_BRBE_SHIFT,
			     ID_AA64DFR0_BRBE_MASK);
}
static unsigned int read_feat_trbe_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_TRACEBUFFER_SHIFT,
			     ID_AA64DFR0_TRACEBUFFER_MASK);
}
static unsigned int read_feat_sme_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_SME_SHIFT,
			     ID_AA64PFR1_EL1_SME_MASK);
}
static unsigned int read_feat_gcs_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_GCS_SHIFT,
			     ID_AA64PFR1_EL1_GCS_MASK);
}

static unsigned int read_feat_rme_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr0_el1(), ID_AA64PFR0_FEAT_RME_SHIFT,
			     ID_AA64PFR0_FEAT_RME_MASK);
}

static unsigned int read_feat_pan_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr1_el1(), ID_AA64MMFR1_EL1_PAN_SHIFT,
			     ID_AA64MMFR1_EL1_PAN_MASK);
}

static unsigned int read_feat_mtpmu_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_MTPMU_SHIFT,
			     ID_AA64DFR0_MTPMU_MASK);

}

/***********************************************************************************
 * TF-A supports many Arm architectural features starting from arch version
 * (8.0 till 8.7+). These features are mostly enabled through build flags. This
 * mechanism helps in validating these build flags in the early boot phase
 * either in BL1 or BL31 depending on the platform and assists in identifying
 * and notifying the features which are enabled but not supported by the PE.
 *
 * It reads all the enabled features ID-registers and ensures the features
 * are supported by the PE.
 * In case if they aren't it stops booting at an early phase and logs the error
 * messages, notifying the platforms about the features that are not supported.
 *
 * Further the procedure is implemented with a tri-state approach for each feature:
 * ENABLE_FEAT_xxx = 0 : The feature is disabled statically at compile time
 * ENABLE_FEAT_xxx = 1 : The feature is enabled and must be present in hardware.
 *                       There will be panic if feature is not present at cold boot.
 * ENABLE_FEAT_xxx = 2 : The feature is enabled but dynamically enabled at runtime
 *                       depending on hardware capability.
 *
 * For better readability, state values are defined with macros, namely:
 * { FEAT_STATE_DISABLED, FEAT_STATE_ALWAYS, FEAT_STATE_CHECK }, taking values
 * { 0, 1, 2 }, respectively, as their naming.
 **********************************************************************************/
void detect_arch_features(void)
{
	tainted = false;

	/* v8.0 features */
	check_feature(ENABLE_FEAT_SB, read_feat_sb_id_field(), "SB", 1, 1);
	check_feature(ENABLE_FEAT_CSV2_2, read_feat_csv2_id_field(),
		      "CSV2_2", 2, 3);
	/*
	 * Even though the PMUv3 is an OPTIONAL feature, it is always
	 * implemented and Arm prescribes so. So assume it will be there and do
	 * away with a flag for it. This is used to check minor PMUv3px
	 * revisions so that we catch them as they come along
	 */
	check_feature(FEAT_STATE_ALWAYS, read_feat_pmuv3_id_field(),
		      "PMUv3", 1, ID_AA64DFR0_PMUVER_PMUV3P7);

	/* v8.1 features */
	check_feature(ENABLE_FEAT_PAN, read_feat_pan_id_field(), "PAN", 1, 3);
	check_feature(ENABLE_FEAT_VHE, read_feat_vhe_id_field(), "VHE", 1, 1);

	/* v8.2 features */
	check_feature(ENABLE_SVE_FOR_NS, read_feat_sve_id_field(),
		      "SVE", 1, 1);
	check_feature(ENABLE_FEAT_RAS, read_feat_ras_id_field(), "RAS", 1, 2);

	/* v8.3 features */
	/* TODO: Pauth yet to convert to tri-state feat detect logic */
	read_feat_pauth();

	/* v8.4 features */
	check_feature(ENABLE_FEAT_DIT, read_feat_dit_id_field(), "DIT", 1, 1);
	check_feature(ENABLE_FEAT_AMU, read_feat_amu_id_field(),
		      "AMUv1", 1, 2);
	check_feature(ENABLE_FEAT_MPAM, read_feat_mpam_version(),
		      "MPAM", 1, 17);
	check_feature(CTX_INCLUDE_NEVE_REGS, read_feat_nv_id_field(),
		      "NV2", 2, 2);
	check_feature(ENABLE_FEAT_SEL2, read_feat_sel2_id_field(),
		      "SEL2", 1, 1);
	check_feature(ENABLE_TRF_FOR_NS, read_feat_trf_id_field(),
		      "TRF", 1, 1);

	/* v8.5 features */
	check_feature(ENABLE_FEAT_MTE2, get_armv8_5_mte_support(), "MTE2",
		      MTE_IMPLEMENTED_ELX, MTE_IMPLEMENTED_ASY);
	check_feature(ENABLE_FEAT_RNG, read_feat_rng_id_field(), "RNG", 1, 1);
	check_feature(ENABLE_BTI, read_feat_bti_id_field(), "BTI", 1, 1);
	check_feature(ENABLE_FEAT_RNG_TRAP, read_feat_rng_trap_id_field(),
		      "RNG_TRAP", 1, 1);

	/* v8.6 features */
	check_feature(ENABLE_FEAT_AMUv1p1, read_feat_amu_id_field(),
		      "AMUv1p1", 2, 2);
	check_feature(ENABLE_FEAT_FGT, read_feat_fgt_id_field(), "FGT", 1, 1);
	check_feature(ENABLE_FEAT_ECV, read_feat_ecv_id_field(), "ECV", 1, 2);
	check_feature(ENABLE_FEAT_TWED, read_feat_twed_id_field(),
		      "TWED", 1, 1);

	/*
	 * even though this is a "DISABLE" it does confusingly perform feature
	 * enablement duties like all other flags here. Check it against the HW
	 * feature when we intend to diverge from the default behaviour
	 */
	check_feature(DISABLE_MTPMU, read_feat_mtpmu_id_field(), "MTPMU", 1, 1);

	/* v8.7 features */
	check_feature(ENABLE_FEAT_HCX, read_feat_hcx_id_field(), "HCX", 1, 1);

	/* v8.9 features */
	check_feature(ENABLE_FEAT_TCR2, read_feat_tcr2_id_field(),
		      "TCR2", 1, 1);
	check_feature(ENABLE_FEAT_S2PIE, read_feat_s2pie_id_field(),
		      "S2PIE", 1, 1);
	check_feature(ENABLE_FEAT_S1PIE, read_feat_s1pie_id_field(),
		      "S1PIE", 1, 1);
	check_feature(ENABLE_FEAT_S2POE, read_feat_s2poe_id_field(),
		      "S2POE", 1, 1);
	check_feature(ENABLE_FEAT_S1POE, read_feat_s1poe_id_field(),
		      "S1POE", 1, 1);
	check_feature(ENABLE_FEAT_CSV2_3, read_feat_csv2_id_field(),
		      "CSV2_3", 3, 3);

	/* v9.0 features */
	check_feature(ENABLE_BRBE_FOR_NS, read_feat_brbe_id_field(),
		      "BRBE", 1, 2);
	check_feature(ENABLE_TRBE_FOR_NS, read_feat_trbe_id_field(),
		      "TRBE", 1, 1);

	/* v9.2 features */
	check_feature(ENABLE_SME_FOR_NS, read_feat_sme_id_field(),
		      "SME", 1, 2);
	check_feature(ENABLE_SME2_FOR_NS, read_feat_sme_id_field(),
		      "SME2", 2, 2);

	/* v9.4 features */
	check_feature(ENABLE_FEAT_GCS, read_feat_gcs_id_field(), "GCS", 1, 1);
	check_feature(ENABLE_RME, read_feat_rme_id_field(), "RME", 1, 1);

	if (tainted) {
		panic();
	}
}
