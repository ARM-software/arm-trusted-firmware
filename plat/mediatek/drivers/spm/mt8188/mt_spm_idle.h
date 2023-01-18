/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_IDLE_H
#define MT_SPM_IDLE_H

#include "mt_spm_internal.h"

typedef int (*spm_idle_conduct)(int state_id,
				struct spm_lp_scen *spm_lp,
				unsigned int *resource_req);
typedef int (*spm_idle_conduct_restore)(int state_id,
					struct spm_lp_scen *spm_lp,
					struct wake_status *status);

int mt_spm_idle_generic_enter(int state_id, unsigned int ext_opand, spm_idle_conduct fn);
void mt_spm_idle_generic_resume(int state_id, unsigned int ext_opand,
				struct wake_status **status,
				spm_idle_conduct_restore fn);

#endif
