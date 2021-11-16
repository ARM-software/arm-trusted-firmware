/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mt_lp_irqremain.h>
#include <mt_lp_rm.h>
#include <mtk_cirq.h>
#include <plat_mtk_lpm.h>

#define KEYPAD_IRQ_ID		U(138)
#define KEYPAD_WAKESRC		(0x4U)

static struct mt_irqremain remain_irqs;

int mt_lp_irqremain_submit(void)
{
	int ret = 0;

	if (remain_irqs.count == 0) {
		ret = -1;
	} else {
		set_wakeup_sources(remain_irqs.irqs, remain_irqs.count);
		mt_lp_rm_do_update(-1, PLAT_RC_UPDATE_REMAIN_IRQS, &remain_irqs);
	}

	return ret;
}

int mt_lp_irqremain_aquire(void)
{
	int ret = 0;

	if (remain_irqs.count == 0) {
		ret = -1;
	} else {
		mt_cirq_sw_reset();
		mt_cirq_clone_gic();
		mt_cirq_enable();
	}

	return ret;
}

int mt_lp_irqremain_release(void)
{
	int ret = 0;

	if (remain_irqs.count == 0) {
		ret = -1;
	} else {
		mt_cirq_flush();
		mt_cirq_disable();
	}

	return ret;
}

void mt_lp_irqremain_init(void)
{
	uint32_t idx;

	remain_irqs.count = 0U;

	/*edge keypad*/
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = KEYPAD_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0U;
	remain_irqs.wakeupsrc[idx] = KEYPAD_WAKESRC;
	remain_irqs.count++;

	mt_lp_irqremain_submit();
}
