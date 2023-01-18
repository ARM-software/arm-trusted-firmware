/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lpm/mt_lpm_smc.h>
#include <mt_spm.h>
#include "mt_spm_rc_api.h"
#include "mt_spm_rc_internal.h"

int spm_rc_condition_modifier(unsigned int id, unsigned int act,
			      const void *val,
			      enum mt_spm_rm_rc_type dest_rc_id,
			      struct mt_spm_cond_tables * const tlb)
{
	unsigned int rc_id, cond_id, cond;
	int res = 0;

	spin_lock(&spm_lock);
	rc_id = SPM_RC_UPDATE_COND_RC_ID_GET(id);
	cond_id = SPM_RC_UPDATE_COND_ID_GET(id);

	do {
		if ((dest_rc_id != rc_id) || (val == NULL) || (tlb == NULL)) {
			res = -1;
			break;
		}

		cond = *((unsigned int *)val);

		if (cond_id < PLAT_SPM_COND_MAX) {
			if ((act & MT_LPM_SMC_ACT_SET) > 0U) {
				SPM_RC_BITS_SET(tlb->table_cg[cond_id], cond);
			} else if ((act & MT_LPM_SMC_ACT_CLR) > 0U) {
				SPM_RC_BITS_CLR(tlb->table_cg[cond_id], cond);
			} else {
				res = -1;
			}
		} else if ((cond_id - PLAT_SPM_COND_MAX) < PLAT_SPM_COND_PLL_MAX) {
			unsigned int pll_idx = cond_id - PLAT_SPM_COND_MAX;

			cond = !!cond;
			if ((act & MT_LPM_SMC_ACT_SET) > 0U) {
				SPM_RC_BITS_SET(tlb->table_pll, (cond << pll_idx));
			} else if ((act & MT_LPM_SMC_ACT_CLR) > 0U) {
				SPM_RC_BITS_CLR(tlb->table_pll, (cond << pll_idx));
			} else {
				res = -1;
			}
		} else {
			res = -1;
		}
	} while (0);

	spin_unlock(&spm_lock);

	return res;
}

int spm_rc_constraint_status_get(unsigned int id, unsigned int type,
				 unsigned int act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) && (id != dest_rc_id)) || (dest == NULL) ||
	    (src == NULL)) {
		return -1;
	}
	spin_lock(&spm_lock);

	switch (type) {
	case CONSTRAINT_GET_ENTER_CNT:
		if (id == MT_RM_CONSTRAINT_ID_ALL) {
			dest->enter_cnt += src->enter_cnt;
		} else {
			dest->enter_cnt = src->enter_cnt;
		}
		break;
	case CONSTRAINT_GET_VALID:
		dest->is_valid = src->is_valid;
		break;
	case CONSTRAINT_COND_BLOCK:
		dest->is_cond_block = src->is_cond_block;
		dest->all_pll_dump = src->all_pll_dump;
		break;
	case CONSTRAINT_GET_COND_BLOCK_DETAIL:
		dest->cond_res = src->cond_res;
		break;
	case CONSTRAINT_GET_RESIDNECY:
		dest->residency = src->residency;
		if (act & MT_LPM_SMC_ACT_CLR) {
			src->residency = 0;
		}
		break;
	default:
		break;
	}

	spin_unlock(&spm_lock);
	return 0;
}

int spm_rc_constraint_status_set(unsigned int id, unsigned int type,
				 unsigned int act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) && (id != dest_rc_id)) || (dest == NULL)) {
		return -1;
	}

	spin_lock(&spm_lock);

	switch (type) {
	case CONSTRAINT_UPDATE_VALID:
		if (src != NULL) {
			if ((act & MT_LPM_SMC_ACT_SET) > 0U) {
				SPM_RC_BITS_SET(dest->is_valid, src->is_valid);
			} else if ((act & MT_LPM_SMC_ACT_CLR) > 0U) {
				SPM_RC_BITS_CLR(dest->is_valid, src->is_valid);
			}
		}
		break;
	case CONSTRAINT_RESIDNECY:
		if (act & MT_LPM_SMC_ACT_CLR) {
			dest->residency = 0;
		}
		break;
	default:
		break;
	}

	spin_unlock(&spm_lock);

	return 0;
}

int spm_rc_constraint_valid_set(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				unsigned int valid,
				struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) && (id != dest_rc_id)) || (dest == NULL)) {
		return -1;
	}

	spin_lock(&spm_lock);
	SPM_RC_BITS_SET(dest->is_valid, valid);
	spin_unlock(&spm_lock);

	return 0;
}

int spm_rc_constraint_valid_clr(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				unsigned int valid,
				struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) && (id != dest_rc_id)) || (dest == NULL)) {
		return -1;
	}

	spin_lock(&spm_lock);
	SPM_RC_BITS_CLR(dest->is_valid, valid);
	spin_unlock(&spm_lock);

	return 0;
}
