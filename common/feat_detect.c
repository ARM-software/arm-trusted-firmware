/*
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
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

/************************************************
 * Feature : FEAT_MTE (Memory Tagging Extension)
 ***********************************************/
static void read_feat_mte(void)
{
#if (CTX_INCLUDE_MTE_REGS == FEAT_STATE_ALWAYS)
	unsigned int mte = get_armv8_5_mte_support();

	feat_detect_panic((mte != MTE_UNIMPLEMENTED), "MTE");
#endif
}

/****************************************************
 * Feature : FEAT_BTI (Branch Target Identification)
 ***************************************************/
static void read_feat_bti(void)
{
#if (ENABLE_BTI == FEAT_STATE_ALWAYS)
	feat_detect_panic(is_armv8_5_bti_present(), "BTI");
#endif
}

/**************************************************
 * Feature : FEAT_RME (Realm Management Extension)
 *************************************************/
static void read_feat_rme(void)
{
#if (ENABLE_RME == FEAT_STATE_ALWAYS)
	feat_detect_panic((get_armv9_2_feat_rme_support() !=
			ID_AA64PFR0_FEAT_RME_NOT_SUPPORTED), "RME");
#endif
}

/******************************************************************
 * Feature : FEAT_RNG_TRAP (Trapping support for RNDR/RNDRRS)
 *****************************************************************/
static void read_feat_rng_trap(void)
{
#if (ENABLE_FEAT_RNG_TRAP == FEAT_STATE_ALWAYS)
	feat_detect_panic(is_feat_rng_trap_present(), "RNG_TRAP");
#endif
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
	read_feat_mte();
	check_feature(ENABLE_FEAT_RNG, read_feat_rng_id_field(), "RNG", 1, 1);
	read_feat_bti();
	read_feat_rng_trap();

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
	check_feature(ENABLE_FEAT_MTE_PERM, read_feat_mte_perm_id_field(),
		      "MTE_PERM", 1, 1);

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

	read_feat_rme();

	if (tainted) {
		panic();
	}
}
