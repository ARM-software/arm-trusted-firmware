/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/libc/errno.h>

#include <lib/mmio.h>
#include <platform_def.h>

#include <mtk_mmap_pool.h>
#include <notifier/inc/mt_spm_notifier.h>
#include <notifier/v4/mt_spm_sspm_intc.h>

#define MT_SPM_SSPM_MBOX_OFF(x)		(SSPM_MBOX_3_BASE + x)
#define MT_SPM_MBOX(slot)		MT_SPM_SSPM_MBOX_OFF((slot<<2UL))

/* LOOKUP SSPM_MBOX_SPM_LP1 */
#define SSPM_MBOX_SPM_LP_LOOKUP1	MT_SPM_MBOX(0)
/* LOOKUP SSPM_MBOX_SPM_LP2 */
#define SSPM_MBOX_SPM_LP_LOOKUP2	MT_SPM_MBOX(1)
#define SSPM_MBOX_SPM_LP1		MT_SPM_MBOX(2)
#define SSPM_MBOX_SPM_LP2		MT_SPM_MBOX(3)
#define SSPM_MBOX_SPM_SIZE		16
/* 4 mbox is in usage */
#define MT_SPM_MBOX_OFFSET(slot)	MT_SPM_MBOX((slot + 4))

enum SSPM_SLEEP_CMD {
	SLP_TASK_NONE = 0,
	SLP_TASK_AP_SUSPEND,
	SLP_TASK_AP_RESUME,
	SLP_TASK_APSRC_OFF,
	SLP_TASK_APSRC_ON,
	SLP_TASK_INFRA_OFF,
	SLP_TASK_INFRA_ON,
	SLP_TASK_VCORE_OFF,
	SLP_TASK_VCORE_ON,
	SLP_TASK_IDLE_OFF,
	SLP_TASK_IDLE_ON,
	SLP_TASK_INFRA_noPERI_OFF,
	SLP_TASK_INFRA_noPERI_ON,
	SLP_TASK_ERR = 32,
};

static unsigned int cur_mbox_index;

static int __mt_spm_is_available_index(int cur_idx)
{
	unsigned int val;

	val = mmio_read_32(MT_SPM_MBOX_OFFSET(cur_idx));
	val = (val >> 30);
	if (val == 0 || val == 3)
		return 1;
	return 0;
}

static int __mt_spm_get_available_index(void)
{
	unsigned int idx = cur_mbox_index;
	int i = 0;

	for (i = 0 ; i < SSPM_MBOX_SPM_SIZE ; i++) {
		if (__mt_spm_is_available_index(idx)) {
			cur_mbox_index = (idx + 1) % SSPM_MBOX_SPM_SIZE;
			return idx;
		}
		idx = (idx + 1) % SSPM_MBOX_SPM_SIZE;
	}
	return -EBUSY;
}

static int __mt_spm_sspm_write_cmd_queue(int idx, int value)
{
	unsigned int val;

	val = mmio_read_32(MT_SPM_MBOX_OFFSET(idx));
	val = val ^ BIT(31);
	val = (value & 0x3fffffff) | (val & 0xc0000000);

	mmio_write_32(MT_SPM_MBOX_OFFSET(idx), val);

	return 0;
}

void mt_spm_sspm_cmd_enqueue(int cmd)
{
	int idx;

	idx = __mt_spm_get_available_index();

	/* Block when queue full */
	if (idx == -EBUSY) {
		while (!__mt_spm_is_available_index(cur_mbox_index))
			;
		idx = cur_mbox_index;
		cur_mbox_index = (cur_mbox_index+1) % SSPM_MBOX_SPM_SIZE;
	}
	__mt_spm_sspm_write_cmd_queue(idx, cmd);

}

int mt_spm_sspm_notify_u32(int type, unsigned int val)
{
	switch (type) {
	case MT_SPM_NOTIFY_LP_ENTER:
		mmio_write_32(SSPM_MBOX_SPM_LP1, val);
		mt_spm_sspm_cmd_enqueue(SLP_TASK_AP_SUSPEND);
		DO_SPM_SSPM_LP_NOTIFY();
		break;
	case MT_SPM_NOTIFY_LP_LEAVE:
		mmio_write_32(SSPM_MBOX_SPM_LP1, val);
		mt_spm_sspm_cmd_enqueue(SLP_TASK_AP_RESUME);
		DO_SPM_SSPM_LP_NOTIFY();
		break;
	case MT_SPM_NOTIFY_IDLE_ENTER:
		mmio_write_32(SSPM_MBOX_SPM_LP1, val);
		mt_spm_sspm_cmd_enqueue(SLP_TASK_IDLE_OFF);
		DO_SPM_SSPM_LP_NOTIFY();
		break;
	case MT_SPM_NOTIFY_IDLE_LEAVE:
		mmio_write_32(SSPM_MBOX_SPM_LP1, val);
		mt_spm_sspm_cmd_enqueue(SLP_TASK_IDLE_ON);
		DO_SPM_SSPM_LP_NOTIFY();
		break;
	default:
		break;
	}
	return 0;
}
