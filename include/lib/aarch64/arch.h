/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ARCH_H__
#define __ARCH_H__


/*******************************************************************************
 * MIDR bit definitions
 ******************************************************************************/
#define MIDR_IMPL_MASK		0xff
#define MIDR_IMPL_SHIFT		0x18
#define MIDR_VAR_SHIFT		20
#define MIDR_VAR_BITS		4
#define MIDR_REV_SHIFT		0
#define MIDR_REV_BITS		4
#define MIDR_PN_MASK		0xfff
#define MIDR_PN_SHIFT		0x4

/*******************************************************************************
 * MPIDR macros
 ******************************************************************************/
#define MPIDR_CPU_MASK		MPIDR_AFFLVL_MASK
#define MPIDR_CLUSTER_MASK	MPIDR_AFFLVL_MASK << MPIDR_AFFINITY_BITS
#define MPIDR_AFFINITY_BITS	8
#define MPIDR_AFFLVL_MASK	0xff
#define MPIDR_AFF0_SHIFT	0
#define MPIDR_AFF1_SHIFT	8
#define MPIDR_AFF2_SHIFT	16
#define MPIDR_AFF3_SHIFT	32
#define MPIDR_AFFINITY_MASK	0xff00ffffff
#define MPIDR_AFFLVL_SHIFT	3
#define MPIDR_AFFLVL0		0
#define MPIDR_AFFLVL1		1
#define MPIDR_AFFLVL2		2
#define MPIDR_AFFLVL3		3
#define MPIDR_AFFLVL0_VAL(mpidr) \
		((mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL1_VAL(mpidr) \
		((mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL2_VAL(mpidr) \
		((mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK)
#define MPIDR_AFFLVL3_VAL(mpidr) \
		((mpidr >> MPIDR_AFF3_SHIFT) & MPIDR_AFFLVL_MASK)
/*
 * The MPIDR_MAX_AFFLVL count starts from 0. Take care to
 * add one while using this macro to define array sizes.
 * TODO: Support only the first 3 affinity levels for now.
 */
#define MPIDR_MAX_AFFLVL	2

/* Constant to highlight the assumption that MPIDR allocation starts from 0 */
#define FIRST_MPIDR		0

/*******************************************************************************
 * Definitions for CPU system register interface to GICv3
 ******************************************************************************/
#define ICC_SRE_EL1     S3_0_C12_C12_5
#define ICC_SRE_EL2     S3_4_C12_C9_5
#define ICC_SRE_EL3     S3_6_C12_C12_5
#define ICC_CTLR_EL1    S3_0_C12_C12_4
#define ICC_CTLR_EL3    S3_6_C12_C12_4
#define ICC_PMR_EL1     S3_0_C4_C6_0
#define ICC_IGRPEN1_EL3 S3_6_c12_c12_7
#define ICC_IGRPEN0_EL1 S3_0_c12_c12_6
#define ICC_HPPIR0_EL1  S3_0_c12_c8_2
#define ICC_HPPIR1_EL1  S3_0_c12_c12_2
#define ICC_IAR0_EL1    S3_0_c12_c8_0
#define ICC_IAR1_EL1    S3_0_c12_c12_0
#define ICC_EOIR0_EL1   S3_0_c12_c8_1
#define ICC_EOIR1_EL1   S3_0_c12_c12_1

/*******************************************************************************
 * Generic timer memory mapped registers & offsets
 ******************************************************************************/
#define CNTCR_OFF			0x000
#define CNTFID_OFF			0x020

#define CNTCR_EN			(1 << 0)
#define CNTCR_HDBG			(1 << 1)
#define CNTCR_FCREQ(x)			((x) << 8)

/*******************************************************************************
 * System register bit definitions
 ******************************************************************************/
/* CLIDR definitions */
#define LOUIS_SHIFT		21
#define LOC_SHIFT		24
#define CLIDR_FIELD_WIDTH	3

/* CSSELR definitions */
#define LEVEL_SHIFT		1

/* D$ set/way op type defines */
#define DCISW			0x0
#define DCCISW			0x1
#define DCCSW			0x2

/* ID_AA64PFR0_EL1 definitions */
#define ID_AA64PFR0_EL0_SHIFT	0
#define ID_AA64PFR0_EL1_SHIFT	4
#define ID_AA64PFR0_EL2_SHIFT	8
#define ID_AA64PFR0_EL3_SHIFT	12
#define ID_AA64PFR0_ELX_MASK	0xf

#define ID_AA64PFR0_GIC_SHIFT	24
#define ID_AA64PFR0_GIC_WIDTH	4
#define ID_AA64PFR0_GIC_MASK	((1 << ID_AA64PFR0_GIC_WIDTH) - 1)

/* ID_PFR1_EL1 definitions */
#define ID_PFR1_VIRTEXT_SHIFT	12
#define ID_PFR1_VIRTEXT_MASK	0xf
#define GET_VIRT_EXT(id)	((id >> ID_PFR1_VIRTEXT_SHIFT) \
				 & ID_PFR1_VIRTEXT_MASK)

/* SCTLR definitions */
#define SCTLR_EL2_RES1  ((1 << 29) | (1 << 28) | (1 << 23) | (1 << 22) | \
			(1 << 18) | (1 << 16) | (1 << 11) | (1 << 5) |  \
			(1 << 4))

#define SCTLR_EL1_RES1  ((1 << 29) | (1 << 28) | (1 << 23) | (1 << 22) | \
			(1 << 20) | (1 << 11))
#define SCTLR_AARCH32_EL1_RES1 \
			((1 << 23) | (1 << 22) | (1 << 11) | (1 << 4) | \
			(1 << 3))

#define SCTLR_M_BIT		(1 << 0)
#define SCTLR_A_BIT		(1 << 1)
#define SCTLR_C_BIT		(1 << 2)
#define SCTLR_SA_BIT		(1 << 3)
#define SCTLR_I_BIT		(1 << 12)
#define SCTLR_WXN_BIT		(1 << 19)
#define SCTLR_EE_BIT		(1 << 25)

/* CPACR_El1 definitions */
#define CPACR_EL1_FPEN(x)	(x << 20)
#define CPACR_EL1_FP_TRAP_EL0	0x1
#define CPACR_EL1_FP_TRAP_ALL	0x2
#define CPACR_EL1_FP_TRAP_NONE	0x3

/* SCR definitions */
#define SCR_RES1_BITS		((1 << 4) | (1 << 5))
#define SCR_TWE_BIT		(1 << 13)
#define SCR_TWI_BIT		(1 << 12)
#define SCR_ST_BIT		(1 << 11)
#define SCR_RW_BIT		(1 << 10)
#define SCR_SIF_BIT		(1 << 9)
#define SCR_HCE_BIT		(1 << 8)
#define SCR_SMD_BIT		(1 << 7)
#define SCR_EA_BIT		(1 << 3)
#define SCR_FIQ_BIT		(1 << 2)
#define SCR_IRQ_BIT		(1 << 1)
#define SCR_NS_BIT		(1 << 0)
#define SCR_VALID_BIT_MASK	0x2f8f

/* HCR definitions */
#define HCR_RW_BIT		(1ull << 31)
#define HCR_AMO_BIT		(1 << 5)
#define HCR_IMO_BIT		(1 << 4)
#define HCR_FMO_BIT		(1 << 3)

/* CNTHCTL_EL2 definitions */
#define EVNTEN_BIT		(1 << 2)
#define EL1PCEN_BIT		(1 << 1)
#define EL1PCTEN_BIT		(1 << 0)

/* CNTKCTL_EL1 definitions */
#define EL0PTEN_BIT		(1 << 9)
#define EL0VTEN_BIT		(1 << 8)
#define EL0PCTEN_BIT		(1 << 0)
#define EL0VCTEN_BIT		(1 << 1)
#define EVNTEN_BIT		(1 << 2)
#define EVNTDIR_BIT		(1 << 3)
#define EVNTI_SHIFT		4
#define EVNTI_MASK		0xf

/* CPTR_EL3 definitions */
#define TCPAC_BIT		(1 << 31)
#define TTA_BIT			(1 << 20)
#define TFP_BIT			(1 << 10)

/* CPSR/SPSR definitions */
#define DAIF_FIQ_BIT		(1 << 0)
#define DAIF_IRQ_BIT		(1 << 1)
#define DAIF_ABT_BIT		(1 << 2)
#define DAIF_DBG_BIT		(1 << 3)
#define SPSR_DAIF_SHIFT		6
#define SPSR_DAIF_MASK		0xf

#define SPSR_AIF_SHIFT		6
#define SPSR_AIF_MASK		0x7

#define SPSR_E_SHIFT		9
#define SPSR_E_MASK			0x1
#define SPSR_E_LITTLE		0x0
#define SPSR_E_BIG			0x1

#define SPSR_T_SHIFT		5
#define SPSR_T_MASK			0x1
#define SPSR_T_ARM			0x0
#define SPSR_T_THUMB		0x1

#define DISABLE_ALL_EXCEPTIONS \
		(DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT | DAIF_DBG_BIT)


/*
 * TCR defintions
 */
#define TCR_EL3_RES1		((1UL << 31) | (1UL << 23))
#define TCR_EL1_IPS_SHIFT	32
#define TCR_EL3_PS_SHIFT	16

/* (internal) physical address size bits in EL3/EL1 */
#define TCR_PS_BITS_4GB		(0x0)
#define TCR_PS_BITS_64GB	(0x1)
#define TCR_PS_BITS_1TB		(0x2)
#define TCR_PS_BITS_4TB		(0x3)
#define TCR_PS_BITS_16TB	(0x4)
#define TCR_PS_BITS_256TB	(0x5)

#define ADDR_MASK_48_TO_63	0xFFFF000000000000UL
#define ADDR_MASK_44_TO_47	0x0000F00000000000UL
#define ADDR_MASK_42_TO_43	0x00000C0000000000UL
#define ADDR_MASK_40_TO_41	0x0000030000000000UL
#define ADDR_MASK_36_TO_39	0x000000F000000000UL
#define ADDR_MASK_32_TO_35	0x0000000F00000000UL

#define TCR_RGN_INNER_NC	(0x0 << 8)
#define TCR_RGN_INNER_WBA	(0x1 << 8)
#define TCR_RGN_INNER_WT	(0x2 << 8)
#define TCR_RGN_INNER_WBNA	(0x3 << 8)

#define TCR_RGN_OUTER_NC	(0x0 << 10)
#define TCR_RGN_OUTER_WBA	(0x1 << 10)
#define TCR_RGN_OUTER_WT	(0x2 << 10)
#define TCR_RGN_OUTER_WBNA	(0x3 << 10)

#define TCR_SH_NON_SHAREABLE	(0x0 << 12)
#define TCR_SH_OUTER_SHAREABLE	(0x2 << 12)
#define TCR_SH_INNER_SHAREABLE	(0x3 << 12)

#define MODE_SP_SHIFT		0x0
#define MODE_SP_MASK		0x1
#define MODE_SP_EL0		0x0
#define MODE_SP_ELX		0x1

#define MODE_RW_SHIFT		0x4
#define MODE_RW_MASK		0x1
#define MODE_RW_64			0x0
#define MODE_RW_32			0x1

#define MODE_EL_SHIFT		0x2
#define MODE_EL_MASK		0x3
#define MODE_EL3		0x3
#define MODE_EL2		0x2
#define MODE_EL1		0x1
#define MODE_EL0		0x0

#define MODE32_SHIFT		0
#define MODE32_MASK		0xf
#define MODE32_usr		0x0
#define MODE32_fiq		0x1
#define MODE32_irq		0x2
#define MODE32_svc		0x3
#define MODE32_mon		0x6
#define MODE32_abt		0x7
#define MODE32_hyp		0xa
#define MODE32_und		0xb
#define MODE32_sys		0xf

#define GET_RW(mode)		(((mode) >> MODE_RW_SHIFT) & MODE_RW_MASK)
#define GET_EL(mode)		(((mode) >> MODE_EL_SHIFT) & MODE_EL_MASK)
#define GET_SP(mode)		(((mode) >> MODE_SP_SHIFT) & MODE_SP_MASK)
#define GET_M32(mode)		(((mode) >> MODE32_SHIFT) & MODE32_MASK)

#define SPSR_64(el, sp, daif)				\
	(MODE_RW_64 << MODE_RW_SHIFT |			\
	((el) & MODE_EL_MASK) << MODE_EL_SHIFT |	\
	((sp) & MODE_SP_MASK) << MODE_SP_SHIFT |	\
	((daif) & SPSR_DAIF_MASK) << SPSR_DAIF_SHIFT)

#define SPSR_MODE32(mode, isa, endian, aif)		\
	(MODE_RW_32 << MODE_RW_SHIFT |			\
	((mode) & MODE32_MASK) << MODE32_SHIFT |	\
	((isa) & SPSR_T_MASK) << SPSR_T_SHIFT |		\
	((endian) & SPSR_E_MASK) << SPSR_E_SHIFT |	\
	((aif) & SPSR_AIF_MASK) << SPSR_AIF_SHIFT)

/*
 * CTR_EL0 definitions
 */
#define CTR_CWG_SHIFT		24
#define CTR_CWG_MASK		0xf
#define CTR_ERG_SHIFT		20
#define CTR_ERG_MASK		0xf
#define CTR_DMINLINE_SHIFT	16
#define CTR_DMINLINE_MASK	0xf
#define CTR_L1IP_SHIFT		14
#define CTR_L1IP_MASK		0x3
#define CTR_IMINLINE_SHIFT	0
#define CTR_IMINLINE_MASK	0xf

#define MAX_CACHE_LINE_SIZE	0x800 /* 2KB */
#define SIZE_FROM_LOG2_WORDS(n)	(4 << (n))


/* Physical timer control register bit fields shifts and masks */
#define CNTP_CTL_ENABLE_SHIFT   0
#define CNTP_CTL_IMASK_SHIFT    1
#define CNTP_CTL_ISTATUS_SHIFT  2

#define CNTP_CTL_ENABLE_MASK    1
#define CNTP_CTL_IMASK_MASK     1
#define CNTP_CTL_ISTATUS_MASK   1

#define get_cntp_ctl_enable(x)  ((x >> CNTP_CTL_ENABLE_SHIFT) & \
					CNTP_CTL_ENABLE_MASK)
#define get_cntp_ctl_imask(x)   ((x >> CNTP_CTL_IMASK_SHIFT) & \
					CNTP_CTL_IMASK_MASK)
#define get_cntp_ctl_istatus(x) ((x >> CNTP_CTL_ISTATUS_SHIFT) & \
					CNTP_CTL_ISTATUS_MASK)

#define set_cntp_ctl_enable(x)  (x |= 1 << CNTP_CTL_ENABLE_SHIFT)
#define set_cntp_ctl_imask(x)   (x |= 1 << CNTP_CTL_IMASK_SHIFT)

#define clr_cntp_ctl_enable(x)  (x &= ~(1 << CNTP_CTL_ENABLE_SHIFT))
#define clr_cntp_ctl_imask(x)   (x &= ~(1 << CNTP_CTL_IMASK_SHIFT))

/* Miscellaneous MMU related constants */
#define NUM_2MB_IN_GB		(1 << 9)
#define NUM_4K_IN_2MB		(1 << 9)
#define NUM_GB_IN_4GB		(1 << 2)

#define TWO_MB_SHIFT		21
#define ONE_GB_SHIFT		30
#define FOUR_KB_SHIFT		12

#define ONE_GB_INDEX(x)		((x) >> ONE_GB_SHIFT)
#define TWO_MB_INDEX(x)		((x) >> TWO_MB_SHIFT)
#define FOUR_KB_INDEX(x)	((x) >> FOUR_KB_SHIFT)

#define INVALID_DESC		0x0
#define BLOCK_DESC		0x1
#define TABLE_DESC		0x3

#define FIRST_LEVEL_DESC_N	ONE_GB_SHIFT
#define SECOND_LEVEL_DESC_N	TWO_MB_SHIFT
#define THIRD_LEVEL_DESC_N	FOUR_KB_SHIFT

#define LEVEL1			1
#define LEVEL2			2
#define LEVEL3			3

#define XN			(1ull << 2)
#define PXN			(1ull << 1)
#define CONT_HINT		(1ull << 0)

#define UPPER_ATTRS(x)		(x & 0x7) << 52
#define NON_GLOBAL		(1 << 9)
#define ACCESS_FLAG		(1 << 8)
#define NSH			(0x0 << 6)
#define OSH			(0x2 << 6)
#define ISH			(0x3 << 6)

#define PAGE_SIZE_SHIFT		FOUR_KB_SHIFT
#define PAGE_SIZE		(1 << PAGE_SIZE_SHIFT)
#define PAGE_SIZE_MASK		(PAGE_SIZE - 1)
#define IS_PAGE_ALIGNED(addr)	(((addr) & PAGE_SIZE_MASK) == 0)

#define XLAT_ENTRY_SIZE_SHIFT	3 /* Each MMU table entry is 8 bytes (1 << 3) */
#define XLAT_ENTRY_SIZE		(1 << XLAT_ENTRY_SIZE_SHIFT)

#define XLAT_TABLE_SIZE_SHIFT	PAGE_SIZE_SHIFT
#define XLAT_TABLE_SIZE		(1 << XLAT_TABLE_SIZE_SHIFT)

/* Values for number of entries in each MMU translation table */
#define XLAT_TABLE_ENTRIES_SHIFT (XLAT_TABLE_SIZE_SHIFT - XLAT_ENTRY_SIZE_SHIFT)
#define XLAT_TABLE_ENTRIES	(1 << XLAT_TABLE_ENTRIES_SHIFT)
#define XLAT_TABLE_ENTRIES_MASK	(XLAT_TABLE_ENTRIES - 1)

/* Values to convert a memory address to an index into a translation table */
#define L3_XLAT_ADDRESS_SHIFT	PAGE_SIZE_SHIFT
#define L2_XLAT_ADDRESS_SHIFT	(L3_XLAT_ADDRESS_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)
#define L1_XLAT_ADDRESS_SHIFT	(L2_XLAT_ADDRESS_SHIFT + XLAT_TABLE_ENTRIES_SHIFT)

/*
 * AP[1] bit is ignored by hardware and is
 * treated as if it is One in EL2/EL3
 */
#define AP_RO			(0x1 << 5)
#define AP_RW			(0x0 << 5)

#define NS				(0x1 << 3)
#define ATTR_SO_INDEX			0x2
#define ATTR_DEVICE_INDEX		0x1
#define ATTR_IWBWA_OWBWA_NTR_INDEX	0x0
#define LOWER_ATTRS(x)			(((x) & 0xfff) << 2)
#define ATTR_SO				(0x0)
#define ATTR_DEVICE			(0x4)
#define ATTR_IWBWA_OWBWA_NTR		(0xff)
#define MAIR_ATTR_SET(attr, index)	(attr << (index << 3))

/* Exception Syndrome register bits and bobs */
#define ESR_EC_SHIFT			26
#define ESR_EC_MASK			0x3f
#define ESR_EC_LENGTH			6
#define EC_UNKNOWN			0x0
#define EC_WFE_WFI			0x1
#define EC_AARCH32_CP15_MRC_MCR		0x3
#define EC_AARCH32_CP15_MRRC_MCRR	0x4
#define EC_AARCH32_CP14_MRC_MCR		0x5
#define EC_AARCH32_CP14_LDC_STC		0x6
#define EC_FP_SIMD			0x7
#define EC_AARCH32_CP10_MRC		0x8
#define EC_AARCH32_CP14_MRRC_MCRR	0xc
#define EC_ILLEGAL			0xe
#define EC_AARCH32_SVC			0x11
#define EC_AARCH32_HVC			0x12
#define EC_AARCH32_SMC			0x13
#define EC_AARCH64_SVC			0x15
#define EC_AARCH64_HVC			0x16
#define EC_AARCH64_SMC			0x17
#define EC_AARCH64_SYS			0x18
#define EC_IABORT_LOWER_EL		0x20
#define EC_IABORT_CUR_EL		0x21
#define EC_PC_ALIGN			0x22
#define EC_DABORT_LOWER_EL		0x24
#define EC_DABORT_CUR_EL		0x25
#define EC_SP_ALIGN			0x26
#define EC_AARCH32_FP			0x28
#define EC_AARCH64_FP			0x2c
#define EC_SERROR			0x2f

#define EC_BITS(x)			(x >> ESR_EC_SHIFT) & ESR_EC_MASK

/*******************************************************************************
 * Definitions of register offsets and fields in the CNTCTLBase Frame of the
 * system level implementation of the Generic Timer.
 ******************************************************************************/
#define CNTNSAR			0x4
#define CNTNSAR_NS_SHIFT(x)	x

#define CNTACR_BASE(x)		(0x40 + (x << 2))
#define CNTACR_RPCT_SHIFT	0x0
#define CNTACR_RVCT_SHIFT	0x1
#define CNTACR_RFRQ_SHIFT	0x2
#define CNTACR_RVOFF_SHIFT	0x3
#define CNTACR_RWVT_SHIFT	0x4
#define CNTACR_RWPT_SHIFT	0x5

#endif /* __ARCH_H__ */
