/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <iommu.h>
#include <platform_def.h>

#define SMMU_BASE		0x64000000
#define ARM_SMMU_MAX_NUM_CNTXT_BANK	64
#define SMMU_CTX_BANK_IDX_SECURE_CRMU	63
#define ARM_SMMU_NUM_SECURE_MASTER	1
#define ARM_SMMU_NSNUMCBO		(ARM_SMMU_MAX_NUM_CNTXT_BANK - \
					 ARM_SMMU_NUM_SECURE_MASTER)
#define ARM_SMMU_NSNUMSMRGO		(ARM_SMMU_MAX_NUM_CNTXT_BANK - \
					 ARM_SMMU_NUM_SECURE_MASTER)
/* Reserved Banks. */
#define SMMU_CTX_BANK_IDX		(SMMU_CTX_BANK_IDX_SECURE_CRMU - \
					 ARM_SMMU_NUM_SECURE_MASTER)
#define NUM_OF_SMRS			1

#define STG1_WITH_STG2_BYPASS		1
#define ARM_LPAE_PGTBL_PHYS_CRMU	0x880000000
#define ARM_LPAE_PGTBL_PHYS		0x880200000
#define ARM_LPAE_PGTBL_PTE_CNT		512
#define ARM_LPAE_PTE_L1_BLOCK_SIZE	0x40000000
#define ARM_LPAE_PTE_L1_ADDR_MASK	0x0000FFFFC0000000UL
#define ARM_LPAE_PTE_TABLE		0x2UL
#define ARM_LPAE_PTE_VALID		0x1UL
#define ARM_LPAE_PTE_ATTRINDX		2
#define ARM_LPAE_PTE_NS			5
#define ARM_LPAE_PTE_AP			6
#define ARM_LPAE_PTE_AP_EL1_RW		0x0
#define ARM_LPAE_PTE_AP_EL0_RW		0x1
#define ARM_LPAE_PTE_SH			8
#define ARM_LPAE_PTE_SH_NON		0x0
#define ARM_LPAE_PTE_SH_OUTER		0x2
#define ARM_LPAE_PTE_SH_INNER		0x3
#define ARM_LPAE_PTE_AF			10
#define ARM_SMMU_RES_SIZE		0x80000

#define ARM_LPAE_PTE_NSTABLE		0x8000000000000000UL
#define ARM_LPAE_PTE_L1_INDEX_SHIFT	30
#define ARM_LPAE_PTE_L1_INDEX_MASK	0x1ff
#define ARM_LPAE_PTE_L0_INDEX_SHIFT	39
#define ARM_LPAE_PTE_L0_INDEX_MASK	0x1ff
#define ARM_LPAE_PTE_TABLE_MASK		~(0xfffUL)
/* Configuration registers */
#define ARM_SMMU_GR0_sCR0	0x0
#define sCR0_CLIENTPD		(1 << 0)
#define sCR0_GFRE		(1 << 1)
#define sCR0_GFIE		(1 << 2)
#define sCR0_GCFGFRE		(1 << 4)
#define sCR0_GCFGFIE		(1 << 5)
#define sCR0_USFCFG		(1 << 10)
#define sCR0_VMIDPNE		(1 << 11)
#define sCR0_PTM		(1 << 12)
#define sCR0_FB			(1 << 13)
#define sCR0_VMID16EN		(1 << 31)
#define sCR0_BSU_SHIFT		14
#define sCR0_BSU_MASK		0x3
#define ARM_SMMU_SMMU_SCR1	0x4
#define SCR1_NSNUMCBO_MASK	0xFF
#define SCR1_NSNUMCBO_SHIFT	0x0
#define SCR1_NSNUMSMRGO_MASK	0xFF00
#define SCR1_NSNUMSMRGO_SHIFT	0x8

/* Identification registers */
#define ARM_SMMU_GR0_ID0		0x20
#define ARM_SMMU_GR0_ID1		0x24
#define ARM_SMMU_GR0_ID2		0x28
#define ARM_SMMU_GR0_ID3		0x2c
#define ARM_SMMU_GR0_ID4		0x30
#define ARM_SMMU_GR0_ID5		0x34
#define ARM_SMMU_GR0_ID6		0x38
#define ARM_SMMU_GR0_ID7		0x3c
#define ARM_SMMU_GR0_sGFSR		0x48
#define ARM_SMMU_GR0_sGFSYNR0		0x50
#define ARM_SMMU_GR0_sGFSYNR1		0x54
#define ARM_SMMU_GR0_sGFSYNR2		0x58

#define ID1_PAGESIZE			(1U << 31)
#define ID1_NUMPAGENDXB_SHIFT		28
#define ID1_NUMPAGENDXB_MASK		7
#define ID1_NUMS2CB_SHIFT		16
#define ID1_NUMS2CB_MASK		0xff
#define ID1_NUMCB_SHIFT			0
#define ID1_NUMCB_MASK			0xff

/* SMMU global address space */
#define ARM_SMMU_GR0(smmu)	((smmu)->base)
#define ARM_SMMU_GR1(smmu)	((smmu)->base + (1 << (smmu)->pgshift))

/* Stream mapping registers */
#define ARM_SMMU_GR0_SMR(n)	(0x800 + (n << 2))
#define SMR_VALID		(1U << 31)
#define SMR_MASK_SHIFT		16
#define SMR_ID_SHIFT		0

#define ARM_SMMU_GR0_S2CR(n)	(0xc00 + (n << 2))
#define S2CR_CBNDX_SHIFT	0
#define S2CR_CBNDX_MASK		0xff
#define S2CR_TYPE_SHIFT		16
#define S2CR_TYPE_MASK		0x3

#define ARM_SMMU_GR1_CBA2R(n)	(0x800 + (n << 2))
#define CBA2R_RW64_32BIT	(0 << 0)
#define CBA2R_RW64_64BIT	(1 << 0)
#define CBA2R_VMID_SHIFT	16
#define CBA2R_VMID_MASK		0xffff

#define ARM_SMMU_GR1_CBAR(n)	(0x0 + (n << 2))
#define CBAR_VMID_SHIFT		0
#define CBAR_VMID_MASK		0xff
#define CBAR_S1_BPSHCFG_SHIFT	8
#define CBAR_S1_BPSHCFG_MASK	3
#define CBAR_S1_BPSHCFG_NSH	3
#define CBAR_S1_MEMATTR_SHIFT	12
#define CBAR_S1_MEMATTR_MASK	0xf
#define CBAR_S1_MEMATTR_WB	0xf
#define CBAR_TYPE_SHIFT		16
#define CBAR_TYPE_MASK		0x3
#define CBAR_TYPE_S2_TRANS		(0 << CBAR_TYPE_SHIFT)
#define CBAR_TYPE_S1_TRANS_S2_BYPASS	(1 << CBAR_TYPE_SHIFT)
#define CBAR_TYPE_S1_TRANS_S2_FAULT	(2 << CBAR_TYPE_SHIFT)
#define CBAR_TYPE_S1_TRANS_S2_TRANS	(3 << CBAR_TYPE_SHIFT)
#define CBAR_IRPTNDX_SHIFT		24
#define CBAR_IRPTNDX_MASK		0xff

/* Translation context bank */
#define ARM_SMMU_CB_BASE(smmu)		((smmu)->base + ((smmu)->size >> 1))
#define ARM_SMMU_CB(smmu, n)		((n) * (1 << (smmu)->pgshift))

#define ARM_SMMU_CB_SCTLR		0x0
#define ARM_SMMU_CB_ACTLR		0x4
#define ARM_SMMU_CB_RESUME		0x8
#define ARM_SMMU_CB_TTBCR2		0x10
#define ARM_SMMU_CB_TTBR0		0x20
#define ARM_SMMU_CB_TTBR1		0x28
#define ARM_SMMU_CB_TTBCR		0x30
#define ARM_SMMU_CB_CONTEXTIDR		0x34
#define ARM_SMMU_CB_S1_MAIR0		0x38
#define ARM_SMMU_CB_S1_MAIR1		0x3c
#define ARM_SMMU_CB_PAR			0x50
#define ARM_SMMU_CB_FSR			0x58
#define ARM_SMMU_CB_FAR			0x60
#define ARM_SMMU_CB_FSYNR0		0x68
#define ARM_SMMU_CB_S1_TLBIVA		0x600
#define ARM_SMMU_CB_S1_TLBIASID		0x610
#define ARM_SMMU_CB_S1_TLBIVAL		0x620
#define ARM_SMMU_CB_S2_TLBIIPAS2	0x630
#define ARM_SMMU_CB_S2_TLBIIPAS2L	0x638
#define ARM_SMMU_CB_ATS1PR		0x800
#define ARM_SMMU_CB_ATSR		0x8f0

#define SCTLR_S1_ASIDPNE		(1 << 12)
#define SCTLR_CFCFG			(1 << 7)
#define SCTLR_CFIE			(1 << 6)
#define SCTLR_CFRE			(1 << 5)
#define SCTLR_E				(1 << 4)
#define SCTLR_AFE			(1 << 2)
#define SCTLR_TRE			(1 << 1)
#define SCTLR_M				(1 << 0)

/* ARM LPAE configuration. */
/**************************************************************/
/* Register bits */
#define ARM_32_LPAE_TCR_EAE		(1 << 31)
#define ARM_64_LPAE_S2_TCR_RES1		(1 << 31)

#define ARM_LPAE_TCR_EPD1		(1 << 23)

#define ARM_LPAE_TCR_TG0_4K		(0 << 14)
#define ARM_LPAE_TCR_TG0_64K		(1 << 14)
#define ARM_LPAE_TCR_TG0_16K		(2 << 14)

#define ARM_LPAE_TCR_SH0_SHIFT		12
#define ARM_LPAE_TCR_SH0_MASK		0x3
#define ARM_LPAE_TCR_SH_NS		0
#define ARM_LPAE_TCR_SH_OS		2
#define ARM_LPAE_TCR_SH_IS		3

#define ARM_LPAE_TCR_ORGN0_SHIFT	10
#define ARM_LPAE_TCR_IRGN0_SHIFT	8
#define ARM_LPAE_TCR_RGN_MASK		0x3
#define ARM_LPAE_TCR_RGN_NC		0
#define ARM_LPAE_TCR_RGN_WBWA		1
#define ARM_LPAE_TCR_RGN_WT		2
#define ARM_LPAE_TCR_RGN_WB		3

#define ARM_LPAE_TCR_SL0_SHIFT		6
#define ARM_LPAE_TCR_SL0_MASK		0x3

#define ARM_LPAE_TCR_T0SZ_SHIFT		0
#define ARM_LPAE_TCR_SZ_MASK		0xf

#define ARM_LPAE_TCR_PS_SHIFT		16
#define ARM_LPAE_TCR_PS_MASK		0x7

#define ARM_LPAE_TCR_IPS_SHIFT		32
#define ARM_LPAE_TCR_IPS_MASK		0x7

#define ARM_LPAE_TCR_PS_32_BIT		0x0ULL
#define ARM_LPAE_TCR_PS_36_BIT		0x1ULL
#define ARM_LPAE_TCR_PS_40_BIT		0x2ULL
#define ARM_LPAE_TCR_PS_42_BIT		0x3ULL
#define ARM_LPAE_TCR_PS_44_BIT		0x4ULL
#define ARM_LPAE_TCR_PS_48_BIT		0x5ULL

#define ARM_LPAE_MAIR_ATTR_SHIFT(n)	((n) << 3)
#define ARM_LPAE_MAIR_ATTR_MASK		0xff
#define ARM_LPAE_MAIR_ATTR_DEVICE	0x04
#define ARM_LPAE_MAIR_ATTR_NC		0x44
#define ARM_LPAE_MAIR_ATTR_WBRWA	0xff
#define ARM_LPAE_MAIR_ATTR_IDX_NC	0
#define ARM_LPAE_MAIR_ATTR_IDX_CACHE	1
#define ARM_LPAE_MAIR_ATTR_IDX_DEV	2

#define TTBRn_ASID_SHIFT		48
#define TTBCR2_SEP_SHIFT		15
#define TTBCR2_SEP_UPSTREAM		(0x7 << TTBCR2_SEP_SHIFT)
#define TTBCR2_AS                       (1 << 4)
#define TTBCR_T0SZ(ia_bits)		(64 - (ia_bits))

#define S2CR_PRIVCFG_SHIFT		24
#define S2CR_PRIVCFG_MASK		0x3

/**************************************************************/

uint16_t paxc_stream_ids[] = { 0x2000 };

uint16_t paxc_stream_ids_mask[] = { 0x1fff };
uint16_t crmu_stream_ids[] = { CRMU_STREAM_ID };
uint16_t crmu_stream_ids_mask[] = { 0x0 };

enum arm_smmu_s2cr_type {
	S2CR_TYPE_TRANS,
	S2CR_TYPE_BYPASS,
	S2CR_TYPE_FAULT,
};

enum arm_smmu_s2cr_privcfg {
	S2CR_PRIVCFG_DEFAULT,
	S2CR_PRIVCFG_DIPAN,
	S2CR_PRIVCFG_UNPRIV,
	S2CR_PRIVCFG_PRIV,
};

struct arm_smmu_smr {
	uint16_t mask;
	uint16_t id;
	uint32_t valid;
};

struct arm_smmu_s2cr {
	int count;
	enum arm_smmu_s2cr_type type;
	enum arm_smmu_s2cr_privcfg privcfg;
	uint8_t cbndx;
};

struct arm_smmu_cfg {
	uint8_t cbndx;
	uint8_t irptndx;
	uint32_t cbar;
};

struct arm_smmu_device {
	uint8_t *base;
	uint32_t streams;
	unsigned long size;
	unsigned long pgshift;
	unsigned long va_size;
	unsigned long ipa_size;
	unsigned long pa_size;
	struct arm_smmu_smr smr[NUM_OF_SMRS];
	struct arm_smmu_s2cr s2cr[NUM_OF_SMRS];
	struct arm_smmu_cfg cfg[NUM_OF_SMRS];
	uint16_t *stream_ids;
	uint16_t *stream_ids_mask;
};

void arm_smmu_enable_secure_client_port(void)
{
	uintptr_t smmu_base = SMMU_BASE;

	mmio_clrbits_32(smmu_base, sCR0_CLIENTPD);
}

void arm_smmu_reserve_secure_cntxt(void)
{
	uintptr_t smmu_base = SMMU_BASE;

	mmio_clrsetbits_32(smmu_base + ARM_SMMU_SMMU_SCR1,
			  (SCR1_NSNUMSMRGO_MASK | SCR1_NSNUMCBO_MASK),
			  ((ARM_SMMU_NSNUMCBO << SCR1_NSNUMCBO_SHIFT) |
			  (ARM_SMMU_NSNUMSMRGO << SCR1_NSNUMSMRGO_SHIFT)));
}

static void arm_smmu_smr_cfg(struct arm_smmu_device *smmu, uint32_t index)
{
	uint32_t idx = smmu->cfg[index].cbndx;
	struct arm_smmu_smr *smr = &smmu->smr[index];
	uint32_t reg = smr->id << SMR_ID_SHIFT | smr->mask << SMR_MASK_SHIFT;

	if (smr->valid)
		reg |= SMR_VALID;

	mmio_write_32((uintptr_t) (ARM_SMMU_GR0(smmu) +
		      ARM_SMMU_GR0_SMR(idx)), reg);
}

static void arm_smmu_s2cr_cfg(struct arm_smmu_device *smmu, uint32_t index)
{
	uint32_t idx = smmu->cfg[index].cbndx;
	struct arm_smmu_s2cr *s2cr = &smmu->s2cr[index];

	uint32_t reg = (s2cr->type & S2CR_TYPE_MASK) << S2CR_TYPE_SHIFT |
		  (s2cr->cbndx & S2CR_CBNDX_MASK) << S2CR_CBNDX_SHIFT |
		  (s2cr->privcfg & S2CR_PRIVCFG_MASK) << S2CR_PRIVCFG_SHIFT;

	mmio_write_32((uintptr_t) (ARM_SMMU_GR0(smmu) +
		      ARM_SMMU_GR0_S2CR(idx)), reg);
}

static void smmu_set_pgtbl(struct arm_smmu_device *smmu,
		    enum iommu_domain dom,
		    uint64_t *pg_table_base)
{
	int i, l0_index, l1_index;
	uint64_t addr, *pte, *l0_base, *l1_base;
	uint64_t addr_space_limit;

	if (dom == PCIE_PAXC) {
		addr_space_limit = 0xffffffffff;
	} else if (dom == DOMAIN_CRMU) {
		addr_space_limit = 0xffffffff;
	} else {
		ERROR("dom is not supported\n");
		return;
	}

	l0_base = pg_table_base;
	/* clear L0 descriptors. */
	for (i = 0; i < ARM_LPAE_PGTBL_PTE_CNT; i++)
		l0_base[i] = 0x0;

	addr = 0x0;
	while (addr < addr_space_limit) {
		/* find L0 pte */
		l0_index = ((addr >> ARM_LPAE_PTE_L0_INDEX_SHIFT) &
			     ARM_LPAE_PTE_L0_INDEX_MASK);
		l1_base = l0_base + ((l0_index + 1) * ARM_LPAE_PGTBL_PTE_CNT);

		/* setup L0 pte if required */
		pte = l0_base + l0_index;
		if (*pte == 0x0) {
			*pte |= ((uint64_t)l1_base & ARM_LPAE_PTE_TABLE_MASK);
			if (dom == PCIE_PAXC)
				*pte |= ARM_LPAE_PTE_NSTABLE;
			*pte |= ARM_LPAE_PTE_TABLE;
			*pte |= ARM_LPAE_PTE_VALID;
		}

		/* find L1 pte */
		l1_index = ((addr >> ARM_LPAE_PTE_L1_INDEX_SHIFT) &
			    ARM_LPAE_PTE_L1_INDEX_MASK);
		pte = l1_base + l1_index;

		/* setup L1 pte */
		*pte = 0x0;
		*pte |= (addr & ARM_LPAE_PTE_L1_ADDR_MASK);
		if (addr < 0x80000000) {
			*pte |= (ARM_LPAE_MAIR_ATTR_IDX_DEV <<
				 ARM_LPAE_PTE_ATTRINDX);
			if (dom == PCIE_PAXC)
				*pte |= (1 << ARM_LPAE_PTE_NS);
		} else {
			*pte |= (ARM_LPAE_MAIR_ATTR_IDX_CACHE <<
				 ARM_LPAE_PTE_ATTRINDX);
			*pte |= (1 << ARM_LPAE_PTE_NS);
		}
		*pte |= (ARM_LPAE_PTE_AP_EL0_RW << ARM_LPAE_PTE_AP);
		*pte |= (ARM_LPAE_PTE_SH_INNER << ARM_LPAE_PTE_SH);
		*pte |= (1 << ARM_LPAE_PTE_AF);
		*pte |= ARM_LPAE_PTE_VALID;

		addr += ARM_LPAE_PTE_L1_BLOCK_SIZE;
	}
}

void arm_smmu_create_identity_map(enum iommu_domain dom)
{
	struct arm_smmu_device iommu;
	struct arm_smmu_device *smmu = &iommu;
	uint32_t reg, reg2;
	unsigned long long reg64;
	uint32_t idx;
	uint16_t asid;
	unsigned int context_bank_index;
	unsigned long long pg_table_base;

	smmu->base = (uint8_t *) SMMU_BASE;
	reg = mmio_read_32((uintptr_t) (ARM_SMMU_GR0(smmu) + ARM_SMMU_GR0_ID1));
	smmu->pgshift = (reg & ID1_PAGESIZE) ? 16 : 12;
	smmu->size = ARM_SMMU_RES_SIZE;
	smmu->stream_ids = NULL;

	switch (dom) {
	case PCIE_PAXC:
		smmu->stream_ids = &paxc_stream_ids[0];
		smmu->stream_ids_mask = &paxc_stream_ids_mask[0];
		smmu->streams = ARRAY_SIZE(paxc_stream_ids);
		context_bank_index = SMMU_CTX_BANK_IDX;
		pg_table_base = ARM_LPAE_PGTBL_PHYS;
		break;
	case DOMAIN_CRMU:
		smmu->stream_ids = &crmu_stream_ids[0];
		smmu->stream_ids_mask = &crmu_stream_ids_mask[0];
		smmu->streams = ARRAY_SIZE(crmu_stream_ids);
		context_bank_index = SMMU_CTX_BANK_IDX_SECURE_CRMU;
		pg_table_base = ARM_LPAE_PGTBL_PHYS_CRMU;
		break;
	default:
		ERROR("domain not supported\n");
		return;
	}

	if (smmu->streams > NUM_OF_SMRS) {
		INFO("can not support more than %d sids\n", NUM_OF_SMRS);
		return;
	}

	/* set up iommu dev. */
	for (idx = 0; idx < smmu->streams; idx++) {
		/* S2CR. */
		smmu->s2cr[idx].type = S2CR_TYPE_TRANS;
		smmu->s2cr[idx].privcfg = S2CR_PRIVCFG_DEFAULT;
		smmu->s2cr[idx].cbndx = context_bank_index;
		smmu->cfg[idx].cbndx = context_bank_index;
		smmu->cfg[idx].cbar = STG1_WITH_STG2_BYPASS << CBAR_TYPE_SHIFT;
		arm_smmu_s2cr_cfg(smmu, idx);

		/* SMR. */
		smmu->smr[idx].mask = smmu->stream_ids_mask[idx];
		smmu->smr[idx].id = smmu->stream_ids[idx];
		smmu->smr[idx].valid = 1;
		arm_smmu_smr_cfg(smmu, idx);

		/* CBA2R. 64-bit Translation */
		mmio_write_32((uintptr_t) (ARM_SMMU_GR1(smmu) +
			      ARM_SMMU_GR1_CBA2R(smmu->cfg[idx].cbndx)),
			      0x1);
		/* CBAR.*/
		reg = smmu->cfg[idx].cbar;
		reg |= (CBAR_S1_BPSHCFG_NSH << CBAR_S1_BPSHCFG_SHIFT) |
		       (CBAR_S1_MEMATTR_WB << CBAR_S1_MEMATTR_SHIFT);

		mmio_write_32((uintptr_t) (ARM_SMMU_GR1(smmu) +
			      ARM_SMMU_GR1_CBAR(smmu->cfg[idx].cbndx)),
			      reg);

		/* TTBCR. */
		reg64 = (ARM_LPAE_TCR_SH_IS << ARM_LPAE_TCR_SH0_SHIFT) |
		      (ARM_LPAE_TCR_RGN_WBWA << ARM_LPAE_TCR_IRGN0_SHIFT) |
		      (ARM_LPAE_TCR_RGN_WBWA << ARM_LPAE_TCR_ORGN0_SHIFT);
		reg64 |= ARM_LPAE_TCR_TG0_4K;
		reg64 |= (ARM_LPAE_TCR_PS_40_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		/* ias 40 bits.*/
		reg64 |= TTBCR_T0SZ(40) << ARM_LPAE_TCR_T0SZ_SHIFT;
		/* Disable speculative walks through TTBR1 */
		reg64 |= ARM_LPAE_TCR_EPD1;
		reg = (uint32_t) reg64;
		reg2 = (uint32_t) (reg64 >> 32);
		reg2 |= TTBCR2_SEP_UPSTREAM;
		reg2 |= TTBCR2_AS;

		mmio_write_32((uintptr_t) (ARM_SMMU_CB_BASE(smmu) +
			      ARM_SMMU_CB(smmu, smmu->cfg[idx].cbndx) +
			      ARM_SMMU_CB_TTBCR2), reg2);

		mmio_write_32((uintptr_t) (ARM_SMMU_CB_BASE(smmu) +
			      ARM_SMMU_CB(smmu, smmu->cfg[idx].cbndx) +
			      ARM_SMMU_CB_TTBCR), reg);

		/* TTBR0. */
		asid = smmu->cfg[idx].cbndx;
		reg64 = pg_table_base;
		reg64 |= (unsigned long long) asid << TTBRn_ASID_SHIFT;

		mmio_write_64((uintptr_t) (ARM_SMMU_CB_BASE(smmu) +
			      ARM_SMMU_CB(smmu, smmu->cfg[idx].cbndx) +
			      ARM_SMMU_CB_TTBR0), reg64);
		/* TTBR1. */
		reg64 = 0;
		reg64 |= (unsigned long long) asid << TTBRn_ASID_SHIFT;

		mmio_write_64((uintptr_t) (ARM_SMMU_CB_BASE(smmu) +
			      ARM_SMMU_CB(smmu, smmu->cfg[idx].cbndx) +
			      ARM_SMMU_CB_TTBR1), reg64);
		/* MAIR. */
		reg = (ARM_LPAE_MAIR_ATTR_NC
			<< ARM_LPAE_MAIR_ATTR_SHIFT
			(ARM_LPAE_MAIR_ATTR_IDX_NC)) |
			(ARM_LPAE_MAIR_ATTR_WBRWA <<
			ARM_LPAE_MAIR_ATTR_SHIFT
			(ARM_LPAE_MAIR_ATTR_IDX_CACHE)) |
			(ARM_LPAE_MAIR_ATTR_DEVICE <<
			ARM_LPAE_MAIR_ATTR_SHIFT
			(ARM_LPAE_MAIR_ATTR_IDX_DEV));

		mmio_write_32((uintptr_t) (ARM_SMMU_CB_BASE(smmu) +
			      ARM_SMMU_CB(smmu, smmu->cfg[idx].cbndx) +
			      ARM_SMMU_CB_S1_MAIR0), reg);

		/* MAIR1. */
		reg = 0;
		mmio_write_32((uintptr_t) (ARM_SMMU_CB_BASE(smmu) +
			      ARM_SMMU_CB(smmu, smmu->cfg[idx].cbndx) +
			      ARM_SMMU_CB_S1_MAIR1), reg);
		/* SCTLR. */
		reg = SCTLR_CFIE | SCTLR_CFRE | SCTLR_AFE | SCTLR_TRE | SCTLR_M;
		/* stage 1.*/
		reg |= SCTLR_S1_ASIDPNE;
		mmio_write_32((uintptr_t) (ARM_SMMU_CB_BASE(smmu) +
			      ARM_SMMU_CB(smmu, smmu->cfg[idx].cbndx) +
			      ARM_SMMU_CB_SCTLR), reg);
	}
	smmu_set_pgtbl(smmu, dom, (uint64_t *)pg_table_base);
}
