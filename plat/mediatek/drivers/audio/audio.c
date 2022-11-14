/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>

#include <common/debug.h>

#include <audio.h>

#include <mtk_sip_svc.h>

#define MODULE_TAG "[AUDIO]"

static u_register_t audio_smc_handler(u_register_t x1, u_register_t x2,
				      u_register_t x3, u_register_t x4,
				      void *handle, struct smccc_res *smccc_ret)
{
	uint32_t request_ops;
	int ret;

	request_ops = (uint32_t)x1;

	switch (request_ops) {
	case MTK_AUDIO_SMC_OP_DOMAIN_SIDEBANDS:
		ret = set_audio_domain_sidebands();
		break;
	default:
		ERROR("%s: %s: Unsupported request_ops %x\n",
		      MODULE_TAG, __func__, request_ops);
		ret = -EIO;
		break;
	}

	VERBOSE("%s: %s, request_ops = %x, ret = %d\n",
		MODULE_TAG, __func__, request_ops, ret);
	return ret;
}
/* Register SiP SMC service */
DECLARE_SMC_HANDLER(MTK_SIP_AUDIO_CONTROL, audio_smc_handler);
