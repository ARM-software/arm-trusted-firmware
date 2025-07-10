/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_API_COMMON_H
#define MT_SPM_RC_API_COMMON_H

#include <constraints/mt_spm_trace.h>
#include <mt_spm_constraint.h>
#include <mt_spm_internal.h>

enum mt_spm_rm_rc_type {
	MT_RM_CONSTRAINT_ID_VCORE,
	MT_RM_CONSTRAINT_ID_BUS26,
	MT_RM_CONSTRAINT_ID_SYSPL,
	MT_RM_CONSTRAINT_ID_ALL = 0xff,
};

enum mt_spm_rc_fp_type {
	MT_SPM_RC_FP_INIT = 0,
	MT_SPM_RC_FP_ENTER_START,
	MT_SPM_RC_FP_ENTER_NOTIFY,
	MT_SPM_RC_FP_ENTER_WAKE_SPM_BEFORE,
	MT_SPM_RC_FP_ENTER_WAKE_SPM_AFTER,
	MT_SPM_RC_FP_RESUME_START,
	MT_SPM_RC_FP_RESUME_NOTIFY,
	MT_SPM_RC_FP_RESUME_RESET_SPM_BEFORE,
	MT_SPM_RC_FP_RESUME_BACKUP_EDGE_INT,
};

#define CONSTRAINT_VCORE_ALLOW (MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF | \
				MT_RM_CONSTRAINT_ALLOW_DRAM_S0 | \
				MT_RM_CONSTRAINT_ALLOW_DRAM_S1 | \
				MT_RM_CONSTRAINT_ALLOW_VCORE_LP | \
				MT_RM_CONSTRAINT_ALLOW_LVTS_STATE | \
				MT_RM_CONSTRAINT_ALLOW_BUS26M_OFF | \
				MT_RM_CONSTRAINT_ALLOW_VCORE_OFF)

#define MT_SPM_RC_INFO(_cpu, _stateid, _rc_id)                       \
	({                                                           \
		MT_SPM_TRACE_COMMON_U32_WR(                          \
			MT_SPM_TRACE_COMM_RC_INFO,                   \
			(_cpu << 28) | ((_stateid & 0xffff) << 12) | \
				(_rc_id & 0xfff));                   \
	})

#define MT_SPM_RC_LAST_TIME(_time)                                           \
	({                                                                   \
		MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_LAST_TIME_H, \
					   (uint32_t)(_time >> 32));         \
		MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_LAST_TIME_L, \
					   (uint32_t)(_time & 0xffffffff));  \
	})

#ifdef MT_SPM_TIMESTAMP_SUPPORT
#define MT_SPM_RC_TAG(_cpu, _stateid, _rc_id)           \
	({                                              \
		uint64_t ktime = 0;                     \
		MT_SPM_TIME_GET(ktime);                 \
		MT_SPM_RC_INFO(_cpu, _stateid, _rc_id); \
		(void)ktime;                            \
		MT_SPM_RC_LAST_TIME(ktime);             \
	})

#else
#define MT_SPM_RC_TAG(_cpu, _stateid, _rc_id) \
	MT_SPM_RC_INFO(_cpu, _stateid, _rc_id)

#endif

#define MT_SPM_RC_FP(fp) \
	({ MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_FP, fp); })

#define MT_SPM_RC_TAG_VALID(_valid) \
	({ MT_SPM_TRACE_COMMON_U32_WR(MT_SPM_TRACE_COMM_RC_VALID, _valid); })

int spm_rc_constraint_status_get(uint32_t id, uint32_t type, uint32_t act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status *const src,
				 struct constraint_status *const dest);

int spm_rc_constraint_status_set(uint32_t id, uint32_t type, uint32_t act,
				 enum mt_spm_rm_rc_type dest_rc_id,
				 struct constraint_status *const src,
				 struct constraint_status *const dest);

int spm_rc_constraint_valid_set(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				uint32_t valid,
				struct constraint_status *const dest);

int spm_rc_constraint_valid_clr(enum mt_spm_rm_rc_type id,
				enum mt_spm_rm_rc_type dest_rc_id,
				uint32_t valid,
				struct constraint_status *const dest);

void mt_spm_dump_pmic_warp_reg(void);
uint32_t spm_allow_rc_vcore(int state_id);
int spm_hwcg_name(uint32_t idex, char *name, size_t sz);

#ifdef MTK_PLAT_CIRQ_UNSUPPORT
#define do_irqs_delivery(_x, _w)
#else
void do_irqs_delivery(struct mt_irqremain *irqs, struct wake_status *wakeup);
#endif

#endif
