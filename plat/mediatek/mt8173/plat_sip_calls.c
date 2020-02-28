/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>

#include <crypt.h>
#include <mtcmos.h>
#include <mtk_sip_svc.h>
#include <plat_sip_calls.h>
#include <wdt.h>

/* Authorized secure register list */
enum {
	SREG_HDMI_COLOR_EN = 0x14000904
};

static const uint32_t authorized_sreg[] = {
	SREG_HDMI_COLOR_EN
};

#define authorized_sreg_cnt	\
	(sizeof(authorized_sreg) / sizeof(authorized_sreg[0]))

uint64_t mt_sip_set_authorized_sreg(uint32_t sreg, uint32_t val)
{
	uint64_t i;

	for (i = 0; i < authorized_sreg_cnt; i++) {
		if (authorized_sreg[i] == sreg) {
			mmio_write_32(sreg, val);
			return MTK_SIP_E_SUCCESS;
		}
	}

	return MTK_SIP_E_INVALID_PARAM;
}

static uint64_t mt_sip_pwr_on_mtcmos(uint32_t val)
{
	uint32_t ret;

	ret = mtcmos_non_cpu_ctrl(1, val);
	if (ret)
		return MTK_SIP_E_INVALID_PARAM;
	else
		return MTK_SIP_E_SUCCESS;
}

static uint64_t mt_sip_pwr_off_mtcmos(uint32_t val)
{
	uint32_t ret;

	ret = mtcmos_non_cpu_ctrl(0, val);
	if (ret)
		return MTK_SIP_E_INVALID_PARAM;
	else
		return MTK_SIP_E_SUCCESS;
}

static uint64_t mt_sip_pwr_mtcmos_support(void)
{
	return MTK_SIP_E_SUCCESS;
}

uint64_t mediatek_plat_sip_handler(uint32_t smc_fid,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	uint64_t ret;

	switch (smc_fid) {
	case MTK_SIP_PWR_ON_MTCMOS:
		ret = mt_sip_pwr_on_mtcmos((uint32_t)x1);
		SMC_RET1(handle, ret);

	case MTK_SIP_PWR_OFF_MTCMOS:
		ret = mt_sip_pwr_off_mtcmos((uint32_t)x1);
		SMC_RET1(handle, ret);

	case MTK_SIP_PWR_MTCMOS_SUPPORT:
		ret = mt_sip_pwr_mtcmos_support();
		SMC_RET1(handle, ret);

	case MTK_SIP_SET_HDCP_KEY_EX:
		ret = crypt_set_hdcp_key_ex(x1, x2, x3);
		SMC_RET1(handle, ret);

	case MTK_SIP_SET_HDCP_KEY_NUM:
		ret = crypt_set_hdcp_key_num((uint32_t)x1);
		SMC_RET1(handle, ret);

	case MTK_SIP_CLR_HDCP_KEY:
		ret = crypt_clear_hdcp_key();
		SMC_RET1(handle, ret);

	case MTK_SIP_SMC_WATCHDOG:
		return wdt_smc_handler(x1, x2, handle);

	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}
