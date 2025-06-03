/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/libc/errno.h>
#include <stdbool.h>

#include <lib/mmio.h>

#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lpm_smc.h>
#include <mt_spm_cond.h>
#include <platform_def.h>

static struct plat_cond_info_t *cond_info;

uint64_t mt_spm_cond_check(const struct mt_spm_cond_tables *src,
			   const struct mt_spm_cond_tables *dest,
			   struct mt_spm_cond_tables *res)
{
	uint64_t b_res = 0;
	unsigned int i = 0;

	if (!src || !dest)
		return SPM_COND_CHECK_FAIL;

	if (cond_info && src->table_cg && dest->table_cg) {
		for (i = 0; i < cond_info->idle_cond_num; i++) {
			if (res && res->table_cg) {
				res->table_cg[i] =
					(src->table_cg[i] & dest->table_cg[i]);

				if (res->table_cg[i])
					b_res |= (((uint64_t)1) << i);

			} else if (src->table_cg[i] & dest->table_cg[i]) {
				b_res |= (((uint64_t)1) << i);
				break;
			}
		}
	}

	if (res) {
		res->table_pll = (src->table_pll & dest->table_pll);

		if (res->table_pll)
			b_res |= (res->table_pll << SPM_COND_BLOCKED_PLL_IDX) |
				 SPM_COND_CHECK_BLOCKED_PLL;
	} else if (src->table_pll & dest->table_pll)
		b_res |= SPM_COND_CHECK_BLOCKED_PLL;

	return b_res;
}

uint64_t mt_spm_dump_all_pll(const struct mt_spm_cond_tables *src,
			     const struct mt_spm_cond_tables *dest,
			     struct mt_spm_cond_tables *res)
{
	uint64_t b_res = 0;

	if (res) {
		res->table_pll = src->table_pll;
		if (res->table_pll)
			b_res |= (res->table_pll << SPM_COND_BLOCKED_PLL_IDX) |
				 SPM_COND_CHECK_BLOCKED_PLL;
	} else if (src->table_pll & dest->table_pll)
		b_res |= SPM_COND_CHECK_BLOCKED_PLL;

	return b_res;
}

int mt_spm_cond_update(struct mt_resource_constraint **con, int num,
		       int stateid, void *priv)
{
	int i, res;
	struct mt_resource_constraint *const *_con;
	struct plat_idle_cond_info *cg_table;
	struct plat_pll_cond_info *pll_table;
	struct mt_spm_cond_tables *spm_cond;

	if (!cond_info)
		return -ENODEV;

	cg_table = cond_info->idle_cond_table;
	pll_table = cond_info->pll_cond_table;
	spm_cond = &cond_info->cond_table_buf;

	if (cg_table && spm_cond->table_cg) {
		for (i = 0; i < cond_info->idle_cond_num; i++) {
			spm_cond->table_cg[i] = 0;
			/* check mtcmos, if off set idle_value and clk to 0 disable */
			if (cond_info->spm_pwr_status_addr)
				if (!(mmio_read_32(
					      cond_info->spm_pwr_status_addr) &
				      cg_table[i].pwr_status_mask))
					continue;

			if (cond_info->spm_pwr_status_2nd_addr)
				if (!(mmio_read_32(
					      cond_info->spm_pwr_status_2nd_addr) &
				      cg_table[i].pwr_status_mask))
					continue;
			/* check clkmux */
			if (cg_table[i].clkmux_addr)
				if (mmio_read_32(cg_table[i].clkmux_addr) &
				    cg_table[i].clkmux_mask)
					continue;

			spm_cond->table_cg[i] =
				cg_table[i].bBitflip ?
					~mmio_read_32(cg_table[i].cg_addr) :
					mmio_read_32(cg_table[i].cg_addr);
		}
	}

	if (pll_table) {
		spm_cond->table_pll = 0;
		for (i = 0; pll_table[i].pll_bit_set != PLL_BIT_MAX; i++) {
			if (mmio_read_32(pll_table[i].pll_addr) &
			    pll_table[i].pll_mask) {
				spm_cond->table_pll |= pll_table[i].pll_bit_set;
			}
		}
	}

	spm_cond->priv = priv;

	for (i = 0, _con = con; *_con && (*_con)->update && (i < num);
	     _con++, i++) {
		res = (*_con)->update(stateid, PLAT_RC_UPDATE_CONDITION,
				      (void const *)spm_cond);
		if (res != MT_RM_STATUS_OK)
			break;
	}

	return 0;
}

int register_plat_cond_info(struct plat_cond_info_t *cond)
{
	if (!cond)
		return -EINVAL;

	cond_info = cond;

	return 0;
}
