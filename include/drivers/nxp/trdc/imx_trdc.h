/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_TRDC_H
#define IMX_TRDC_H

#define MBC_BLK_ALL	U(255)
#define MRC_REG_ALL	U(16)
#define GLBAC_NUM	U(8)

#define DID_NUM		U(16)
#define MBC_MAX_NUM	U(4)
#define MRC_MAX_NUM	U(2)
#define MBC_NUM(HWCFG)	(((HWCFG) >> 16) & 0xF)
#define MRC_NUM(HWCFG)	(((HWCFG) >> 24) & 0x1F)

#define MBC_BLK_NUM(GLBCFG)	((GLBCFG) & 0x3FF)
#define MRC_RGN_NUM(GLBCFG)	((GLBCFG) & 0x1F)

#define MDAC_W_X(m, r)	(0x800 + (m) * 0x20 + (r) * 0x4)

/* CPU/non-CPU domain common bits */
#define MDA_VLD		BIT(31)
#define MDA_LK1		BIT(30)
#define MDA_DFMT	BIT(29)

/* CPU domain bits */
#define MDA_DFMT0_DID(x)	((x) & 0xF)
#define MDA_DFMT0_DIDS(x)	(((x) & 0x3) << 4)
#define MDA_DFMT0_PE(x)		(((x) & 0x3) << 6)
#define MDA_DFMT0_PIDM(x)	(((x) & 0x3F) << 8)
#define MDA_DFMT0_SA(x)		(((x) & 0x3) << 14)
#define MDA_DFMT0_PID(x)	(((x) & 0x3F) << 16)

/* non-CPU domain bits */
#define MDA_DFMT1_DID(x)	((x) & 0xF)
#define MDA_DFMT1_PA(x)		(((x) & 0x3) << 4)
#define MDA_DFMT1_SA(x)		(((x) & 0x3) << 6)
#define MDA_DFMT1_DIDB(x)	((x) << 8)

#define SP(X)	((X) << 12)
#define SU(X)	((X) << 8)
#define NP(X)	((X) << 4)
#define NU(X)	((X) << 0)

#define RWX	U(7)
#define RW	U(6)
#define RX	U(5)
#define R	U(4)
#define X	U(1)

struct mbc_mem_dom {
	uint32_t mem_glbcfg[4];
	uint32_t nse_blk_index;
	uint32_t nse_blk_set;
	uint32_t nse_blk_clr;
	uint32_t nsr_blk_clr_all;
	uint32_t memn_glbac[8];
	/* The upper only existed in the beginning of each MBC */
	uint32_t mem0_blk_cfg_w[64];
	uint32_t mem0_blk_nse_w[16];
	uint32_t mem1_blk_cfg_w[8];
	uint32_t mem1_blk_nse_w[2];
	uint32_t mem2_blk_cfg_w[8];
	uint32_t mem2_blk_nse_w[2];
	uint32_t mem3_blk_cfg_w[8];
	uint32_t mem3_blk_nse_w[2]; /*0x1F0, 0x1F4 */
	uint32_t reserved[2];
};

struct mrc_rgn_dom {
	uint32_t mrc_glbcfg[4];
	uint32_t nse_rgn_indirect;
	uint32_t nse_rgn_set;
	uint32_t nse_rgn_clr;
	uint32_t nse_rgn_clr_all;
	uint32_t memn_glbac[8];
	/* The upper only existed in the beginning of each MRC */
	uint32_t rgn_desc_words[16][2]; /* 16 regions at max, 2 words per region */
	uint32_t rgn_nse;
	uint32_t reserved2[15];
};

struct mda_inst {
	uint32_t mda_w[8];
};

struct trdc_mgr {
	uint32_t trdc_cr;
	uint32_t res0[59];
	uint32_t trdc_hwcfg0;
	uint32_t trdc_hwcfg1;
	uint32_t res1[450];
	struct mda_inst mda[128];
};

struct trdc_mbc {
	struct mbc_mem_dom mem_dom[DID_NUM];
};

struct trdc_mrc {
	struct mrc_rgn_dom mrc_dom[DID_NUM];
};

/***************************************************************
 * Below structs used fro provding the TRDC configuration info
 * that will be used to init the TRDC based on use case.
 ***************************************************************/
struct trdc_glbac_config {
	uint8_t mbc_mrc_id;
	uint8_t glbac_id;
	uint32_t glbac_val;
};

struct trdc_mbc_config {
	uint8_t mbc_id;
	uint8_t dom_id;
	uint8_t mem_id;
	uint8_t blk_id;
	uint8_t glbac_id;
	bool secure;
};

struct trdc_mrc_config {
	uint8_t mrc_id;
	uint8_t dom_id;
	uint8_t region_id;
	uint32_t region_start;
	uint32_t region_size;
	uint8_t glbac_id;
	bool secure;
};

struct trdc_mgr_info {
	uintptr_t trdc_base;
	uint8_t mbc_id;
	uint8_t mbc_mem_id;
	uint8_t blk_mgr;
	uint8_t blk_mc;
};

struct trdc_config_info {
	uintptr_t trdc_base;
	struct trdc_glbac_config *mbc_glbac;
	uint32_t num_mbc_glbac;
	struct trdc_mbc_config *mbc_cfg;
	uint32_t num_mbc_cfg;
	struct trdc_glbac_config *mrc_glbac;
	uint32_t num_mrc_glbac;
	struct trdc_mrc_config *mrc_cfg;
	uint32_t num_mrc_cfg;
};

extern struct trdc_mgr_info trdc_mgr_blks[];
extern unsigned int trdc_mgr_num;
/* APIs to apply and enable TRDC */
int trdc_mda_set_cpu(uintptr_t trdc_base, uint32_t mda_inst,
		     uint32_t mda_reg, uint8_t sa, uint8_t dids,
		     uint8_t did, uint8_t pe, uint8_t pidm, uint8_t pid);

int trdc_mda_set_noncpu(uintptr_t trdc_base, uint32_t mda_inst,
			bool did_bypass, uint8_t sa, uint8_t pa,
			uint8_t did);

void trdc_mgr_mbc_setup(struct trdc_mgr_info *mgr);
void trdc_setup(struct trdc_config_info *cfg);
void trdc_config(void);

#endif /* IMX_TRDC_H */
