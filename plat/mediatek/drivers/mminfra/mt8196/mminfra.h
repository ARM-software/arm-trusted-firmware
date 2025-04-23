/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MMINFRA_H
#define MMINFRA_H

#include <lib/spinlock.h>

#include "../mminfra_common.h"
#include <platform_def.h>

#define VLP_AO_RSVD6			(MTK_VLP_TRACER_MON_BASE + 0x918)
#define MMINFRA_DONE			(1U << 0)

#define SPM_SEMA_MMINFRA                (1U << 5)
#define SPM_SEMA_MMINFRA_NR             (8)

#define SEMA_RETRY_CNT			(500)

#define SPM_SEMAPHORE_M0		(0x69C)
#define SPM_SEMAPHORE_M1		(0x6A0)
#define SPM_SEMAPHORE_M2		(0x6A4)
#define SPM_SEMAPHORE_M3		(0x6A8)
#define SPM_SEMAPHORE_M4		(0x6AC)
#define SPM_SEMAPHORE_M5		(0x6B0)
#define SPM_SEMAPHORE_M6		(0x6B4)
#define SPM_SEMAPHORE_M7		(0x6B8)

#define MMINFRA_HW_VOTER_BASE		(0x31A80000)
#define MTK_POLL_HWV_VOTE_US		(2)
#define MTK_POLL_HWV_VOTE_CNT		(2500)
#define MTK_POLL_DONE_DELAY_US		(1)
#define MTK_POLL_DONE_RETRY		(3000)

struct mminfra_hw_sema {
	uint32_t base;
	uint32_t offset;
	uint32_t offset_all[SPM_SEMA_MMINFRA_NR];
	uint32_t set_val;
};

struct mminfra_hw_voter {
	uint32_t base;
	uint32_t set_ofs;
	uint32_t clr_ofs;
	uint32_t en_ofs;
	uint32_t en_shift;
	uint32_t done_bits;
};

struct mtk_mminfra_pwr_ctrl {
	spinlock_t lock;
	struct mminfra_hw_voter hw_voter;
	struct mminfra_hw_sema hw_sema;
	uint32_t ref_cnt;
	bool active;
};

#endif
