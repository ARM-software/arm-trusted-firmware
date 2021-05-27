/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "platform_def.h"
#include "pmic_wrap_init.h"

/* pmic wrap module wait_idle and read polling interval (in microseconds) */
enum pwrap_polling_interval {
	WAIT_IDLE_POLLING_DELAY_US	= 1,
	READ_POLLING_DELAY_US		= 2
};

static uint32_t pwrap_check_idle(void *wacs_register, uint32_t timeout_us)
{
	uint32_t reg_rdata = 0U, retry;

	retry = (timeout_us + WAIT_IDLE_POLLING_DELAY_US) /
		WAIT_IDLE_POLLING_DELAY_US;
	while (retry != 0) {
		udelay(WAIT_IDLE_POLLING_DELAY_US);
		reg_rdata = mmio_read_32((uintptr_t)wacs_register);
		/* if last read command timeout,clear vldclr bit
		 * read command state machine:FSM_REQ-->wfdle-->WFVLDCLR;
		 * write:FSM_REQ-->idle
		 */
		switch (GET_WACS_FSM(reg_rdata)) {
		case SWINF_FSM_WFVLDCLR:
			mmio_write_32((uintptr_t)&mtk_pwrap->wacs2_vldclr, 0x1);
			INFO("WACS_FSM = SWINF_FSM_WFVLDCLR\n");
			break;
		case SWINF_FSM_WFDLE:
			INFO("WACS_FSM = SWINF_FSM_WFDLE\n");
			break;
		case SWINF_FSM_REQ:
			INFO("WACS_FSM = SWINF_FSM_REQ\n");
			break;
		case SWINF_FSM_IDLE:
			goto done;
		default:
			break;
		}
		retry--;
	};

done:
	if (retry == 0) {
		/* timeout */
		return E_PWR_WAIT_IDLE_TIMEOUT;
	}

	return 0U;
}

static uint32_t pwrap_check_vldclr(void *wacs_register, uint32_t timeout_us)
{
	uint32_t reg_rdata = 0U, retry;

	retry = (timeout_us + READ_POLLING_DELAY_US) / READ_POLLING_DELAY_US;
	while (retry != 0) {
		udelay(READ_POLLING_DELAY_US);
		reg_rdata = mmio_read_32((uintptr_t)wacs_register);
		if (GET_WACS_FSM(reg_rdata) == SWINF_FSM_WFVLDCLR) {
			break;
		}
		retry--;
	};

	if (retry == 0) {
		/* timeout */
		return E_PWR_WAIT_IDLE_TIMEOUT;
	}

	return 0U;
}

static int32_t pwrap_wacs2(uint32_t write, uint32_t adr, uint32_t wdata,
			   uint32_t *rdata, uint32_t init_check)
{
	uint32_t reg_rdata, return_value;

	if (init_check != 0) {
		if ((mmio_read_32((uintptr_t)&mtk_pwrap->init_done) & 0x1) == 0) {
			ERROR("initialization isn't finished\n");
			return E_PWR_NOT_INIT_DONE;
		}
	}

	/* Wait for Software Interface FSM state to be IDLE. */
	return_value = pwrap_check_idle(&mtk_pwrap->wacs2_sta,
					PWRAP_WAIT_IDLE_US);
	if (return_value != 0) {
		return return_value;
	}

	/* Set the write data */
	if (write == 1) {
		/* Set the write data. */
		mmio_write_32((uintptr_t)&mtk_pwrap->wacs2_wdata, wdata);
	}

	/* Send the command. */
	mmio_write_32((uintptr_t)&mtk_pwrap->wacs2_cmd, (write << 29) | adr);

	if (write == 0) {
		/*
		 * Wait for Software Interface FSM state to be WFVLDCLR,
		 * read the data and clear the valid flag.
		 */
		return_value = pwrap_check_vldclr(&mtk_pwrap->wacs2_sta,
						  PWRAP_READ_US);
		if (return_value != 0) {
			return return_value;
		}

		if (rdata == NULL) {
			return E_PWR_INVALID_ARG;
		}

		reg_rdata = mmio_read_32((uintptr_t)&mtk_pwrap->wacs2_rdata);
		*rdata = reg_rdata;
		mmio_write_32((uintptr_t)&mtk_pwrap->wacs2_vldclr, 0x1);
	}

	return return_value;
}

/* external API for pmic_wrap user */
int32_t pwrap_read(uint32_t adr, uint32_t *rdata)
{
	return pwrap_wacs2(0, adr, 0, rdata, 1);
}

int32_t pwrap_write(uint32_t adr, uint32_t wdata)
{
	return pwrap_wacs2(1, adr, wdata, 0, 1);
}
