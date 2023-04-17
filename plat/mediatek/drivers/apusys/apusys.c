/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* TF-A system header */
#include <common/debug.h>

/* Vendor header */
#include "apusys.h"
#include "apusys_power.h"
#include <lib/mtk_init/mtk_init.h>
#include <mtk_sip_svc.h>

static u_register_t apusys_kernel_handler(u_register_t x1,
					  u_register_t x2,
					  u_register_t x3,
					  u_register_t x4,
					  void *handle,
					  struct smccc_res *smccc_ret)
{
	uint32_t request_ops;
	int32_t ret = -1;

	request_ops = (uint32_t)x1;

	switch (request_ops) {
	case MTK_APUSYS_KERNEL_OP_APUSYS_PWR_TOP_ON:
		ret = apusys_kernel_apusys_pwr_top_on();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_PWR_TOP_OFF:
		ret = apusys_kernel_apusys_pwr_top_off();
		break;
	default:
		ERROR(MODULE_TAG "%s unknown request_ops = %x\n", MODULE_TAG, request_ops);
		break;
	}

	return ret;
}
DECLARE_SMC_HANDLER(MTK_SIP_APUSYS_CONTROL, apusys_kernel_handler);

int apusys_init(void)
{
	apusys_power_init();
	return 0;
}
MTK_PLAT_SETUP_1_INIT(apusys_init);
