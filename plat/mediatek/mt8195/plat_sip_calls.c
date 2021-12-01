/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <mt_dp.h>
#include <mt_spm.h>
#include <mt_spm_vcorefs.h>
#include <mtk_apusys.h>
#include <mtk_sip_svc.h>
#include <plat_dfd.h>
#include "plat_sip_calls.h"

uintptr_t mediatek_plat_sip_handler(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags)
{
	int32_t ret;
	uint32_t ret_val;

	switch (smc_fid) {
	case MTK_SIP_DP_CONTROL_AARCH32:
	case MTK_SIP_DP_CONTROL_AARCH64:
		ret = dp_secure_handler(x1, x2, &ret_val);
		SMC_RET2(handle, ret, ret_val);
		break;
	case MTK_SIP_VCORE_CONTROL_ARCH32:
	case MTK_SIP_VCORE_CONTROL_ARCH64:
		ret = spm_vcorefs_v2_args(x1, x2, x3, &x4);
		SMC_RET2(handle, ret, x4);
		break;
	case MTK_SIP_KERNEL_DFD_AARCH32:
	case MTK_SIP_KERNEL_DFD_AARCH64:
		ret = dfd_smc_dispatcher(x1, x2, x3, x4);
		SMC_RET1(handle, ret);
		break;
	case MTK_SIP_APUSYS_CONTROL_AARCH32:
	case MTK_SIP_APUSYS_CONTROL_AARCH64:
		ret = apusys_kernel_ctrl(x1, x2, x3, x4, &ret_val);
		SMC_RET2(handle, ret, ret_val);
		break;
	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}
