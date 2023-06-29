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
	return ISOLATE_FIELD(read_id_pfr1(), ID_PFR1_GENTIMER) != 0U;
}

static inline bool is_armv8_2_ttcnp_present(void)
{
	return ISOLATE_FIELD(read_id_mmfr4(), ID_MMFR4_CNP) != 0U;
}

static unsigned int read_feat_amu_id_field(void)
{
	return ISOLATE_FIELD(read_id_pfr0(), ID_PFR0_AMU);
}

static inline bool is_feat_amu_supported(void)
{
	if (ENABLE_FEAT_AMU == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_AMU == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_amu_id_field() >= ID_PFR0_AMU_V1;
}

static inline bool is_feat_amuv1p1_supported(void)
{
	if (ENABLE_FEAT_AMUv1p1 == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_FEAT_AMUv1p1 == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_amu_id_field() >= ID_PFR0_AMU_V1P1;
}

static inline unsigned int read_feat_trf_id_field(void)
{
	return ISOLATE_FIELD(read_id_dfr0(), ID_DFR0_TRACEFILT);
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

static inline unsigned int read_feat_coptrc_id_field(void)
{
	return ISOLATE_FIELD(read_id_dfr0(), ID_DFR0_COPTRC);
}

static inline bool is_feat_sys_reg_trace_supported(void)
{
	if (ENABLE_SYS_REG_TRACE_FOR_NS == FEAT_STATE_DISABLED) {
		return false;
	}

	if (ENABLE_SYS_REG_TRACE_FOR_NS == FEAT_STATE_ALWAYS) {
		return true;
	}

	return read_feat_coptrc_id_field() != 0U;
}

static inline unsigned int read_feat_dit_id_field(void)
{
	return ISOLATE_FIELD(read_id_pfr0(), ID_PFR0_DIT);
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

static inline unsigned int read_feat_pan_id_field(void)
{
	return ISOLATE_FIELD(read_id_mmfr3(), ID_MMFR3_PAN);
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

/*
 * TWED, ECV, CSV2, RAS are only used by the AArch64 EL2 context switch
 * code. In fact, EL2 context switching is only needed for AArch64 (since
 * there is no secure AArch32 EL2), so just disable these features here.
 */
static inline bool is_feat_twed_supported(void) { return false; }
static inline bool is_feat_ecv_supported(void) { return false; }
static inline bool is_feat_ecv_v2_supported(void) { return false; }
static inline bool is_feat_csv2_2_supported(void) { return false; }
static inline bool is_feat_ras_supported(void) { return false; }

/* The following features are supported in AArch64 only. */
static inline bool is_feat_vhe_supported(void) { return false; }
static inline bool is_feat_sel2_supported(void) { return false; }
static inline bool is_feat_fgt_supported(void) { return false; }
static inline bool is_feat_tcr2_supported(void) { return false; }
static inline bool is_feat_spe_supported(void) { return false; }
static inline bool is_feat_rng_supported(void) { return false; }
static inline bool is_feat_gcs_supported(void) { return false; }
static inline bool is_feat_mpam_supported(void) { return false; }
static inline bool is_feat_hcx_supported(void) { return false; }
static inline bool is_feat_sve_supported(void) { return false; }
static inline bool is_feat_brbe_supported(void) { return false; }
static inline bool is_feat_trbe_supported(void) { return false; }
static inline bool is_feat_nv2_supported(void) { return false; }
static inline bool is_feat_sme_supported(void) { return false; }
static inline bool is_feat_sme2_supported(void) { return false; }
static inline bool is_feat_s2poe_supported(void) { return false; }
static inline bool is_feat_s1poe_supported(void) { return false; }
static inline bool is_feat_sxpoe_supported(void) { return false; }
static inline bool is_feat_s2pie_supported(void) { return false; }
static inline bool is_feat_s1pie_supported(void) { return false; }
static inline bool is_feat_sxpie_supported(void) { return false; }

static inline unsigned int read_feat_pmuv3_id_field(void)
{
	return ISOLATE_FIELD(read_id_dfr0(), ID_DFR0_PERFMON);
}

static inline unsigned int read_feat_mtpmu_id_field(void)
{
	return ISOLATE_FIELD(read_id_dfr1(), ID_DFR1_MTPMU);
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

	return mtpmu != 0U && mtpmu != ID_DFR1_MTPMU_DISABLED;
}

#endif /* ARCH_FEATURES_H */
