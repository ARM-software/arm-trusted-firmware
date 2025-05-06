/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>

#include <mtk_bl31_interface.h>
#include <mtk_sip_svc.h>

#define MODULE_TAG "[SLBC]"

enum {
	MTK_SLBC_KERNEL_OP_CPU_DCC = 0,
};

static u_register_t slbc_kernel_handler(u_register_t x1, u_register_t x2,
					u_register_t x3, u_register_t x4,
					void *handle,
					struct smccc_res *smccc_ret)
{
	uint32_t request_ops = (uint32_t)x1;
	u_register_t ret = 0;

	switch (request_ops) {
	case MTK_SLBC_KERNEL_OP_CPU_DCC:
		cpu_qos_change_dcc(x2, x3);
		break;
	default:
		ERROR("%s: %s, unknown request_ops = %x\n", MODULE_TAG, __func__, request_ops);
		ret = EIO;
		break;
	}

	VERBOSE("%s: %s, request_ops = %x, ret = %lu\n", MODULE_TAG, __func__, request_ops, ret);
	return ret;
}

/* Register SiP SMC service */
DECLARE_SMC_HANDLER(MTK_SIP_KERNEL_SLBC_CONTROL, slbc_kernel_handler);
