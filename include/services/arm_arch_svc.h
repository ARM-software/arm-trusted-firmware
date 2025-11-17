/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_ARCH_SVC_H
#define ARM_ARCH_SVC_H

#define SMCCC_VERSION			U(0x80000000)
#define SMCCC_ARCH_FEATURES		U(0x80000001)
#define SMCCC_ARCH_SOC_ID		U(0x80000002)
#define SMCCC_ARCH_WORKAROUND_1		U(0x80008000)
#define SMCCC_ARCH_WORKAROUND_2		U(0x80007FFF)
#define SMCCC_ARCH_WORKAROUND_3		U(0x80003FFF)
#define SMCCC_ARCH_FEATURE_AVAILABILITY		U(0x80000003)
#define SMCCC_ARCH_WORKAROUND_4		U(0x80000004)

#define SMCCC_GET_SOC_VERSION		U(0)
#define SMCCC_GET_SOC_REVISION		U(1)
#define SMCCC_GET_SOC_NAME		U(2)

#define SMCCC_SOC_NAME_LEN		U(136)

#ifndef __ASSEMBLER__
#if ARCH_FEATURE_AVAILABILITY
#include <lib/cassert.h>

#if ENABLE_FEAT_FGT2
#define SCR_FEAT_FGT2 SCR_FGTEN2_BIT
#else
#define SCR_FEAT_FGT2 (0)
#endif

#if ENABLE_FEAT_FPMR
#define SCR_FEAT_FPMR SCR_EnFPM_BIT
#else
#define SCR_FEAT_FPMR (0)
#endif

#if ENABLE_FEAT_D128
#define SCR_FEAT_D128 SCR_D128En_BIT
#else
#define SCR_FEAT_D128 (0)
#endif

#if ENABLE_FEAT_S1PIE
#define SCR_FEAT_S1PIE SCR_PIEN_BIT
#else
#define SCR_FEAT_S1PIE (0)
#endif

#if ENABLE_FEAT_SCTLR2
#define SCR_FEAT_SCTLR2 SCR_SCTLR2En_BIT
#else
#define SCR_FEAT_SCTLR2 (0)
#endif

#if ENABLE_FEAT_TCR2
#define SCR_FEAT_TCR2 SCR_TCR2EN_BIT
#else
#define SCR_FEAT_TCR2 (0)
#endif

#if ENABLE_FEAT_THE
#define SCR_FEAT_THE SCR_RCWMASKEn_BIT
#else
#define SCR_FEAT_THE (0)
#endif

#if ENABLE_SME_FOR_NS
#define SCR_FEAT_SME SCR_ENTP2_BIT
#else
#define SCR_FEAT_SME (0)
#endif

#if ENABLE_FEAT_GCS
#define SCR_FEAT_GCS SCR_GCSEn_BIT
#else
#define SCR_FEAT_GCS (0)
#endif

#if ENABLE_FEAT_HCX
#define SCR_FEAT_HCX SCR_HXEn_BIT
#else
#define SCR_FEAT_HCX (0)
#endif

#if ENABLE_FEAT_LS64_ACCDATA
#define SCR_FEAT_LS64_ACCDATA (SCR_ADEn_BIT | SCR_EnAS0_BIT)
#else
#define SCR_FEAT_LS64_ACCDATA (0)
#endif

#if ENABLE_FEAT_AMUv1p1
#define SCR_FEAT_AMUv1p1 SCR_AMVOFFEN_BIT
#else
#define SCR_FEAT_AMUv1p1 (0)
#endif

#if ENABLE_FEAT_TWED
#define SCR_FEAT_TWED SCR_TWEDEn_BIT
#else
#define SCR_FEAT_TWED (0)
#endif

#if ENABLE_FEAT_ECV
#define SCR_FEAT_ECV SCR_ECVEN_BIT
#else
#define SCR_FEAT_ECV (0)
#endif

#if ENABLE_FEAT_FGT
#define SCR_FEAT_FGT SCR_FGTEN_BIT
#else
#define SCR_FEAT_FGT (0)
#endif

#if ENABLE_FEAT_MTE2
#define SCR_FEAT_MTE2 SCR_ATA_BIT
#else
#define SCR_FEAT_MTE2 (0)
#endif

#if ENABLE_FEAT_CSV2_2
#define SCR_FEAT_CSV2_2 SCR_EnSCXT_BIT
#else
#define SCR_FEAT_CSV2_2 (0)
#endif

#if !RAS_TRAP_NS_ERR_REC_ACCESS
#define SCR_FEAT_RAS SCR_TERR_BIT
#else
#define SCR_FEAT_RAS (0)
#endif

#if ENABLE_FEAT_MEC
#define SCR_FEAT_MEC SCR_MECEn_BIT
#else
#define SCR_FEAT_MEC (0)
#endif

#if ENABLE_FEAT_AIE
#define SCR_FEAT_AIE SCR_AIEn_BIT
#else
#define SCR_FEAT_AIE (0)
#endif

#if ENABLE_FEAT_PFAR
#define SCR_FEAT_PFAR SCR_PFAREn_BIT
#else
#define SCR_FEAT_PFAR (0)
#endif

#if ENABLE_FEAT_IDTE3
#define SCR_FEAT_IDTE3 (SCR_TID3_BIT | SCR_TID5_BIT)
#else
#define SCR_FEAT_IDTE3 (0)
#endif

#ifndef SCR_PLAT_FEATS
#define SCR_PLAT_FEATS (0)
#endif
#ifndef SCR_PLAT_FLIPPED
#define SCR_PLAT_FLIPPED (0)
#endif
#ifndef SCR_PLAT_IGNORED
#define SCR_PLAT_IGNORED (0)
#endif

#ifndef CPTR_PLAT_FEATS
#define CPTR_PLAT_FEATS (0)
#endif
#ifndef CPTR_PLAT_FLIPPED
#define CPTR_PLAT_FLIPPED (0)
#endif

#ifndef MDCR_PLAT_FEATS
#define MDCR_PLAT_FEATS (0)
#endif
#ifndef MDCR_PLAT_FLIPPED
#define MDCR_PLAT_FLIPPED (0)
#endif
#ifndef MDCR_PLAT_IGNORED
#define MDCR_PLAT_IGNORED (0)
#endif
/*
 * XYZ_EL3_FEATS - list all bits that are relevant for feature enablement. It's
 * a constant list based on what features are expected. This relies on the fact
 * that if the feature is in any way disabled, then the relevant bit will not be
 * written by context management.
 *
 * XYZ_EL3_FLIPPED - bits with an active 0, rather than the usual active 1. The
 * spec always uses active 1 to mean that the feature will not trap.
 *
 * XYZ_EL3_IGNORED - list of all bits that are not relevant for feature
 * enablement and should not be reported to lower ELs
 */
#define SCR_EL3_FEATS (								\
	SCR_FEAT_FGT2		|						\
	SCR_FEAT_FPMR		|						\
	SCR_FEAT_MEC		|						\
	SCR_FEAT_D128		|						\
	SCR_FEAT_S1PIE		|						\
	SCR_FEAT_SCTLR2		|						\
	SCR_FEAT_TCR2		|						\
	SCR_FEAT_THE		|						\
	SCR_FEAT_SME		|						\
	SCR_FEAT_GCS		|						\
	SCR_FEAT_HCX		|						\
	SCR_FEAT_LS64_ACCDATA	|						\
	SCR_FEAT_AMUv1p1	|						\
	SCR_FEAT_TWED		|						\
	SCR_FEAT_ECV		|						\
	SCR_FEAT_FGT		|						\
	SCR_FEAT_MTE2		|						\
	SCR_FEAT_CSV2_2		|						\
	SCR_APK_BIT		| /* FEAT_Pauth */				\
	SCR_FEAT_RAS		|						\
	SCR_FEAT_AIE		|						\
	SCR_FEAT_PFAR		|						\
	SCR_FEAT_IDTE3		|						\
	SCR_PLAT_FEATS)
#define SCR_EL3_FLIPPED (							\
	SCR_FEAT_RAS		|						\
	SCR_PLAT_FLIPPED)
#define SCR_EL3_IGNORED (							\
	SCR_API_BIT		|						\
	SCR_RW_BIT		|						\
	SCR_SIF_BIT		|						\
	SCR_HCE_BIT		|						\
	SCR_FIQ_BIT		|						\
	SCR_IRQ_BIT		|						\
	SCR_NS_BIT		|						\
	SCR_NSE_BIT		|						\
	SCR_RES1_BITS		|						\
	SCR_EEL2_BIT		|						\
	SCR_PLAT_IGNORED)
CASSERT((SCR_EL3_FEATS & SCR_EL3_IGNORED) == 0, scr_feat_is_ignored);
CASSERT((SCR_EL3_FLIPPED & SCR_EL3_FEATS) == SCR_EL3_FLIPPED, scr_flipped_not_a_feat);

#if ENABLE_SYS_REG_TRACE_FOR_NS
#define CPTR_SYS_REG_TRACE (TCPAC_BIT | TTA_BIT)
#else
#define CPTR_SYS_REG_TRACE (0)
#endif

#if ENABLE_FEAT_AMU
#define CPTR_FEAT_AMU TAM_BIT
#else
#define CPTR_FEAT_AMU (0)
#endif

#if ENABLE_SME_FOR_NS
#define CPTR_FEAT_SME ESM_BIT
#else
#define CPTR_FEAT_SME (0)
#endif

#if ENABLE_SVE_FOR_NS
#define CPTR_FEAT_SVE CPTR_EZ_BIT
#else
#define CPTR_FEAT_SVE (0)
#endif

#define CPTR_EL3_FEATS (							\
	CPTR_SYS_REG_TRACE	|						\
	CPTR_FEAT_AMU		|						\
	CPTR_FEAT_SME		|						\
	TFP_BIT			|						\
	CPTR_FEAT_SVE		|						\
	CPTR_PLAT_FEATS)
#define CPTR_EL3_FLIPPED (							\
	CPTR_SYS_REG_TRACE	|						\
	CPTR_FEAT_AMU		|						\
	TFP_BIT			|						\
	CPTR_PLAT_FLIPPED)
CASSERT((CPTR_EL3_FLIPPED & CPTR_EL3_FEATS) == CPTR_EL3_FLIPPED, cptr_flipped_not_a_feat);

/*
 * Some features enables are expressed with more than 1 bit in order to cater
 * for multi world enablement. In those cases (BRB, TRB, SPE) only the last bit
 * is used and reported. This (ab)uses the convenient fact that the last bit
 * always means "enabled for this world" when context switched correctly.
 * The per-world values have been adjusted such that this is always true.
 */
#if ENABLE_BRBE_FOR_NS
#define MDCR_FEAT_BRBE MDCR_SBRBE(1UL)
#else
#define MDCR_FEAT_BRBE (0)
#endif

#if ENABLE_FEAT_FGT
#define MDCR_FEAT_FGT MDCR_TDCC_BIT
#else
#define MDCR_FEAT_FGT (0)
#endif

#if ENABLE_TRBE_FOR_NS
#define MDCR_FEAT_TRBE MDCR_NSTB_EN_BIT
#else
#define MDCR_FEAT_TRBE (0)
#endif

#if ENABLE_TRF_FOR_NS
#define MDCR_FEAT_TRF MDCR_TTRF_BIT
#else
#define MDCR_FEAT_TRF (0)
#endif

#if ENABLE_SPE_FOR_NS
#define MDCR_FEAT_SPE MDCR_NSPB_EN_BIT
#else
#define MDCR_FEAT_SPE (0)
#endif

#if ENABLE_FEAT_DEBUGV8P9
#define MDCR_DEBUGV8P9 MDCR_EBWE_BIT
#else
#define MDCR_DEBUGV8P9 (0)
#endif

#if ENABLE_FEAT_EBEP
#define MDCR_FEAT_EBEP MDCR_PMEE(MDCR_PMEE_CTRL_EL2)
#else
#define MDCR_FEAT_EBEP (0)
#endif

#define MDCR_EL3_FEATS (							\
	MDCR_DEBUGV8P9		|						\
	MDCR_FEAT_BRBE		|						\
	MDCR_FEAT_FGT		|						\
	MDCR_FEAT_TRBE		|						\
	MDCR_FEAT_TRF		|						\
	MDCR_FEAT_SPE		|						\
	MDCR_FEAT_EBEP		|						\
	MDCR_TDOSA_BIT		|						\
	MDCR_TDA_BIT		|						\
	MDCR_EnPM2_BIT		|						\
	MDCR_TPM_BIT		| /* FEAT_PMUv3 */				\
	MDCR_PLAT_FEATS)
#define MDCR_EL3_FLIPPED (							\
	MDCR_FEAT_FGT		|						\
	MDCR_FEAT_TRF		|						\
	MDCR_TDOSA_BIT		|						\
	MDCR_TDA_BIT		|						\
	MDCR_TPM_BIT		|						\
	MDCR_PLAT_FLIPPED)
#define MDCR_EL3_IGNORED (							\
	MDCR_EnPMS3_BIT		|						\
	MDCR_EnPMSN_BIT		|						\
	MDCR_SBRBE(2UL)		|						\
	MDCR_MTPME_BIT		|						\
	MDCR_NSTBE_BIT		|						\
	MDCR_NSTB_SS_BIT	|						\
	MDCR_MCCD_BIT		|						\
	MDCR_SCCD_BIT		|						\
	MDCR_SDD_BIT		|						\
	MDCR_SPD32(3UL)		|						\
	MDCR_NSPB_SS_BIT	|						\
	MDCR_NSPBE_BIT		|						\
	MDCR_PLAT_IGNORED)
CASSERT((MDCR_EL3_FEATS & MDCR_EL3_IGNORED) == 0, mdcr_feat_is_ignored);
CASSERT((MDCR_EL3_FLIPPED & MDCR_EL3_FEATS) == MDCR_EL3_FLIPPED, mdcr_flipped_not_a_feat);

#define MPAM3_EL3_FEATS		(MPAM3_EL3_TRAPLOWER_BIT)
#define MPAM3_EL3_FLIPPED	(MPAM3_EL3_TRAPLOWER_BIT)
#define MPAM3_EL3_IGNORED	(MPAM3_EL3_MPAMEN_BIT)
CASSERT((MPAM3_EL3_FEATS & MPAM3_EL3_IGNORED) == 0, mpam3_feat_is_ignored);
CASSERT((MPAM3_EL3_FLIPPED & MPAM3_EL3_FEATS) == MPAM3_EL3_FLIPPED, mpam3_flipped_not_a_feat);

/* The hex representations of these registers' S3 encoding */
#define SCR_EL3_OPCODE  			U(0x1E1100)
#define CPTR_EL3_OPCODE 			U(0x1E1140)
#define MDCR_EL3_OPCODE 			U(0x1E1320)
#define MPAM3_EL3_OPCODE 			U(0x1EA500)

#endif /* ARCH_FEATURE_AVAILABILITY */
#endif /* __ASSEMBLER__ */
#endif /* ARM_ARCH_SVC_H */
