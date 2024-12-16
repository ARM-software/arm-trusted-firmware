/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_CONSTRAINT_H
#define MT_SPM_CONSTRAINT_H

#include <lpm_v2/mt_lp_rm.h>

#define MT_RM_CONSTRAINT_ALLOW_CPU_BUCK_OFF	BIT(0)
#define MT_RM_CONSTRAINT_ALLOW_DRAM_S0		BIT(1)
#define MT_RM_CONSTRAINT_ALLOW_DRAM_S1		BIT(2)
#define MT_RM_CONSTRAINT_ALLOW_VCORE_LP		BIT(3)
#define MT_RM_CONSTRAINT_ALLOW_INFRA_PDN	BIT(4)
#define MT_RM_CONSTRAINT_ALLOW_BUS26M_OFF	BIT(5)
#define MT_RM_CONSTRAINT_ALLOW_AP_SUSPEND	BIT(6) /* System suspend */
#define MT_RM_CONSTRAINT_ALLOW_BBLPM		BIT(7)
#define MT_RM_CONSTRAINT_ALLOW_XO_UFS		BIT(8)
#define MT_RM_CONSTRAINT_ALLOW_GPS_STATE	BIT(9)
#define MT_RM_CONSTRAINT_ALLOW_LVTS_STATE	BIT(10)
#define MT_RM_CONSTRAINT_ALLOW_AP_PLAT_SUSPEND	BIT(11) /* Kernel suspend */
#define MT_RM_CONSTRAINT_ALLOW_VCORE_OFF	BIT(12)

#define MT_SPM_RC_INVALID		0x0
#define MT_SPM_RC_VALID_SW		BIT(0)
#define MT_SPM_RC_VALID_FW		BIT(1)
#define MT_SPM_RC_VALID_RESIDNECY	BIT(2)
#define MT_SPM_RC_VALID_COND_CHECK	BIT(3)
#define MT_SPM_RC_VALID_COND_LATCH	BIT(4)
#define MT_SPM_RC_VALID_UFS_H8		BIT(5)
#define MT_SPM_RC_VALID_FLIGHTMODE	BIT(6)
#define MT_SPM_RC_VALID_XSOC_BBLPM	BIT(7)
#define MT_SPM_RC_VALID_TRACE_EVENT	BIT(8)
#define MT_SPM_RC_VALID_TRACE_TIME	BIT(9)
#define MT_SPM_RC_VALID_NOTIFY		BIT(10)

#define MT_SPM_RC_VALID		(MT_SPM_RC_VALID_SW | MT_SPM_RC_VALID_FW)

#define IS_MT_RM_RC_READY(status) \
	((status & MT_SPM_RC_VALID) == MT_SPM_RC_VALID)

struct constraint_status {
	uint16_t id;
	uint16_t is_valid;
	uint64_t is_cond_block;
	uint32_t enter_cnt;
	uint64_t all_pll_dump;
	unsigned long long residency;
	struct mt_spm_cond_tables *cond_res;
};

enum constraint_status_update_type {
	CONSTRAINT_UPDATE_VALID,
	CONSTRAINT_UPDATE_COND_CHECK,
	CONSTRAINT_RESIDNECY,
};

enum constraint_status_get_type {
	CONSTRAINT_GET_VALID = 0xD0000000,
	CONSTRAINT_GET_ENTER_CNT,
	CONSTRAINT_GET_RESIDENCY,
	CONSTRAINT_GET_COND_EN,
	CONSTRAINT_COND_BLOCK,
	CONSTRAINT_GET_COND_BLOCK_LATCH,
	CONSTRAINT_GET_COND_BLOCK_DETAIL,
	CONSTRAINT_GET_RESIDNECY,
};

struct rc_common_state {
	unsigned int id;
	unsigned int act;
	unsigned int type;
	void *value;
};

#define MT_SPM_RC_BBLPM_MODE	(MT_SPM_RC_VALID_UFS_H8 | \
				 MT_SPM_RC_VALID_FLIGHTMODE | \
				 MT_SPM_RC_VALID_XSOC_BBLPM)

#define IS_MT_SPM_RC_BBLPM_MODE(st) \
	((st & (MT_SPM_RC_BBLPM_MODE)) == MT_SPM_RC_BBLPM_MODE)

#define IS_MT_SPM_RC_NOTIFY_ENABLE(st) \
	((st & (MT_SPM_RC_VALID_NOTIFY)))

#define MT_SPM_RC_EXTERN_STATUS_SET(v, st)	({v |= (st & 0xffff); })
#define MT_SPM_RC_EXTERN_STATUS_CLR(v, st)	({v &= ~(st & 0xffff); })

#endif /* MT_SPM_CONSTRAINT_H */
