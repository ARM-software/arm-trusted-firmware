/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>

#include <drivers/spm/mt_spm_vcorefs_api.h>
#include <mt_spm_vcorefs_common.h>
#include <mt_spm_vcorefs_exp.h>
#include <mt_spm_vcorefs_ext.h>
#include <mtk_sip_svc.h>

static u_register_t mtk_vcorefs_handler(u_register_t x1,
					u_register_t x2,
					u_register_t x3,
					u_register_t x4,
					void *handle,
					struct smccc_res *smccc_ret)
{
	uint64_t ret = VCOREFS_E_NOT_SUPPORTED;
	uint64_t cmd = x1;
	uint32_t val = 0;

	switch (cmd) {
	case VCOREFS_SMC_VCORE_DVFS_INIT:
		ret = spm_vcorefs_plat_init(x2, x3, &val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_VCORE_DVFS_KICK:
		ret = spm_vcorefs_plat_kick();
		break;
	case VCOREFS_SMC_CMD_OPP_TYPE:
		ret = spm_vcorefs_get_opp_type(&val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_CMD_FW_TYPE:
		ret = spm_vcorefs_get_fw_type(&val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_CMD_GET_UV:
		ret = spm_vcorefs_get_vcore_uv(x2, &val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_CMD_GET_FREQ:
		ret = spm_vcorefs_get_dram_freq(x2, &val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_CMD_GET_NUM_V:
		ret = spm_vcorefs_get_vcore_opp_num(&val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_CMD_GET_NUM_F:
		ret = spm_vcorefs_get_dram_opp_num(&val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_CMD_GET_VCORE_INFO:
		ret = spm_vcorefs_get_vcore_info(x2, &val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_CMD_QOS_MODE:
		ret = spm_vcorefs_qos_mode(x2);
		break;
#ifdef MTK_VCORE_DVFS_PAUSE
	case VCOREFS_SMC_CMD_PAUSE_ENABLE:
		ret = spm_vcorefs_pause_enable(x2);
		break;
#endif
#ifdef MTK_VCORE_DVFS_RES_MEM
	case VCOREFS_SMC_RSC_MEM_REQ:
		ret = spm_vcorefs_rsc_mem_req(true);
		break;
	case VCOREFS_SMC_RSC_MEM_REL:
		ret = spm_vcorefs_rsc_mem_req(false);
		break;
#endif
	default:
		break;
	}
	return ret;
}

DECLARE_SMC_HANDLER(MTK_SIP_VCORE_CONTROL, mtk_vcorefs_handler);

#ifdef CONFIG_MTK_VCOREDVFS_LK_SUPPORT
static u_register_t mtk_vcorefs_bl_handler(u_register_t x1,
					   u_register_t x2,
					   u_register_t x3,
					   u_register_t x4,
					   void *handle,
					   struct smccc_res *smccc_ret)
{
	uint64_t = VCOREFS_E_NOT_SUPPORTED;
	uint32_t val = 0;

	switch (x1) {
	case VCOREFS_SMC_VCORE_DVFS_INIT:
		ret = spm_vcorefs_plat_init(x2, x3, &val);
		smccc_ret->a1 = val;
		break;
	case VCOREFS_SMC_VCORE_DVFS_KICK:
		ret = spm_vcorefs_plat_kick();
		break;
	default:
		break;
	}
	return ret;
}

DECLARE_SMC_HANDLER(MTK_SIP_BL_SPM_VCOREFS_CONTROL, mtk_vcorefs_bl_handler);
#endif
