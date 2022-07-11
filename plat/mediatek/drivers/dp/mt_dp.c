/*
 * Copyright (c) 2020-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <mt_dp.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>

static uint32_t dp_write_sec_reg(uint32_t is_edp, uint32_t offset,
				uint32_t value, uint32_t mask)
{
	uint32_t reg = (is_edp != 0U) ? EDP_SEC_BASE : DP_SEC_BASE;

	mmio_clrsetbits_32(reg + offset, mask, value);

	return mmio_read_32(reg + offset);
}

int32_t dp_secure_handler(uint64_t cmd, uint64_t para, uint32_t *val)
{
	int32_t ret = 0L;
	uint32_t is_edp = 0UL;
	uint32_t regval = 0UL;
	uint32_t regmsk = 0UL;
	uint32_t fldmask = 0UL;

	if ((cmd > DP_ATF_CMD_COUNT) || (val == NULL)) {
		INFO("dp_secure_handler error cmd 0x%" PRIx64 "\n", cmd);
		return MTK_SIP_E_INVALID_PARAM;
	}

	switch (cmd) {
	case DP_ATF_DP_VIDEO_UNMUTE:
		INFO("[%s] DP_ATF_DP_VIDEO_UNMUTE\n", __func__);
		is_edp = DP_ATF_TYPE_DP;
		ret = MTK_SIP_E_SUCCESS;
		break;
	case DP_ATF_EDP_VIDEO_UNMUTE:
		INFO("[%s] DP_ATF_EDP_VIDEO_UNMUTE\n", __func__);
		is_edp = DP_ATF_TYPE_EDP;
		ret = MTK_SIP_E_SUCCESS;
		break;
	default:
		ret = MTK_SIP_E_INVALID_PARAM;
		break;
	}

	if (ret == MTK_SIP_E_SUCCESS) {
		regmsk = (VIDEO_MUTE_SEL_SECURE_FLDMASK |
			  VIDEO_MUTE_SW_SECURE_FLDMASK);
		if (para > 0U) {
			fldmask = VIDEO_MUTE_SW_SECURE_FLDMASK;
		} else {
			fldmask = 0;
		}

		regval = (VIDEO_MUTE_SEL_SECURE_FLDMASK | fldmask);
		*val = dp_write_sec_reg(is_edp, DP_TX_SECURE_REG11,
					regval, regmsk);
	}

	return ret;
}

u_register_t mtk_dp_sip_handler(u_register_t x1, u_register_t x2,
				u_register_t x3, u_register_t x4,
				void *handle, struct smccc_res *smccc_ret)
{
	uint32_t ret_val;

	return dp_secure_handler(x1, x2, &ret_val);
}
DECLARE_SMC_HANDLER(MTK_SIP_DP_CONTROL, mtk_dp_sip_handler);
