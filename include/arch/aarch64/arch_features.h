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
#include <lib/cpus/errata.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>

#if ENABLE_RMM
#define FEAT_ENABLE_ALL_WORLDS			\
	((1u << CPU_CONTEXT_SECURE)	|	\
	(1u << CPU_CONTEXT_NS)		|	\
	(1u << CPU_CONTEXT_REALM))
#define FEAT_ENABLE_REALM		(1 << CPU_CONTEXT_REALM)
#else
#define FEAT_ENABLE_ALL_WORLDS			\
	((1u << CPU_CONTEXT_SECURE)	|	\
	(1u << CPU_CONTEXT_NS))
#define FEAT_ENABLE_REALM		U(0)
#endif

#define FEAT_ENABLE_SECURE		(1 << CPU_CONTEXT_SECURE)
#define FEAT_ENABLE_NS			(1 << CPU_CONTEXT_NS)

#define CREATE_FEATURE_ID_FIELD		_CREATE_FEATURE_ID_FIELD
#define CREATE_FEATURE_PRESENT		_CREATE_FEATURE_PRESENT
#define CREATE_FEATURE_SUPPORTED	_CREATE_FEATURE_SUPPORTED

#define CREATE_FEATURE_FUNCS(name, idreg, guard, field, min, max, worlds)	\
	CREATE_FEATURE_ID_FIELD(name, idreg, guard, field, min, max, worlds)	\
	CREATE_FEATURE_PRESENT(name, idreg, guard, field, min, max, worlds)	\
	CREATE_FEATURE_SUPPORTED(name, idreg, guard, field, min, max, worlds)

/*******************************************************************************
 * THE ARCHITECTURE FEATURES LIST.
 *
 * TF-A supports many Arm architectural features starting from arch version 8.0
 * onwards. These features are mostly enabled through build flags, and are
 * discoverable in hardware through standard ID registers. This is the central
 * database to encode all necessary information to manipulate any given feature.
 * If TF-A needs to interact with a feature, it must be registered here with a
 * templating macro that's to be called with 7 arguments:
 *  * name - the lowercase name of the feature as it appears in the ARM Arm
 *  * idreg - the name of the ID register where this feature is listed
 *  * guard - the tri-state (``ENABLE_FEAT`` compliant) flag for the feature
 *  * field - the base name of the macros for a field of the ID register above
 *            that identifies this feature.
 *  * min - the minimum value the field above can take for the feature to be
 *          considered implemented.
 *  * max - the maximum value for the field that support has been validated for
 *  * worlds - a bitmap of which non-root worlds the feature is enabled for

 * The database is used to automatically generate accessors, validators, and
 * meta-feature (like FEAT_IDTE3) boilerplate.
 *
 * Some features don't need or don't fit into this pattern. The template can be
 * unravelled and unnecessary/illogical information should be replaced with "NA"
 * placeholders. You should familiarize with the usage of CPUFEAT_LIST before
 * going that path. A good example of this is FEAT_PAuth.
 *
 * Please put features in the version section according to the feature's chapter
 * in the Arm ARM. For example, if a feature is part of the Armv9.5 extension,
 * it is optional from Armv9.4 and mandatory from Armv9.6, then it should be in
 * the Armv9.5 section below.
 ******************************************************************************/

/* === v8.0 features === */
#define FEAT_CSV2_2(gen)							\
	gen(feat_csv2_2, id_aa64pfr0_el1, ENABLE_FEAT_CSV2_2,			\
	    ID_AA64PFR0_CSV2, 2U, 3U, FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

#define FEAT_CSV2_3(gen)							\
	gen(feat_csv2_3, id_aa64pfr0_el1, ENABLE_FEAT_CSV2_3,			\
	    ID_AA64PFR0_CSV2, 3U, 3U, FEAT_ENABLE_ALL_WORLDS)

/*
 * Even though the PMUv3 is an OPTIONAL feature, it is always implemented and
 * Arm prescribes so. So assume it will be there and do away with a flag for it.
 * This is used to check minor PMUv3px revisions so that we catch them as they
 * come along
 */
#define FEAT_PMUV3(gen)								\
	gen(feat_pmuv3, id_aa64dfr0_el1, FEAT_STATE_ALWAYS,			\
	    ID_AA64DFR0_PMUVER, 1U, 9U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_TGRAN16K(gen)							\
	gen(feat_tgran16K, id_aa64mmfr0_el1, FEAT_STATE_CHECKED,		\
	    ID_AA64MMFR0_EL1_TGRAN16, 1U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_AES(gen)								\
	gen(feat_aes, id_aa64isar0_el1, ENABLE_FEAT_CRYPTO,			\
	    ID_AA64ISAR0_AES, 1U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SHA1(gen)								\
	gen(feat_sha1, id_aa64isar0_el1, ENABLE_FEAT_CRYPTO,			\
	    ID_AA64ISAR0_SHA1, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* Not named in the architecture. Relates to FEAT_TRC_SR */
#define FEAT_SYS_REG_TRACE(gen)							\
	gen(feat_sys_reg_trace, id_aa64dfr0_el1, ENABLE_SYS_REG_TRACE_FOR_NS,	\
	    ID_AA64DFR0_TRACEVER, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* === v8.1 features === */
#define FEAT_PAN(gen)								\
	gen(feat_pan, id_aa64mmfr1_el1, ENABLE_FEAT_PAN,			\
	    ID_AA64MMFR1_EL1_PAN, 1U, 3U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_LSE(gen)								\
	gen(feat_lse, id_aa64isar0_el1, USE_SPINLOCK_CAS,			\
	    ID_AA64ISAR0_ATOMIC, 2U, 3U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_VHE(gen)								\
	gen(feat_vhe, id_aa64mmfr1_el1, ENABLE_FEAT_VHE,			\
	    ID_AA64MMFR1_EL1_VHE, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* === v8.2 features === */
#define FEAT_RAS(gen)								\
	gen(feat_ras, id_aa64pfr0_el1, ENABLE_FEAT_RAS,				\
	    ID_AA64PFR0_RAS, 1U, 3U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_IESB(gen)								\
	gen(feat_iesb, id_aa64mmfr2_el1, ENABLE_FEAT_RAS,			\
	    ID_AA64MMFR2_EL1_IESB, 1U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SVE(gen)								\
	gen(feat_sve, id_aa64pfr0_el1, ENABLE_SVE_FOR_NS,			\
	    ID_AA64PFR0_SVE, 1U, 3U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_TTCNP(gen)								\
	gen(feat_ttcnp, id_aa64mmfr2_el1, FEAT_STATE_CHECKED,			\
	    ID_AA64MMFR2_EL1_CNP, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_UAO(gen)								\
	gen(feat_uao, id_aa64mmfr2_el1, FEAT_STATE_CHECKED,			\
	    ID_AA64MMFR2_EL1_UAO, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SPE(gen)								\
	gen(feat_spe, id_aa64dfr0_el1, ENABLE_SPE_FOR_NS,			\
	    ID_AA64DFR0_PMS, 1U, 6U, FEAT_ENABLE_NS)

/* === v8.3 features === */
#define FEAT_PAUTH(gen)							\
	gen(feat_pauth, NA, ENABLE_PAUTH, NA, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* === v8.4 features === */
#define FEAT_DIT(gen)								\
	gen(feat_dit, id_aa64pfr0_el1, ENABLE_FEAT_DIT,				\
	    ID_AA64PFR0_DIT, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_TTST(gen)								\
	gen(feat_ttst, id_aa64mmfr2_el1, FEAT_STATE_CHECKED,			\
	    ID_AA64MMFR2_EL1_ST, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* FEAT_AMUv1 in the architecture */
#define FEAT_AMU(gen)								\
	gen(feat_amu, id_aa64pfr0_el1, ENABLE_FEAT_AMU,				\
	    ID_AA64PFR0_AMU, 1U, 2U, FEAT_ENABLE_NS)

#define FEAT_MPAM(gen)								\
	gen(feat_mpam, NA, ENABLE_FEAT_MPAM,					\
	    NA, 1U, 17U, FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

#define FEAT_NV2(gen)								\
	gen(feat_nv2, id_aa64mmfr2_el1, CTX_INCLUDE_NEVE_REGS,			\
	    ID_AA64MMFR2_EL1_NV, 2U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SEL2(gen)								\
	gen(feat_sel2, id_aa64pfr0_el1, ENABLE_FEAT_SEL2,			\
	    ID_AA64PFR0_SEL2, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_TRF(gen)								\
	gen(feat_trf, id_aa64dfr0_el1, ENABLE_TRF_FOR_NS,			\
	    ID_AA64DFR0_TRACEFILT, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* === v8.5 features === */
#define FEAT_SB(gen)								\
	gen(feat_sb, id_aa64isar1_el1, ENABLE_FEAT_SB,				\
	    ID_AA64ISAR1_SB, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_MTE2(gen)								\
	gen(feat_mte2, id_aa64pfr1_el1, ENABLE_FEAT_MTE2,			\
	    ID_AA64PFR1_EL1_MTE, 2U, 3U, FEAT_ENABLE_SECURE | FEAT_ENABLE_NS)

#define FEAT_RNG(gen)								\
	gen(feat_rng, id_aa64isar0_el1, ENABLE_FEAT_RNG,			\
	    ID_AA64ISAR0_RNDR, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_BTI(gen)								\
	gen(feat_bti, id_aa64pfr1_el1, ENABLE_BTI,				\
	    ID_AA64PFR1_EL1_BT, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_RNG_TRAP(gen)							\
	gen(feat_rng_trap, id_aa64pfr1_el1, ENABLE_FEAT_RNG_TRAP,		\
	    ID_AA64PFR1_EL1_RNDR_TRAP, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SSBS(gen)								\
	gen(feat_ssbs, id_aa64pfr1_el1, FEAT_STATE_CHECKED,			\
	    ID_AA64PFR1_EL1_SSBS, 1U, 2U, FEAT_ENABLE_ALL_WORLDS)

/* === v8.6 features === */
#define FEAT_AMUV1P1(gen)							\
	gen(feat_amuv1p1, id_aa64pfr0_el1, ENABLE_FEAT_AMUv1p1,			\
	    ID_AA64PFR0_AMU, 2U, 2U, FEAT_ENABLE_NS)

#define FEAT_FGT(gen)								\
	gen(feat_fgt, id_aa64mmfr0_el1, ENABLE_FEAT_FGT,			\
	    ID_AA64MMFR0_EL1_FGT, 1U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_ECV(gen)								\
	gen(feat_ecv, id_aa64mmfr0_el1, ENABLE_FEAT_ECV,			\
	    ID_AA64MMFR0_EL1_ECV, 1U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_TWED(gen)								\
	gen(feat_twed, id_aa64mmfr1_el1, ENABLE_FEAT_TWED,			\
	    ID_AA64MMFR1_EL1_TWED, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_MTPMU(gen)								\
	gen(feat_mtpmu, id_aa64dfr0_el1, DISABLE_MTPMU,				\
	    ID_AA64DFR0_MTPMU, 1U, 15U, FEAT_ENABLE_ALL_WORLDS)

/* FEAT_ECV_POFF in the architecture */
#define FEAT_ECV_V2(gen)							\
	gen(feat_ecv_v2, id_aa64mmfr0_el1, ENABLE_FEAT_ECV,			\
	    ID_AA64MMFR0_EL1_ECV, 2U, 2U, FEAT_ENABLE_ALL_WORLDS)

/* === v8.7 features === */
#define FEAT_HCX(gen)								\
	gen(feat_hcx, id_aa64mmfr1_el1, ENABLE_FEAT_HCX,			\
	    ID_AA64MMFR1_EL1_HCX, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_LS64_ACCDATA(gen)							\
	gen(feat_ls64_accdata, id_aa64isar1_el1, ENABLE_FEAT_LS64_ACCDATA,	\
	    ID_AA64ISAR1_LS64, 3U, 3U,  FEAT_ENABLE_ALL_WORLDS)

/* === v8.8 features === */
#define FEAT_MOPS(gen)								\
	gen(feat_mops, id_aa64isar2_el1, ENABLE_FEAT_MOPS,			\
	    ID_AA64ISAR2_EL1_MOPS, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_TCR2(gen)								\
	gen(feat_tcr2, id_aa64mmfr3_el1, ENABLE_FEAT_TCR2,			\
	    ID_AA64MMFR3_EL1_TCRX, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_NMI(gen)								\
	gen(feat_nmi, id_aa64pfr1_el1, FEAT_STATE_CHECKED,			\
	    ID_AA64PFR1_EL1_NMI, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* === v8.9 features === */
#define FEAT_FGT2(gen)								\
	gen(feat_fgt2, id_aa64mmfr0_el1, ENABLE_FEAT_FGT2,			\
	    ID_AA64MMFR0_EL1_FGT, 2U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_CLRBHB(gen)							\
	gen(feat_clrbhb, id_aa64isar2_el1, ENABLE_FEAT_CLRBHB,			\
	    ID_AA64ISAR2_CLRBHB, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_S2PIE(gen)								\
	gen(feat_s2pie, id_aa64mmfr3_el1, ENABLE_FEAT_S2PIE,			\
	    ID_AA64MMFR3_EL1_S2PIE, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_S1PIE(gen)								\
	gen(feat_s1pie, id_aa64mmfr3_el1, ENABLE_FEAT_S1PIE,			\
	    ID_AA64MMFR3_EL1_S1PIE, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_S2POE(gen)								\
	gen(feat_s2poe, id_aa64mmfr3_el1, ENABLE_FEAT_S2POE,			\
	    ID_AA64MMFR3_EL1_S2POE, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_S1POE(gen)								\
	gen(feat_s1poe, id_aa64mmfr3_el1, ENABLE_FEAT_S1POE,			\
	    ID_AA64MMFR3_EL1_S1POE, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_DEBUGV8P9(gen)							\
	gen(feat_debugv8p9, id_aa64dfr0_el1, ENABLE_FEAT_DEBUGV8P9,		\
	    ID_AA64DFR0_DEBUGVER, 11U, 11U, FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

#define FEAT_THE(gen)								\
	gen(feat_the, id_aa64pfr1_el1, ENABLE_FEAT_THE,				\
	    ID_AA64PFR1_EL1_THE, 1U, 1U, FEAT_ENABLE_NS)

#define FEAT_SCTLR2(gen)							\
	gen(feat_sctlr2, id_aa64mmfr3_el1, ENABLE_FEAT_SCTLR2,			\
	    ID_AA64MMFR3_EL1_SCTLR2, 1U, 1U, FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

#define FEAT_AIE(gen)								\
	gen(feat_aie, id_aa64mmfr3_el1, ENABLE_FEAT_AIE,			\
	    ID_AA64MMFR3_EL1_AIE, 1U, 1U, FEAT_ENABLE_NS)

#define FEAT_PFAR(gen)								\
	gen(feat_pfar, id_aa64pfr1_el1, ENABLE_FEAT_PFAR,			\
	    ID_AA64PFR1_EL1_PFAR, 1U, 1U, FEAT_ENABLE_NS)

#define FEAT_ADERR(gen)								\
	gen(feat_aderr, id_aa64mmfr3_el1, ENABLE_FEAT_AxERR,			\
	    ID_AA64MMFR3_EL1_ADERR, 2U, 3U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_ANERR(gen)								\
	gen(feat_anerr, id_aa64mmfr3_el1, ENABLE_FEAT_AxERR,			\
	    ID_AA64MMFR3_EL1_ANERR, 2U, 3U, FEAT_ENABLE_ALL_WORLDS)

/* === v9.0 features === */
#define FEAT_TRBE(gen)								\
	gen(feat_trbe, id_aa64dfr0_el1, ENABLE_TRBE_FOR_NS,			\
	    ID_AA64DFR0_TRACEBUFFER, 1U, 1U, FEAT_ENABLE_NS)

/* === v9.2 features === */
#define FEAT_RME(gen)								\
	gen(feat_rme, id_aa64pfr0_el1, ENABLE_FEAT_RME,				\
	    ID_AA64PFR0_FEAT_RME, 1U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_BRBE(gen)								\
	gen(feat_brbe, id_aa64dfr0_el1, ENABLE_BRBE_FOR_NS,			\
	    ID_AA64DFR0_BRBE, 1U, 2U, FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

#define FEAT_SME(gen)								\
	gen(feat_sme, id_aa64pfr1_el1, ENABLE_SME_FOR_NS,			\
	    ID_AA64PFR1_EL1_SME, 1U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SME_FA64(gen)							\
	gen(feat_sme_fa64, id_aa64smfr0_el1, FEAT_STATE_CHECKED,		\
	    ID_AA64SMFR0_EL1_SME_FA64, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* === v9.3 features === */
#define FEAT_SME2(gen)								\
	gen(feat_sme2, id_aa64pfr1_el1, ENABLE_SME2_FOR_NS,			\
	    ID_AA64PFR1_EL1_SME, 2U, 3U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_GCIE(gen)								\
	gen(feat_gcie, id_aa64pfr2_el1, ENABLE_FEAT_GCIE,			\
	    ID_AA64PFR2_EL1_GCIE, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_EBEP(gen)								\
	gen(feat_ebep, id_aa64dfr1_el1, ENABLE_FEAT_EBEP,			\
	    ID_AA64DFR1_EBEP, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_MEC(gen)								\
	gen(feat_mec, id_aa64mmfr3_el1, ENABLE_FEAT_MEC,			\
	    ID_AA64MMFR3_EL1_MEC, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_BRBEV1P1(gen)							\
	gen(feat_brbev1p1, id_aa64dfr0_el1, ENABLE_FEAT_BRBEV1P1,		\
	    ID_AA64DFR0_BRBE, 2U, 2U, FEAT_ENABLE_ALL_WORLDS)

/* === v9.4 features === */
#define FEAT_D128(gen)								\
	gen(feat_d128, id_aa64mmfr3_el1, ENABLE_FEAT_D128,			\
	    ID_AA64MMFR3_EL1_D128, 1U, 1U, FEAT_ENABLE_NS | FEAT_ENABLE_REALM)

#define FEAT_GCS(gen)								\
	gen(feat_gcs, id_aa64pfr1_el1, ENABLE_FEAT_GCS,				\
	    ID_AA64PFR1_EL1_GCS, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_RME_GDI(gen)							\
	gen(feat_rme_gdi, id_aa64mmfr4_el1, ENABLE_FEAT_RME_GDI,		\
	    ID_AA64MMFR4_EL1_RME_GDI, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_IDTE3(gen)								\
	gen(feat_idte3, id_aa64mmfr2_el1, ENABLE_FEAT_IDTE3,			\
	    ID_AA64MMFR2_EL1_IDS, 2U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SEBEP(gen)								\
	gen(feat_sebep, id_aa64dfr0_el1, FEAT_STATE_CHECKED,			\
	    ID_AA64DFR0_SEBEP, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* === v9.5 features === */
#define FEAT_FGWTE3(gen)							\
	gen(feat_fgwte3, id_aa64mmfr4_el1, ENABLE_FEAT_FGWTE3,			\
	    ID_AA64MMFR4_EL1_FGWTE3, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_PAUTH_LR(gen)							\
	gen(feat_pauth_lr, NA, ENABLE_FEAT_PAUTH_LR,				\
	    NA, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_FPMR(gen)								\
	gen(feat_fpmr, id_aa64pfr2_el1, ENABLE_FEAT_FPMR,			\
	    ID_AA64PFR2_EL1_FPMR, 1U, 1U, FEAT_ENABLE_NS)

#define FEAT_CPA2(gen)								\
	gen(feat_cpa2, id_aa64isar3_el1, ENABLE_FEAT_CPA2,			\
	    ID_AA64ISAR3_EL1_CPA, 2U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_HDBSS(gen)								\
	gen(feat_hdbss, id_aa64mmfr1_el1, ENABLE_FEAT_HDBSS,			\
	    ID_AA64MMFR1_EL1_HAFDBS, 4U, 4U, FEAT_ENABLE_NS)

#define FEAT_HACDBS(gen)							\
	gen(feat_hacdbs, id_aa64mmfr4_el1, ENABLE_FEAT_HACDBS,			\
	    ID_AA64MMFR4_EL1_HACDBS, 1U, 1U, FEAT_ENABLE_NS)

#define FEAT_RME_GPC2(gen)							\
	gen(feat_rme_gpc2, id_aa64pfr0_el1, FEAT_STATE_CHECKED,			\
	    ID_AA64PFR0_FEAT_RME, 2U, 2U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SRMASK(gen)							\
	gen(feat_srmask, id_aa64mmfr4_el1, ENABLE_FEAT_SRMASK,			\
	    ID_AA64MMFR4_EL1_SRMASK, 1U, 1U, FEAT_ENABLE_NS)

/* === v9.6 features === */
#define FEAT_UINJ(gen)								\
	gen(feat_uinj, id_aa64pfr2_el1, ENABLE_FEAT_UINJ,			\
	    ID_AA64PFR2_EL1_UINJ, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_STEP2(gen)								\
	gen(feat_step2, id_aa64dfr2_el1, ENABLE_FEAT_STEP2,			\
	    ID_AA64DFR2_STEP, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define FEAT_SPE_EXC(gen)							\
	gen(feat_spe_exc, id_aa64dfr2_el1, ENABLE_FEAT_SPEV1P5,			\
	    ID_AA64DFR2_SPE_EXC, 1U, 1U, FEAT_ENABLE_NS)

#define FEAT_SPE_NVM(gen)							\
	gen(feat_spe_nvm, id_aa64dfr2_el1, ENABLE_FEAT_SPEV1P5,			\
	    ID_AA64DFR2_SPE_NVM, 1U, 1U, FEAT_ENABLE_NS)

/* Auxiliary features. Don't relate to an architectural feature directly */
#define FEAT_AMU_AUX(gen)							\
	gen(feat_amu_aux, amcfgr_el0, ENABLE_AMU_AUXILIARY_COUNTERS,		\
	    AMCFGR_EL0_NCG, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/* 9.3 but no standalone ID field */
#define FEAT_MPAM_PE_BW_CTRL(gen)						\
	gen(feat_mpam_pe_bw_ctrl, mpamidr_el1, ENABLE_FEAT_MPAM_PE_BW_CTRL,	\
	    MPAMIDR_HAS_BW_CTRL, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

#define CTX_PAUTH(gen)								\
	gen(ctx_pauth, NA, CTX_INCLUDE_PAUTH_REGS,				\
	    NA, NA, NA, NA)

/* Non-Architectural features */
#define FEAT_MORELLO(gen)							\
	gen(feat_morello, id_aa64pfr1_el1, ENABLE_FEAT_MORELLO,			\
	    ID_AA64PFR1_EL1_CE, 1U, 1U, FEAT_ENABLE_ALL_WORLDS)

/*
 * List of "standard" features. They obey the common format of the ID register
 * scheme (D24.1.3 in the ARM) and can have all boilerplate generated
 * automatically.
 */
#define CPUFEAT_LIST(gen)							\
	FEAT_CSV2_2(gen)							\
	FEAT_CSV2_3(gen)							\
	FEAT_TGRAN16K(gen)							\
	FEAT_AES(gen)								\
	FEAT_SHA1(gen)								\
	FEAT_PAN(gen)								\
	FEAT_LSE(gen)								\
	FEAT_VHE(gen)								\
	FEAT_RAS(gen)								\
	FEAT_IESB(gen)								\
	FEAT_SVE(gen)								\
	FEAT_TTCNP(gen)								\
	FEAT_UAO(gen)								\
	FEAT_DIT(gen)								\
	FEAT_TTST(gen)								\
	FEAT_AMU(gen)								\
	FEAT_NV2(gen)								\
	FEAT_SEL2(gen)								\
	FEAT_SB(gen)								\
	FEAT_MTE2(gen)								\
	FEAT_RNG(gen)								\
	FEAT_BTI(gen)								\
	FEAT_RNG_TRAP(gen)							\
	FEAT_SSBS(gen)								\
	FEAT_AMUV1P1(gen)							\
	FEAT_FGT(gen)								\
	FEAT_ECV(gen)								\
	FEAT_TWED(gen)								\
	FEAT_ECV_V2(gen)							\
	FEAT_HCX(gen)								\
	FEAT_LS64_ACCDATA(gen)							\
	FEAT_MOPS(gen)								\
	FEAT_TCR2(gen)								\
	FEAT_NMI(gen)								\
	FEAT_FGT2(gen)								\
	FEAT_CLRBHB(gen)							\
	FEAT_S2PIE(gen)								\
	FEAT_S1PIE(gen)								\
	FEAT_S2POE(gen)								\
	FEAT_S1POE(gen)								\
	FEAT_THE(gen)								\
	FEAT_SCTLR2(gen)							\
	FEAT_AIE(gen)								\
	FEAT_PFAR(gen)								\
	FEAT_ADERR(gen)								\
	FEAT_ANERR(gen)								\
	FEAT_RME(gen)								\
	FEAT_SME(gen)								\
	FEAT_SME_FA64(gen)							\
	FEAT_SME2(gen)								\
	FEAT_GCIE(gen)								\
	FEAT_MEC(gen)								\
	FEAT_D128(gen)								\
	FEAT_GCS(gen)								\
	FEAT_RME_GDI(gen)							\
	FEAT_IDTE3(gen)								\
	FEAT_FGWTE3(gen)							\
	FEAT_FPMR(gen)								\
	FEAT_CPA2(gen)								\
	FEAT_HDBSS(gen)								\
	FEAT_HACDBS(gen)							\
	FEAT_RME_GPC2(gen)							\
	FEAT_UINJ(gen)								\
	FEAT_STEP2(gen)								\
	FEAT_SPE_EXC(gen)							\
	FEAT_SPE_NVM(gen)							\
	FEAT_SRMASK(gen)							\
	FEAT_MORELLO(gen)

/*
 * Same as the above list, except presence of the feature is allowed to be
 * asymmetric across cores.
 */
#define CPUFEAT_PERCPU_LIST(gen)						\
	FEAT_PMUV3(gen)								\
	FEAT_SYS_REG_TRACE(gen)							\
	FEAT_SPE(gen)								\
	FEAT_TRF(gen)								\
	FEAT_MTPMU(gen)								\
	FEAT_DEBUGV8P9(gen)							\
	FEAT_TRBE(gen)								\
	FEAT_BRBE(gen)								\
	FEAT_EBEP(gen)								\
	FEAT_BRBEV1P1(gen)							\
	FEAT_SEBEP(gen)

CPUFEAT_LIST(CREATE_FEATURE_FUNCS)
CPUFEAT_PERCPU_LIST(CREATE_FEATURE_FUNCS)

/*******************************************************************************
 * Helpers that can't match the standard pattern
 ******************************************************************************/
__attribute__((always_inline))
static inline bool is_feat_pauth_present(void)
{
	uint64_t mask_id_aa64isar1 =
		MASK(ID_AA64ISAR1_GPI) |
		MASK(ID_AA64ISAR1_GPA) |
		MASK(ID_AA64ISAR1_API) |
		MASK(ID_AA64ISAR1_APA);

	uint64_t mask_id_aa64isar2 =
		MASK(ID_AA64ISAR2_GPA3) |
		MASK(ID_AA64ISAR2_APA3);

	/*
	 * If any of the fields is not zero or QARMA3 is present,
	 * PAuth is present
	 */
	return (read_id_aa64isar1_el1() & mask_id_aa64isar1) != 0U ||
		(read_id_aa64isar2_el1() & mask_id_aa64isar2) != 0U;

}

/* Make this simple enough to fit into the pattern */
static inline unsigned int _read_feat_pauth_id_field(void)
{
	return (is_feat_pauth_present()) ? 1 : 0;
}

static inline bool is_ctx_pauth_present(void) { return is_feat_pauth_present(); }

__attribute__((always_inline))
static inline bool is_feat_pauth_lr_present(void)
{
	/*
	 * FEAT_PAUTH_LR support is indicated by up to 3 fields, if one or more
	 * of these is 0b0110 then the feature is present.
	 *   1) id_aa64isr1_el1.api
	 *   2) id_aa64isr1_el1.apa
	 *   3) id_aa64isr2_el1.apa3
	 */
	if (EXTRACT(ID_AA64ISAR1_API, read_id_aa64isar1_el1()) == 0b0110) {
		return true;
	}
	if (EXTRACT(ID_AA64ISAR1_APA, read_id_aa64isar1_el1()) == 0b0110) {
		return true;
	}
	if (EXTRACT(ID_AA64ISAR2_APA3, read_id_aa64isar2_el1()) == 0b0110) {
		return true;
	}
	return false;
}

/* Make this simple enough to fit into the pattern */
static inline unsigned int _read_feat_pauth_lr_id_field(void)
{
	return (is_feat_pauth_lr_present()) ? 1 : 0;
}

__attribute__((always_inline))
static inline unsigned int _read_feat_mpam_id_field(void)
{
	return EXTRACT(ID_AA64PFR0_MPAM, read_id_aa64pfr0_el1()) << ID_AA64PFR0_MPAM_WIDTH |
	       EXTRACT(ID_AA64PFR1_MPAM_FRAC, read_id_aa64pfr1_el1());
}

/*******************************************************************************
 * Features who have at least one helper that doesn't match the standard
 * pattern. These need special attention when integrating with FEAT_IDTE3 and
 * FEATURE_DETECTION
 ******************************************************************************/
FEAT_PAUTH(CREATE_FEATURE_SUPPORTED)
FEAT_PAUTH_LR(CREATE_FEATURE_SUPPORTED)

FEAT_MPAM(CREATE_FEATURE_PRESENT)
FEAT_MPAM(CREATE_FEATURE_SUPPORTED)

FEAT_AMU_AUX(CREATE_FEATURE_FUNCS)

/* Automatic inclusion for FEATURE_DETECTION. IDTE3 is on a case-by-case basis */
#define CPUFEAT_SPECIAL_FEAT_DETECT_LIST(gen)					\
	FEAT_PAUTH(gen)								\
	FEAT_PAUTH_LR(gen)							\
	FEAT_MPAM(gen)

/* Exclude these from FEATURE_DETECTION and IDTE3 */
CTX_PAUTH(CREATE_FEATURE_SUPPORTED)
FEAT_MPAM_PE_BW_CTRL(CREATE_FEATURE_FUNCS)

/*******************************************************************************
 * Non-standard, not directly architectural helpers
 ******************************************************************************/
__attribute__((always_inline))
static inline bool is_armv7_gentimer_present(void)
{
	/* The Generic Timer is always present in an ARMv8-A implementation */
	return true;
}

__attribute__((always_inline))
static inline bool is_feat_sxpoe_supported(void)
{
	return is_feat_s1poe_supported() || is_feat_s2poe_supported();
}

__attribute__((always_inline))
static inline bool is_feat_sxpie_supported(void)
{
	return is_feat_s1pie_supported() || is_feat_s2pie_supported();
}

__attribute__((always_inline))
static inline bool is_feat_crypto_supported(void)
{
	return is_feat_aes_supported() || is_feat_sha1_supported();
}

__attribute__((always_inline))
static inline bool is_feat_tgran4K_present(void)
{
	unsigned int tgranx = (unsigned int)EXTRACT(ID_AA64MMFR0_EL1_TGRAN4,
						    read_id_aa64mmfr0_el1());
	return (tgranx < 8U);
}

__attribute__((always_inline))
static inline bool is_feat_tgran64K_present(void)
{
	unsigned int tgranx = (unsigned int)EXTRACT(ID_AA64MMFR0_EL1_TGRAN64,
						    read_id_aa64mmfr0_el1());
	return (tgranx < 8U);
}

/* FEAT_DoubleFault2: Double Fault Extension v2 */
__attribute__((always_inline))
static inline bool is_feat_doublefault2_supported(void)
{
	/* this function is a placeholder until proper support is added */
	return false;
}
#endif /* ARCH_FEATURES_H */
