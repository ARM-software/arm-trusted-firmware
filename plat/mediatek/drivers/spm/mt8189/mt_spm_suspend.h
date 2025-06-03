/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
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
	MT_SPM_SUSPEND_SYSTEM_PDN,
	MT_SPM_SUSPEND_SLEEP,
};

int mt_spm_suspend_mode_set(enum mt_spm_suspend_mode mode, void *prv);

int mt_spm_suspend_enter(int state_id, uint32_t ext_opand,
			 uint32_t reosuce_req);

void mt_spm_suspend_resume(int state_id, uint32_t ext_opand,
			   struct wake_status **status);

int mt_spm_suspend_get_spm_lp(struct spm_lp_scen **lp);

#endif /* MT_SPM_SUSPEND_H */
