/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mt_lp_rm.h>
#include <mt_lp_irqremain.h>
#include <mtk_cirq.h>
#include <plat_mtk_lpm.h>

#define EDMA0_IRQ_ID		U(448)
#define MDLA_IRQ_ID		U(446)
#define MALI4_IRQ_ID		U(399)
#define MALI3_IRQ_ID		U(398)
#define MALI2_IRQ_ID		U(397)
#define MALI1_IRQ_ID		U(396)
#define MALI0_IRQ_ID		U(395)
#define VPU_CORE1_IRQ_ID	U(453)
#define VPU_CORE0_IRQ_ID	U(452)
#define MD_WDT_IRQ_ID		U(110)
#define KEYPAD_IRQ_ID		U(106)

#define MD_WDT_WAKESRC		0x2000000
#define KEYPAD_WAKESRC		0x4

static struct mt_irqremain remain_irqs;

int mt_lp_irqremain_submit(void)
{
	if (remain_irqs.count == 0) {
		return -1;
	}

	set_wakeup_sources(remain_irqs.irqs, remain_irqs.count);
	mt_lp_rm_do_update(-1, PLAT_RC_UPDATE_REMAIN_IRQS, &remain_irqs);

	return 0;
}

int mt_lp_irqremain_aquire(void)
{
	if (remain_irqs.count == 0) {
		return -1;
	}

	mt_cirq_sw_reset();
	mt_cirq_clone_gic();
	mt_cirq_enable();

	return 0;
}

int mt_lp_irqremain_release(void)
{
	if (remain_irqs.count == 0) {
		return -1;
	}

	mt_cirq_flush();
	mt_cirq_disable();

	return 0;
}

void mt_lp_irqremain_init(void)
{
	uint32_t idx;

	remain_irqs.count = 0;

	/* level edma0 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = EDMA0_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mdla */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = MDLA_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali4 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = MALI4_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali3 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = MALI3_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali2 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = MALI2_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali1 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = MALI1_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level mali0 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = MALI0_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level vpu core1 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = VPU_CORE1_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* level vpu core0 */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = VPU_CORE0_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = 0;
	remain_irqs.count++;

	/* edge mdwdt */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = MD_WDT_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = MD_WDT_WAKESRC;
	remain_irqs.count++;

	/* edge keypad */
	idx = remain_irqs.count;
	remain_irqs.irqs[idx] = KEYPAD_IRQ_ID;
	remain_irqs.wakeupsrc_cat[idx] = 0;
	remain_irqs.wakeupsrc[idx] = KEYPAD_WAKESRC;
	remain_irqs.count++;

	mt_lp_irqremain_submit();
}
