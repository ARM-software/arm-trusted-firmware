/*
 * Copyright (c) 2021-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_WRAP_INIT_H
#define PMIC_WRAP_INIT_H

#include <stdint.h>

#include "platform_def.h"
#include <pmic_wrap_init_common.h>

static struct mt8186_pmic_wrap_regs *const mtk_pwrap = (void *)PMIC_WRAP_BASE;

/* timeout setting */
enum {
	TIMEOUT_RESET       = 50,	/* us */
	TIMEOUT_READ        = 50,	/* us */
	TIMEOUT_WAIT_IDLE   = 50	/* us */
};

/* PMIC_WRAP registers */
struct mt8186_pmic_wrap_regs {
	uint32_t unused[776];
	uint32_t wacs2_cmd;
	uint32_t wacs2_rdata;
	uint32_t wacs2_vldclr;
};

enum {
	RDATA_WACS_RDATA_SHIFT = 0,
	RDATA_WACS_FSM_SHIFT = 16,
	RDATA_WACS_REQ_SHIFT = 19,
	RDATA_SYNC_IDLE_SHIFT = 20,
	RDATA_INIT_DONE_SHIFT = 22,
	RDATA_SYS_IDLE_SHIFT = 23,
};

enum {
	RDATA_WACS_RDATA_MASK = 0xffff,
	RDATA_WACS_FSM_MASK = 0x7,
	RDATA_WACS_REQ_MASK = 0x1,
	RDATA_SYNC_IDLE_MASK = 0x1,
	RDATA_INIT_DONE_MASK = 0x1,
	RDATA_SYS_IDLE_MASK = 0x1,
};

/* WACS_FSM */
enum {
	WACS_FSM_IDLE            = 0x00,
	WACS_FSM_REQ             = 0x02,
	WACS_FSM_WFDLE           = 0x04,
	WACS_FSM_WFVLDCLR        = 0x06,
	WACS_INIT_DONE           = 0x01,
	WACS_SYNC_IDLE           = 0x01,
	WACS_SYNC_BUSY           = 0x00
};

#endif /* PMIC_WRAP_INIT_H */
