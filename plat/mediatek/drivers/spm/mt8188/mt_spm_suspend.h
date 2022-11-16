/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_SUSPEND_H
#define MT_SPM_SUSPEND_H

#include <mt_spm_internal.h>

struct suspend_dbg_ctrl {
	uint32_t sleep_suspend_cnt;
};

enum mt_spm_suspend_mode {
	MT_SPM_SUSPEND_SYSTEM_PDN = 0,
	MT_SPM_SUSPEND_SLEEP,
};

int mt_spm_suspend_mode_set(int mode, void *prv);
int mt_spm_suspend_enter(int state_id, unsigned int ext_opand, unsigned int reosuce_req);
void mt_spm_suspend_resume(int state_id, unsigned int ext_opand, struct wake_status **status);

#endif
