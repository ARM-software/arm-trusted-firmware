/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <constraints/mt_spm_rc_api.h>
#include <lpm_v2/mt_lpm_smc.h>
#ifndef MTK_PLAT_CIRQ_UNSUPPORT
#include <mt_gic_v3.h>
#endif
#include <mt_plat_spm_setting.h>
#include <mt_spm_common.h>

int spm_rc_condition_modifier(uint32_t id, uint32_t act, const void *val,
			      enum mt_spm_rm_rc_type dest_rc_id,
			      struct mt_spm_cond_tables *const tlb)
{
	uint32_t rc_id, cond_id, cond = 0;
	int res = 0;

	plat_spm_lock();
	rc_id = SPM_RC_UPDATE_COND_RC_ID_GET(id);
	cond_id = SPM_RC_UPDATE_COND_ID_GET(id);

	do {
		if ((dest_rc_id != rc_id) || !val || !tlb) {
			res = -1;
			break;
		}

		cond = *((uint32_t *)val);

		if ((cond_id - PLAT_SPM_COND_MAX) < PLAT_SPM_COND_PLL_MAX) {
			uint32_t pll_idx = cond_id - PLAT_SPM_COND_MAX;

			cond = !!cond;
			if (act & MT_LPM_SMC_ACT_SET)
				tlb->table_pll |= (cond << pll_idx);
			else if (act & MT_LPM_SMC_ACT_CLR)
				tlb->table_pll &= ~(cond << pll_idx);
			else
				res = -1;
		} else
			res = -1;
	} while (0);

	plat_spm_unlock();

	return res;
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

void do_irqs_delivery(struct mt_irqremain *irqs, struct wake_status *wakeup)
{
	uint32_t idx;

	if (!irqs || !wakeup)
		return;

	for (idx = 0; idx < irqs->count; idx++) {
		if ((wakeup->tr.comm.raw_sta & irqs->wakeupsrc[idx]) ||
		    (wakeup->tr.comm.r12 & irqs->wakeupsrc[idx])) {
			if ((irqs->wakeupsrc_cat[idx] & MT_IRQ_REMAIN_CAT_LOG))
				mt_spm_irq_remain_dump(irqs, idx, wakeup);

			mt_irq_set_pending(irqs->irqs[idx]);
		}
	}
}
#endif
