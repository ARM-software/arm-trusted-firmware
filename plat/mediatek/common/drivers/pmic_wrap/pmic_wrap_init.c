/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <pmic_wrap_init.h>

/* pmic wrap module wait_idle and read polling interval (in microseconds) */
enum {
	WAIT_IDLE_POLLING_DELAY_US	= 1,
	READ_POLLING_DELAY_US		= 2
};

static inline uint32_t wait_for_state_idle(uint32_t timeout_us,
					   void *wacs_register,
					   void *wacs_vldclr_register,
					   uint32_t *read_reg)
{
	uint32_t reg_rdata;
	uint32_t retry;

	retry = (timeout_us + WAIT_IDLE_POLLING_DELAY_US) /
		WAIT_IDLE_POLLING_DELAY_US;

	do {
		udelay(WAIT_IDLE_POLLING_DELAY_US);
		reg_rdata = mmio_read_32((uintptr_t)wacs_register);
		/* if last read command timeout,clear vldclr bit
		 * read command state machine:FSM_REQ-->wfdle-->WFVLDCLR;
		 * write:FSM_REQ-->idle
		 */
		switch (((reg_rdata >> RDATA_WACS_FSM_SHIFT) &
			RDATA_WACS_FSM_MASK)) {
		case WACS_FSM_WFVLDCLR:
			mmio_write_32((uintptr_t)wacs_vldclr_register, 1);
			ERROR("WACS_FSM = PMIC_WRAP_WACS_VLDCLR\n");
			break;
		case WACS_FSM_WFDLE:
			ERROR("WACS_FSM = WACS_FSM_WFDLE\n");
			break;
		case WACS_FSM_REQ:
			ERROR("WACS_FSM = WACS_FSM_REQ\n");
			break;
		case WACS_FSM_IDLE:
			goto done;
		default:
			break;
		}

		retry--;
	} while (retry);

done:
	if (!retry)	/* timeout */
		return E_PWR_WAIT_IDLE_TIMEOUT;

	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

static inline uint32_t wait_for_state_ready(uint32_t timeout_us,
					    void *wacs_register,
					    uint32_t *read_reg)
{
	uint32_t reg_rdata;
	uint32_t retry;

	retry = (timeout_us + READ_POLLING_DELAY_US) / READ_POLLING_DELAY_US;

	do {
		udelay(READ_POLLING_DELAY_US);
		reg_rdata = mmio_read_32((uintptr_t)wacs_register);

		if (((reg_rdata >> RDATA_WACS_FSM_SHIFT) & RDATA_WACS_FSM_MASK)
		    == WACS_FSM_WFVLDCLR)
			break;

		retry--;
	} while (retry);

	if (!retry) {	/* timeout */
		ERROR("timeout when waiting for idle\n");
		return E_PWR_WAIT_IDLE_TIMEOUT_READ;
	}

	if (read_reg)
		*read_reg = reg_rdata;
	return 0;
}

static int32_t pwrap_wacs2(uint32_t write,
		    uint32_t adr,
		    uint32_t wdata,
		    uint32_t *rdata,
		    uint32_t init_check)
{
	uint32_t reg_rdata = 0;
	uint32_t wacs_write = 0;
	uint32_t wacs_adr = 0;
	uint32_t wacs_cmd = 0;
	uint32_t return_value = 0;

	if (init_check) {
		reg_rdata = mmio_read_32((uintptr_t)&mtk_pwrap->wacs2_rdata);
		/* Prevent someone to used pwrap before pwrap init */
		if (((reg_rdata >> RDATA_INIT_DONE_SHIFT) &
		    RDATA_INIT_DONE_MASK) != WACS_INIT_DONE) {
			ERROR("initialization isn't finished\n");
			return E_PWR_NOT_INIT_DONE;
		}
	}
	reg_rdata = 0;
	/* Check IDLE in advance */
	return_value = wait_for_state_idle(TIMEOUT_WAIT_IDLE,
				&mtk_pwrap->wacs2_rdata,
				&mtk_pwrap->wacs2_vldclr,
				0);
	if (return_value != 0) {
		ERROR("wait_for_fsm_idle fail,return_value=%d\n", return_value);
		goto FAIL;
	}
	wacs_write = write << 31;
	wacs_adr = (adr >> 1) << 16;
	wacs_cmd = wacs_write | wacs_adr | wdata;

	mmio_write_32((uintptr_t)&mtk_pwrap->wacs2_cmd, wacs_cmd);
	if (write == 0) {
		if (rdata == NULL) {
			ERROR("rdata is a NULL pointer\n");
			return_value = E_PWR_INVALID_ARG;
			goto FAIL;
		}
		return_value = wait_for_state_ready(TIMEOUT_READ,
					&mtk_pwrap->wacs2_rdata,
					&reg_rdata);
		if (return_value != 0) {
			ERROR("wait_for_fsm_vldclr fail,return_value=%d\n",
				 return_value);
			goto FAIL;
		}
		*rdata = ((reg_rdata >> RDATA_WACS_RDATA_SHIFT)
			  & RDATA_WACS_RDATA_MASK);
		mmio_write_32((uintptr_t)&mtk_pwrap->wacs2_vldclr, 1);
	}
FAIL:
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
