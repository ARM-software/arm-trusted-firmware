/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <mtk_disp_priv.h>
#include <mtk_sip_svc.h>

/* definition */
static int mtk_disp_disable_sec(uint32_t larb_id, uint32_t mmu_en_msk)
{
	const struct mtk_disp_config *disp_cfg_t;
	uint32_t cfg_cnt;

	for (cfg_cnt = 0; cfg_cnt < (disp_cfg_count - 1); cfg_cnt++) {
		disp_cfg_t = &disp_cfg[cfg_cnt];
		mmio_setbits_32(disp_cfg_t->base, disp_cfg_t->ns_mask);
	}

	disp_cfg_t = &disp_cfg[disp_cfg_count - 1];
	mmio_clrbits_32(disp_cfg_t->base, disp_cfg_t->ns_mask);

	return MTK_SIP_E_SUCCESS;
}

static u_register_t mtk_disp_handler(u_register_t x1, u_register_t x2,
				     u_register_t x3, u_register_t x4,
				     void *handle, struct smccc_res *smccc_ret)
{
	uint32_t cmd_id = x1, mdl_id = x2, val = x3;
	int ret = MTK_SIP_E_NOT_SUPPORTED;

	(void)x4;
	(void)handle;
	(void)smccc_ret;

	switch (cmd_id) {
	case DISP_ATF_CMD_CONFIG_DISP_CONFIG:
		ret = mtk_disp_disable_sec(mdl_id, val);
		break;
	default:
		break;
	}

	return ret;
}
DECLARE_SMC_HANDLER(MTK_SIP_DISP_CONTROL, mtk_disp_handler);
