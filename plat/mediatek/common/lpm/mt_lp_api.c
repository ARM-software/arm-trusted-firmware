/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lpm/mt_lp_api.h>

int mt_audio_update(int type)
{
	int ret, val;

	switch (type) {
	case AUDIO_AFE_ENTER:
	case AUDIO_AFE_LEAVE:
		val = (type == AUDIO_AFE_ENTER) ? 1 : 0;
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_FMAUDIO, &val);
		break;
	case AUDIO_DSP_ENTER:
	case AUDIO_DSP_LEAVE:
		val = (type == AUDIO_DSP_ENTER) ? 1 : 0;
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_ADSP, &val);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

int mtk_usb_update(int type)
{
	int ret, val;

	switch (type) {
	case LPM_USB_ENTER:
	case LPM_USB_LEAVE:
		val = (type == LPM_USB_ENTER) ? 1 : 0;
		ret = mt_lp_rm_do_update(-1, PLAT_RC_IS_USB_INFRA, &val);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}
