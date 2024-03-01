/*
 * Copyright 2020-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include "upower_api.h"
#include "upower_defs.h"

#define UPOWER_AP_MU1_ADDR	U(0x29280000)

struct MU_t *muptr = (struct MU_t *)UPOWER_AP_MU1_ADDR;

void upower_apd_inst_isr(upwr_isr_callb txrx_isr,
			 upwr_isr_callb excp_isr)
{
	/* Do nothing */
}

int upower_status(int status)
{
	int ret = -1;

	switch (status) {
	case 0:
		VERBOSE("finished successfully!\n");
		ret = 0;
		break;
	case -1:
		VERBOSE("memory allocation or resource failed!\n");
		break;
	case -2:
		VERBOSE("invalid argument!\n");
		break;
	case -3:
		VERBOSE("called in an invalid API state!\n");
		break;
	default:
		VERBOSE("invalid return status\n");
		break;
	}

	return ret;
}


void upower_wait_resp(void)
{
	while (muptr->RSR.B.RF0 == 0) {
		udelay(100);
	}
	upwr_txrx_isr();
}

static void user_upwr_rdy_callb(uint32_t soc, uint32_t vmajor, uint32_t vminor)
{
	NOTICE("%s: soc=%x\n", __func__, soc);
	NOTICE("%s: RAM version:%d.%d\n", __func__, vmajor, vminor);
}

int upower_init(void)
{
	int status;

	status = upwr_init(APD_DOMAIN, muptr, NULL, NULL, upower_apd_inst_isr, NULL);
	if (upower_status(status)) {
		ERROR("%s: upower init failure\n", __func__);
		return -EINVAL;
	}

	NOTICE("%s: start uPower RAM service\n", __func__);
	status = upwr_start(1, user_upwr_rdy_callb);
	upower_wait_resp();
	/* poll status */
	if (upower_status(status)) {
		NOTICE("%s: upower init failure\n", __func__);
		return status;
	}

	return 0;
}

int upower_pwm(int domain_id, bool pwr_on)
{
	int ret, ret_val;
	uint32_t swt;

	if (domain_id == 9U || domain_id == 11U || domain_id == 12U) {
		swt = BIT_32(12) | BIT_32(11) | BIT_32(10) | BIT_32(9);
	} else {
		swt = BIT_32(domain_id);
	}

	if (pwr_on) {
		ret = upwr_pwm_power_on(&swt, NULL, NULL);
	} else {
		ret = upwr_pwm_power_off(&swt, NULL, NULL);
	}

	if (ret) {
		NOTICE("%s failed: ret: %d, pwr_on: %d\n", __func__, ret, pwr_on);
		return ret;
	}
	upower_wait_resp();

	ret = upwr_poll_req_status(UPWR_SG_PWRMGMT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		NOTICE("Failure %d, %s\n", ret, __func__);
		if (ret == UPWR_REQ_BUSY) {
			return -EBUSY;
		} else {
			return -EINVAL;
		}
	}

	return 0;
}

int upower_read_temperature(uint32_t sensor_id, int32_t *temperature)
{
	int ret, ret_val;
	upwr_resp_t err_code;
	int64_t t;

	ret = upwr_tpm_get_temperature(sensor_id, NULL);
	if (ret) {
		return ret;
	}

	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_TEMPM, NULL, &err_code, &ret_val, 1000);
	if (ret > UPWR_REQ_OK) {
		return ret;
	}

	t = ret_val & 0xff;
	*temperature = (2673049 * t * t * t / 10000000 + 3734262 * t * t / 100000 +
			4487042 * t / 100 - 4698694) / 100000;

	return 0;
}

int upower_pmic_i2c_write(uint32_t reg_addr, uint32_t reg_val)
{
	int ret, ret_val;
	upwr_resp_t err_code;

	ret = upwr_xcp_i2c_access(0x32, 1, 1, reg_addr, reg_val, NULL);
	if (ret) {
		WARN("pmic i2c read failed ret %d\n", ret);
		return ret;
	}

	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_EXCEPT, NULL, &err_code, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		WARN("i2c poll Failure %d, err_code %d, ret_val 0x%x\n",
		     ret, err_code, ret_val);
		return ret;
	}

	VERBOSE("PMIC write reg[0x%x], val[0x%x]\n", reg_addr, reg_val);

	return 0;
}

int upower_pmic_i2c_read(uint32_t reg_addr, uint32_t *reg_val)
{
	int ret, ret_val;
	upwr_resp_t err_code;

	if (reg_val == NULL) {
		return -1;
	}

	ret = upwr_xcp_i2c_access(0x32, -1, 1, reg_addr, 0, NULL);
	if (ret) {
		WARN("pmic i2c read failed ret %d\n", ret);
		return ret;
	}

	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_EXCEPT, NULL, &err_code, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		WARN("i2c poll Failure %d, err_code %d, ret_val 0x%x\n",
			ret, err_code, ret_val);
		return ret;
	}

	*reg_val = ret_val;

	VERBOSE("PMIC read reg[0x%x], val[0x%x]\n", reg_addr, *reg_val);

	return 0;
}
