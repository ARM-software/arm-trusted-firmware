/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <dfd.h>
#include <mtk_sip_svc.h>
#include <plat_dfd.h>

static u_register_t dfd_smc_dispatcher(u_register_t arg0, u_register_t arg1,
				       u_register_t arg2, u_register_t arg3,
				       void *handle, struct smccc_res *smccc_ret)
{
	int ret = MTK_SIP_E_SUCCESS;

	switch (arg0) {
	case PLAT_MTK_DFD_SETUP_MAGIC:
		INFO("[%s] DFD setup call from kernel\n", __func__);
		dfd_setup(arg1, arg2, arg3);
		break;
	case PLAT_MTK_DFD_READ_MAGIC:
		/* only allow to access DFD register base + 0x200 */
		if (arg1 <= 0x200) {
			ret = mmio_read_32(MISC1_CFG_BASE + arg1);
		}
		break;
	case PLAT_MTK_DFD_WRITE_MAGIC:
		/* only allow to access DFD register base + 0x200 */
		if (arg1 <= 0x200) {
			sync_writel(MISC1_CFG_BASE + arg1, arg2);
		}
		break;
	default:
		ret = MTK_SIP_E_INVALID_PARAM;
		break;
	}

	return ret;
}
DECLARE_SMC_HANDLER(MTK_SIP_KERNEL_DFD, dfd_smc_dispatcher);
