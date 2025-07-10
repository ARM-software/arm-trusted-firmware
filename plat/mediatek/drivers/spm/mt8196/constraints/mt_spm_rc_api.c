/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <constraints/mt_spm_rc_api.h>
#include <lpm_v2/mt_lpm_smc.h>
#include <mt_plat_spm_setting.h>
#include <mt_spm_common.h>

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
