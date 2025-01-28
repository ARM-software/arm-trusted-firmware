/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMI_SW_H
#define SPMI_SW_H

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <mt_timer.h>

enum spmi_regs {
	SPMI_OP_ST_CTRL,
	SPMI_GRP_ID_EN,
	SPMI_OP_ST_STA,
	SPMI_MST_SAMPL,
	SPMI_MST_REQ_EN,
	/* RCS support */
	SPMI_RCS_CTRL,
	SPMI_SLV_3_0_EINT,
	SPMI_SLV_7_4_EINT,
	SPMI_SLV_B_8_EINT,
	SPMI_SLV_F_C_EINT,
	SPMI_REC_CTRL,
	SPMI_REC0,
	SPMI_REC1,
	SPMI_REC2,
	SPMI_REC3,
	SPMI_REC4,
	SPMI_REC_CMD_DEC,
	SPMI_DEC_DBG,
	SPMI_MST_DBG
};

/* DEBUG MARCO */
#define SPMITAG			"[SPMI] "
#define SPMI_ERR(fmt, arg...)	ERROR(SPMITAG fmt, ##arg)
#define SPMI_ERRL(fmt, arg...)	ERROR(fmt, ##arg)
#define SPMI_INFO(fmt, arg...)	INFO(SPMITAG fmt, ##arg)

#define wait_us(cond, timeout)			\
({						\
	uint64_t __now, __end, __ret;		\
						\
	__end = sched_clock() + timeout;	\
	for (;;) {				\
		if (cond) {			\
			__ret = timeout;	\
			break;			\
		}				\
		__now = sched_clock();		\
		if (__end <= __now) {		\
			__ret = 0;		\
			break;			\
		}				\
	}					\
	__ret;					\
})

enum {
	SPMI_RESET = 0,
	SPMI_SLEEP,
	SPMI_SHUTDOWN,
	SPMI_WAKEUP
};

#endif
