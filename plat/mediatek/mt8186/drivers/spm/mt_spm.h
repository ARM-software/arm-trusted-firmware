/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
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
 * power down. Therefore, there is no doubt to use the spin_lock here.
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
#define MT_SPM_EX_OP_CLR_26M_RECORD		BIT(0)
#define MT_SPM_EX_OP_SET_WDT			BIT(1)
#define MT_SPM_EX_OP_NON_GENERIC_RESOURCE_REQ	BIT(2)
#define MT_SPM_EX_OP_SET_SUSPEND_MODE		BIT(3)
#define MT_SPM_EX_OP_SET_IS_ADSP		BIT(4)
#define MT_SPM_EX_OP_SRCLKEN_RC_BBLPM		BIT(5)
#define MT_SPM_EX_OP_HW_S1_DETECT		BIT(6)
#define MT_SPM_EX_OP_TRACE_LP			BIT(7)
#define MT_SPM_EX_OP_TRACE_SUSPEND		BIT(8)
#define MT_SPM_EX_OP_TRACE_TIMESTAMP_EN		BIT(9)
#define MT_SPM_EX_OP_TIME_CHECK			BIT(10)
#define MT_SPM_EX_OP_TIME_OBS			BIT(11)

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

/* for suspend vol. bin settings */
enum MT_PLAT_SUSPEND_VCORE {
	SPM_SUSPEND_VCORE_5500 = 0,
	SPM_SUSPEND_VCORE_5250 = 1,
	SPM_SUSPEND_VCORE_5000 = 2,
};

extern void spm_boot_init(void);

static inline void spm_lock_get(void)
{
	plat_spm_lock();
}

static inline void spm_lock_release(void)
{
	plat_spm_unlock();
}

unsigned int spm_get_suspend_vcore_voltage_idx(void);

#endif /* MT_SPM_H */
