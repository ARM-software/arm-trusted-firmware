/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* TF-A system header */
#include <common/debug.h>

/* Vendor header */
#include "apusys.h"
#include "apusys_devapc.h"
#include "apusys_power.h"
#include "apusys_rv.h"
#include "apusys_security_ctrl_plat.h"
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
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_SETUP_REVISER:
		ret = apusys_kernel_apusys_rv_setup_reviser();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_RESET_MP:
		ret = apusys_kernel_apusys_rv_reset_mp();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_SETUP_BOOT:
		ret = apusys_kernel_apusys_rv_setup_boot();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_START_MP:
		ret = apusys_kernel_apusys_rv_start_mp();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_STOP_MP:
		ret = apusys_kernel_apusys_rv_stop_mp();
		break;
	case MTK_APUSYS_KERNEL_OP_DEVAPC_INIT_RCX:
		ret = apusys_devapc_rcx_init();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_SETUP_SEC_MEM:
		ret = apusys_kernel_apusys_rv_setup_sec_mem();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_DISABLE_WDT_ISR:
		ret = apusys_kernel_apusys_rv_disable_wdt_isr();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_CLEAR_WDT_ISR:
		ret = apusys_kernel_apusys_rv_clear_wdt_isr();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_CG_GATING:
		ret = apusys_kernel_apusys_rv_cg_gating();
		break;
	case MTK_APUSYS_KERNEL_OP_APUSYS_RV_CG_UNGATING:
		ret = apusys_kernel_apusys_rv_cg_ungating();
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
	if (apusys_power_init() != 0) {
		return -1;
	}

	if (apusys_devapc_ao_init() != 0) {
		return -1;
	}

	apusys_security_ctrl_init();
	apusys_rv_mbox_mpu_init();

	return 0;
}
MTK_PLAT_SETUP_1_INIT(apusys_init);
