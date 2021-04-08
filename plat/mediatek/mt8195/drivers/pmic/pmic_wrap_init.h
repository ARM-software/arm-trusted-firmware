/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_WRAP_INIT_H
#define PMIC_WRAP_INIT_H

#include <stdint.h>

#include "platform_def.h"

/* external API */
int32_t pwrap_read(uint32_t adr, uint32_t *rdata);
int32_t pwrap_write(uint32_t adr, uint32_t wdata);

static struct mt8195_pmic_wrap_regs *const mtk_pwrap = (void *)PMIC_WRAP_BASE;

/* PMIC_WRAP registers */
struct mt8195_pmic_wrap_regs {
	uint32_t init_done;
	uint32_t reserved[543];
	uint32_t wacs2_cmd;
	uint32_t wacs2_wdata;
	uint32_t reserved1[3];
	uint32_t wacs2_rdata;
	uint32_t reserved2[3];
	uint32_t wacs2_vldclr;
	uint32_t wacs2_sta;
};

#define GET_WACS_FSM(x)	((x >> 1) & 0x7)

/* macro for SWINF_FSM */
#define SWINF_FSM_IDLE		(0x00)
#define SWINF_FSM_REQ		(0x02)
#define SWINF_FSM_WFDLE		(0x04)
#define SWINF_FSM_WFVLDCLR	(0x06)
#define SWINF_INIT_DONE		(0x01)

/* timeout setting */
#define PWRAP_READ_US	1000
#define PWRAP_WAIT_IDLE_US	1000

/* error information flag */
enum pwrap_errno {
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
