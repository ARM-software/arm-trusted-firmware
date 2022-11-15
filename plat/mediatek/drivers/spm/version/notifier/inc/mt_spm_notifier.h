/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_NOTIFIER_H
#define MT_SPM_NOTIFIER_H

enum mt_spm_sspm_notify_id {
	MT_SPM_NOTIFY_LP_ENTER = 0,
	MT_SPM_NOTIFY_LP_LEAVE,
	MT_SPM_NOTIFY_SUSPEND_VCORE_VOLTAGE,
};

#ifdef MTK_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT
static inline int mt_spm_sspm_notify_u32(int type, unsigned int val)
{
	(void)type;
	(void)val;
	return 0;
}
#else
int mt_spm_sspm_notify_u32(int type, unsigned int val);
#endif

#endif /* MT_SPM_NOTIFIER_H */
