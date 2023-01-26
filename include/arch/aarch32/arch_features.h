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

static inline bool is_feat_spe_supported(void)
{
	/* FEAT_SPE is AArch64 only */
	return false;
}

#endif /* ARCH_FEATURES_H */
