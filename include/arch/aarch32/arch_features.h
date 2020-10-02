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

#define ISOLATE_FIELD(reg, feat, mask)						\
	((unsigned int)(((reg) >> (feat)) & mask))

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

#define CREATE_FEATURE_PRESENT(name, idreg, idfield, mask, idval)		\
__attribute__((always_inline))							\
static inline bool is_ ## name ## _present(void)				\
{										\
	return (ISOLATE_FIELD(read_ ## idreg(), idfield, mask) >= idval) 	\
		? true : false;							\
}

#define CREATE_FEATURE_FUNCS(name, idreg, idfield, mask, idval, guard)		\
CREATE_FEATURE_PRESENT(name, idreg, idfield, mask, idval)			\
CREATE_FEATURE_SUPPORTED(name, is_ ## name ## _present, guard)


/*
 * +----------------------------+
 * |	Features supported	|
 * +----------------------------+
 * |	GENTIMER		|
 * +----------------------------+
 * |	FEAT_TTCNP		|
 * +----------------------------+
 * |	FEAT_AMU		|
 * +----------------------------+
 * |	FEAT_AMUV1P1		|
 * +----------------------------+
 * |	FEAT_TRF		|
 * +----------------------------+
 * |	FEAT_SYS_REG_TRACE 	|
 * +----------------------------+
 * |	FEAT_DIT		|
 * +----------------------------+
 * |	FEAT_PAN		|
 * +----------------------------+
 * |	FEAT_SSBS		|
 * +----------------------------+
 * |	FEAT_PMUV3		|
 * +----------------------------+
 * |	FEAT_MTPMU		|
 * +----------------------------+
 */

/* GENTIMER */
__attribute__((always_inline))
static inline bool is_armv7_gentimer_present(void)
{
	return ISOLATE_FIELD(read_id_pfr1(), ID_PFR1_GENTIMER_SHIFT,
			    ID_PFR1_GENTIMER_MASK) != 0U;
}

/* FEAT_TTCNP: Translation table common not private */
CREATE_FEATURE_PRESENT(feat_ttcnp, id_mmfr4, ID_MMFR4_CNP_SHIFT,
		      ID_MMFR4_CNP_MASK, 1U)

/* FEAT_AMU: Activity Monitors Extension */
CREATE_FEATURE_FUNCS(feat_amu, id_pfr0, ID_PFR0_AMU_SHIFT,
		    ID_PFR0_AMU_MASK, ID_PFR0_AMU_V1, ENABLE_FEAT_AMU)

/* Auxiliary counters for FEAT_AMU */
CREATE_FEATURE_FUNCS(feat_amu_aux, amcfgr, AMCFGR_NCG_SHIFT,
		    AMCFGR_NCG_MASK, 1U, ENABLE_AMU_AUXILIARY_COUNTERS)

/* FEAT_AMUV1P1: AMU Extension v1.1 */
CREATE_FEATURE_FUNCS(feat_amuv1p1, id_pfr0, ID_PFR0_AMU_SHIFT,
		    ID_PFR0_AMU_MASK, ID_PFR0_AMU_V1P1, ENABLE_FEAT_AMUv1p1)

/* FEAT_TRF: Tracefilter */
CREATE_FEATURE_FUNCS(feat_trf, id_dfr0, ID_DFR0_TRACEFILT_SHIFT,
		    ID_DFR0_TRACEFILT_MASK, 1U, ENABLE_TRF_FOR_NS)

/* FEAT_SYS_REG_TRACE */
CREATE_FEATURE_FUNCS(feat_sys_reg_trace, id_dfr0, ID_DFR0_COPTRC_SHIFT,
		    ID_DFR0_COPTRC_MASK, 1U, ENABLE_SYS_REG_TRACE_FOR_NS)

/* FEAT_DIT: Data independent timing */
CREATE_FEATURE_FUNCS(feat_dit, id_pfr0, ID_PFR0_DIT_SHIFT,
		    ID_PFR0_DIT_MASK, 1U, ENABLE_FEAT_DIT)

/* FEAT_PAN: Privileged access never */
CREATE_FEATURE_FUNCS(feat_pan, id_mmfr3, ID_MMFR3_PAN_SHIFT,
		    ID_MMFR3_PAN_MASK, 1U, ENABLE_FEAT_PAN)

/* FEAT_SSBS: Speculative store bypass safe */
CREATE_FEATURE_PRESENT(feat_ssbs, id_pfr2, ID_PFR2_SSBS_SHIFT,
		      ID_PFR2_SSBS_MASK, 1U)

/* FEAT_PMUV3 */
CREATE_FEATURE_PRESENT(feat_pmuv3, id_dfr0, ID_DFR0_PERFMON_SHIFT,
		      ID_DFR0_PERFMON_MASK, 3U)

/* FEAT_MTPMU */
__attribute__((always_inline))
static inline bool is_feat_mtpmu_present(void)
{
	unsigned int mtpmu = ISOLATE_FIELD(read_id_dfr1(), ID_DFR1_MTPMU_SHIFT,
			    ID_DFR1_MTPMU_MASK);
	return (mtpmu != 0U) && (mtpmu != MTPMU_NOT_IMPLEMENTED);
}
CREATE_FEATURE_SUPPORTED(feat_mtpmu, is_feat_mtpmu_present, DISABLE_MTPMU)

/*
 * TWED, ECV, CSV2, RAS are only used by the AArch64 EL2 context switch
 * code. In fact, EL2 context switching is only needed for AArch64 (since
 * there is no secure AArch32 EL2), so just disable these features here.
 */
__attribute__((always_inline))
static inline bool is_feat_twed_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_ecv_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_ecv_v2_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_csv2_2_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_csv2_3_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_ras_supported(void) { return false; }

/* The following features are supported in AArch64 only. */
__attribute__((always_inline))
static inline bool is_feat_vhe_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_sel2_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_fgt_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_tcr2_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_spe_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_rng_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_gcs_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_mte2_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_mpam_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_hcx_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_sve_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_brbe_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_trbe_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_nv2_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_sme_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_sme2_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_s2poe_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_s1poe_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_sxpoe_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_s2pie_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_s1pie_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_sxpie_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_uao_present(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_nmi_present(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_ebep_present(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_sebep_present(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_d128_present(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_ls64_accdata_present(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_mops_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_bti_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_pauth_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_lse_supported(void) { return false; }
__attribute__((always_inline))
static inline bool is_feat_morello_supported(void) { return false; }

#endif /* ARCH_FEATURES_H */
