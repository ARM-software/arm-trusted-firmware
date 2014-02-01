/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <bl_common.h>

/*******************************************************************************
 * MIDR bit definitions
 ******************************************************************************/
#define MIDR_PN_MASK		0xfff
#define MIDR_PN_SHIFT		0x4
#define MIDR_PN_AEM		0xd0f
#define MIDR_PN_A57		0xd07
#define MIDR_PN_A53		0xd03

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
/* TODO: Support only the first 3 affinity levels for now */
#define MPIDR_MAX_AFFLVL	2

/* Constant to highlight the assumption that MPIDR allocation starts from 0 */
#define FIRST_MPIDR		0

/*******************************************************************************
 * Implementation defined sysreg encodings
 ******************************************************************************/
#define CPUECTLR_EL1	S3_1_C15_C2_1

/*******************************************************************************
 * Generic timer memory mapped registers & offsets
 ******************************************************************************/
#define CNTCR_OFF			0x000
#define CNTFID_OFF			0x020

#define CNTCR_EN			(1 << 0)
#define CNTCR_HDBG			(1 << 1)
#define CNTCR_FCREQ(x)			(1 << (8 + (x)))

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
			(1 << 11))
#define SCTLR_M_BIT		(1 << 0)
#define SCTLR_A_BIT		(1 << 1)
#define SCTLR_C_BIT		(1 << 2)
#define SCTLR_SA_BIT		(1 << 3)
#define SCTLR_B_BIT		(1 << 7)
#define SCTLR_Z_BIT		(1 << 11)
#define SCTLR_I_BIT		(1 << 12)
#define SCTLR_WXN_BIT		(1 << 19)
#define SCTLR_EXCEPTION_BITS	(0x3 << 6)
#define SCTLR_EE_BIT		(1 << 25)

/* CPUECTLR definitions */
#define CPUECTLR_SMP_BIT	(1 << 6)

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

/* HCR definitions */
#define HCR_RW_BIT		(1ull << 31)
#define HCR_AMO_BIT		(1 << 5)
#define HCR_IMO_BIT		(1 << 4)
#define HCR_FMO_BIT		(1 << 3)

/* CNTHCTL_EL2 definitions */
#define EL1PCEN_BIT		(1 << 1)
#define EL1PCTEN_BIT		(1 << 0)

/* CNTKCTL_EL1 definitions */
#define EL0PTEN_BIT		(1 << 9)
#define EL0VTEN_BIT		(1 << 8)
#define EL0PCTEN_BIT		(1 << 0)
#define EL0VCTEN_BIT		(1 << 1)

/* CPTR_EL3 definitions */
#define TCPAC_BIT		(1 << 31)
#define TTA_BIT			(1 << 20)
#define TFP_BIT			(1 << 10)

/* CPSR/SPSR definitions */
#define DAIF_FIQ_BIT		(1 << 0)
#define DAIF_IRQ_BIT		(1 << 1)
#define DAIF_ABT_BIT		(1 << 2)
#define DAIF_DBG_BIT		(1 << 3)
#define PSR_DAIF_SHIFT		0x6

/*
 * TCR defintions
 */
#define TCR_EL3_RES1		((1UL << 31) | (1UL << 23))

#define TCR_T0SZ_4GB		32

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

#define MODE_RW_64		0x0
#define MODE_RW_32		0x1
#define MODE_SP_EL0		0x0
#define MODE_SP_ELX		0x1
#define MODE_EL3		0x3
#define MODE_EL2		0x2
#define MODE_EL1		0x1
#define MODE_EL0		0x0

#define MODE_RW_SHIFT		0x4
#define MODE_EL_SHIFT		0x2
#define MODE_SP_SHIFT		0x0

#define GET_RW(mode)		((mode >> MODE_RW_SHIFT) & 0x1)
#define GET_EL(mode)		((mode >> MODE_EL_SHIFT) & 0x3)
#define PSR_MODE(rw, el, sp)	(rw << MODE_RW_SHIFT | el << MODE_EL_SHIFT \
				 | sp << MODE_SP_SHIFT)

#define SPSR32_EE_BIT		(1 << 9)
#define SPSR32_T_BIT		(1 << 5)

#define AARCH32_MODE_SVC	0x13
#define AARCH32_MODE_HYP	0x1a

/* Miscellaneous MMU related constants */
#define NUM_2MB_IN_GB		(1 << 9)
#define NUM_4K_IN_2MB		(1 << 9)

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

#define IS_PAGE_ALIGNED(addr)	(((addr) & 0xFFF) == 0)

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

#ifndef __ASSEMBLY__
/*******************************************************************************
 * Function prototypes
 ******************************************************************************/

extern void early_exceptions(void);
extern void runtime_exceptions(void);
extern void bl1_arch_setup(void);
extern void bl2_arch_setup(void);
extern void bl31_arch_setup(void);

#endif /*__ASSEMBLY__*/

#endif /* __ARCH_H__ */
