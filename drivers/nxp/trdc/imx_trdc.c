/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/nxp/trdc/imx_trdc.h>
#include <lib/mmio.h>


int trdc_mda_set_cpu(uintptr_t trdc_base, uint32_t mda_inst,
		     uint32_t mda_reg, uint8_t sa, uint8_t dids,
		     uint8_t did, uint8_t pe, uint8_t pidm, uint8_t pid)
{
	uint32_t val = mmio_read_32(trdc_base + MDAC_W_X(mda_inst, mda_reg));
	/* invalid: config non-cpu master with cpu config format. */
	if ((val & MDA_DFMT) != 0U) {
		return -EINVAL;
	}

	val = MDA_VLD | MDA_DFMT0_DID(pid) | MDA_DFMT0_PIDM(pidm) | MDA_DFMT0_PE(pe) |
	      MDA_DFMT0_SA(sa) | MDA_DFMT0_DIDS(dids) | MDA_DFMT0_DID(did);

	mmio_write_32(trdc_base + MDAC_W_X(mda_inst, mda_reg), val);

	return 0;
}

int trdc_mda_set_noncpu(uintptr_t trdc_base, uint32_t mda_inst,
			bool did_bypass, uint8_t sa, uint8_t pa,
			uint8_t did)
{
	uint32_t val = mmio_read_32(trdc_base + MDAC_W_X(mda_inst, 0));

	/* invalid: config cpu master with non-cpu config format. */
	if ((val & MDA_DFMT) == 0U) {
		return -EINVAL;
	}

	val = MDA_VLD | MDA_DFMT1_SA(sa) | MDA_DFMT1_PA(pa) | MDA_DFMT1_DID(did) |
	      MDA_DFMT1_DIDB(did_bypass ? 1U : 0U);

	mmio_write_32(trdc_base + MDAC_W_X(mda_inst, 0), val);

	return 0;
}

static uintptr_t trdc_get_mbc_base(uintptr_t trdc_reg, uint32_t mbc_x)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	uint32_t mbc_num = MBC_NUM(trdc_base->trdc_hwcfg0);

	if (mbc_x >= mbc_num) {
		return 0U;
	}

	return trdc_reg + 0x10000 + 0x2000 * mbc_x;
}

static uintptr_t trdc_get_mrc_base(uintptr_t trdc_reg, uint32_t mrc_x)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	uint32_t mbc_num = MBC_NUM(trdc_base->trdc_hwcfg0);
	uint32_t mrc_num = MRC_NUM(trdc_base->trdc_hwcfg0);

	if (mrc_x >= mrc_num) {
		return 0U;
	}

	return trdc_reg + 0x10000 + 0x2000 * mbc_num + 0x1000 * mrc_x;
}

uint32_t trdc_mbc_blk_num(uintptr_t trdc_reg, uint32_t mbc_x, uint32_t mem_x)
{
	uint32_t glbcfg;
	struct mbc_mem_dom *mbc_dom;
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);

	if (mbc_base == NULL) {
		return 0;
	}

	/* only first dom has the glbcfg */
	mbc_dom = &mbc_base->mem_dom[0];
	glbcfg = mmio_read_32((uintptr_t)&mbc_dom->mem_glbcfg[mem_x]);

	return MBC_BLK_NUM(glbcfg);
}

uint32_t trdc_mrc_rgn_num(uintptr_t trdc_reg, uint32_t mrc_x)
{
	uint32_t glbcfg;
	struct mrc_rgn_dom *mrc_dom;
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);

	if (mrc_base == NULL) {
		return 0;
	}

	/* only first dom has the glbcfg */
	mrc_dom = &mrc_base->mrc_dom[0];
	glbcfg = mmio_read_32((uintptr_t)&mrc_dom->mrc_glbcfg[0]);

	return MBC_BLK_NUM(glbcfg);
}

int trdc_mbc_set_control(uintptr_t trdc_reg, uint32_t mbc_x,
			 uint32_t glbac_id, uint32_t glbac_val)
{
	struct mbc_mem_dom *mbc_dom;
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);

	if (mbc_base == NULL || glbac_id >= GLBAC_NUM) {
		return -EINVAL;
	}

	/* only first dom has the glbac */
	mbc_dom = &mbc_base->mem_dom[0];

	mmio_write_32((uintptr_t)&mbc_dom->memn_glbac[glbac_id], glbac_val);

	return 0;
}

int trdc_mbc_blk_config(uintptr_t trdc_reg, uint32_t mbc_x,
			uint32_t dom_x, uint32_t mem_x, uint32_t blk_x,
			bool sec_access, uint32_t glbac_id)
{
	uint32_t *cfg_w;
	uint32_t index, offset, val;
	struct mbc_mem_dom *mbc_dom;
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);

	if (mbc_base == NULL || glbac_id >= GLBAC_NUM) {
		return -EINVAL;
	}

	mbc_dom = &mbc_base->mem_dom[dom_x];

	switch (mem_x) {
	case 0:
		cfg_w = &mbc_dom->mem0_blk_cfg_w[blk_x / 8];
		break;
	case 1:
		cfg_w = &mbc_dom->mem1_blk_cfg_w[blk_x / 8];
		break;
	case 2:
		cfg_w = &mbc_dom->mem2_blk_cfg_w[blk_x / 8];
		break;
	case 3:
		cfg_w = &mbc_dom->mem3_blk_cfg_w[blk_x / 8];
		break;
	default:
		return -EINVAL;
	};

	index = blk_x % 8;
	offset = index * 4;

	val = mmio_read_32((uintptr_t)cfg_w);
	val &= ~(0xF << offset);

	/*
	 * MBC0-3
	 * Global 0, 0x7777 secure pri/user read/write/execute,
	 * S400 has already set it. So select MBC0_MEMN_GLBAC0
	 */
	if (sec_access) {
		val |= ((0x0 | (glbac_id & 0x7)) << offset);
		mmio_write_32((uintptr_t)cfg_w, val);
	} else {
		/* nse bit set */
		val |= ((0x8 | (glbac_id & 0x7)) << offset);
		mmio_write_32((uintptr_t)cfg_w, val);
	}

	return 0;
}

int trdc_mrc_set_control(uintptr_t trdc_reg, uint32_t mrc_x,
			 uint32_t glbac_id, uint32_t glbac_val)
{
	struct mrc_rgn_dom *mrc_dom;
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);

	if (mrc_base == NULL || glbac_id >= GLBAC_NUM) {
		return -EINVAL;
	}

	/* only first dom has the glbac */
	mrc_dom = &mrc_base->mrc_dom[0];

	mmio_write_32((uintptr_t)&mrc_dom->memn_glbac[glbac_id], glbac_val);

	return 0;
}

int trdc_mrc_rgn_config(uintptr_t trdc_reg, uint32_t mrc_x,
			uint32_t dom_x, uint32_t rgn_id,
			uint32_t addr_start, uint32_t addr_size,
			bool sec_access, uint32_t glbac_id)
{
	uint32_t *desc_w;
	uint32_t addr_end;
	struct mrc_rgn_dom *mrc_dom;
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);

	if (mrc_base == NULL || glbac_id >= GLBAC_NUM || rgn_id >= MRC_REG_ALL) {
		return -EINVAL;
	}

	mrc_dom = &mrc_base->mrc_dom[dom_x];

	addr_end = addr_start + addr_size - 1;
	addr_start &= ~0x3fff;
	addr_end &= ~0x3fff;

	desc_w = &mrc_dom->rgn_desc_words[rgn_id][0];

	if (sec_access) {
		mmio_write_32((uintptr_t)desc_w, addr_start | (glbac_id & 0x7));
		mmio_write_32((uintptr_t)(desc_w + 1), addr_end | 0x1);
	} else {
		mmio_write_32((uintptr_t)desc_w, addr_start | (glbac_id & 0x7));
		mmio_write_32((uintptr_t)(desc_w + 1), (addr_end | 0x1 | 0x10));
	}

	return 0;
}

bool trdc_mrc_enabled(uintptr_t mrc_base)
{
	return (mmio_read_32(mrc_base) & BIT(15));
}

bool trdc_mbc_enabled(uintptr_t mbc_base)
{
	return (mmio_read_32(mbc_base) & BIT(14));
}

static bool is_trdc_mgr_slot(uintptr_t trdc_base, uint8_t mbc_id,
			     uint8_t mem_id, uint16_t blk_id)
{
	unsigned int i;

	for (i = 0U; i < trdc_mgr_num; i++) {
		if (trdc_mgr_blks[i].trdc_base == trdc_base) {
			if (mbc_id == trdc_mgr_blks[i].mbc_id &&
			    mem_id == trdc_mgr_blks[i].mbc_mem_id &&
			   (blk_id == trdc_mgr_blks[i].blk_mgr ||
			    blk_id == trdc_mgr_blks[i].blk_mc)) {
				return true;
			}
		}
	}

	return false;
}

/*
 * config the TRDC MGR & MC's access policy. only the secure privilege
 * mode SW can access it.
 */
void trdc_mgr_mbc_setup(struct trdc_mgr_info *mgr)
{
	unsigned int i;

	/*
	 * If the MBC is global enabled, need to cconfigure the MBCs of
	 * TRDC MGR & MC correctly.
	 */
	if (trdc_mbc_enabled(mgr->trdc_base)) {
		/* ONLY secure privilige can access */
		trdc_mbc_set_control(mgr->trdc_base, mgr->mbc_id, 7, 0x6000);
		for (i = 0U; i < 16U; i++) {
			trdc_mbc_blk_config(mgr->trdc_base, mgr->mbc_id, i,
				mgr->mbc_mem_id, mgr->blk_mgr, true, 7);

			trdc_mbc_blk_config(mgr->trdc_base, mgr->mbc_id, i,
				mgr->mbc_mem_id, mgr->blk_mc, true, 7);
		}
	}
}

/*
 * Set up the TRDC access policy for all the resources under
 * the TRDC control.
 */
void trdc_setup(struct trdc_config_info *cfg)
{
	unsigned int i, j, num;
	bool is_mgr;

	/* config the MRCs */
	if (trdc_mrc_enabled(cfg->trdc_base)) {
		/* set global access policy */
		for (i = 0U; i < cfg->num_mrc_glbac; i++) {
			trdc_mrc_set_control(cfg->trdc_base,
					     cfg->mrc_glbac[i].mbc_mrc_id,
					     cfg->mrc_glbac[i].glbac_id,
					     cfg->mrc_glbac[i].glbac_val);
		}

		/* set each MRC region access policy */
		for (i = 0U; i < cfg->num_mrc_cfg; i++) {
			trdc_mrc_rgn_config(cfg->trdc_base, cfg->mrc_cfg[i].mrc_id,
					    cfg->mrc_cfg[i].dom_id,
					    cfg->mrc_cfg[i].region_id,
					    cfg->mrc_cfg[i].region_start,
					    cfg->mrc_cfg[i].region_size,
					    cfg->mrc_cfg[i].secure,
					    cfg->mrc_cfg[i].glbac_id);
		}
	}

	/* config the MBCs */
	if (trdc_mbc_enabled(cfg->trdc_base)) {
		/* set MBC global access policy */
		for (i = 0U; i < cfg->num_mbc_glbac; i++) {
			trdc_mbc_set_control(cfg->trdc_base,
					     cfg->mbc_glbac[i].mbc_mrc_id,
					     cfg->mbc_glbac[i].glbac_id,
					     cfg->mbc_glbac[i].glbac_val);
		}

		for (i = 0U; i < cfg->num_mbc_cfg; i++) {
			if (cfg->mbc_cfg[i].blk_id == MBC_BLK_ALL) {
				num = trdc_mbc_blk_num(cfg->trdc_base,
						       cfg->mbc_cfg[i].mbc_id,
						       cfg->mbc_cfg[i].mem_id);

				for (j = 0U; j < num; j++) {
					/* Skip mgr and mc */
					is_mgr = is_trdc_mgr_slot(cfg->trdc_base,
								  cfg->mbc_cfg[i].mbc_id,
								  cfg->mbc_cfg[i].mem_id, j);
					if (is_mgr) {
						continue;
					}

					trdc_mbc_blk_config(cfg->trdc_base,
							    cfg->mbc_cfg[i].mbc_id,
							    cfg->mbc_cfg[i].dom_id,
							    cfg->mbc_cfg[i].mem_id, j,
							    cfg->mbc_cfg[i].secure,
							    cfg->mbc_cfg[i].glbac_id);
				}
			} else {
				trdc_mbc_blk_config(cfg->trdc_base,
						    cfg->mbc_cfg[i].mbc_id,
						    cfg->mbc_cfg[i].dom_id,
						    cfg->mbc_cfg[i].mem_id,
						    cfg->mbc_cfg[i].blk_id,
						    cfg->mbc_cfg[i].secure,
						    cfg->mbc_cfg[i].glbac_id);
			}
		}
	}
}
