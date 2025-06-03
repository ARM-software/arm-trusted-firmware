/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_API_H
#define MT_SPM_RC_API_H

#include <mt_spm_cond.h>
#include <mt_spm_rc_api_common.h>

int spm_rc_condition_modifier(uint32_t id, uint32_t act, const void *val,
			      enum mt_spm_rm_rc_type dest_rc_id,
			      struct mt_spm_cond_tables *const tlb);

#endif /* MT_SPM_RC_API_H */
