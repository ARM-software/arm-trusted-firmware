/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <constraints/mt_spm_rc_api.h>
#include <constraints/mt_spm_rc_internal.h>
#include <lpm_v2/mt_lpm_smc.h>
#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_common.h>

#define SPM_RC_VALID_SET(dest, src)	({ (dest) |= (src); })
#define SPM_RC_VALID_CLR(dest, src)	({ (dest) &= ~(src); })

int spm_rc_constraint_status_get(uint32_t id, uint32_t type,
				 uint32_t act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest || !src)
		return -1;
	spm_lock_get();

	switch (type) {
	case CONSTRAINT_GET_ENTER_CNT:
		if (id == MT_RM_CONSTRAINT_ID_ALL)
			dest->enter_cnt += src->enter_cnt;
		else
			dest->enter_cnt = src->enter_cnt;
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
		if (act & MT_LPM_SMC_ACT_CLR)
			src->residency = 0;
		break;
	default:
		break;
	}

	spm_lock_release();
	return 0;
}

int spm_rc_constraint_status_set(uint32_t id, uint32_t type,
				 uint32_t act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest)
		return -1;

	spm_lock_get();

	switch (type) {
	case CONSTRAINT_UPDATE_VALID:
		if (src) {
			if (act & MT_LPM_SMC_ACT_SET)
				SPM_RC_VALID_SET(dest->is_valid, src->is_valid);
			else if (act & MT_LPM_SMC_ACT_CLR)
				SPM_RC_VALID_CLR(dest->is_valid, src->is_valid);
		}
		break;
	case CONSTRAINT_RESIDNECY:
		if (act & MT_LPM_SMC_ACT_CLR)
			dest->residency = 0;
		break;
	default:
		break;
	}

	spm_lock_release();

	return 0;
}

int spm_rc_constraint_valid_set(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				uint32_t valid,
				struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest)
		return -1;

	spm_lock_get();
	SPM_RC_VALID_SET(dest->is_valid, valid);
	spm_lock_release();

	return 0;
}

int spm_rc_constraint_valid_clr(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				uint32_t valid,
				struct constraint_status * const dest)
{
	if (((id != MT_RM_CONSTRAINT_ID_ALL) &&
	    (id != dest_rc_id)) || !dest)
		return -1;

	spm_lock_get();
	SPM_RC_VALID_CLR(dest->is_valid, valid);
	spm_lock_release();

	return 0;
}

/*
 * CIRQ will respresent interrupt which type is edge
 * when gic mask. But if the 26 clk have turned off before
 * then cirq won't work normally.
 * So we need to set irq pending for specific wakeup source.
 */
#ifndef MTK_PLAT_CIRQ_UNSUPPORT
static void mt_spm_irq_remain_dump(struct mt_irqremain *irqs,
				   uint32_t irq_index,
				   struct wake_status *wakeup)
{
	INFO("[SPM] r12=0x%08x(0x%08x), irq:%u(0x%08x) set pending\n",
	     wakeup->tr.comm.r12,
	     wakeup->md32pcm_wakeup_sta,
	     irqs->wakeupsrc[irq_index],
	     irqs->irqs[irq_index]);
}

void do_irqs_delivery(struct mt_irqremain *irqs,
		      struct wake_status *wakeup)
{
	uint32_t idx;

	if (!irqs || !wakeup)
		return;

	for (idx = 0; idx < irqs->count; idx++) {
		if ((wakeup->tr.comm.raw_sta & irqs->wakeupsrc[idx]) ||
		    (wakeup->tr.comm.r12 & irqs->wakeupsrc[idx])) {
			if ((irqs->wakeupsrc_cat[idx] & MT_IRQ_REMAIN_CAT_LOG))
				mt_spm_irq_remain_dump(irqs, idx, wakeup);
		}
	}
}
#endif
