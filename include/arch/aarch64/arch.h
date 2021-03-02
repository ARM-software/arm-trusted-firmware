/*
 * Copyright (c) 2013-2021, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_H
#define ARCH_H

#include <lib/utils_def.h>

/*******************************************************************************
 * MIDR bit definitions
 ******************************************************************************/
#define MIDR_IMPL_MASK		U(0xff)
#define MIDR_IMPL_SHIFT		U(0x18)
#define MIDR_VAR_SHIFT		U(20)
#define MIDR_VAR_BITS		U(4)
#define MIDR_VAR_MASK		U(0xf)
#define MIDR_REV_SHIFT		U(0)
#define MIDR_REV_BITS		U(4)
#define MIDR_REV_MASK		U(0xf)
#define MIDR_PN_MASK		U(0xfff)
#define MIDR_PN_SHIFT		U(0x4)

/*******************************************************************************
 * MPIDR macros
 ******************************************************************************/
#define MPIDR_MT_MASK		(ULL(1) << 24)
#define MPIDR_CPU_MASK		MPIDR_AFFLVL_MASK
#define MPIDR_CLUSTER_MASK	(MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS)
#define MPIDR_AFFINITY_BITS	U(8)
#define MPIDR_AFFLVL_MASK	ULL(0xff)
#define MPIDR_AFF0_SHIFT	U(0)
#define MPIDR_AFF1_SHIFT	U(8)
#define MPIDR_AFF2_SHIFT	U(16)
#define MPIDR_AFF3_SHIFT	U(32)
#define MPIDR_AFF_SHIFT(_n)	MPIDR_AFF##_n##_SHIFT
#define MPIDR_AFFINITY_MASK	ULL(0xff00ffffff)
#define MPIDR_AFFLVL_SHIFT	U(3)
#define MPIDR_AFFLVL0		ULL(0x0)
#define MPIDR_AFFLVL1		ULL(0x1)
#define MPIDR_AFFLVL2		ULL(0x2)
#define MPIDR_AFFLVL3		ULL(0x3)
#define MPIDR_AFFLVL(_n)	MPIDR_AFFLVL##_n
#define MPIDR_AFFLVL0_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL1_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL2_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL3_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF3_SHIFT) & MPIDR_AFFLVL_MASK)
/*
 * The MPIDR_MAX_AFFLVL count starts from 0. Take care to
 * add one while using this macro to define array sizes.
 * TODO: Support only the first 3 affinity levels for now.
 */
#define MPIDR_MAX_AFFLVL	U(2)

#define MPID_MASK		(MPIDR_MT_MASK				 | \
				 (MPIDR_AFFLVL_MASK << MPIDR_AFF3_SHIFT) | \
				 (MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT) | \
				 (MPIDR_AFFLVL_MASK << MPIDR_AFF1_SHIFT) | \
				 (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT))

#define MPIDR_AFF_ID(mpid, n)					\
	(((mpid) >> MPIDR_AFF_SHIFT(n)) & MPIDR_AFFLVL_MASK)

/*
 * An invalid MPID. This value can be used by functions that return an MPID to
 * indicate an error.
 */
#define INVALID_MPID		U(0xFFFFFFFF)

/*******************************************************************************
 * Definitions for CPU system register interface to GICv3
 ******************************************************************************/
#define ICC_IGRPEN1_EL1		S3_0_C12_C12_7
#define ICC_SGI1R		S3_0_C12_C11_5
#define ICC_SRE_EL1		S3_0_C12_C12_5
#define ICC_SRE_EL2		S3_4_C12_C9_5
#define ICC_SRE_EL3		S3_6_C12_C12_5
#define ICC_CTLR_EL1		S3_0_C12_C12_4
#define ICC_CTLR_EL3		S3_6_C12_C12_4
#define ICC_PMR_EL1		S3_0_C4_C6_0
#define ICC_RPR_EL1		S3_0_C12_C11_3
#define ICC_IGRPEN1_EL3		S3_6_c12_c12_7
#define ICC_IGRPEN0_EL1		S3_0_c12_c12_6
#define ICC_HPPIR0_EL1		S3_0_c12_c8_2
#define ICC_HPPIR1_EL1		S3_0_c12_c12_2
#define ICC_IAR0_EL1		S3_0_c12_c8_0
#define ICC_IAR1_EL1		S3_0_c12_c12_0
#define ICC_EOIR0_EL1		S3_0_c12_c8_1
#define ICC_EOIR1_EL1		S3_0_c12_c12_1
#define ICC_SGI0R_EL1		S3_0_c12_c11_7

/*******************************************************************************
 * Definitions for EL2 system registers for save/restore routine
 ******************************************************************************/

#define CNTPOFF_EL2		S3_4_C14_C0_6
#define HAFGRTR_EL2		S3_4_C3_C1_6
#define HDFGRTR_EL2		S3_4_C3_C1_4
#define HDFGWTR_EL2		S3_4_C3_C1_5
#define HFGITR_EL2		S3_4_C1_C1_6
#define HFGRTR_EL2		S3_4_C1_C1_4
#define HFGWTR_EL2		S3_4_C1_C1_5
#define ICH_HCR_EL2		S3_4_C12_C11_0
#define ICH_VMCR_EL2		S3_4_C12_C11_7
#define MPAMVPM0_EL2		S3_4_C10_C5_0
#define MPAMVPM1_EL2		S3_4_C10_C5_1
#define MPAMVPM2_EL2		S3_4_C10_C5_2
#define MPAMVPM3_EL2		S3_4_C10_C5_3
#define MPAMVPM4_EL2		S3_4_C10_C5_4
#define MPAMVPM5_EL2		S3_4_C10_C5_5
#define MPAMVPM6_EL2		S3_4_C10_C5_6
#define MPAMVPM7_EL2		S3_4_C10_C5_7
#define MPAMVPMV_EL2		S3_4_C10_C4_1
#define TRFCR_EL2		S3_4_C1_C2_1
#define PMSCR_EL2		S3_4_C9_C9_0
#define TFSR_EL2		S3_4_C5_C6_0

/*******************************************************************************
 * Generic timer memory mapped registers & offsets
 ******************************************************************************/
#define CNTCR_OFF			U(0x000)
#define CNTCV_OFF			U(0x008)
#define CNTFID_OFF			U(0x020)

#define CNTCR_EN			(U(1) << 0)
#define CNTCR_HDBG			(U(1) << 1)
#define CNTCR_FCREQ(x)			((x) << 8)

/*******************************************************************************
 * System register bit definitions
 ******************************************************************************/
/* CLIDR definitions */
#define LOUIS_SHIFT		U(21)
#define LOC_SHIFT		U(24)
#define CTYPE_SHIFT(n)		U(3 * (n - 1))
#define CLIDR_FIELD_WIDTH	U(3)

/* CSSELR definitions */
#define LEVEL_SHIFT		U(1)

/* Data cache set/way op type defines */
#define DCISW			U(0x0)
#define DCCISW			U(0x1)
#if ERRATA_A53_827319
#define DCCSW			DCCISW
#else
#define DCCSW			U(0x2)
#endif

/* ID_AA64PFR0_EL1 definitions */
#define ID_AA64PFR0_EL0_SHIFT	U(0)
#define ID_AA64PFR0_EL1_SHIFT	U(4)
#define ID_AA64PFR0_EL2_SHIFT	U(8)
#define ID_AA64PFR0_EL3_SHIFT	U(12)
#define ID_AA64PFR0_AMU_SHIFT	U(44)
#define ID_AA64PFR0_AMU_MASK	ULL(0xf)
#define ID_AA64PFR0_AMU_NOT_SUPPORTED	U(0x0)
#define ID_AA64PFR0_AMU_V1	U(0x1)
#define ID_AA64PFR0_AMU_V1P1	U(0x2)
#define ID_AA64PFR0_ELX_MASK	ULL(0xf)
#define ID_AA64PFR0_GIC_SHIFT	U(24)
#define ID_AA64PFR0_GIC_WIDTH	U(4)
#define ID_AA64PFR0_GIC_MASK	ULL(0xf)
#define ID_AA64PFR0_SVE_SHIFT	U(32)
#define ID_AA64PFR0_SVE_MASK	ULL(0xf)
#define ID_AA64PFR0_SEL2_SHIFT	U(36)
#define ID_AA64PFR0_SEL2_MASK	ULL(0xf)
#define ID_AA64PFR0_MPAM_SHIFT	U(40)
#define ID_AA64PFR0_MPAM_MASK	ULL(0xf)
#define ID_AA64PFR0_DIT_SHIFT	U(48)
#define ID_AA64PFR0_DIT_MASK	ULL(0xf)
#define ID_AA64PFR0_DIT_LENGTH	U(4)
#define ID_AA64PFR0_DIT_SUPPORTED	U(1)
#define ID_AA64PFR0_CSV2_SHIFT	U(56)
#define ID_AA64PFR0_CSV2_MASK	ULL(0xf)
#define ID_AA64PFR0_CSV2_LENGTH	U(4)

/* Exception level handling */
#define EL_IMPL_NONE		ULL(0)
#define EL_IMPL_A64ONLY		ULL(1)
#define EL_IMPL_A64_A32		ULL(2)

/* ID_AA64DFR0_EL1.PMS definitions (for ARMv8.2+) */
#define ID_AA64DFR0_PMS_SHIFT	U(32)
#define ID_AA64DFR0_PMS_MASK	ULL(0xf)

/* ID_AA64DFR0_EL1.MTPMU definitions (for ARMv8.6+) */
#define ID_AA64DFR0_MTPMU_SHIFT		U(48)
#define ID_AA64DFR0_MTPMU_MASK		ULL(0xf)
#define ID_AA64DFR0_MTPMU_SUPPORTED	ULL(1)

/* ID_AA64ISAR0_EL1 definitions */
#define ID_AA64ISAR0_RNDR_SHIFT U(60)
#define ID_AA64ISAR0_RNDR_MASK  ULL(0xf)

/* ID_AA64ISAR1_EL1 definitions */
#define ID_AA64ISAR1_EL1	S3_0_C0_C6_1
#define ID_AA64ISAR1_GPI_SHIFT	U(28)
#define ID_AA64ISAR1_GPI_MASK	ULL(0xf)
#define ID_AA64ISAR1_GPA_SHIFT	U(24)
#define ID_AA64ISAR1_GPA_MASK	ULL(0xf)
#define ID_AA64ISAR1_API_SHIFT	U(8)
#define ID_AA64ISAR1_API_MASK	ULL(0xf)
#define ID_AA64ISAR1_APA_SHIFT	U(4)
#define ID_AA64ISAR1_APA_MASK	ULL(0xf)

/* ID_AA64MMFR0_EL1 definitions */
#define ID_AA64MMFR0_EL1_PARANGE_SHIFT	U(0)
#define ID_AA64MMFR0_EL1_PARANGE_MASK	ULL(0xf)

#define PARANGE_0000	U(32)
#define PARANGE_0001	U(36)
#define PARANGE_0010	U(40)
#define PARANGE_0011	U(42)
#define PARANGE_0100	U(44)
#define PARANGE_0101	U(48)
#define PARANGE_0110	U(52)

#define ID_AA64MMFR0_EL1_ECV_SHIFT		U(60)
#define ID_AA64MMFR0_EL1_ECV_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_ECV_NOT_SUPPORTED	ULL(0x0)
#define ID_AA64MMFR0_EL1_ECV_SUPPORTED		ULL(0x1)
#define ID_AA64MMFR0_EL1_ECV_SELF_SYNCH	ULL(0x2)

#define ID_AA64MMFR0_EL1_FGT_SHIFT		U(56)
#define ID_AA64MMFR0_EL1_FGT_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_FGT_SUPPORTED		ULL(0x1)
#define ID_AA64MMFR0_EL1_FGT_NOT_SUPPORTED	ULL(0x0)

#define ID_AA64MMFR0_EL1_TGRAN4_SHIFT		U(28)
#define ID_AA64MMFR0_EL1_TGRAN4_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_TGRAN4_SUPPORTED	ULL(0x0)
#define ID_AA64MMFR0_EL1_TGRAN4_NOT_SUPPORTED	ULL(0xf)

#define ID_AA64MMFR0_EL1_TGRAN64_SHIFT		U(24)
#define ID_AA64MMFR0_EL1_TGRAN64_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_TGRAN64_SUPPORTED	ULL(0x0)
#define ID_AA64MMFR0_EL1_TGRAN64_NOT_SUPPORTED	ULL(0xf)

#define ID_AA64MMFR0_EL1_TGRAN16_SHIFT		U(20)
#define ID_AA64MMFR0_EL1_TGRAN16_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_TGRAN16_SUPPORTED	ULL(0x1)
#define ID_AA64MMFR0_EL1_TGRAN16_NOT_SUPPORTED	ULL(0x0)

/* ID_AA64MMFR1_EL1 definitions */
#define ID_AA64MMFR1_EL1_TWED_SHIFT		U(32)
#define ID_AA64MMFR1_EL1_TWED_MASK		ULL(0xf)
#define ID_AA64MMFR1_EL1_TWED_SUPPORTED		ULL(0x1)
#define ID_AA64MMFR1_EL1_TWED_NOT_SUPPORTED	ULL(0x0)

#define ID_AA64MMFR1_EL1_PAN_SHIFT		U(20)
#define ID_AA64MMFR1_EL1_PAN_MASK		ULL(0xf)
#define ID_AA64MMFR1_EL1_PAN_NOT_SUPPORTED	ULL(0x0)
#define ID_AA64MMFR1_EL1_PAN_SUPPORTED		ULL(0x1)
#define ID_AA64MMFR1_EL1_PAN2_SUPPORTED		ULL(0x2)
#define ID_AA64MMFR1_EL1_PAN3_SUPPORTED		ULL(0x3)

/* ID_AA64MMFR2_EL1 definitions */
#define ID_AA64MMFR2_EL1		S3_0_C0_C7_2

#define ID_AA64MMFR2_EL1_ST_SHIFT	U(28)
#define ID_AA64MMFR2_EL1_ST_MASK	ULL(0xf)

#define ID_AA64MMFR2_EL1_CNP_SHIFT	U(0)
#define ID_AA64MMFR2_EL1_CNP_MASK	ULL(0xf)

/* ID_AA64PFR1_EL1 definitions */
#define ID_AA64PFR1_EL1_SSBS_SHIFT	U(4)
#define ID_AA64PFR1_EL1_SSBS_MASK	ULL(0xf)

#define SSBS_UNAVAILABLE	ULL(0)	/* No architectural SSBS support */

#define ID_AA64PFR1_EL1_BT_SHIFT	U(0)
#define ID_AA64PFR1_EL1_BT_MASK		ULL(0xf)

#define BTI_IMPLEMENTED		ULL(1)	/* The BTI mechanism is implemented */

#define ID_AA64PFR1_EL1_MTE_SHIFT	U(8)
#define ID_AA64PFR1_EL1_MTE_MASK	ULL(0xf)

/* Memory Tagging Extension is not implemented */
#define MTE_UNIMPLEMENTED	U(0)
/* FEAT_MTE: MTE instructions accessible at EL0 are implemented */
#define MTE_IMPLEMENTED_EL0	U(1)
/* FEAT_MTE2: Full MTE is implemented */
#define MTE_IMPLEMENTED_ELX	U(2)
/*
 * FEAT_MTE3: MTE is implemented with support for
 * asymmetric Tag Check Fault handling
 */
#define MTE_IMPLEMENTED_ASY	U(3)

#define ID_AA64PFR1_MPAM_FRAC_SHIFT	ULL(16)
#define ID_AA64PFR1_MPAM_FRAC_MASK	ULL(0xf)

/* ID_PFR1_EL1 definitions */
#define ID_PFR1_VIRTEXT_SHIFT	U(12)
#define ID_PFR1_VIRTEXT_MASK	U(0xf)
#define GET_VIRT_EXT(id)	(((id) >> ID_PFR1_VIRTEXT_SHIFT) \
				 & ID_PFR1_VIRTEXT_MASK)

/* SCTLR definitions */
#define SCTLR_EL2_RES1	((U(1) << 29) | (U(1) << 28) | (U(1) << 23) | \
			 (U(1) << 22) | (U(1) << 18) | (U(1) << 16) | \
			 (U(1) << 11) | (U(1) << 5) | (U(1) << 4))

#define SCTLR_EL1_RES1	((UL(1) << 29) | (UL(1) << 28) | (UL(1) << 23) | \
			 (UL(1) << 22) | (UL(1) << 20) | (UL(1) << 11))

#define SCTLR_AARCH32_EL1_RES1 \
			((U(1) << 23) | (U(1) << 22) | (U(1) << 11) | \
			 (U(1) << 4) | (U(1) << 3))

#define SCTLR_EL3_RES1	((U(1) << 29) | (U(1) << 28) | (U(1) << 23) | \
			(U(1) << 22) | (U(1) << 18) | (U(1) << 16) | \
			(U(1) << 11) | (U(1) << 5) | (U(1) << 4))

#define SCTLR_M_BIT		(ULL(1) << 0)
#define SCTLR_A_BIT		(ULL(1) << 1)
#define SCTLR_C_BIT		(ULL(1) << 2)
#define SCTLR_SA_BIT		(ULL(1) << 3)
#define SCTLR_SA0_BIT		(ULL(1) << 4)
#define SCTLR_CP15BEN_BIT	(ULL(1) << 5)
#define SCTLR_nAA_BIT		(ULL(1) << 6)
#define SCTLR_ITD_BIT		(ULL(1) << 7)
#define SCTLR_SED_BIT		(ULL(1) << 8)
#define SCTLR_UMA_BIT		(ULL(1) << 9)
#define SCTLR_EnRCTX_BIT	(ULL(1) << 10)
#define SCTLR_EOS_BIT		(ULL(1) << 11)
#define SCTLR_I_BIT		(ULL(1) << 12)
#define SCTLR_EnDB_BIT		(ULL(1) << 13)
#define SCTLR_DZE_BIT		(ULL(1) << 14)
#define SCTLR_UCT_BIT		(ULL(1) << 15)
#define SCTLR_NTWI_BIT		(ULL(1) << 16)
#define SCTLR_NTWE_BIT		(ULL(1) << 18)
#define SCTLR_WXN_BIT		(ULL(1) << 19)
#define SCTLR_TSCXT_BIT		(ULL(1) << 20)
#define SCTLR_IESB_BIT		(ULL(1) << 21)
#define SCTLR_EIS_BIT		(ULL(1) << 22)
#define SCTLR_SPAN_BIT		(ULL(1) << 23)
#define SCTLR_E0E_BIT		(ULL(1) << 24)
#define SCTLR_EE_BIT		(ULL(1) << 25)
#define SCTLR_UCI_BIT		(ULL(1) << 26)
#define SCTLR_EnDA_BIT		(ULL(1) << 27)
#define SCTLR_nTLSMD_BIT	(ULL(1) << 28)
#define SCTLR_LSMAOE_BIT	(ULL(1) << 29)
#define SCTLR_EnIB_BIT		(ULL(1) << 30)
#define SCTLR_EnIA_BIT		(ULL(1) << 31)
#define SCTLR_BT0_BIT		(ULL(1) << 35)
#define SCTLR_BT1_BIT		(ULL(1) << 36)
#define SCTLR_BT_BIT		(ULL(1) << 36)
#define SCTLR_ITFSB_BIT		(ULL(1) << 37)
#define SCTLR_TCF0_SHIFT	U(38)
#define SCTLR_TCF0_MASK		ULL(3)

/* Tag Check Faults in EL0 have no effect on the PE */
#define	SCTLR_TCF0_NO_EFFECT	U(0)
/* Tag Check Faults in EL0 cause a synchronous exception */
#define	SCTLR_TCF0_SYNC		U(1)
/* Tag Check Faults in EL0 are asynchronously accumulated */
#define	SCTLR_TCF0_ASYNC	U(2)
/*
 * Tag Check Faults in EL0 cause a synchronous exception on reads,
 * and are asynchronously accumulated on writes
 */
#define	SCTLR_TCF0_SYNCR_ASYNCW	U(3)

#define SCTLR_TCF_SHIFT		U(40)
#define SCTLR_TCF_MASK		ULL(3)

/* Tag Check Faults in EL1 have no effect on the PE */
#define	SCTLR_TCF_NO_EFFECT	U(0)
/* Tag Check Faults in EL1 cause a synchronous exception */
#define	SCTLR_TCF_SYNC		U(1)
/* Tag Check Faults in EL1 are asynchronously accumulated */
#define	SCTLR_TCF_ASYNC		U(2)
/*
 * Tag Check Faults in EL1 cause a synchronous exception on reads,
 * and are asynchronously accumulated on writes
 */
#define	SCTLR_TCF_SYNCR_ASYNCW	U(3)

#define SCTLR_ATA0_BIT		(ULL(1) << 42)
#define SCTLR_ATA_BIT		(ULL(1) << 43)
#define SCTLR_DSSBS_BIT		(ULL(1) << 44)
#define SCTLR_TWEDEn_BIT	(ULL(1) << 45)
#define SCTLR_TWEDEL_SHIFT	U(46)
#define SCTLR_TWEDEL_MASK	ULL(0xf)
#define SCTLR_EnASR_BIT		(ULL(1) << 54)
#define SCTLR_EnAS0_BIT		(ULL(1) << 55)
#define SCTLR_EnALS_BIT		(ULL(1) << 56)
#define SCTLR_EPAN_BIT		(ULL(1) << 57)
#define SCTLR_RESET_VAL		SCTLR_EL3_RES1

/* CPACR_EL1 definitions */
#define CPACR_EL1_FPEN(x)	((x) << 20)
#define CPACR_EL1_FP_TRAP_EL0	UL(0x1)
#define CPACR_EL1_FP_TRAP_ALL	UL(0x2)
#define CPACR_EL1_FP_TRAP_NONE	UL(0x3)

/* SCR definitions */
#define SCR_RES1_BITS		((U(1) << 4) | (U(1) << 5))
#define SCR_TWEDEL_SHIFT	U(30)
#define SCR_TWEDEL_MASK		ULL(0xf)
#define SCR_AMVOFFEN_BIT	(UL(1) << 35)
#define SCR_TWEDEn_BIT		(UL(1) << 29)
#define SCR_ECVEN_BIT		(UL(1) << 28)
#define SCR_FGTEN_BIT		(UL(1) << 27)
#define SCR_ATA_BIT		(UL(1) << 26)
#define SCR_FIEN_BIT		(UL(1) << 21)
#define SCR_EEL2_BIT		(UL(1) << 18)
#define SCR_API_BIT		(UL(1) << 17)
#define SCR_APK_BIT		(UL(1) << 16)
#define SCR_TERR_BIT		(UL(1) << 15)
#define SCR_TWE_BIT		(UL(1) << 13)
#define SCR_TWI_BIT		(UL(1) << 12)
#define SCR_ST_BIT		(UL(1) << 11)
#define SCR_RW_BIT		(UL(1) << 10)
#define SCR_SIF_BIT		(UL(1) << 9)
#define SCR_HCE_BIT		(UL(1) << 8)
#define SCR_SMD_BIT		(UL(1) << 7)
#define SCR_EA_BIT		(UL(1) << 3)
#define SCR_FIQ_BIT		(UL(1) << 2)
#define SCR_IRQ_BIT		(UL(1) << 1)
#define SCR_NS_BIT		(UL(1) << 0)
#define SCR_VALID_BIT_MASK	U(0x2f8f)
#define SCR_RESET_VAL		SCR_RES1_BITS

/* MDCR_EL3 definitions */
#define MDCR_MTPME_BIT		(ULL(1) << 28)
#define MDCR_SCCD_BIT		(ULL(1) << 23)
#define MDCR_SPME_BIT		(ULL(1) << 17)
#define MDCR_SDD_BIT		(ULL(1) << 16)
#define MDCR_SPD32(x)		((x) << 14)
#define MDCR_SPD32_LEGACY	ULL(0x0)
#define MDCR_SPD32_DISABLE	ULL(0x2)
#define MDCR_SPD32_ENABLE	ULL(0x3)
#define MDCR_NSPB(x)		((x) << 12)
#define MDCR_NSPB_EL1		ULL(0x3)
#define MDCR_TDOSA_BIT		(ULL(1) << 10)
#define MDCR_TDA_BIT		(ULL(1) << 9)
#define MDCR_TPM_BIT		(ULL(1) << 6)
#define MDCR_EL3_RESET_VAL	ULL(0x0)

/* MDCR_EL2 definitions */
#define MDCR_EL2_MTPME		(U(1) << 28)
#define MDCR_EL2_HLP		(U(1) << 26)
#define MDCR_EL2_HCCD		(U(1) << 23)
#define MDCR_EL2_TTRF		(U(1) << 19)
#define MDCR_EL2_HPMD		(U(1) << 17)
#define MDCR_EL2_TPMS		(U(1) << 14)
#define MDCR_EL2_E2PB(x)	((x) << 12)
#define MDCR_EL2_E2PB_EL1	U(0x3)
#define MDCR_EL2_TDRA_BIT	(U(1) << 11)
#define MDCR_EL2_TDOSA_BIT	(U(1) << 10)
#define MDCR_EL2_TDA_BIT	(U(1) << 9)
#define MDCR_EL2_TDE_BIT	(U(1) << 8)
#define MDCR_EL2_HPME_BIT	(U(1) << 7)
#define MDCR_EL2_TPM_BIT	(U(1) << 6)
#define MDCR_EL2_TPMCR_BIT	(U(1) << 5)
#define MDCR_EL2_RESET_VAL	U(0x0)

/* HSTR_EL2 definitions */
#define HSTR_EL2_RESET_VAL	U(0x0)
#define HSTR_EL2_T_MASK		U(0xff)

/* CNTHP_CTL_EL2 definitions */
#define CNTHP_CTL_ENABLE_BIT	(U(1) << 0)
#define CNTHP_CTL_RESET_VAL	U(0x0)

/* VTTBR_EL2 definitions */
#define VTTBR_RESET_VAL		ULL(0x0)
#define VTTBR_VMID_MASK		ULL(0xff)
#define VTTBR_VMID_SHIFT	U(48)
#define VTTBR_BADDR_MASK	ULL(0xffffffffffff)
#define VTTBR_BADDR_SHIFT	U(0)

/* HCR definitions */
#define HCR_AMVOFFEN_BIT	(ULL(1) << 51)
#define HCR_API_BIT		(ULL(1) << 41)
#define HCR_APK_BIT		(ULL(1) << 40)
#define HCR_E2H_BIT		(ULL(1) << 34)
#define HCR_TGE_BIT		(ULL(1) << 27)
#define HCR_RW_SHIFT		U(31)
#define HCR_RW_BIT		(ULL(1) << HCR_RW_SHIFT)
#define HCR_AMO_BIT		(ULL(1) << 5)
#define HCR_IMO_BIT		(ULL(1) << 4)
#define HCR_FMO_BIT		(ULL(1) << 3)

/* ISR definitions */
#define ISR_A_SHIFT		U(8)
#define ISR_I_SHIFT		U(7)
#define ISR_F_SHIFT		U(6)

/* CNTHCTL_EL2 definitions */
#define CNTHCTL_RESET_VAL	U(0x0)
#define EVNTEN_BIT		(U(1) << 2)
#define EL1PCEN_BIT		(U(1) << 1)
#define EL1PCTEN_BIT		(U(1) << 0)

/* CNTKCTL_EL1 definitions */
#define EL0PTEN_BIT		(U(1) << 9)
#define EL0VTEN_BIT		(U(1) << 8)
#define EL0PCTEN_BIT		(U(1) << 0)
#define EL0VCTEN_BIT		(U(1) << 1)
#define EVNTEN_BIT		(U(1) << 2)
#define EVNTDIR_BIT		(U(1) << 3)
#define EVNTI_SHIFT		U(4)
#define EVNTI_MASK		U(0xf)

/* CPTR_EL3 definitions */
#define TCPAC_BIT		(U(1) << 31)
#define TAM_BIT			(U(1) << 30)
#define TTA_BIT			(U(1) << 20)
#define TFP_BIT			(U(1) << 10)
#define CPTR_EZ_BIT		(U(1) << 8)
#define CPTR_EL3_RESET_VAL	U(0x0)

/* CPTR_EL2 definitions */
#define CPTR_EL2_RES1		((U(1) << 13) | (U(1) << 12) | (U(0x3ff)))
#define CPTR_EL2_TCPAC_BIT	(U(1) << 31)
#define CPTR_EL2_TAM_BIT	(U(1) << 30)
#define CPTR_EL2_TTA_BIT	(U(1) << 20)
#define CPTR_EL2_TFP_BIT	(U(1) << 10)
#define CPTR_EL2_TZ_BIT		(U(1) << 8)
#define CPTR_EL2_RESET_VAL	CPTR_EL2_RES1

/* CPSR/SPSR definitions */
#define DAIF_FIQ_BIT		(U(1) << 0)
#define DAIF_IRQ_BIT		(U(1) << 1)
#define DAIF_ABT_BIT		(U(1) << 2)
#define DAIF_DBG_BIT		(U(1) << 3)
#define SPSR_DAIF_SHIFT		U(6)
#define SPSR_DAIF_MASK		U(0xf)

#define SPSR_AIF_SHIFT		U(6)
#define SPSR_AIF_MASK		U(0x7)

#define SPSR_E_SHIFT		U(9)
#define SPSR_E_MASK		U(0x1)
#define SPSR_E_LITTLE		U(0x0)
#define SPSR_E_BIG		U(0x1)

#define SPSR_T_SHIFT		U(5)
#define SPSR_T_MASK		U(0x1)
#define SPSR_T_ARM		U(0x0)
#define SPSR_T_THUMB		U(0x1)

#define SPSR_M_SHIFT		U(4)
#define SPSR_M_MASK		U(0x1)
#define SPSR_M_AARCH64		U(0x0)
#define SPSR_M_AARCH32		U(0x1)

#define SPSR_EL_SHIFT		U(2)
#define SPSR_EL_WIDTH		U(2)

#define SPSR_SSBS_BIT_AARCH64	BIT_64(12)
#define SPSR_SSBS_BIT_AARCH32	BIT_64(23)

#define DISABLE_ALL_EXCEPTIONS \
		(DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT | DAIF_DBG_BIT)

#define DISABLE_INTERRUPTS	(DAIF_FIQ_BIT | DAIF_IRQ_BIT)

/*
 * RMR_EL3 definitions
 */
#define RMR_EL3_RR_BIT		(U(1) << 1)
#define RMR_EL3_AA64_BIT	(U(1) << 0)

/*
 * HI-VECTOR address for AArch32 state
 */
#define HI_VECTOR_BASE		U(0xFFFF0000)

/*
 * TCR defintions
 */
#define TCR_EL3_RES1		((ULL(1) << 31) | (ULL(1) << 23))
#define TCR_EL2_RES1		((ULL(1) << 31) | (ULL(1) << 23))
#define TCR_EL1_IPS_SHIFT	U(32)
#define TCR_EL2_PS_SHIFT	U(16)
#define TCR_EL3_PS_SHIFT	U(16)

#define TCR_TxSZ_MIN		ULL(16)
#define TCR_TxSZ_MAX		ULL(39)
#define TCR_TxSZ_MAX_TTST	ULL(48)

#define TCR_T0SZ_SHIFT		U(0)
#define TCR_T1SZ_SHIFT		U(16)

/* (internal) physical address size bits in EL3/EL1 */
#define TCR_PS_BITS_4GB		ULL(0x0)
#define TCR_PS_BITS_64GB	ULL(0x1)
#define TCR_PS_BITS_1TB		ULL(0x2)
#define TCR_PS_BITS_4TB		ULL(0x3)
#define TCR_PS_BITS_16TB	ULL(0x4)
#define TCR_PS_BITS_256TB	ULL(0x5)

#define ADDR_MASK_48_TO_63	ULL(0xFFFF000000000000)
#define ADDR_MASK_44_TO_47	ULL(0x0000F00000000000)
#define ADDR_MASK_42_TO_43	ULL(0x00000C0000000000)
#define ADDR_MASK_40_TO_41	ULL(0x0000030000000000)
#define ADDR_MASK_36_TO_39	ULL(0x000000F000000000)
#define ADDR_MASK_32_TO_35	ULL(0x0000000F00000000)

#define TCR_RGN_INNER_NC	(ULL(0x0) << 8)
#define TCR_RGN_INNER_WBA	(ULL(0x1) << 8)
#define TCR_RGN_INNER_WT	(ULL(0x2) << 8)
#define TCR_RGN_INNER_WBNA	(ULL(0x3) << 8)

#define TCR_RGN_OUTER_NC	(ULL(0x0) << 10)
#define TCR_RGN_OUTER_WBA	(ULL(0x1) << 10)
#define TCR_RGN_OUTER_WT	(ULL(0x2) << 10)
#define TCR_RGN_OUTER_WBNA	(ULL(0x3) << 10)

#define TCR_SH_NON_SHAREABLE	(ULL(0x0) << 12)
#define TCR_SH_OUTER_SHAREABLE	(ULL(0x2) << 12)
#define TCR_SH_INNER_SHAREABLE	(ULL(0x3) << 12)

#define TCR_RGN1_INNER_NC	(ULL(0x0) << 24)
#define TCR_RGN1_INNER_WBA	(ULL(0x1) << 24)
#define TCR_RGN1_INNER_WT	(ULL(0x2) << 24)
#define TCR_RGN1_INNER_WBNA	(ULL(0x3) << 24)

#define TCR_RGN1_OUTER_NC	(ULL(0x0) << 26)
#define TCR_RGN1_OUTER_WBA	(ULL(0x1) << 26)
#define TCR_RGN1_OUTER_WT	(ULL(0x2) << 26)
#define TCR_RGN1_OUTER_WBNA	(ULL(0x3) << 26)

#define TCR_SH1_NON_SHAREABLE	(ULL(0x0) << 28)
#define TCR_SH1_OUTER_SHAREABLE	(ULL(0x2) << 28)
#define TCR_SH1_INNER_SHAREABLE	(ULL(0x3) << 28)

#define TCR_TG0_SHIFT		U(14)
#define TCR_TG0_MASK		ULL(3)
#define TCR_TG0_4K		(ULL(0) << TCR_TG0_SHIFT)
#define TCR_TG0_64K		(ULL(1) << TCR_TG0_SHIFT)
#define TCR_TG0_16K		(ULL(2) << TCR_TG0_SHIFT)

#define TCR_TG1_SHIFT		U(30)
#define TCR_TG1_MASK		ULL(3)
#define TCR_TG1_16K		(ULL(1) << TCR_TG1_SHIFT)
#define TCR_TG1_4K		(ULL(2) << TCR_TG1_SHIFT)
#define TCR_TG1_64K		(ULL(3) << TCR_TG1_SHIFT)

#define TCR_EPD0_BIT		(ULL(1) << 7)
#define TCR_EPD1_BIT		(ULL(1) << 23)

#define MODE_SP_SHIFT		U(0x0)
#define MODE_SP_MASK		U(0x1)
#define MODE_SP_EL0		U(0x0)
#define MODE_SP_ELX		U(0x1)

#define MODE_RW_SHIFT		U(0x4)
#define MODE_RW_MASK		U(0x1)
#define MODE_RW_64		U(0x0)
#define MODE_RW_32		U(0x1)

#define MODE_EL_SHIFT		U(0x2)
#define MODE_EL_MASK		U(0x3)
#define MODE_EL_WIDTH		U(0x2)
#define MODE_EL3		U(0x3)
#define MODE_EL2		U(0x2)
#define MODE_EL1		U(0x1)
#define MODE_EL0		U(0x0)

#define MODE32_SHIFT		U(0)
#define MODE32_MASK		U(0xf)
#define MODE32_usr		U(0x0)
#define MODE32_fiq		U(0x1)
#define MODE32_irq		U(0x2)
#define MODE32_svc		U(0x3)
#define MODE32_mon		U(0x6)
#define MODE32_abt		U(0x7)
#define MODE32_hyp		U(0xa)
#define MODE32_und		U(0xb)
#define MODE32_sys		U(0xf)

#define GET_RW(mode)		(((mode) >> MODE_RW_SHIFT) & MODE_RW_MASK)
#define GET_EL(mode)		(((mode) >> MODE_EL_SHIFT) & MODE_EL_MASK)
#define GET_SP(mode)		(((mode) >> MODE_SP_SHIFT) & MODE_SP_MASK)
#define GET_M32(mode)		(((mode) >> MODE32_SHIFT) & MODE32_MASK)

#define SPSR_64(el, sp, daif)					\
	(((MODE_RW_64 << MODE_RW_SHIFT) |			\
	(((el) & MODE_EL_MASK) << MODE_EL_SHIFT) |		\
	(((sp) & MODE_SP_MASK) << MODE_SP_SHIFT) |		\
	(((daif) & SPSR_DAIF_MASK) << SPSR_DAIF_SHIFT)) &	\
	(~(SPSR_SSBS_BIT_AARCH64)))

#define SPSR_MODE32(mode, isa, endian, aif)		\
	(((MODE_RW_32 << MODE_RW_SHIFT) |		\
	(((mode) & MODE32_MASK) << MODE32_SHIFT) |	\
	(((isa) & SPSR_T_MASK) << SPSR_T_SHIFT) |	\
	(((endian) & SPSR_E_MASK) << SPSR_E_SHIFT) |	\
	(((aif) & SPSR_AIF_MASK) << SPSR_AIF_SHIFT)) &	\
	(~(SPSR_SSBS_BIT_AARCH32)))

/*
 * TTBR Definitions
 */
#define TTBR_CNP_BIT		ULL(0x1)

/*
 * CTR_EL0 definitions
 */
#define CTR_CWG_SHIFT		U(24)
#define CTR_CWG_MASK		U(0xf)
#define CTR_ERG_SHIFT		U(20)
#define CTR_ERG_MASK		U(0xf)
#define CTR_DMINLINE_SHIFT	U(16)
#define CTR_DMINLINE_MASK	U(0xf)
#define CTR_L1IP_SHIFT		U(14)
#define CTR_L1IP_MASK		U(0x3)
#define CTR_IMINLINE_SHIFT	U(0)
#define CTR_IMINLINE_MASK	U(0xf)

#define MAX_CACHE_LINE_SIZE	U(0x800) /* 2KB */

/* Physical timer control register bit fields shifts and masks */
#define CNTP_CTL_ENABLE_SHIFT	U(0)
#define CNTP_CTL_IMASK_SHIFT	U(1)
#define CNTP_CTL_ISTATUS_SHIFT	U(2)

#define CNTP_CTL_ENABLE_MASK	U(1)
#define CNTP_CTL_IMASK_MASK	U(1)
#define CNTP_CTL_ISTATUS_MASK	U(1)

/* Physical timer control macros */
#define CNTP_CTL_ENABLE_BIT	(U(1) << CNTP_CTL_ENABLE_SHIFT)
#define CNTP_CTL_IMASK_BIT	(U(1) << CNTP_CTL_IMASK_SHIFT)

/* Exception Syndrome register bits and bobs */
#define ESR_EC_SHIFT			U(26)
#define ESR_EC_MASK			U(0x3f)
#define ESR_EC_LENGTH			U(6)
#define ESR_ISS_SHIFT			U(0)
#define ESR_ISS_LENGTH			U(25)
#define EC_UNKNOWN			U(0x0)
#define EC_WFE_WFI			U(0x1)
#define EC_AARCH32_CP15_MRC_MCR		U(0x3)
#define EC_AARCH32_CP15_MRRC_MCRR	U(0x4)
#define EC_AARCH32_CP14_MRC_MCR		U(0x5)
#define EC_AARCH32_CP14_LDC_STC		U(0x6)
#define EC_FP_SIMD			U(0x7)
#define EC_AARCH32_CP10_MRC		U(0x8)
#define EC_AARCH32_CP14_MRRC_MCRR	U(0xc)
#define EC_ILLEGAL			U(0xe)
#define EC_AARCH32_SVC			U(0x11)
#define EC_AARCH32_HVC			U(0x12)
#define EC_AARCH32_SMC			U(0x13)
#define EC_AARCH64_SVC			U(0x15)
#define EC_AARCH64_HVC			U(0x16)
#define EC_AARCH64_SMC			U(0x17)
#define EC_AARCH64_SYS			U(0x18)
#define EC_IABORT_LOWER_EL		U(0x20)
#define EC_IABORT_CUR_EL		U(0x21)
#define EC_PC_ALIGN			U(0x22)
#define EC_DABORT_LOWER_EL		U(0x24)
#define EC_DABORT_CUR_EL		U(0x25)
#define EC_SP_ALIGN			U(0x26)
#define EC_AARCH32_FP			U(0x28)
#define EC_AARCH64_FP			U(0x2c)
#define EC_SERROR			U(0x2f)
#define EC_BRK				U(0x3c)

/*
 * External Abort bit in Instruction and Data Aborts synchronous exception
 * syndromes.
 */
#define ESR_ISS_EABORT_EA_BIT		U(9)

#define EC_BITS(x)			(((x) >> ESR_EC_SHIFT) & ESR_EC_MASK)

/* Reset bit inside the Reset management register for EL3 (RMR_EL3) */
#define RMR_RESET_REQUEST_SHIFT 	U(0x1)
#define RMR_WARM_RESET_CPU		(U(1) << RMR_RESET_REQUEST_SHIFT)

/*******************************************************************************
 * Definitions of register offsets, fields and macros for CPU system
 * instructions.
 ******************************************************************************/

#define TLBI_ADDR_SHIFT		U(12)
#define TLBI_ADDR_MASK		ULL(0x00000FFFFFFFFFFF)
#define TLBI_ADDR(x)		(((x) >> TLBI_ADDR_SHIFT) & TLBI_ADDR_MASK)

/*******************************************************************************
 * Definitions of register offsets and fields in the CNTCTLBase Frame of the
 * system level implementation of the Generic Timer.
 ******************************************************************************/
#define CNTCTLBASE_CNTFRQ	U(0x0)
#define CNTNSAR			U(0x4)
#define CNTNSAR_NS_SHIFT(x)	(x)

#define CNTACR_BASE(x)		(U(0x40) + ((x) << 2))
#define CNTACR_RPCT_SHIFT	U(0x0)
#define CNTACR_RVCT_SHIFT	U(0x1)
#define CNTACR_RFRQ_SHIFT	U(0x2)
#define CNTACR_RVOFF_SHIFT	U(0x3)
#define CNTACR_RWVT_SHIFT	U(0x4)
#define CNTACR_RWPT_SHIFT	U(0x5)

/*******************************************************************************
 * Definitions of register offsets and fields in the CNTBaseN Frame of the
 * system level implementation of the Generic Timer.
 ******************************************************************************/
/* Physical Count register. */
#define CNTPCT_LO		U(0x0)
/* Counter Frequency register. */
#define CNTBASEN_CNTFRQ		U(0x10)
/* Physical Timer CompareValue register. */
#define CNTP_CVAL_LO		U(0x20)
/* Physical Timer Control register. */
#define CNTP_CTL		U(0x2c)

/* PMCR_EL0 definitions */
#define PMCR_EL0_RESET_VAL	U(0x0)
#define PMCR_EL0_N_SHIFT	U(11)
#define PMCR_EL0_N_MASK		U(0x1f)
#define PMCR_EL0_N_BITS		(PMCR_EL0_N_MASK << PMCR_EL0_N_SHIFT)
#define PMCR_EL0_LP_BIT		(U(1) << 7)
#define PMCR_EL0_LC_BIT		(U(1) << 6)
#define PMCR_EL0_DP_BIT		(U(1) << 5)
#define PMCR_EL0_X_BIT		(U(1) << 4)
#define PMCR_EL0_D_BIT		(U(1) << 3)
#define PMCR_EL0_C_BIT		(U(1) << 2)
#define PMCR_EL0_P_BIT		(U(1) << 1)
#define PMCR_EL0_E_BIT		(U(1) << 0)

/*******************************************************************************
 * Definitions for system register interface to SVE
 ******************************************************************************/
#define ZCR_EL3			S3_6_C1_C2_0
#define ZCR_EL2			S3_4_C1_C2_0

/* ZCR_EL3 definitions */
#define ZCR_EL3_LEN_MASK	U(0xf)

/* ZCR_EL2 definitions */
#define ZCR_EL2_LEN_MASK	U(0xf)

/*******************************************************************************
 * Definitions of MAIR encodings for device and normal memory
 ******************************************************************************/
/*
 * MAIR encodings for device memory attributes.
 */
#define MAIR_DEV_nGnRnE		ULL(0x0)
#define MAIR_DEV_nGnRE		ULL(0x4)
#define MAIR_DEV_nGRE		ULL(0x8)
#define MAIR_DEV_GRE		ULL(0xc)

/*
 * MAIR encodings for normal memory attributes.
 *
 * Cache Policy
 *  WT:	 Write Through
 *  WB:	 Write Back
 *  NC:	 Non-Cacheable
 *
 * Transient Hint
 *  NTR: Non-Transient
 *  TR:	 Transient
 *
 * Allocation Policy
 *  RA:	 Read Allocate
 *  WA:	 Write Allocate
 *  RWA: Read and Write Allocate
 *  NA:	 No Allocation
 */
#define MAIR_NORM_WT_TR_WA	ULL(0x1)
#define MAIR_NORM_WT_TR_RA	ULL(0x2)
#define MAIR_NORM_WT_TR_RWA	ULL(0x3)
#define MAIR_NORM_NC		ULL(0x4)
#define MAIR_NORM_WB_TR_WA	ULL(0x5)
#define MAIR_NORM_WB_TR_RA	ULL(0x6)
#define MAIR_NORM_WB_TR_RWA	ULL(0x7)
#define MAIR_NORM_WT_NTR_NA	ULL(0x8)
#define MAIR_NORM_WT_NTR_WA	ULL(0x9)
#define MAIR_NORM_WT_NTR_RA	ULL(0xa)
#define MAIR_NORM_WT_NTR_RWA	ULL(0xb)
#define MAIR_NORM_WB_NTR_NA	ULL(0xc)
#define MAIR_NORM_WB_NTR_WA	ULL(0xd)
#define MAIR_NORM_WB_NTR_RA	ULL(0xe)
#define MAIR_NORM_WB_NTR_RWA	ULL(0xf)

#define MAIR_NORM_OUTER_SHIFT	U(4)

#define MAKE_MAIR_NORMAL_MEMORY(inner, outer)	\
		((inner) | ((outer) << MAIR_NORM_OUTER_SHIFT))

/* PAR_EL1 fields */
#define PAR_F_SHIFT	U(0)
#define PAR_F_MASK	ULL(0x1)
#define PAR_ADDR_SHIFT	U(12)
#define PAR_ADDR_MASK	(BIT(40) - ULL(1)) /* 40-bits-wide page address */

/*******************************************************************************
 * Definitions for system register interface to SPE
 ******************************************************************************/
#define PMBLIMITR_EL1		S3_0_C9_C10_0

/*******************************************************************************
 * Definitions for system register interface to MPAM
 ******************************************************************************/
#define MPAMIDR_EL1		S3_0_C10_C4_4
#define MPAM2_EL2		S3_4_C10_C5_0
#define MPAMHCR_EL2		S3_4_C10_C4_0
#define MPAM3_EL3		S3_6_C10_C5_0

/*******************************************************************************
 * Definitions for system register interface to AMU for FEAT_AMUv1
 ******************************************************************************/
#define AMCR_EL0		S3_3_C13_C2_0
#define AMCFGR_EL0		S3_3_C13_C2_1
#define AMCGCR_EL0		S3_3_C13_C2_2
#define AMUSERENR_EL0		S3_3_C13_C2_3
#define AMCNTENCLR0_EL0		S3_3_C13_C2_4
#define AMCNTENSET0_EL0		S3_3_C13_C2_5
#define AMCNTENCLR1_EL0		S3_3_C13_C3_0
#define AMCNTENSET1_EL0		S3_3_C13_C3_1

/* Activity Monitor Group 0 Event Counter Registers */
#define AMEVCNTR00_EL0		S3_3_C13_C4_0
#define AMEVCNTR01_EL0		S3_3_C13_C4_1
#define AMEVCNTR02_EL0		S3_3_C13_C4_2
#define AMEVCNTR03_EL0		S3_3_C13_C4_3

/* Activity Monitor Group 0 Event Type Registers */
#define AMEVTYPER00_EL0		S3_3_C13_C6_0
#define AMEVTYPER01_EL0		S3_3_C13_C6_1
#define AMEVTYPER02_EL0		S3_3_C13_C6_2
#define AMEVTYPER03_EL0		S3_3_C13_C6_3

/* Activity Monitor Group 1 Event Counter Registers */
#define AMEVCNTR10_EL0		S3_3_C13_C12_0
#define AMEVCNTR11_EL0		S3_3_C13_C12_1
#define AMEVCNTR12_EL0		S3_3_C13_C12_2
#define AMEVCNTR13_EL0		S3_3_C13_C12_3
#define AMEVCNTR14_EL0		S3_3_C13_C12_4
#define AMEVCNTR15_EL0		S3_3_C13_C12_5
#define AMEVCNTR16_EL0		S3_3_C13_C12_6
#define AMEVCNTR17_EL0		S3_3_C13_C12_7
#define AMEVCNTR18_EL0		S3_3_C13_C13_0
#define AMEVCNTR19_EL0		S3_3_C13_C13_1
#define AMEVCNTR1A_EL0		S3_3_C13_C13_2
#define AMEVCNTR1B_EL0		S3_3_C13_C13_3
#define AMEVCNTR1C_EL0		S3_3_C13_C13_4
#define AMEVCNTR1D_EL0		S3_3_C13_C13_5
#define AMEVCNTR1E_EL0		S3_3_C13_C13_6
#define AMEVCNTR1F_EL0		S3_3_C13_C13_7

/* Activity Monitor Group 1 Event Type Registers */
#define AMEVTYPER10_EL0		S3_3_C13_C14_0
#define AMEVTYPER11_EL0		S3_3_C13_C14_1
#define AMEVTYPER12_EL0		S3_3_C13_C14_2
#define AMEVTYPER13_EL0		S3_3_C13_C14_3
#define AMEVTYPER14_EL0		S3_3_C13_C14_4
#define AMEVTYPER15_EL0		S3_3_C13_C14_5
#define AMEVTYPER16_EL0		S3_3_C13_C14_6
#define AMEVTYPER17_EL0		S3_3_C13_C14_7
#define AMEVTYPER18_EL0		S3_3_C13_C15_0
#define AMEVTYPER19_EL0		S3_3_C13_C15_1
#define AMEVTYPER1A_EL0		S3_3_C13_C15_2
#define AMEVTYPER1B_EL0		S3_3_C13_C15_3
#define AMEVTYPER1C_EL0		S3_3_C13_C15_4
#define AMEVTYPER1D_EL0		S3_3_C13_C15_5
#define AMEVTYPER1E_EL0		S3_3_C13_C15_6
#define AMEVTYPER1F_EL0		S3_3_C13_C15_7

/* AMCFGR_EL0 definitions */
#define AMCFGR_EL0_NCG_SHIFT	U(28)
#define AMCFGR_EL0_NCG_MASK	U(0xf)
#define AMCFGR_EL0_N_SHIFT	U(0)
#define AMCFGR_EL0_N_MASK	U(0xff)

/* AMCGCR_EL0 definitions */
#define AMCGCR_EL0_CG1NC_SHIFT	U(8)
#define AMCGCR_EL0_CG1NC_MASK	U(0xff)

/* MPAM register definitions */
#define MPAM3_EL3_MPAMEN_BIT		(ULL(1) << 63)
#define MPAMHCR_EL2_TRAP_MPAMIDR_EL1	(ULL(1) << 31)

#define MPAM2_EL2_TRAPMPAM0EL1		(ULL(1) << 49)
#define MPAM2_EL2_TRAPMPAM1EL1		(ULL(1) << 48)

#define MPAMIDR_HAS_HCR_BIT		(ULL(1) << 17)

/*******************************************************************************
 * Definitions for system register interface to AMU for FEAT_AMUv1p1
 ******************************************************************************/

/* Definition for register defining which virtual offsets are implemented. */
#define AMCG1IDR_EL0		S3_3_C13_C2_6
#define AMCG1IDR_CTR_MASK	ULL(0xffff)
#define AMCG1IDR_CTR_SHIFT	U(0)
#define AMCG1IDR_VOFF_MASK	ULL(0xffff)
#define AMCG1IDR_VOFF_SHIFT	U(16)

/* New bit added to AMCR_EL0 */
#define AMCR_CG1RZ_BIT		(ULL(0x1) << 17)

/*
 * Definitions for virtual offset registers for architected activity monitor
 * event counters.
 * AMEVCNTVOFF01_EL2 intentionally left undefined, as it does not exist.
 */
#define AMEVCNTVOFF00_EL2	S3_4_C13_C8_0
#define AMEVCNTVOFF02_EL2	S3_4_C13_C8_2
#define AMEVCNTVOFF03_EL2	S3_4_C13_C8_3

/*
 * Definitions for virtual offset registers for auxiliary activity monitor event
 * counters.
 */
#define AMEVCNTVOFF10_EL2	S3_4_C13_C10_0
#define AMEVCNTVOFF11_EL2	S3_4_C13_C10_1
#define AMEVCNTVOFF12_EL2	S3_4_C13_C10_2
#define AMEVCNTVOFF13_EL2	S3_4_C13_C10_3
#define AMEVCNTVOFF14_EL2	S3_4_C13_C10_4
#define AMEVCNTVOFF15_EL2	S3_4_C13_C10_5
#define AMEVCNTVOFF16_EL2	S3_4_C13_C10_6
#define AMEVCNTVOFF17_EL2	S3_4_C13_C10_7
#define AMEVCNTVOFF18_EL2	S3_4_C13_C11_0
#define AMEVCNTVOFF19_EL2	S3_4_C13_C11_1
#define AMEVCNTVOFF1A_EL2	S3_4_C13_C11_2
#define AMEVCNTVOFF1B_EL2	S3_4_C13_C11_3
#define AMEVCNTVOFF1C_EL2	S3_4_C13_C11_4
#define AMEVCNTVOFF1D_EL2	S3_4_C13_C11_5
#define AMEVCNTVOFF1E_EL2	S3_4_C13_C11_6
#define AMEVCNTVOFF1F_EL2	S3_4_C13_C11_7

/*******************************************************************************
 * RAS system registers
 ******************************************************************************/
#define DISR_EL1		S3_0_C12_C1_1
#define DISR_A_BIT		U(31)

#define ERRIDR_EL1		S3_0_C5_C3_0
#define ERRIDR_MASK		U(0xffff)

#define ERRSELR_EL1		S3_0_C5_C3_1

/* System register access to Standard Error Record registers */
#define ERXFR_EL1		S3_0_C5_C4_0
#define ERXCTLR_EL1		S3_0_C5_C4_1
#define ERXSTATUS_EL1		S3_0_C5_C4_2
#define ERXADDR_EL1		S3_0_C5_C4_3
#define ERXPFGF_EL1		S3_0_C5_C4_4
#define ERXPFGCTL_EL1		S3_0_C5_C4_5
#define ERXPFGCDN_EL1		S3_0_C5_C4_6
#define ERXMISC0_EL1		S3_0_C5_C5_0
#define ERXMISC1_EL1		S3_0_C5_C5_1

#define ERXCTLR_ED_BIT		(U(1) << 0)
#define ERXCTLR_UE_BIT		(U(1) << 4)

#define ERXPFGCTL_UC_BIT	(U(1) << 1)
#define ERXPFGCTL_UEU_BIT	(U(1) << 2)
#define ERXPFGCTL_CDEN_BIT	(U(1) << 31)

/*******************************************************************************
 * Armv8.3 Pointer Authentication Registers
 ******************************************************************************/
#define APIAKeyLo_EL1		S3_0_C2_C1_0
#define APIAKeyHi_EL1		S3_0_C2_C1_1
#define APIBKeyLo_EL1		S3_0_C2_C1_2
#define APIBKeyHi_EL1		S3_0_C2_C1_3
#define APDAKeyLo_EL1		S3_0_C2_C2_0
#define APDAKeyHi_EL1		S3_0_C2_C2_1
#define APDBKeyLo_EL1		S3_0_C2_C2_2
#define APDBKeyHi_EL1		S3_0_C2_C2_3
#define APGAKeyLo_EL1		S3_0_C2_C3_0
#define APGAKeyHi_EL1		S3_0_C2_C3_1

/*******************************************************************************
 * Armv8.4 Data Independent Timing Registers
 ******************************************************************************/
#define DIT			S3_3_C4_C2_5
#define DIT_BIT			BIT(24)

/*******************************************************************************
 * Armv8.5 - new MSR encoding to directly access PSTATE.SSBS field
 ******************************************************************************/
#define SSBS			S3_3_C4_C2_6

/*******************************************************************************
 * Armv8.5 - Memory Tagging Extension Registers
 ******************************************************************************/
#define TFSRE0_EL1		S3_0_C5_C6_1
#define TFSR_EL1		S3_0_C5_C6_0
#define RGSR_EL1		S3_0_C1_C0_5
#define GCR_EL1			S3_0_C1_C0_6

/*******************************************************************************
 * Definitions for DynamicIQ Shared Unit registers
 ******************************************************************************/
#define CLUSTERPWRDN_EL1	S3_0_c15_c3_6

/* CLUSTERPWRDN_EL1 register definitions */
#define DSU_CLUSTER_PWR_OFF	0
#define DSU_CLUSTER_PWR_ON	1
#define DSU_CLUSTER_PWR_MASK	U(1)

#endif /* ARCH_H */
