/*
 * Copyright (c) 2022-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <common/debug.h>
#include <common/feat_detect.h>
#include <plat/common/platform.h>

static bool detection_done[PLATFORM_CORE_COUNT] = { false };

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
static inline bool __attribute((__always_inline__))
check_feature(int state, unsigned long field, const char *feat_name,
	      unsigned int min, unsigned int max)
{
	if (state == FEAT_STATE_ALWAYS && field < min) {
		ERROR("FEAT_%s not supported by the PE\n", feat_name);
		return true;
	}
	if (state >= FEAT_STATE_ALWAYS && field > max) {
		ERROR("FEAT_%s is version %ld, but is only known up to version %d\n",
		      feat_name, field, max);
		return true;
	}

	return false;
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

static unsigned int read_feat_debugv8p9_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr0_el1(), ID_AA64DFR0_DEBUGVER_SHIFT,
			     ID_AA64DFR0_DEBUGVER_MASK);
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
static unsigned int read_feat_ls64_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar1_el1(), ID_AA64ISAR1_LS64_SHIFT,
			     ID_AA64ISAR1_LS64_MASK);
}
static unsigned int read_feat_aie_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_AIE_SHIFT,
			     ID_AA64MMFR3_EL1_AIE_MASK);
}
static unsigned int read_feat_pfar_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_PFAR_SHIFT,
			     ID_AA64PFR1_EL1_PFAR_MASK);
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

static unsigned int read_feat_the_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_THE_SHIFT,
			     ID_AA64PFR1_EL1_THE_MASK);
}

static unsigned int read_feat_sctlr2_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_SCTLR2_SHIFT,
			     ID_AA64MMFR3_EL1_SCTLR2_MASK);
}

static unsigned int read_feat_d128_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr3_el1(), ID_AA64MMFR3_EL1_D128_SHIFT,
			     ID_AA64MMFR3_EL1_D128_MASK);
}
static unsigned int read_feat_gcie_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr2_el1(), ID_AA64PFR2_EL1_GCIE_SHIFT,
			     ID_AA64PFR2_EL1_GCIE_MASK);
}

static unsigned int read_feat_ebep_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64dfr1_el1(), ID_AA64DFR1_EBEP_SHIFT,
			     ID_AA64DFR1_EBEP_MASK);
}

static unsigned int read_feat_fpmr_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr2_el1(), ID_AA64PFR2_EL1_FPMR_SHIFT,
			     ID_AA64PFR2_EL1_FPMR_MASK);
}

static unsigned int read_feat_mops_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar2_el1(), ID_AA64ISAR2_EL1_MOPS_SHIFT,
			     ID_AA64ISAR2_EL1_MOPS_MASK);
}

static unsigned int read_feat_fgwte3_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr4_el1(), ID_AA64MMFR4_EL1_FGWTE3_SHIFT,
			     ID_AA64MMFR4_EL1_FGWTE3_MASK);
}

static unsigned int read_feat_cpa_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar3_el1(),
			     ID_AA64ISAR3_EL1_CPA_SHIFT,
			     ID_AA64ISAR3_EL1_CPA_MASK);
}

static unsigned int read_feat_clrbhb_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar2_el1(), ID_AA64ISAR2_CLRBHB_SHIFT,
			     ID_AA64ISAR2_CLRBHB_MASK);
}

static unsigned int read_feat_rme_gdi_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr4_el1(),
			     ID_AA64MMFR4_EL1_RME_GDI_SHIFT,
			     ID_AA64MMFR4_EL1_RME_GDI_MASK);
}

static unsigned int read_feat_idte3_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64mmfr2_el1(), ID_AA64MMFR2_EL1_IDS_SHIFT,
			     ID_AA64MMFR2_EL1_IDS_MASK);
}

static unsigned int read_feat_uinj_id_field(void)
{
    return ISOLATE_FIELD(read_id_aa64pfr2_el1(),
			 ID_AA64PFR2_EL1_UINJ_SHIFT,
			 ID_AA64PFR2_EL1_UINJ_MASK);
}

static unsigned int read_feat_lse_id_field(void)
{
	return ISOLATE_FIELD(read_id_aa64isar0_el1(), ID_AA64ISAR0_ATOMIC_SHIFT,
			     ID_AA64ISAR0_ATOMIC_MASK);
}

static unsigned int read_feat_morello_field(void)
{
	return ISOLATE_FIELD(read_id_aa64pfr1_el1(), ID_AA64PFR1_EL1_CE_SHIFT,
			     ID_AA64PFR1_EL1_CE_MASK);
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
void detect_arch_features(unsigned int core_pos)
{
	/* No need to keep checking after the first time for each core. */
	if (detection_done[core_pos]) {
		return;
	}

	bool tainted = false;

	/* v8.0 features */
	tainted |= check_feature(ENABLE_FEAT_SB, read_feat_sb_id_field(),
				 "SB", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_CSV2_2, read_feat_csv2_id_field(),
				 "CSV2_2", 2, 3);
	tainted |= check_feature(ENABLE_FEAT_CLRBHB, read_feat_clrbhb_id_field(),
				 "CLRBHB", 1, 1);
	/*
	 * Even though the PMUv3 is an OPTIONAL feature, it is always
	 * implemented and Arm prescribes so. So assume it will be there and do
	 * away with a flag for it. This is used to check minor PMUv3px
	 * revisions so that we catch them as they come along
	 */
	tainted |= check_feature(FEAT_STATE_ALWAYS, read_feat_pmuv3_id_field(),
				 "PMUv3", 1, ID_AA64DFR0_PMUVER_PMUV3P9);

	tainted |= check_feature(USE_SPINLOCK_CAS, read_feat_lse_id_field(),
				 "LSE", 2, 2);

	/* v8.1 features */
	tainted |= check_feature(ENABLE_FEAT_PAN, read_feat_pan_id_field(),
				 "PAN", 1, 3);
	tainted |= check_feature(ENABLE_FEAT_VHE, read_feat_vhe_id_field(),
				 "VHE", 1, 1);

	/* v8.2 features */
	tainted |= check_feature(ENABLE_SVE_FOR_NS, read_feat_sve_id_field(),
				 "SVE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_RAS, read_feat_ras_id_field(),
				 "RAS", 1, 2);

	/* v8.3 features */
	/* the PAuth fields are very complicated, no min/max is checked */
	tainted |= check_feature(ENABLE_PAUTH, is_feat_pauth_present(),
				 "PAUTH", 1, 1);

	/* v8.4 features */
	tainted |= check_feature(ENABLE_FEAT_DIT, read_feat_dit_id_field(),
				 "DIT", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_AMU, read_feat_amu_id_field(),
				 "AMUv1", 1, 2);
	tainted |= check_feature(ENABLE_FEAT_MOPS, read_feat_mops_id_field(),
				 "MOPS", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_MPAM, read_feat_mpam_version(),
				 "MPAM", 1, 17);
	tainted |= check_feature(CTX_INCLUDE_NEVE_REGS, read_feat_nv_id_field(),
				 "NV2", 2, 2);
	tainted |= check_feature(ENABLE_FEAT_SEL2, read_feat_sel2_id_field(),
				 "SEL2", 1, 1);
	tainted |= check_feature(ENABLE_TRF_FOR_NS, read_feat_trf_id_field(),
				 "TRF", 1, 1);

	/* v8.5 features */
	tainted |= check_feature(ENABLE_FEAT_MTE2, get_armv8_5_mte_support(),
				 "MTE2", MTE_IMPLEMENTED_ELX, MTE_IMPLEMENTED_ASY);
	tainted |= check_feature(ENABLE_FEAT_RNG, read_feat_rng_id_field(),
				 "RNG", 1, 1);
	tainted |= check_feature(ENABLE_BTI, read_feat_bti_id_field(),
				 "BTI", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_RNG_TRAP, read_feat_rng_trap_id_field(),
				 "RNG_TRAP", 1, 1);

	/* v8.6 features */
	tainted |= check_feature(ENABLE_FEAT_AMUv1p1, read_feat_amu_id_field(),
				 "AMUv1p1", 2, 2);
	tainted |= check_feature(ENABLE_FEAT_FGT, read_feat_fgt_id_field(),
				 "FGT", 1, 2);
	tainted |= check_feature(ENABLE_FEAT_FGT2, read_feat_fgt_id_field(),
				 "FGT2", 2, 2);
	tainted |= check_feature(ENABLE_FEAT_ECV, read_feat_ecv_id_field(),
				 "ECV", 1, 2);
	tainted |= check_feature(ENABLE_FEAT_TWED, read_feat_twed_id_field(),
				 "TWED", 1, 1);

	/*
	 * even though this is a "DISABLE" it does confusingly perform feature
	 * enablement duties like all other flags here. Check it against the HW
	 * feature when we intend to diverge from the default behaviour
	 */
	tainted |= check_feature(DISABLE_MTPMU, read_feat_mtpmu_id_field(),
				 "MTPMU", 1, 1);

	/* v8.7 features */
	tainted |= check_feature(ENABLE_FEAT_HCX, read_feat_hcx_id_field(),
				 "HCX", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_LS64_ACCDATA, read_feat_ls64_id_field(),
				 "LS64", 1, 3);

	/* v8.9 features */
	tainted |= check_feature(ENABLE_FEAT_TCR2, read_feat_tcr2_id_field(),
				 "TCR2", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_S2PIE, read_feat_s2pie_id_field(),
				 "S2PIE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_S1PIE, read_feat_s1pie_id_field(),
				 "S1PIE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_S2POE, read_feat_s2poe_id_field(),
				 "S2POE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_S1POE, read_feat_s1poe_id_field(),
				 "S1POE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_CSV2_3, read_feat_csv2_id_field(),
				 "CSV2_3", 3, 3);
	tainted |= check_feature(ENABLE_FEAT_DEBUGV8P9, read_feat_debugv8p9_id_field(),
				 "DEBUGV8P9", 11, 11);
	tainted |= check_feature(ENABLE_FEAT_THE, read_feat_the_id_field(),
				 "THE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_SCTLR2, read_feat_sctlr2_id_field(),
				 "SCTLR2", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_AIE, read_feat_aie_id_field(),
				 "AIE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_PFAR, read_feat_pfar_id_field(),
				 "PFAR", 1, 1);

	/* v9.0 features */
	tainted |= check_feature(ENABLE_BRBE_FOR_NS, read_feat_brbe_id_field(),
				 "BRBE", 1, 2);
	tainted |= check_feature(ENABLE_TRBE_FOR_NS, read_feat_trbe_id_field(),
				 "TRBE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_UINJ, read_feat_uinj_id_field(),
				 "UINJ", 1, 1);

	/* v9.2 features */
	tainted |= check_feature(ENABLE_SME_FOR_NS, read_feat_sme_id_field(),
				 "SME", 1, 2);
	tainted |= check_feature(ENABLE_SME2_FOR_NS, read_feat_sme_id_field(),
				 "SME2", 2, 2);
	tainted |= check_feature(ENABLE_FEAT_FPMR, read_feat_fpmr_id_field(),
				 "FPMR", 1, 1);

	/* v9.3 features */
	tainted |= check_feature(ENABLE_FEAT_D128, read_feat_d128_id_field(),
				 "D128", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_GCIE, read_feat_gcie_id_field(),
				 "GCIE", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_MPAM_PE_BW_CTRL,
				is_feat_mpam_pe_bw_ctrl_present(),
				"MPAM_PE_BW_CTRL", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_EBEP, read_feat_ebep_id_field(),
				 "EBEP", 1, 1);

	/* v9.4 features */
	tainted |= check_feature(ENABLE_FEAT_GCS, read_feat_gcs_id_field(),
				 "GCS", 1, 1);
	tainted |= check_feature(ENABLE_RME, read_feat_rme_id_field(),
				 "RME", 1, 2);
	tainted |= check_feature(ENABLE_FEAT_PAUTH_LR, is_feat_pauth_lr_present(),
				 "PAUTH_LR", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_FGWTE3, read_feat_fgwte3_id_field(),
				 "FGWTE3", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_CPA2, read_feat_cpa_id_field(),
				 "CPA2", 2, 2);
	tainted |= check_feature(ENABLE_FEAT_RME_GDI, read_feat_rme_gdi_id_field(),
				 "RME_GDI", 1, 1);
	tainted |= check_feature(ENABLE_FEAT_IDTE3, read_feat_idte3_id_field(),
				 "IDTE3", 2, 2);

	/* Morello Arch feature */
	tainted |= check_feature(ENABLE_FEAT_MORELLO, read_feat_morello_field(),
				 "MORELLO_ARCH", 1, 1);

	if (tainted) {
		panic();
	}

	detection_done[core_pos] = true;
}
