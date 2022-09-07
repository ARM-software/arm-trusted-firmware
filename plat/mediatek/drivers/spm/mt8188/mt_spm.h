/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_H
#define MT_SPM_H

#include <stdint.h>
#include <stdio.h>
#include <lib/spinlock.h>
#include <lib/pm/mtk_pm.h>
#include <lpm/mt_lp_rq.h>

/*
 * ARM v8.2, the cache will turn off automatically when cpu
 * power down. Therefore, there is no doubt to use the spin_lock here.
 */
extern spinlock_t spm_lock;

#ifdef __GNUC__
#define spm_likely(x)	__builtin_expect(!!(x), 1)
#define spm_unlikely(x)	__builtin_expect(!!(x), 0)
#else
#define spm_likely(x)	(x)
#define spm_unlikely(x)	(x)
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
#define MT_SPM_EX_OP_PERI_ON			BIT(12)
#define MT_SPM_EX_OP_INFRA_ON			BIT(13)

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

struct mt_lp_resource_user *get_spm_res_user(void);
int spm_boot_init(void);

#endif /* MT_SPM_H */
