/*
 * Copyright (c) 2016-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/st/bsec.h>

#include <stm32mp1_smc.h>

#include "bsec_svc.h"

uint32_t bsec_main(uint32_t x1, uint32_t x2, uint32_t x3,
		   uint32_t *ret_otp_value)
{
	uint32_t result;
	uint32_t tmp_data = 0U;

	switch (x1) {
	case STM32_SMC_READ_SHADOW:
		result = bsec_read_otp(ret_otp_value, x2);
		break;
	case STM32_SMC_PROG_OTP:
		*ret_otp_value = 0U;
		result = bsec_program_otp(x3, x2);
		break;
	case STM32_SMC_WRITE_SHADOW:
		*ret_otp_value = 0U;
		result = bsec_write_otp(x3, x2);
		break;
	case STM32_SMC_READ_OTP:
		*ret_otp_value = 0U;
		result = bsec_read_otp(&tmp_data, x2);
		if (result != BSEC_OK) {
			break;
		}

		result = bsec_shadow_register(x2);
		if (result != BSEC_OK) {
			break;
		}

		result = bsec_read_otp(ret_otp_value, x2);
		if (result != BSEC_OK) {
			break;
		}

		result = bsec_write_otp(tmp_data, x2);
		break;

	default:
		return STM32_SMC_INVALID_PARAMS;
	}

	return (result == BSEC_OK) ? STM32_SMC_OK : STM32_SMC_FAILED;
}
