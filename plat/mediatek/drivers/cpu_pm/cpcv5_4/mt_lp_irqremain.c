/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>

#include <platform_def.h>

#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lp_rm.h>
#include <mt_cirq.h>
#include "mt_cpu_pm.h"
#include "mt_lp_irqremain.h"

static struct mt_irqremain remain_irqs;
static struct mt_irqremain *p_irqs;

int mt_lp_irqremain_push(void)
{
	if (remain_irqs.count >= MT_IRQ_REMAIN_MAX)
		return -1;
	remain_irqs.count += 1;
	return 0;
}

int mt_lp_irqremain_pop(void)
{
	if (remain_irqs.count == 0)
		return -1;
	remain_irqs.count -= 1;
	return 0;
}

int mt_lp_irqremain_set(unsigned int type,
			const struct mt_lp_irqinfo *info)
{
	unsigned int idx;

	if (p_irqs || !info)
		return -1;

	idx = remain_irqs.count;
	switch (type) {
	case IRQS_REMAIN_IRQ:
		remain_irqs.irqs[idx] = info->val;
		break;
	case IRQS_REMAIN_WAKEUP_CAT:
		remain_irqs.wakeupsrc_cat[idx] = info->val;
		break;
	case IRQS_REMAIN_WAKEUP_SRC:
		remain_irqs.wakeupsrc[idx] = info->val;
		break;
	}
	return 0;
}

int mt_lp_irqremain_get(unsigned int idx, unsigned int type,
			struct mt_lp_irqinfo *info)
{
	if (!p_irqs || !info || (idx > remain_irqs.count))
		return -1;

	switch (type) {
	case IRQS_REMAIN_IRQ:
		info->val = remain_irqs.irqs[idx];
		break;
	case IRQS_REMAIN_WAKEUP_CAT:
		info->val = remain_irqs.wakeupsrc_cat[idx];
		break;
	case IRQS_REMAIN_WAKEUP_SRC:
		info->val = remain_irqs.wakeupsrc[idx];
		break;
	}
	return 0;
}

unsigned int mt_lp_irqremain_count(void)
{
	return remain_irqs.count;
}

int mt_lp_irqremain_submit(void)
{
	if (remain_irqs.count == 0)
		return -1;
	set_wakeup_sources(remain_irqs.irqs, remain_irqs.count);
	mt_lp_rm_do_update(-1, PLAT_RC_UPDATE_REMAIN_IRQS, &remain_irqs);
	p_irqs = &remain_irqs;
	return 0;
}

int mt_lp_irqremain_aquire(void)
{
	if (!p_irqs)
		return -1;

	mt_cirq_sw_reset();
	mt_cirq_clone_gic();
	mt_cirq_enable();
	return 0;
}

int mt_lp_irqremain_release(void)
{
	if (!p_irqs)
		return -1;
	mt_cirq_flush();
	mt_cirq_disable();
	return 0;
}

void mt_lp_irqremain_init(void)
{
	p_irqs = NULL;
}
