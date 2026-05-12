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

#define CREATE_FEATURE_PRESENT(name, idreg, field, min)				\
	_CREATE_FEATURE_ID_FIELD(name, idreg, guard, field, min, NA, NA)	\
	_CREATE_FEATURE_PRESENT(name, NA, NA, field, min, NA, NA)
#define CREATE_FEATURE_SUPPORTED(name, idreg, field, min, guard)		\
	_CREATE_FEATURE_SUPPORTED(name, idreg, guard, field, min, NA, NA)

#define CREATE_FEATURE_FUNCS(name, idreg, field, min, guard)			\
	CREATE_FEATURE_PRESENT(name, idreg, field, min)				\
	CREATE_FEATURE_SUPPORTED(name, idreg, field, min, guard)

__attribute__((always_inline))
static inline bool is_armv7_gentimer_present(void)
{
	return EXTRACT(ID_PFR1_GENTIMER, read_id_pfr1()) != 0U;
}

CREATE_FEATURE_PRESENT(feat_ttcnp, id_mmfr4, ID_MMFR4_CNP, 1U)

CREATE_FEATURE_FUNCS(feat_amu, id_pfr0, ID_PFR0_AMU,
		    ID_PFR0_AMU_V1, ENABLE_FEAT_AMU)

CREATE_FEATURE_FUNCS(feat_amu_aux, amcfgr, AMCFGR_NCG,
		    1U, ENABLE_AMU_AUXILIARY_COUNTERS)

CREATE_FEATURE_FUNCS(feat_amuv1p1, id_pfr0, ID_PFR0_AMU,
		    ID_PFR0_AMU_V1P1, ENABLE_FEAT_AMUv1p1)

CREATE_FEATURE_FUNCS(feat_trf, id_dfr0, ID_DFR0_TRACEFILT,
		    1U, ENABLE_TRF_FOR_NS)

CREATE_FEATURE_FUNCS(feat_sys_reg_trace, id_dfr0, ID_DFR0_COPTRC,
		    1U, ENABLE_SYS_REG_TRACE_FOR_NS)

CREATE_FEATURE_FUNCS(feat_dit, id_pfr0, ID_PFR0_DIT,
		    1U, ENABLE_FEAT_DIT)

CREATE_FEATURE_FUNCS(feat_pan, id_mmfr3, ID_MMFR3_PAN,
		    1U, ENABLE_FEAT_PAN)

CREATE_FEATURE_PRESENT(feat_ssbs, id_pfr2, ID_PFR2_SSBS, 1U)

CREATE_FEATURE_PRESENT(feat_pmuv3, id_dfr0, ID_DFR0_PERFMON, 3U)

CREATE_FEATURE_FUNCS(feat_mtpmu, id_dfr1, ID_DFR1_MTPMU,
		    1U, DISABLE_MTPMU)

/*
 * These features are only used by AArch64 code, so just disable these features
 * for AArch32.
 */
__attribute__((always_inline))
static inline bool is_feat_bti_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_pauth_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_morello_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_crypto_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_rme_supported(void) { return false; }

#endif /* ARCH_FEATURES_H */
