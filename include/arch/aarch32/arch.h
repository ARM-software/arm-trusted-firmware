/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
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
#define MIDR_IMPL_SHIFT		U(24)
#define MIDR_VAR_SHIFT		U(20)
#define MIDR_VAR_BITS		U(4)
#define MIDR_REV_SHIFT		U(0)
#define MIDR_REV_BITS		U(4)
#define MIDR_PN_MASK		U(0xfff)
#define MIDR_PN_SHIFT		U(4)

/*******************************************************************************
 * MPIDR macros
 ******************************************************************************/
#define MPIDR_MT_MASK		(U(1) << 24)
#define MPIDR_CPU_MASK		MPIDR_AFFLVL_MASK
#define MPIDR_CLUSTER_MASK	(MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS)
#define MPIDR_AFFINITY_BITS	U(8)
#define MPIDR_AFFLVL_MASK	U(0xff)
#define MPIDR_AFFLVL_SHIFT	U(3)
#define MPIDR_AFF0_SHIFT	U(0)
#define MPIDR_AFF1_SHIFT	U(8)
#define MPIDR_AFF2_SHIFT	U(16)
#define MPIDR_AFF_SHIFT(_n)	MPIDR_AFF##_n##_SHIFT
#define MPIDR_AFFINITY_MASK	U(0x00ffffff)
#define MPIDR_AFFLVL0		U(0)
#define MPIDR_AFFLVL1		U(1)
#define MPIDR_AFFLVL2		U(2)
#define MPIDR_AFFLVL(_n)	MPIDR_AFFLVL##_n

#define MPIDR_AFFLVL0_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL1_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL2_VAL(mpidr) \
		(((mpidr) >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL3_VAL(mpidr)	U(0)

#define MPIDR_AFF_ID(mpid, n)					\
	(((mpid) >> MPIDR_AFF_SHIFT(n)) & MPIDR_AFFLVL_MASK)

#define MPID_MASK		(MPIDR_MT_MASK				|\
				 (MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT)|\
				 (MPIDR_AFFLVL_MASK << MPIDR_AFF1_SHIFT)|\
				 (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT))

/*
 * An invalid MPID. This value can be used by functions that return an MPID to
 * indicate an error.
 */
#define INVALID_MPID		U(0xFFFFFFFF)

/*
 * The MPIDR_MAX_AFFLVL count starts from 0. Take care to
 * add one while using this macro to define array sizes.
 */
#define MPIDR_MAX_AFFLVL	U(2)

/* Data Cache set/way op type defines */
#define DC_OP_ISW			U(0x0)
#define DC_OP_CISW			U(0x1)
#if ERRATA_A53_827319
#define DC_OP_CSW			DC_OP_CISW
#else
#define DC_OP_CSW			U(0x2)
#endif

/*******************************************************************************
 * Generic timer memory mapped registers & offsets
 ******************************************************************************/
#define CNTCR_OFF			U(0x000)
/* Counter Count Value Lower register */
#define CNTCVL_OFF			U(0x008)
/* Counter Count Value Upper register */
#define CNTCVU_OFF			U(0x00C)
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
#define CLIDR_FIELD_WIDTH	U(3)

/* CSSELR definitions */
#define LEVEL_SHIFT		U(1)

/* ID_MMFR4 definitions */
#define ID_MMFR4_CNP_SHIFT	U(12)
#define ID_MMFR4_CNP_LENGTH	U(4)
#define ID_MMFR4_CNP_MASK	U(0xf)

/* ID_PFR0 definitions */
#define ID_PFR0_AMU_SHIFT	U(20)
#define ID_PFR0_AMU_LENGTH	U(4)
#define ID_PFR0_AMU_MASK	U(0xf)

#define ID_PFR0_DIT_SHIFT	U(24)
#define ID_PFR0_DIT_LENGTH	U(4)
#define ID_PFR0_DIT_MASK	U(0xf)
#define ID_PFR0_DIT_SUPPORTED	(U(1) << ID_PFR0_DIT_SHIFT)

/* ID_PFR1 definitions */
#define ID_PFR1_VIRTEXT_SHIFT	U(12)
#define ID_PFR1_VIRTEXT_MASK	U(0xf)
#define GET_VIRT_EXT(id)	(((id) >> ID_PFR1_VIRTEXT_SHIFT) \
				 & ID_PFR1_VIRTEXT_MASK)
#define ID_PFR1_GENTIMER_SHIFT	U(16)
#define ID_PFR1_GENTIMER_MASK	U(0xf)
#define ID_PFR1_GIC_SHIFT	U(28)
#define ID_PFR1_GIC_MASK	U(0xf)

/* SCTLR definitions */
#define SCTLR_RES1_DEF		((U(1) << 23) | (U(1) << 22) | (U(1) << 4) | \
				 (U(1) << 3))
#if ARM_ARCH_MAJOR == 7
#define SCTLR_RES1		SCTLR_RES1_DEF
#else
#define SCTLR_RES1		(SCTLR_RES1_DEF | (U(1) << 11))
#endif
#define SCTLR_M_BIT		(U(1) << 0)
#define SCTLR_A_BIT		(U(1) << 1)
#define SCTLR_C_BIT		(U(1) << 2)
#define SCTLR_CP15BEN_BIT	(U(1) << 5)
#define SCTLR_ITD_BIT		(U(1) << 7)
#define SCTLR_Z_BIT		(U(1) << 11)
#define SCTLR_I_BIT		(U(1) << 12)
#define SCTLR_V_BIT		(U(1) << 13)
#define SCTLR_RR_BIT		(U(1) << 14)
#define SCTLR_NTWI_BIT		(U(1) << 16)
#define SCTLR_NTWE_BIT		(U(1) << 18)
#define SCTLR_WXN_BIT		(U(1) << 19)
#define SCTLR_UWXN_BIT		(U(1) << 20)
#define SCTLR_EE_BIT		(U(1) << 25)
#define SCTLR_TRE_BIT		(U(1) << 28)
#define SCTLR_AFE_BIT		(U(1) << 29)
#define SCTLR_TE_BIT		(U(1) << 30)
#define SCTLR_DSSBS_BIT		(U(1) << 31)
#define SCTLR_RESET_VAL         (SCTLR_RES1 | SCTLR_NTWE_BIT |		\
				SCTLR_NTWI_BIT | SCTLR_CP15BEN_BIT)

/* SDCR definitions */
#define SDCR_SPD(x)		((x) << 14)
#define SDCR_SPD_LEGACY		U(0x0)
#define SDCR_SPD_DISABLE	U(0x2)
#define SDCR_SPD_ENABLE		U(0x3)
#define SDCR_SCCD_BIT		(U(1) << 23)
#define SDCR_SPME_BIT		(U(1) << 17)
#define SDCR_RESET_VAL		U(0x0)

/* HSCTLR definitions */
#define HSCTLR_RES1	((U(1) << 29) | (U(1) << 28) | (U(1) << 23) | \
			 (U(1) << 22) | (U(1) << 18) | (U(1) << 16) | \
			 (U(1) << 11) | (U(1) << 4) | (U(1) << 3))

#define HSCTLR_M_BIT		(U(1) << 0)
#define HSCTLR_A_BIT		(U(1) << 1)
#define HSCTLR_C_BIT		(U(1) << 2)
#define HSCTLR_CP15BEN_BIT	(U(1) << 5)
#define HSCTLR_ITD_BIT		(U(1) << 7)
#define HSCTLR_SED_BIT		(U(1) << 8)
#define HSCTLR_I_BIT		(U(1) << 12)
#define HSCTLR_WXN_BIT		(U(1) << 19)
#define HSCTLR_EE_BIT		(U(1) << 25)
#define HSCTLR_TE_BIT		(U(1) << 30)

/* CPACR definitions */
#define CPACR_FPEN(x)		((x) << 20)
#define CPACR_FP_TRAP_PL0	UL(0x1)
#define CPACR_FP_TRAP_ALL	UL(0x2)
#define CPACR_FP_TRAP_NONE	UL(0x3)

/* SCR definitions */
#define SCR_TWE_BIT		(UL(1) << 13)
#define SCR_TWI_BIT		(UL(1) << 12)
#define SCR_SIF_BIT		(UL(1) << 9)
#define SCR_HCE_BIT		(UL(1) << 8)
#define SCR_SCD_BIT		(UL(1) << 7)
#define SCR_NET_BIT		(UL(1) << 6)
#define SCR_AW_BIT		(UL(1) << 5)
#define SCR_FW_BIT		(UL(1) << 4)
#define SCR_EA_BIT		(UL(1) << 3)
#define SCR_FIQ_BIT		(UL(1) << 2)
#define SCR_IRQ_BIT		(UL(1) << 1)
#define SCR_NS_BIT		(UL(1) << 0)
#define SCR_VALID_BIT_MASK	U(0x33ff)
#define SCR_RESET_VAL		U(0x0)

#define GET_NS_BIT(scr)		((scr) & SCR_NS_BIT)

/* HCR definitions */
#define HCR_TGE_BIT		(U(1) << 27)
#define HCR_AMO_BIT		(U(1) << 5)
#define HCR_IMO_BIT		(U(1) << 4)
#define HCR_FMO_BIT		(U(1) << 3)
#define HCR_RESET_VAL		U(0x0)

/* CNTHCTL definitions */
#define CNTHCTL_RESET_VAL	U(0x0)
#define PL1PCEN_BIT		(U(1) << 1)
#define PL1PCTEN_BIT		(U(1) << 0)

/* CNTKCTL definitions */
#define PL0PTEN_BIT		(U(1) << 9)
#define PL0VTEN_BIT		(U(1) << 8)
#define PL0PCTEN_BIT		(U(1) << 0)
#define PL0VCTEN_BIT		(U(1) << 1)
#define EVNTEN_BIT		(U(1) << 2)
#define EVNTDIR_BIT		(U(1) << 3)
#define EVNTI_SHIFT		U(4)
#define EVNTI_MASK		U(0xf)

/* HCPTR definitions */
#define HCPTR_RES1		((U(1) << 13) | (U(1) << 12) | U(0x3ff))
#define TCPAC_BIT		(U(1) << 31)
#define TAM_BIT			(U(1) << 30)
#define TTA_BIT			(U(1) << 20)
#define TCP11_BIT		(U(1) << 11)
#define TCP10_BIT		(U(1) << 10)
#define HCPTR_RESET_VAL		HCPTR_RES1

/* VTTBR defintions */
#define VTTBR_RESET_VAL		ULL(0x0)
#define VTTBR_VMID_MASK		ULL(0xff)
#define VTTBR_VMID_SHIFT	U(48)
#define VTTBR_BADDR_MASK	ULL(0xffffffffffff)
#define VTTBR_BADDR_SHIFT	U(0)

/* HDCR definitions */
#define HDCR_HLP_BIT		(U(1) << 26)
#define HDCR_HPME_BIT		(U(1) << 7)
#define HDCR_RESET_VAL		U(0x0)

/* HSTR definitions */
#define HSTR_RESET_VAL		U(0x0)

/* CNTHP_CTL definitions */
#define CNTHP_CTL_RESET_VAL	U(0x0)

/* NSACR definitions */
#define NSASEDIS_BIT		(U(1) << 15)
#define NSTRCDIS_BIT		(U(1) << 20)
#define NSACR_CP11_BIT		(U(1) << 11)
#define NSACR_CP10_BIT		(U(1) << 10)
#define NSACR_IMP_DEF_MASK	(U(0x7) << 16)
#define NSACR_ENABLE_FP_ACCESS	(NSACR_CP11_BIT | NSACR_CP10_BIT)
#define NSACR_RESET_VAL		U(0x0)

/* CPACR definitions */
#define ASEDIS_BIT		(U(1) << 31)
#define TRCDIS_BIT		(U(1) << 28)
#define CPACR_CP11_SHIFT	U(22)
#define CPACR_CP10_SHIFT	U(20)
#define CPACR_ENABLE_FP_ACCESS	((U(0x3) << CPACR_CP11_SHIFT) |\
				 (U(0x3) << CPACR_CP10_SHIFT))
#define CPACR_RESET_VAL         U(0x0)

/* FPEXC definitions */
#define FPEXC_RES1		((U(1) << 10) | (U(1) << 9) | (U(1) << 8))
#define FPEXC_EN_BIT		(U(1) << 30)
#define FPEXC_RESET_VAL		FPEXC_RES1

/* SPSR/CPSR definitions */
#define SPSR_FIQ_BIT		(U(1) << 0)
#define SPSR_IRQ_BIT		(U(1) << 1)
#define SPSR_ABT_BIT		(U(1) << 2)
#define SPSR_AIF_SHIFT		U(6)
#define SPSR_AIF_MASK		U(0x7)

#define SPSR_E_SHIFT		U(9)
#define SPSR_E_MASK		U(0x1)
#define SPSR_E_LITTLE		U(0)
#define SPSR_E_BIG		U(1)

#define SPSR_T_SHIFT		U(5)
#define SPSR_T_MASK		U(0x1)
#define SPSR_T_ARM		U(0)
#define SPSR_T_THUMB		U(1)

#define SPSR_MODE_SHIFT		U(0)
#define SPSR_MODE_MASK		U(0x7)

#define SPSR_SSBS_BIT		BIT_32(23)

#define DISABLE_ALL_EXCEPTIONS \
		(SPSR_FIQ_BIT | SPSR_IRQ_BIT | SPSR_ABT_BIT)

#define CPSR_DIT_BIT		(U(1) << 21)
/*
 * TTBCR definitions
 */
#define TTBCR_EAE_BIT		(U(1) << 31)

#define TTBCR_SH1_NON_SHAREABLE		(U(0x0) << 28)
#define TTBCR_SH1_OUTER_SHAREABLE	(U(0x2) << 28)
#define TTBCR_SH1_INNER_SHAREABLE	(U(0x3) << 28)

#define TTBCR_RGN1_OUTER_NC	(U(0x0) << 26)
#define TTBCR_RGN1_OUTER_WBA	(U(0x1) << 26)
#define TTBCR_RGN1_OUTER_WT	(U(0x2) << 26)
#define TTBCR_RGN1_OUTER_WBNA	(U(0x3) << 26)

#define TTBCR_RGN1_INNER_NC	(U(0x0) << 24)
#define TTBCR_RGN1_INNER_WBA	(U(0x1) << 24)
#define TTBCR_RGN1_INNER_WT	(U(0x2) << 24)
#define TTBCR_RGN1_INNER_WBNA	(U(0x3) << 24)

#define TTBCR_EPD1_BIT		(U(1) << 23)
#define TTBCR_A1_BIT		(U(1) << 22)

#define TTBCR_T1SZ_SHIFT	U(16)
#define TTBCR_T1SZ_MASK		U(0x7)
#define TTBCR_TxSZ_MIN		U(0)
#define TTBCR_TxSZ_MAX		U(7)

#define TTBCR_SH0_NON_SHAREABLE		(U(0x0) << 12)
#define TTBCR_SH0_OUTER_SHAREABLE	(U(0x2) << 12)
#define TTBCR_SH0_INNER_SHAREABLE	(U(0x3) << 12)

#define TTBCR_RGN0_OUTER_NC	(U(0x0) << 10)
#define TTBCR_RGN0_OUTER_WBA	(U(0x1) << 10)
#define TTBCR_RGN0_OUTER_WT	(U(0x2) << 10)
#define TTBCR_RGN0_OUTER_WBNA	(U(0x3) << 10)

#define TTBCR_RGN0_INNER_NC	(U(0x0) << 8)
#define TTBCR_RGN0_INNER_WBA	(U(0x1) << 8)
#define TTBCR_RGN0_INNER_WT	(U(0x2) << 8)
#define TTBCR_RGN0_INNER_WBNA	(U(0x3) << 8)

#define TTBCR_EPD0_BIT		(U(1) << 7)
#define TTBCR_T0SZ_SHIFT	U(0)
#define TTBCR_T0SZ_MASK		U(0x7)

/*
 * HTCR definitions
 */
#define HTCR_RES1			((U(1) << 31) | (U(1) << 23))

#define HTCR_SH0_NON_SHAREABLE		(U(0x0) << 12)
#define HTCR_SH0_OUTER_SHAREABLE	(U(0x2) << 12)
#define HTCR_SH0_INNER_SHAREABLE	(U(0x3) << 12)

#define HTCR_RGN0_OUTER_NC	(U(0x0) << 10)
#define HTCR_RGN0_OUTER_WBA	(U(0x1) << 10)
#define HTCR_RGN0_OUTER_WT	(U(0x2) << 10)
#define HTCR_RGN0_OUTER_WBNA	(U(0x3) << 10)

#define HTCR_RGN0_INNER_NC	(U(0x0) << 8)
#define HTCR_RGN0_INNER_WBA	(U(0x1) << 8)
#define HTCR_RGN0_INNER_WT	(U(0x2) << 8)
#define HTCR_RGN0_INNER_WBNA	(U(0x3) << 8)

#define HTCR_T0SZ_SHIFT		U(0)
#define HTCR_T0SZ_MASK		U(0x7)

#define MODE_RW_SHIFT		U(0x4)
#define MODE_RW_MASK		U(0x1)
#define MODE_RW_32		U(0x1)

#define MODE32_SHIFT		U(0)
#define MODE32_MASK		U(0x1f)
#define MODE32_usr		U(0x10)
#define MODE32_fiq		U(0x11)
#define MODE32_irq		U(0x12)
#define MODE32_svc		U(0x13)
#define MODE32_mon		U(0x16)
#define MODE32_abt		U(0x17)
#define MODE32_hyp		U(0x1a)
#define MODE32_und		U(0x1b)
#define MODE32_sys		U(0x1f)

#define GET_M32(mode)		(((mode) >> MODE32_SHIFT) & MODE32_MASK)

#define SPSR_MODE32(mode, isa, endian, aif) \
( \
	( \
		(MODE_RW_32 << MODE_RW_SHIFT) | \
		(((mode) & MODE32_MASK) << MODE32_SHIFT) | \
		(((isa) & SPSR_T_MASK) << SPSR_T_SHIFT) | \
		(((endian) & SPSR_E_MASK) << SPSR_E_SHIFT) | \
		(((aif) & SPSR_AIF_MASK) << SPSR_AIF_SHIFT) \
	) & \
	(~(SPSR_SSBS_BIT)) \
)

/*
 * TTBR definitions
 */
#define TTBR_CNP_BIT		ULL(0x1)

/*
 * CTR definitions
 */
#define CTR_CWG_SHIFT		U(24)
#define CTR_CWG_MASK		U(0xf)
#define CTR_ERG_SHIFT		U(20)
#define CTR_ERG_MASK		U(0xf)
#define CTR_DMINLINE_SHIFT	U(16)
#define CTR_DMINLINE_WIDTH	U(4)
#define CTR_DMINLINE_MASK	((U(1) << 4) - U(1))
#define CTR_L1IP_SHIFT		U(14)
#define CTR_L1IP_MASK		U(0x3)
#define CTR_IMINLINE_SHIFT	U(0)
#define CTR_IMINLINE_MASK	U(0xf)

#define MAX_CACHE_LINE_SIZE	U(0x800) /* 2KB */

/* PMCR definitions */
#define PMCR_N_SHIFT		U(11)
#define PMCR_N_MASK		U(0x1f)
#define PMCR_N_BITS		(PMCR_N_MASK << PMCR_N_SHIFT)
#define PMCR_LP_BIT		(U(1) << 7)
#define PMCR_LC_BIT		(U(1) << 6)
#define PMCR_DP_BIT		(U(1) << 5)
#define	PMCR_RESET_VAL		U(0x0)

/*******************************************************************************
 * Definitions of register offsets, fields and macros for CPU system
 * instructions.
 ******************************************************************************/

#define TLBI_ADDR_SHIFT		U(0)
#define TLBI_ADDR_MASK		U(0xFFFFF000)
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

/* Physical timer control register bit fields shifts and masks */
#define CNTP_CTL_ENABLE_SHIFT   0
#define CNTP_CTL_IMASK_SHIFT    1
#define CNTP_CTL_ISTATUS_SHIFT  2

#define CNTP_CTL_ENABLE_MASK    U(1)
#define CNTP_CTL_IMASK_MASK     U(1)
#define CNTP_CTL_ISTATUS_MASK   U(1)

/* MAIR macros */
#define MAIR0_ATTR_SET(attr, index)	((attr) << ((index) << U(3)))
#define MAIR1_ATTR_SET(attr, index)	((attr) << (((index) - U(3)) << U(3)))

/* System register defines The format is: coproc, opt1, CRn, CRm, opt2 */
#define SCR		p15, 0, c1, c1, 0
#define SCTLR		p15, 0, c1, c0, 0
#define ACTLR		p15, 0, c1, c0, 1
#define SDCR		p15, 0, c1, c3, 1
#define MPIDR		p15, 0, c0, c0, 5
#define MIDR		p15, 0, c0, c0, 0
#define HVBAR		p15, 4, c12, c0, 0
#define VBAR		p15, 0, c12, c0, 0
#define MVBAR		p15, 0, c12, c0, 1
#define NSACR		p15, 0, c1, c1, 2
#define CPACR		p15, 0, c1, c0, 2
#define DCCIMVAC	p15, 0, c7, c14, 1
#define DCCMVAC		p15, 0, c7, c10, 1
#define DCIMVAC		p15, 0, c7, c6, 1
#define DCCISW		p15, 0, c7, c14, 2
#define DCCSW		p15, 0, c7, c10, 2
#define DCISW		p15, 0, c7, c6, 2
#define CTR		p15, 0, c0, c0, 1
#define CNTFRQ		p15, 0, c14, c0, 0
#define ID_MMFR4	p15, 0, c0, c2, 6
#define ID_PFR0		p15, 0, c0, c1, 0
#define ID_PFR1		p15, 0, c0, c1, 1
#define MAIR0		p15, 0, c10, c2, 0
#define MAIR1		p15, 0, c10, c2, 1
#define TTBCR		p15, 0, c2, c0, 2
#define TTBR0		p15, 0, c2, c0, 0
#define TTBR1		p15, 0, c2, c0, 1
#define TLBIALL		p15, 0, c8, c7, 0
#define TLBIALLH	p15, 4, c8, c7, 0
#define TLBIALLIS	p15, 0, c8, c3, 0
#define TLBIMVA		p15, 0, c8, c7, 1
#define TLBIMVAA	p15, 0, c8, c7, 3
#define TLBIMVAAIS	p15, 0, c8, c3, 3
#define TLBIMVAHIS	p15, 4, c8, c3, 1
#define BPIALLIS	p15, 0, c7, c1, 6
#define BPIALL		p15, 0, c7, c5, 6
#define ICIALLU		p15, 0, c7, c5, 0
#define HSCTLR		p15, 4, c1, c0, 0
#define HCR		p15, 4, c1, c1, 0
#define HCPTR		p15, 4, c1, c1, 2
#define HSTR		p15, 4, c1, c1, 3
#define CNTHCTL		p15, 4, c14, c1, 0
#define CNTKCTL		p15, 0, c14, c1, 0
#define VPIDR		p15, 4, c0, c0, 0
#define VMPIDR		p15, 4, c0, c0, 5
#define ISR		p15, 0, c12, c1, 0
#define CLIDR		p15, 1, c0, c0, 1
#define CSSELR		p15, 2, c0, c0, 0
#define CCSIDR		p15, 1, c0, c0, 0
#define HTCR		p15, 4, c2, c0, 2
#define HMAIR0		p15, 4, c10, c2, 0
#define ATS1CPR		p15, 0, c7, c8, 0
#define ATS1HR		p15, 4, c7, c8, 0
#define DBGOSDLR	p14, 0, c1, c3, 4

/* Debug register defines. The format is: coproc, opt1, CRn, CRm, opt2 */
#define HDCR		p15, 4, c1, c1, 1
#define PMCR		p15, 0, c9, c12, 0
#define CNTHP_TVAL	p15, 4, c14, c2, 0
#define CNTHP_CTL	p15, 4, c14, c2, 1

/* AArch32 coproc registers for 32bit MMU descriptor support */
#define PRRR		p15, 0, c10, c2, 0
#define NMRR		p15, 0, c10, c2, 1
#define DACR		p15, 0, c3, c0, 0

/* GICv3 CPU Interface system register defines. The format is: coproc, opt1, CRn, CRm, opt2 */
#define ICC_IAR1	p15, 0, c12, c12, 0
#define ICC_IAR0	p15, 0, c12, c8, 0
#define ICC_EOIR1	p15, 0, c12, c12, 1
#define ICC_EOIR0	p15, 0, c12, c8, 1
#define ICC_HPPIR1	p15, 0, c12, c12, 2
#define ICC_HPPIR0	p15, 0, c12, c8, 2
#define ICC_BPR1	p15, 0, c12, c12, 3
#define ICC_BPR0	p15, 0, c12, c8, 3
#define ICC_DIR		p15, 0, c12, c11, 1
#define ICC_PMR		p15, 0, c4, c6, 0
#define ICC_RPR		p15, 0, c12, c11, 3
#define ICC_CTLR	p15, 0, c12, c12, 4
#define ICC_MCTLR	p15, 6, c12, c12, 4
#define ICC_SRE		p15, 0, c12, c12, 5
#define ICC_HSRE	p15, 4, c12, c9, 5
#define ICC_MSRE	p15, 6, c12, c12, 5
#define ICC_IGRPEN0	p15, 0, c12, c12, 6
#define ICC_IGRPEN1	p15, 0, c12, c12, 7
#define ICC_MGRPEN1	p15, 6, c12, c12, 7

/* 64 bit system register defines The format is: coproc, opt1, CRm */
#define TTBR0_64	p15, 0, c2
#define TTBR1_64	p15, 1, c2
#define CNTVOFF_64	p15, 4, c14
#define VTTBR_64	p15, 6, c2
#define CNTPCT_64	p15, 0, c14
#define HTTBR_64	p15, 4, c2
#define CNTHP_CVAL_64	p15, 6, c14
#define PAR_64		p15, 0, c7

/* 64 bit GICv3 CPU Interface system register defines. The format is: coproc, opt1, CRm */
#define ICC_SGI1R_EL1_64	p15, 0, c12
#define ICC_ASGI1R_EL1_64	p15, 1, c12
#define ICC_SGI0R_EL1_64	p15, 2, c12

/*******************************************************************************
 * Definitions of MAIR encodings for device and normal memory
 ******************************************************************************/
/*
 * MAIR encodings for device memory attributes.
 */
#define MAIR_DEV_nGnRnE		U(0x0)
#define MAIR_DEV_nGnRE		U(0x4)
#define MAIR_DEV_nGRE		U(0x8)
#define MAIR_DEV_GRE		U(0xc)

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
#define MAIR_NORM_WT_TR_WA	U(0x1)
#define MAIR_NORM_WT_TR_RA	U(0x2)
#define MAIR_NORM_WT_TR_RWA	U(0x3)
#define MAIR_NORM_NC		U(0x4)
#define MAIR_NORM_WB_TR_WA	U(0x5)
#define MAIR_NORM_WB_TR_RA	U(0x6)
#define MAIR_NORM_WB_TR_RWA	U(0x7)
#define MAIR_NORM_WT_NTR_NA	U(0x8)
#define MAIR_NORM_WT_NTR_WA	U(0x9)
#define MAIR_NORM_WT_NTR_RA	U(0xa)
#define MAIR_NORM_WT_NTR_RWA	U(0xb)
#define MAIR_NORM_WB_NTR_NA	U(0xc)
#define MAIR_NORM_WB_NTR_WA	U(0xd)
#define MAIR_NORM_WB_NTR_RA	U(0xe)
#define MAIR_NORM_WB_NTR_RWA	U(0xf)

#define MAIR_NORM_OUTER_SHIFT	U(4)

#define MAKE_MAIR_NORMAL_MEMORY(inner, outer)	\
		((inner) | ((outer) << MAIR_NORM_OUTER_SHIFT))

/* PAR fields */
#define PAR_F_SHIFT	U(0)
#define PAR_F_MASK	ULL(0x1)
#define PAR_ADDR_SHIFT	U(12)
#define PAR_ADDR_MASK	(BIT_64(40) - ULL(1)) /* 40-bits-wide page address */

/*******************************************************************************
 * Definitions for system register interface to AMU for ARMv8.4 onwards
 ******************************************************************************/
#define AMCR		p15, 0, c13, c2, 0
#define AMCFGR		p15, 0, c13, c2, 1
#define AMCGCR		p15, 0, c13, c2, 2
#define AMUSERENR	p15, 0, c13, c2, 3
#define AMCNTENCLR0	p15, 0, c13, c2, 4
#define AMCNTENSET0	p15, 0, c13, c2, 5
#define AMCNTENCLR1	p15, 0, c13, c3, 0
#define AMCNTENSET1	p15, 0, c13, c3, 1

/* Activity Monitor Group 0 Event Counter Registers */
#define AMEVCNTR00	p15, 0, c0
#define AMEVCNTR01	p15, 1, c0
#define AMEVCNTR02	p15, 2, c0
#define AMEVCNTR03	p15, 3, c0

/* Activity Monitor Group 0 Event Type Registers */
#define AMEVTYPER00	p15, 0, c13, c6, 0
#define AMEVTYPER01	p15, 0, c13, c6, 1
#define AMEVTYPER02	p15, 0, c13, c6, 2
#define AMEVTYPER03	p15, 0, c13, c6, 3

/* Activity Monitor Group 1 Event Counter Registers */
#define AMEVCNTR10	p15, 0, c4
#define AMEVCNTR11	p15, 1, c4
#define AMEVCNTR12	p15, 2, c4
#define AMEVCNTR13	p15, 3, c4
#define AMEVCNTR14	p15, 4, c4
#define AMEVCNTR15	p15, 5, c4
#define AMEVCNTR16	p15, 6, c4
#define AMEVCNTR17	p15, 7, c4
#define AMEVCNTR18	p15, 0, c5
#define AMEVCNTR19	p15, 1, c5
#define AMEVCNTR1A	p15, 2, c5
#define AMEVCNTR1B	p15, 3, c5
#define AMEVCNTR1C	p15, 4, c5
#define AMEVCNTR1D	p15, 5, c5
#define AMEVCNTR1E	p15, 6, c5
#define AMEVCNTR1F	p15, 7, c5

/* Activity Monitor Group 1 Event Type Registers */
#define AMEVTYPER10	p15, 0, c13, c14, 0
#define AMEVTYPER11	p15, 0, c13, c14, 1
#define AMEVTYPER12	p15, 0, c13, c14, 2
#define AMEVTYPER13	p15, 0, c13, c14, 3
#define AMEVTYPER14	p15, 0, c13, c14, 4
#define AMEVTYPER15	p15, 0, c13, c14, 5
#define AMEVTYPER16	p15, 0, c13, c14, 6
#define AMEVTYPER17	p15, 0, c13, c14, 7
#define AMEVTYPER18	p15, 0, c13, c15, 0
#define AMEVTYPER19	p15, 0, c13, c15, 1
#define AMEVTYPER1A	p15, 0, c13, c15, 2
#define AMEVTYPER1B	p15, 0, c13, c15, 3
#define AMEVTYPER1C	p15, 0, c13, c15, 4
#define AMEVTYPER1D	p15, 0, c13, c15, 5
#define AMEVTYPER1E	p15, 0, c13, c15, 6
#define AMEVTYPER1F	p15, 0, c13, c15, 7

/* AMCFGR definitions */
#define AMCFGR_NCG_SHIFT	U(28)
#define AMCFGR_NCG_MASK		U(0xf)
#define AMCFGR_N_SHIFT		U(0)
#define AMCFGR_N_MASK		U(0xff)

/* AMCGCR definitions */
#define AMCGCR_CG1NC_SHIFT	U(8)
#define AMCGCR_CG1NC_MASK	U(0xff)

/*******************************************************************************
 * Definitions for DynamicIQ Shared Unit registers
 ******************************************************************************/
#define CLUSTERPWRDN	p15, 0, c15, c3, 6

/* CLUSTERPWRDN register definitions */
#define DSU_CLUSTER_PWR_OFF	0
#define DSU_CLUSTER_PWR_ON	1
#define DSU_CLUSTER_PWR_MASK	U(1)

#endif /* ARCH_H */
