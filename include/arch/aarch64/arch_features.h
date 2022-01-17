/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_FEATURES_H
#define ARCH_FEATURES_H

#include <stdbool.h>

#include <arch_helpers.h>

static inline bool is_armv7_gentimer_present(void)
{
	/* The Generic Timer is always present in an ARMv8-A implementation */
	return true;
}

static inline bool is_armv8_1_pan_present(void)
{
	return ((read_id_aa64mmfr1_el1() >> ID_AA64MMFR1_EL1_PAN_SHIFT) &
		ID_AA64MMFR1_EL1_PAN_MASK) != 0U;
}

static inline bool is_armv8_1_vhe_present(void)
{
	return ((read_id_aa64mmfr1_el1() >> ID_AA64MMFR1_EL1_VHE_SHIFT) &
		ID_AA64MMFR1_EL1_VHE_MASK) != 0U;
}

static inline bool is_armv8_2_ttcnp_present(void)
{
	return ((read_id_aa64mmfr2_el1() >> ID_AA64MMFR2_EL1_CNP_SHIFT) &
		ID_AA64MMFR2_EL1_CNP_MASK) != 0U;
}

static inline bool is_armv8_3_pauth_present(void)
{
	uint64_t mask = (ID_AA64ISAR1_GPI_MASK << ID_AA64ISAR1_GPI_SHIFT) |
			(ID_AA64ISAR1_GPA_MASK << ID_AA64ISAR1_GPA_SHIFT) |
			(ID_AA64ISAR1_API_MASK << ID_AA64ISAR1_API_SHIFT) |
			(ID_AA64ISAR1_APA_MASK << ID_AA64ISAR1_APA_SHIFT);

	/* If any of the fields is not zero, PAuth is present */
	return (read_id_aa64isar1_el1() & mask) != 0U;
}

static inline bool is_armv8_4_dit_present(void)
{
	return ((read_id_aa64pfr0_el1() >> ID_AA64PFR0_DIT_SHIFT) &
		ID_AA64PFR0_DIT_MASK) == 1U;
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

static inline bool is_armv8_4_sel2_present(void)
{
	return ((read_id_aa64pfr0_el1() >> ID_AA64PFR0_SEL2_SHIFT) &
		ID_AA64PFR0_SEL2_MASK) == 1ULL;
}

static inline bool is_armv8_6_twed_present(void)
{
	return (((read_id_aa64mmfr1_el1() >> ID_AA64MMFR1_EL1_TWED_SHIFT) &
		ID_AA64MMFR1_EL1_TWED_MASK) == ID_AA64MMFR1_EL1_TWED_SUPPORTED);
}

static inline bool is_armv8_6_fgt_present(void)
{
	return ((read_id_aa64mmfr0_el1() >> ID_AA64MMFR0_EL1_FGT_SHIFT) &
		ID_AA64MMFR0_EL1_FGT_MASK) == ID_AA64MMFR0_EL1_FGT_SUPPORTED;
}

static inline unsigned long int get_armv8_6_ecv_support(void)
{
	return ((read_id_aa64mmfr0_el1() >> ID_AA64MMFR0_EL1_ECV_SHIFT) &
		ID_AA64MMFR0_EL1_ECV_MASK);
}

static inline bool is_armv8_5_rng_present(void)
{
	return ((read_id_aa64isar0_el1() >> ID_AA64ISAR0_RNDR_SHIFT) &
		ID_AA64ISAR0_RNDR_MASK);
}

static inline bool is_armv8_6_feat_amuv1p1_present(void)
{
	return (((read_id_aa64pfr0_el1() >> ID_AA64PFR0_AMU_SHIFT) &
		ID_AA64PFR0_AMU_MASK) >= ID_AA64PFR0_AMU_V1P1);
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
static inline unsigned int get_mpam_version(void)
{
	return (unsigned int)((((read_id_aa64pfr0_el1() >>
		ID_AA64PFR0_MPAM_SHIFT) & ID_AA64PFR0_MPAM_MASK) << 4) |
				((read_id_aa64pfr1_el1() >>
		ID_AA64PFR1_MPAM_FRAC_SHIFT) & ID_AA64PFR1_MPAM_FRAC_MASK));
}

static inline bool is_feat_hcx_present(void)
{
	return (((read_id_aa64mmfr1_el1() >> ID_AA64MMFR1_EL1_HCX_SHIFT) &
		ID_AA64MMFR1_EL1_HCX_MASK) == ID_AA64MMFR1_EL1_HCX_SUPPORTED);
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
static inline bool is_armv8_0_feat_sb_present(void)
{
	return (((read_id_aa64isar1_el1() >> ID_AA64ISAR1_SB_SHIFT) &
		ID_AA64ISAR1_SB_MASK) == ID_AA64ISAR1_SB_SUPPORTED);
}

/*********************************************************************************
 * Function to identify the presence of FEAT_CSV2_2 (Cache Speculation Variant 2)
 ********************************************************************************/
static inline bool is_armv8_0_feat_csv2_2_present(void)
{
	return (((read_id_aa64pfr0_el1() >> ID_AA64PFR0_CSV2_SHIFT) &
		ID_AA64PFR0_CSV2_MASK) == ID_AA64PFR0_CSV2_2_SUPPORTED);
}

/**********************************************************************************
 * Function to identify the presence of FEAT_SPE (Statistical Profiling Extension)
 *********************************************************************************/
static inline bool is_armv8_2_feat_spe_present(void)
{
	return (((read_id_aa64dfr0_el1() >> ID_AA64DFR0_PMS_SHIFT) &
		ID_AA64DFR0_PMS_MASK) != ID_AA64DFR0_SPE_NOT_SUPPORTED);
}

/*******************************************************************************
 * Function to identify the presence of FEAT_SVE (Scalable Vector Extension)
 ******************************************************************************/
static inline bool is_armv8_2_feat_sve_present(void)
{
	return (((read_id_aa64pfr0_el1() >> ID_AA64PFR0_SVE_SHIFT) &
		ID_AA64PFR0_SVE_MASK) == ID_AA64PFR0_SVE_SUPPORTED);
}

/*******************************************************************************
 * Function to identify the presence of FEAT_RAS (Reliability,Availability,
 * and Serviceability Extension)
 ******************************************************************************/
static inline bool is_armv8_2_feat_ras_present(void)
{
	return (((read_id_aa64pfr0_el1() >> ID_AA64PFR0_RAS_SHIFT) &
		ID_AA64PFR0_RAS_MASK) != ID_AA64PFR0_RAS_NOT_SUPPORTED);
}

/**************************************************************************
 * Function to identify the presence of FEAT_DIT (Data Independent Timing)
 *************************************************************************/
static inline bool is_armv8_4_feat_dit_present(void)
{
	return (((read_id_aa64pfr0_el1() >> ID_AA64PFR0_DIT_SHIFT) &
		ID_AA64PFR0_DIT_MASK) == ID_AA64PFR0_DIT_SUPPORTED);
}

/*************************************************************************
 * Function to identify the presence of FEAT_TRF (TraceLift)
 ************************************************************************/
static inline bool is_arm8_4_feat_trf_present(void)
{
	return (((read_id_aa64dfr0_el1() >> ID_AA64DFR0_TRACEFILT_SHIFT) &
		ID_AA64DFR0_TRACEFILT_MASK) == ID_AA64DFR0_TRACEFILT_SUPPORTED);
}

/*******************************************************************************
 * Function to identify the presence of FEAT_AMUv1 (Activity Monitors-
 * Extension v1)
 ******************************************************************************/
static inline bool is_armv8_4_feat_amuv1_present(void)
{
	return (((read_id_aa64pfr0_el1() >> ID_AA64PFR0_AMU_SHIFT) &
		ID_AA64PFR0_AMU_MASK) >= ID_AA64PFR0_AMU_V1);
}

/********************************************************************************
 * Function to identify the presence of FEAT_NV2 (Enhanced Nested Virtualization
 * Support)
 *******************************************************************************/
static inline unsigned int get_armv8_4_feat_nv_support(void)
{
	return (((read_id_aa64mmfr2_el1() >> ID_AA64MMFR2_EL1_NV_SHIFT) &
		ID_AA64MMFR2_EL1_NV_MASK));
}

#endif /* ARCH_FEATURES_H */
