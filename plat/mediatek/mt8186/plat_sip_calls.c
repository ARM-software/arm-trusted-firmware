/*
 * Copyright (c) 2021-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <mt_msdc.h>
#include <mt_spm_vcorefs.h>
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
	uint64_t ret;

	switch (smc_fid) {
	case MTK_SIP_VCORE_CONTROL_AARCH32:
	case MTK_SIP_VCORE_CONTROL_AARCH64:
		ret = spm_vcorefs_args(x1, x2, x3, (uint64_t *)&x4);
		SMC_RET2(handle, ret, x4);
		break;
	case MTK_SIP_KERNEL_DFD_AARCH32:
	case MTK_SIP_KERNEL_DFD_AARCH64:
		ret = dfd_smc_dispatcher(x1, x2, x3, x4);
		SMC_RET1(handle, ret);
		break;
	case MTK_SIP_KERNEL_MSDC_AARCH32:
	case MTK_SIP_KERNEL_MSDC_AARCH64:
		ret = msdc_smc_dispatcher(x1, x2, x3, x4);
		SMC_RET1(handle, ret);
		break;
	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}
