/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lpm_v2/mt_lp_api.h>

#define UPDATE_STATUS(val, status, bit) \
	((val) ? ((status) | (1 << (bit))) : ((status) & ~(1 << (bit))))

static uint64_t lp_status;

int mt_audio_update(int type)
{
	int ret, val;

	switch (type) {
	case AUDIO_AFE_ENTER:
	case AUDIO_AFE_LEAVE:
		val = (type == AUDIO_AFE_ENTER) ? 1 : 0;
		lp_status = UPDATE_STATUS(val, lp_status, AUDIO_AFE);
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_FMAUDIO, &val);
		break;
	case AUDIO_DSP_ENTER:
	case AUDIO_DSP_LEAVE:
		val = (type == AUDIO_DSP_ENTER) ? 1 : 0;
		lp_status = UPDATE_STATUS(val, lp_status, AUDIO_DSP);
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_ADSP, &val);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

int mt_usb_update(int type)
{
	int ret, val;

	switch (type) {
	case LPM_USB_ENTER:
	case LPM_USB_LEAVE:
		val = (type == LPM_USB_ENTER) ? 1 : 0;
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_USB_INFRA, &val);
		break;
	case USB_HEADSET_ENTER:
	case USB_HEADSET_LEAVE:
		val = (type == USB_HEADSET_ENTER) ? 1 : 0;
		lp_status = UPDATE_STATUS(val, lp_status, USB_HEADSET);
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_USB_HEADSET, &val);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

uint64_t mt_get_lp_scenario_status(void)
{
	return lp_status;
}

int mt_gpueb_hwctrl(int type, void *priv)
{
	int ret, val;

	switch (type) {
	case GPUEB_PLL_EN:
	case GPUEB_PLL_DIS:
		val = (type == GPUEB_PLL_EN) ? 1 : 0;
		ret = mt_lp_rm_do_hwctrl(PLAT_AP_GPUEB_PLL_CONTROL, val, priv);
		break;
	case GPUEB_GET_PWR_STATUS:
		ret = mt_lp_rm_do_hwctrl(PLAT_AP_GPUEB_PWR_STATUS, 0, priv);
		break;
	case GPUEB_GET_MFG0_PWR_CON:
		ret = mt_lp_rm_do_hwctrl(PLAT_AP_GPUEB_MFG0_PWR_CON, 0, priv);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}
