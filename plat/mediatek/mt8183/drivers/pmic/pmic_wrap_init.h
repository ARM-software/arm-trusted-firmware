/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_WRAP_INIT_H
#define PMIC_WRAP_INIT_H

#include <platform_def.h>
#include <stdint.h>

/* external API */
int32_t pwrap_read(uint32_t adr, uint32_t *rdata);
int32_t pwrap_write(uint32_t adr, uint32_t wdata);

static struct mt8183_pmic_wrap_regs *const mtk_pwrap =
	(void *)PMIC_WRAP_BASE;

/* timeout setting */
enum {
	TIMEOUT_READ        = 255,	/* us */
	TIMEOUT_WAIT_IDLE   = 255	/* us */
};

/* PMIC_WRAP registers */
struct mt8183_pmic_wrap_regs {
	uint32_t reserved[776];
	uint32_t wacs2_cmd;
	uint32_t wacs2_rdata;
	uint32_t wacs2_vldclr;
	uint32_t reserved1[4];
};

enum {
	RDATA_WACS_RDATA_SHIFT = 0,
	RDATA_WACS_FSM_SHIFT = 16,
	RDATA_WACS_REQ_SHIFT = 19,
	RDATA_SYNC_IDLE_SHIFT,
	RDATA_INIT_DONE_SHIFT,
	RDATA_SYS_IDLE_SHIFT,
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

/* error information flag */
enum {
	E_PWR_INVALID_ARG             = 1,
	E_PWR_INVALID_RW              = 2,
	E_PWR_INVALID_ADDR            = 3,
	E_PWR_INVALID_WDAT            = 4,
	E_PWR_INVALID_OP_MANUAL       = 5,
	E_PWR_NOT_IDLE_STATE          = 6,
	E_PWR_NOT_INIT_DONE           = 7,
	E_PWR_NOT_INIT_DONE_READ      = 8,
	E_PWR_WAIT_IDLE_TIMEOUT       = 9,
	E_PWR_WAIT_IDLE_TIMEOUT_READ  = 10,
	E_PWR_INIT_SIDLY_FAIL         = 11,
	E_PWR_RESET_TIMEOUT           = 12,
	E_PWR_TIMEOUT                 = 13,
	E_PWR_INIT_RESET_SPI          = 20,
	E_PWR_INIT_SIDLY              = 21,
	E_PWR_INIT_REG_CLOCK          = 22,
	E_PWR_INIT_ENABLE_PMIC        = 23,
	E_PWR_INIT_DIO                = 24,
	E_PWR_INIT_CIPHER             = 25,
	E_PWR_INIT_WRITE_TEST         = 26,
	E_PWR_INIT_ENABLE_CRC         = 27,
	E_PWR_INIT_ENABLE_DEWRAP      = 28,
	E_PWR_INIT_ENABLE_EVENT       = 29,
	E_PWR_READ_TEST_FAIL          = 30,
	E_PWR_WRITE_TEST_FAIL         = 31,
	E_PWR_SWITCH_DIO              = 32
};

#endif /* PMIC_WRAP_INIT_H */
