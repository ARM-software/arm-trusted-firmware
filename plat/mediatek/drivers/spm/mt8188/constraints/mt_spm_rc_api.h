/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_API_H
#define MT_SPM_RC_API_H

#include <mt_spm.h>
#include <mt_spm_cond.h>
#include <mt_spm_constraint.h>
#include <mt_spm_internal.h>

#define SPM_RC_BITS_SET(dest, src) ({ (dest) |= (src); })
#define SPM_RC_BITS_CLR(dest, src) ({ (dest) &= (~src); })

int spm_rc_condition_modifier(unsigned int id, unsigned int act,
			      const void *val,
			      enum mt_spm_rm_rc_type dest_rc_id,
			      struct mt_spm_cond_tables * const tlb);

int spm_rc_constraint_status_get(unsigned int id, unsigned int type,
				 unsigned int act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest);

int spm_rc_constraint_status_set(unsigned int id, unsigned int type,
				 unsigned int act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status * const src,
				 struct constraint_status * const dest);

int spm_rc_constraint_valid_set(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				unsigned int valid,
				struct constraint_status * const dest);

int spm_rc_constraint_valid_clr(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				unsigned int valid,
				struct constraint_status * const dest);

#endif
