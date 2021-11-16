/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef MT_SPM_SUSPEDN_H
#define MT_SPM_SUSPEDN_H

#include <mt_spm_internal.h>

#define MCUPM_MBOX_OFFSET_PDN	(0x0C55FDA8)
#define MCUPM_POWER_DOWN	(0x4D50444E)

enum MT_SPM_SUSPEND_MODE {
	MT_SPM_SUSPEND_SYSTEM_PDN	= 0U,
	MT_SPM_SUSPEND_SLEEP		= 1U,
};

extern int mt_spm_suspend_mode_set(int mode);
extern int mt_spm_suspend_enter(int state_id, unsigned int ext_opand,
				unsigned int reosuce_req);
extern void mt_spm_suspend_resume(int state_id, unsigned int ext_opand,
				  struct wake_status **status);
extern void mt_spm_suspend_init(void);

#endif /* MT_SPM_SUSPEND_H */
