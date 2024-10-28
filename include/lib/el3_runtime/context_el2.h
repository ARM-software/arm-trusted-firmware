/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONTEXT_EL2_H
#define CONTEXT_EL2_H

#include <lib/extensions/sysreg128.h>

#ifndef __ASSEMBLER__

/*******************************************************************************
 * EL2 Registers:
 * AArch64 EL2 system register context structure for preserving the
 * architectural state during world switches.
 ******************************************************************************/
typedef struct el2_common_regs {
	uint64_t actlr_el2;
	uint64_t afsr0_el2;
	uint64_t afsr1_el2;
	uint64_t amair_el2;
	uint64_t cnthctl_el2;
	uint64_t cntvoff_el2;
	uint64_t cptr_el2;
	uint64_t dbgvcr32_el2;
	uint64_t elr_el2;
	uint64_t esr_el2;
	uint64_t far_el2;
	uint64_t hacr_el2;
	uint64_t hcr_el2;
	uint64_t hpfar_el2;
	uint64_t hstr_el2;
	uint64_t icc_sre_el2;
	uint64_t ich_hcr_el2;
	uint64_t ich_vmcr_el2;
	uint64_t mair_el2;
	uint64_t mdcr_el2;
	uint64_t pmscr_el2;
	uint64_t sctlr_el2;
	uint64_t spsr_el2;
	uint64_t sp_el2;
	uint64_t tcr_el2;
	uint64_t tpidr_el2;
	uint64_t vbar_el2;
	uint64_t vmpidr_el2;
	uint64_t vpidr_el2;
	uint64_t vtcr_el2;
	sysreg_t vttbr_el2;
	sysreg_t ttbr0_el2;
} el2_common_regs_t;

typedef struct el2_mte2_regs {
	uint64_t tfsr_el2;
} el2_mte2_regs_t;

typedef struct el2_fgt_regs {
	uint64_t hdfgrtr_el2;
	uint64_t hafgrtr_el2;
	uint64_t hdfgwtr_el2;
	uint64_t hfgitr_el2;
	uint64_t hfgrtr_el2;
	uint64_t hfgwtr_el2;
} el2_fgt_regs_t;

typedef struct el2_fgt2_regs {
	uint64_t hdfgrtr2_el2;
	uint64_t hdfgwtr2_el2;
	uint64_t hfgitr2_el2;
	uint64_t hfgrtr2_el2;
	uint64_t hfgwtr2_el2;
} el2_fgt2_regs_t;

typedef struct el2_ecv_regs {
	uint64_t cntpoff_el2;
} el2_ecv_regs_t;

typedef struct el2_vhe_regs {
	uint64_t contextidr_el2;
	sysreg_t ttbr1_el2;
} el2_vhe_regs_t;

typedef struct el2_ras_regs {
	uint64_t vdisr_el2;
	uint64_t vsesr_el2;
} el2_ras_regs_t;

typedef struct el2_neve_regs {
	uint64_t vncr_el2;
} el2_neve_regs_t;

typedef struct el2_trf_regs {
	uint64_t trfcr_el2;
} el2_trf_regs_t;

typedef struct el2_csv2_regs {
	uint64_t scxtnum_el2;
} el2_csv2_regs_t;

typedef struct el2_hcx_regs {
	uint64_t hcrx_el2;
} el2_hcx_regs_t;

typedef struct el2_tcr2_regs {
	uint64_t tcr2_el2;
} el2_tcr2_regs_t;

typedef struct el2_sxpoe_regs {
	uint64_t por_el2;
} el2_sxpoe_regs_t;

typedef struct el2_sxpie_regs {
	uint64_t pire0_el2;
	uint64_t pir_el2;
} el2_sxpie_regs_t;

typedef struct el2_s2pie_regs {
	uint64_t s2pir_el2;
} el2_s2pie_regs_t;

typedef struct el2_gcs_regs {
	uint64_t gcscr_el2;
	uint64_t gcspr_el2;
} el2_gcs_regs_t;

typedef struct el2_mpam_regs {
	uint64_t mpam2_el2;
	uint64_t mpamhcr_el2;
	uint64_t mpamvpm0_el2;
	uint64_t mpamvpm1_el2;
	uint64_t mpamvpm2_el2;
	uint64_t mpamvpm3_el2;
	uint64_t mpamvpm4_el2;
	uint64_t mpamvpm5_el2;
	uint64_t mpamvpm6_el2;
	uint64_t mpamvpm7_el2;
	uint64_t mpamvpmv_el2;
} el2_mpam_regs_t;

typedef struct el2_sctlr2_regs {
	uint64_t sctlr2_el2;
} el2_sctlr2_regs_t;

typedef struct el2_sysregs {

	el2_common_regs_t common;

#if ENABLE_FEAT_MTE2
	el2_mte2_regs_t mte2;
#endif

#if ENABLE_FEAT_FGT
	el2_fgt_regs_t fgt;
#endif

#if ENABLE_FEAT_FGT2
	el2_fgt2_regs_t fgt2;
#endif

#if ENABLE_FEAT_ECV
	el2_ecv_regs_t ecv;
#endif

#if ENABLE_FEAT_VHE
	el2_vhe_regs_t vhe;
#endif

#if ENABLE_FEAT_RAS
	el2_ras_regs_t ras;
#endif

#if CTX_INCLUDE_NEVE_REGS
	el2_neve_regs_t neve;
#endif

#if ENABLE_TRF_FOR_NS
	el2_trf_regs_t trf;
#endif

#if ENABLE_FEAT_CSV2_2
	el2_csv2_regs_t csv2;
#endif

#if ENABLE_FEAT_HCX
	el2_hcx_regs_t hcx;
#endif

#if ENABLE_FEAT_TCR2
	el2_tcr2_regs_t tcr2;
#endif

#if (ENABLE_FEAT_S1POE || ENABLE_FEAT_S2POE)
	el2_sxpoe_regs_t sxpoe;
#endif

#if (ENABLE_FEAT_S1PIE || ENABLE_FEAT_S2PIE)
	el2_sxpie_regs_t sxpie;
#endif

#if ENABLE_FEAT_S2PIE
	el2_s2pie_regs_t s2pie;
#endif

#if ENABLE_FEAT_GCS
	el2_gcs_regs_t gcs;
#endif

#if CTX_INCLUDE_MPAM_REGS
	el2_mpam_regs_t mpam;
#endif

#if ENABLE_FEAT_SCTLR2
	el2_sctlr2_regs_t sctlr2;
#endif

} el2_sysregs_t;

/*
 * Macros to access members related to individual features of the el2_sysregs_t
 * structures.
 */
#define read_el2_ctx_common(ctx, reg)		(((ctx)->common).reg)

#define write_el2_ctx_common(ctx, reg, val)	((((ctx)->common).reg)	\
							= (uint64_t) (val))

#define write_el2_ctx_sysreg128(ctx, reg, val)	((((ctx)->common).reg)	\
							= (sysreg_t) (val))

#if ENABLE_FEAT_MTE2
#define read_el2_ctx_mte2(ctx, reg)		(((ctx)->mte2).reg)
#define write_el2_ctx_mte2(ctx, reg, val)	((((ctx)->mte2).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_mte2(ctx, reg)		ULL(0)
#define write_el2_ctx_mte2(ctx, reg, val)
#endif /* ENABLE_FEAT_MTE2 */

#if ENABLE_FEAT_FGT
#define read_el2_ctx_fgt(ctx, reg)		(((ctx)->fgt).reg)
#define write_el2_ctx_fgt(ctx, reg, val)	((((ctx)->fgt).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_fgt(ctx, reg)		ULL(0)
#define write_el2_ctx_fgt(ctx, reg, val)
#endif /* ENABLE_FEAT_FGT */

#if ENABLE_FEAT_FGT2
#define read_el2_ctx_fgt2(ctx, reg)		(((ctx)->fgt2).reg)
#define write_el2_ctx_fgt2(ctx, reg, val)	((((ctx)->fgt2).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_fgt2(ctx, reg)		ULL(0)
#define write_el2_ctx_fgt2(ctx, reg, val)
#endif /* ENABLE_FEAT_FGT */

#if ENABLE_FEAT_ECV
#define read_el2_ctx_ecv(ctx, reg)		(((ctx)->ecv).reg)
#define write_el2_ctx_ecv(ctx, reg, val)	((((ctx)->ecv).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_ecv(ctx, reg)		ULL(0)
#define write_el2_ctx_ecv(ctx, reg, val)
#endif /* ENABLE_FEAT_ECV */

#if ENABLE_FEAT_VHE
#define read_el2_ctx_vhe(ctx, reg)		(((ctx)->vhe).reg)
#define write_el2_ctx_vhe(ctx, reg, val)	((((ctx)->vhe).reg)	\
							= (uint64_t) (val))

#define write_el2_ctx_vhe_sysreg128(ctx, reg, val)	((((ctx)->vhe).reg)	\
								= (sysreg_t) (val))
#else
#define read_el2_ctx_vhe(ctx, reg)		ULL(0)
#define write_el2_ctx_vhe(ctx, reg, val)
#endif /* ENABLE_FEAT_VHE */

#if ENABLE_FEAT_RAS
#define read_el2_ctx_ras(ctx, reg)		(((ctx)->ras).reg)
#define write_el2_ctx_ras(ctx, reg, val)	((((ctx)->ras).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_ras(ctx, reg)		ULL(0)
#define write_el2_ctx_ras(ctx, reg, val)
#endif /* ENABLE_FEAT_RAS */

#if CTX_INCLUDE_NEVE_REGS
#define read_el2_ctx_neve(ctx, reg)		(((ctx)->neve).reg)
#define write_el2_ctx_neve(ctx, reg, val)	((((ctx)->neve).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_neve(ctx, reg)		ULL(0)
#define write_el2_ctx_neve(ctx, reg, val)
#endif /* CTX_INCLUDE_NEVE_REGS */

#if ENABLE_TRF_FOR_NS
#define read_el2_ctx_trf(ctx, reg)		(((ctx)->trf).reg)
#define write_el2_ctx_trf(ctx, reg, val)	((((ctx)->trf).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_trf(ctx, reg)		ULL(0)
#define write_el2_ctx_trf(ctx, reg, val)
#endif /* ENABLE_TRF_FOR_NS */

#if ENABLE_FEAT_CSV2_2
#define read_el2_ctx_csv2_2(ctx, reg)		(((ctx)->csv2).reg)
#define write_el2_ctx_csv2_2(ctx, reg, val)	((((ctx)->csv2).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_csv2_2(ctx, reg)		ULL(0)
#define write_el2_ctx_csv2_2(ctx, reg, val)
#endif /* ENABLE_FEAT_CSV2_2 */

#if ENABLE_FEAT_HCX
#define read_el2_ctx_hcx(ctx, reg)		(((ctx)->hcx).reg)
#define write_el2_ctx_hcx(ctx, reg, val)	((((ctx)->hcx).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_hcx(ctx, reg)		ULL(0)
#define write_el2_ctx_hcx(ctx, reg, val)
#endif /* ENABLE_FEAT_HCX */

#if ENABLE_FEAT_TCR2
#define read_el2_ctx_tcr2(ctx, reg)		(((ctx)->tcr2).reg)
#define write_el2_ctx_tcr2(ctx, reg, val)	((((ctx)->tcr2).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_tcr2(ctx, reg)		ULL(0)
#define write_el2_ctx_tcr2(ctx, reg, val)
#endif /* ENABLE_FEAT_TCR2 */

#if (ENABLE_FEAT_S1POE || ENABLE_FEAT_S2POE)
#define read_el2_ctx_sxpoe(ctx, reg)		(((ctx)->sxpoe).reg)
#define write_el2_ctx_sxpoe(ctx, reg, val)	((((ctx)->sxpoe).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_sxpoe(ctx, reg)		ULL(0)
#define write_el2_ctx_sxpoe(ctx, reg, val)
#endif /*(ENABLE_FEAT_S1POE || ENABLE_FEAT_S2POE) */

#if (ENABLE_FEAT_S1PIE || ENABLE_FEAT_S2PIE)
#define read_el2_ctx_sxpie(ctx, reg)		(((ctx)->sxpie).reg)
#define write_el2_ctx_sxpie(ctx, reg, val)	((((ctx)->sxpie).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_sxpie(ctx, reg)		ULL(0)
#define write_el2_ctx_sxpie(ctx, reg, val)
#endif /*(ENABLE_FEAT_S1PIE || ENABLE_FEAT_S2PIE) */

#if ENABLE_FEAT_S2PIE
#define read_el2_ctx_s2pie(ctx, reg)		(((ctx)->s2pie).reg)
#define write_el2_ctx_s2pie(ctx, reg, val)	((((ctx)->s2pie).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_s2pie(ctx, reg)		ULL(0)
#define write_el2_ctx_s2pie(ctx, reg, val)
#endif /* ENABLE_FEAT_S2PIE */

#if ENABLE_FEAT_GCS
#define read_el2_ctx_gcs(ctx, reg)		(((ctx)->gcs).reg)
#define write_el2_ctx_gcs(ctx, reg, val)	((((ctx)->gcs).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_gcs(ctx, reg)		ULL(0)
#define write_el2_ctx_gcs(ctx, reg, val)
#endif /* ENABLE_FEAT_GCS */

#if CTX_INCLUDE_MPAM_REGS
#define read_el2_ctx_mpam(ctx, reg)		(((ctx)->mpam).reg)
#define write_el2_ctx_mpam(ctx, reg, val)	((((ctx)->mpam).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_mpam(ctx, reg)		ULL(0)
#define write_el2_ctx_mpam(ctx, reg, val)
#endif /* CTX_INCLUDE_MPAM_REGS */

#if ENABLE_FEAT_SCTLR2
#define read_el2_ctx_sctlr2(ctx, reg)		(((ctx)->sctlr2).reg)
#define write_el2_ctx_sctlr2(ctx, reg, val)	((((ctx)->sctlr2).reg)	\
							= (uint64_t) (val))
#else
#define read_el2_ctx_sctlr2(ctx, reg)		ULL(0)
#define write_el2_ctx_sctlr2(ctx, reg, val)
#endif /* ENABLE_FEAT_SCTLR2 */

/******************************************************************************/

#endif /* __ASSEMBLER__ */

#endif /* CONTEXT_EL2_H */
