/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#include <mtk_mmap_pool.h>
#include "pmic_wrap_init.h"
#include "pmic_wrap_v3.h"

static spinlock_t wrp_lock;

static uint32_t pwrap_check_idle(uintptr_t wacs_register, uintptr_t wacs_vldclr_register,
				 uint32_t timeout_us)
{
	uint32_t reg_rdata = 0U, retry;

	retry = (timeout_us + PWRAP_POLL_STEP_US) / PWRAP_POLL_STEP_US;
	while (retry != 0) {
		udelay(PWRAP_POLL_STEP_US);
		reg_rdata = mmio_read_32(wacs_register);
		/* if last read command timeout,clear vldclr bit
		 * read command state machine:FSM_REQ-->wfdle-->WFVLDCLR;
		 * write:FSM_REQ-->idle
		 */
		switch (GET_WACS_FSM(reg_rdata)) {
		case SWINF_FSM_WFVLDCLR:
			mmio_write_32(wacs_vldclr_register, 0x1);
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
	}

done:
	if (retry == 0) {
		/* timeout */
		return E_PWR_WAIT_IDLE_TIMEOUT;
	}

	return 0U;
}

static uint32_t pwrap_check_vldclr(uintptr_t wacs_register, uint32_t timeout_us)
{
	uint32_t reg_rdata = 0U, retry;

	retry = (timeout_us + PWRAP_POLL_STEP_US) / PWRAP_POLL_STEP_US;
	while (retry != 0) {
		udelay(PWRAP_POLL_STEP_US);
		reg_rdata = mmio_read_32(wacs_register);
		if (GET_WACS_FSM(reg_rdata) == SWINF_FSM_WFVLDCLR) {
			break;
		}
		retry--;
	}

	if (retry == 0) {
		/* timeout */
		return E_PWR_WAIT_IDLE_TIMEOUT;
	}

	return 0U;
}

static int32_t pwrap_swinf_acc(uint32_t swinf_no, uint32_t cmd, uint32_t write,
			       uint32_t pmifid, uint32_t slvid, uint32_t addr,
			       uint32_t bytecnt, uint32_t wdata, uint32_t *rdata)
{
	uint32_t reg_rdata = 0x0;
	int32_t ret = 0x0;

	/* Check argument validation */
	if ((swinf_no & ~(0x3)) != 0)
		return -E_PWR_INVALID_SWINF;
	if ((cmd & ~(0x3)) != 0)
		return -E_PWR_INVALID_CMD;
	if ((write & ~(0x1)) != 0)
		return -E_PWR_INVALID_RW;
	if ((pmifid & ~(0x1)) != 0)
		return -E_PWR_INVALID_PMIFID;
	if ((slvid & ~(0xf)) != 0)
		return -E_PWR_INVALID_SLVID;
	if ((addr & ~(0xffff)) != 0)
		return -E_PWR_INVALID_ADDR;
	if ((bytecnt & ~(0x1)) != 0)
		return -E_PWR_INVALID_BYTECNT;
	if ((wdata & ~(0xffff)) != 0)
		return -E_PWR_INVALID_WDAT;

	spin_lock(&wrp_lock);
	/* Check whether INIT_DONE is set */
	if (pmifid == 0)
		reg_rdata = mmio_read_32((uintptr_t)(&mtk_pwrap->wacs[swinf_no].sta));

	if (GET_SWINF_INIT_DONE(reg_rdata) != 0x1) {
		ERROR("[PWRAP] init not finish\n");
		ret = -E_PWR_NOT_INIT_DONE;
		goto end;
	}

	/* Wait for Software Interface FSM state to be IDLE */
	ret = pwrap_check_idle((uintptr_t)(&mtk_pwrap->wacs[swinf_no].sta),
			(uintptr_t)(&mtk_pwrap->wacs[swinf_no].vldclr), TIMEOUT_WAIT_IDLE);
	if (ret != 0) {
		ERROR("[PWRAP] fsm_idle fail\n");
		goto end;
	}

	/* Set the write data */
	if (write == 1) {
		if (pmifid == 0)
			mmio_write_32((uintptr_t)(&mtk_pwrap->wacs[swinf_no].wdata), wdata);
	}

	/* Send the command */
	if (pmifid == 0)
		mmio_write_32((uintptr_t)(&mtk_pwrap->wacs[swinf_no].cmd),
			  (cmd << 30) | (write << 29) | (slvid << 24) | (bytecnt << 16) | addr);

	if (write == 0) {
		if (rdata == NULL) {
			ERROR("[PWRAP] rdata null\n");
			ret = -E_PWR_INVALID_ARG;
			goto end;
		}

		/* Wait for Software Interface FSM to be WFVLDCLR */
		/* read the data and clear the valid flag         */
		ret = pwrap_check_vldclr((uintptr_t)(&mtk_pwrap->wacs[swinf_no].sta), TIMEOUT_READ);
		if (ret != 0) {
			ERROR("[PWRAP] fsm_vldclr fail\n");
			goto end;
		}

		if (pmifid == 0) {
			*rdata = mmio_read_32((uintptr_t)(&mtk_pwrap->wacs[swinf_no].rdata));
			mmio_write_32((uintptr_t)(&mtk_pwrap->wacs[swinf_no].vldclr), 0x1);
		}
	}

end:
	spin_unlock(&wrp_lock);
	if (ret < 0)
		ERROR("%s fail, ret=%d\n", __func__, ret);
	return ret;
}

/* external API for pmic_wrap user */

int32_t pwrap_read(uint32_t adr, uint32_t *rdata)
{
	return pwrap_swinf_acc(PMIF_SPI_SWINF_NO, DEFAULT_CMD, 0, PMIF_SPI_PMIFID,
		DEFAULT_SLVID, adr, DEFAULT_BYTECNT, 0x0, rdata);
}

int32_t pwrap_write(uint32_t adr, uint32_t wdata)
{
	return pwrap_swinf_acc(PMIF_SPI_SWINF_NO, DEFAULT_CMD, 1, PMIF_SPI_PMIFID,
		DEFAULT_SLVID, adr, DEFAULT_BYTECNT, wdata, 0x0);
}

int32_t pwrap_read_field(uint32_t reg, uint32_t *val, uint32_t mask, uint32_t shift)
{
	uint32_t rdata;
	int32_t ret;

	if (!val)
		return -E_PWR_INVALID_ARG;
	ret = pwrap_read(reg, &rdata);
	if (ret == 0x0)
		*val = (rdata >> shift) & mask;

	return ret;
}

int32_t pwrap_write_field(uint32_t reg, uint32_t val, uint32_t mask, uint32_t shift)
{
	uint32_t data;
	int32_t ret;

	ret = pwrap_read(reg, &data);
	if (ret != 0)
		return ret;

	data = data & ~(mask << shift);
	data |= (val << shift);
	ret = pwrap_write(reg, data);

	return ret;
}

#if PWRAP_DEBUG
static int32_t pwrap_read_test(void)
{
	uint32_t rdata = 0;
	int32_t ret;

	ret = pwrap_read(DEW_READ_TEST, &rdata);
	if (rdata != DEFAULT_VALUE_READ_TEST) {
		ERROR("[PWRAP] Read fail,rdata=0x%x,exp=0x5aa5,ret=0x%x\n", rdata, ret);
		return -E_PWR_READ_TEST_FAIL;
	}
	INFO("[PWRAP] Read Test pass,ret=%x\n", ret);
	return 0;
}

static int32_t pwrap_write_test(void)
{
	uint32_t rdata = 0;
	int32_t ret;
	int32_t ret1;

	ret = pwrap_write(DEW_WRITE_TEST, PWRAP_WRITE_TEST_VALUE);
	ret1 = pwrap_read(DEW_WRITE_TEST, &rdata);
	if ((rdata != PWRAP_WRITE_TEST_VALUE) || (ret != 0) || (ret1 != 0)) {
		ERROR("[PWRAP] Write fail,rdata=0x%x,exp=0xa55a,ret=0x%x,ret1=0x%x\n",
			rdata, ret, ret1);
		return -E_PWR_WRITE_TEST_FAIL;
	}
	INFO("[PWRAP] Write Test pass\n");
	return 0;
}

int32_t pmic_wrap_test(void)
{
	int32_t ret;

	INFO("[PWRAP] Read/Write Test start\n");

	ret = pwrap_read_test();
	if (ret != 0) {
		return ret;
	}

	ret = pwrap_write_test();
	if (ret != 0) {
		return ret;
	}
	INFO("[PWRAP] Read/Write Test done\n");

	return 0;
}
#endif
