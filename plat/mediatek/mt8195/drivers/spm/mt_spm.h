/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_H
#define MT_SPM_H

#include <lib/bakery_lock.h>
#include <lib/spinlock.h>

#include <plat_mtk_lpm.h>

/*
 * ARM v8.2, the cache will turn off automatically when cpu
 * power down. So, there is no doubt to use the spin_lock here
 */
#if !HW_ASSISTED_COHERENCY
#define MT_SPM_USING_BAKERY_LOCK
#endif

#ifdef MT_SPM_USING_BAKERY_LOCK
DECLARE_BAKERY_LOCK(spm_lock);
#define plat_spm_lock() bakery_lock_get(&spm_lock)
#define plat_spm_unlock() bakery_lock_release(&spm_lock)
#else
extern spinlock_t spm_lock;
#define plat_spm_lock() spin_lock(&spm_lock)
#define plat_spm_unlock() spin_unlock(&spm_lock)
#endif

#define MT_SPM_USING_SRCLKEN_RC

/* spm extern operand definition */
#define MT_SPM_EX_OP_CLR_26M_RECORD			(1U << 0)
#define MT_SPM_EX_OP_SET_WDT				(1U << 1)
#define MT_SPM_EX_OP_NON_GENERIC_RESOURCE_REQ		(1U << 2)
#define MT_SPM_EX_OP_SET_SUSPEND_MODE			(1U << 3)
#define MT_SPM_EX_OP_SET_IS_ADSP			(1U << 4)
#define MT_SPM_EX_OP_SRCLKEN_RC_BBLPM			(1U << 5)
#define MT_SPM_EX_OP_HW_S1_DETECT			(1U << 6)

typedef enum {
	WR_NONE = 0,
	WR_UART_BUSY = 1,
	WR_ABORT = 2,
	WR_PCM_TIMER = 3,
	WR_WAKE_SRC = 4,
	WR_DVFSRC = 5,
	WR_TWAM = 6,
	WR_PMSR = 7,
	WR_SPM_ACK_CHK = 8,
	WR_UNKNOWN = 9,
} wake_reason_t;

static inline void spm_lock_get(void)
{
	plat_spm_lock();
}

static inline void spm_lock_release(void)
{
	plat_spm_unlock();
}

extern void spm_boot_init(void);
#endif /* MT_SPM_H */
