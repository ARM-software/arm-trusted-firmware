/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <drivers/dbgtop.h>
#include <drivers/dramc.h>
#include <drivers/spmi/spmi_common.h>
#include <drivers/spmi_api.h>
#include <drivers/sramrc.h>
#include <lib/mtk_init/mtk_init.h>
#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <mt_spm_vcorefs_common.h>
#include <mt_spm_vcorefs_ext.h>
#include <mt_spm_vcorefs_reg.h>
#include <mt_vcore_dvfsrc_plat_def.h>
#include <mtk_mmap_pool.h>
#include <mtk_sip_svc.h>
#include <pmic_wrap/inc/mt_spm_pmic_wrap.h>
#include <sleep_def.h>

#define VCORE_SPM_INIT_PCM_FLAG	 (SPM_FLAG_RUN_COMMON_SCENARIO | \
				  SPM_FLAG_DISABLE_VLP_PDN)

#define V_VB_EN			BIT(5)
#define V_DRM_ENABLE		BIT(31)
#define V_B0_EN_SHIFT		BIT(23)

#define V_VMODE_SHIFT		0
#define V_VMODE_MASK		0x3
#define V_OPP_TYPE_SHIFT	20
#define V_OPP_TYPE_SHIFT_MASK	0x3

#define DVFSRC_DISABLE_DVS	0x1
#define DVFSRC_DISABLE_DFS	0x2

#define CFG_DVFSRC_BW_ASYM_ENABLE

#ifdef MT8196_VCORE_SUPPORT
#define VCORE_MAX_OPP		6
#define DRAM_MAX_OPP		11

#define VCORE_OPP0_UV		850000
#define VCORE_OPP1_UV		750000
#define VCORE_OPP2_UV		675000
#define VCORE_OPP3_UV		625000
#define VCORE_OPP4_UV		600000

static unsigned int v_opp_uv[VCORE_MAX_OPP] = {
	VCORE_OPP0_UV,
	VCORE_OPP0_UV,
	VCORE_OPP1_UV,
	VCORE_OPP2_UV,
	VCORE_OPP3_UV,
	VCORE_OPP4_UV
};

#else
#define VCORE_MAX_OPP		6
#define DRAM_MAX_OPP		11

#define VCORE_OPP0_UV		875000
#define VCORE_OPP1_UV		825000
#define VCORE_OPP2_UV		725000
#define VCORE_OPP3_UV		650000
#define VCORE_OPP4_UV		600000
#define VCORE_OPP5_UV		575000

static unsigned int v_opp_uv[VCORE_MAX_OPP] = {
	VCORE_OPP0_UV,
	VCORE_OPP1_UV,
	VCORE_OPP2_UV,
	VCORE_OPP3_UV,
	VCORE_OPP4_UV,
	VCORE_OPP5_UV,
};

static unsigned int v_opp_df_uv[VCORE_MAX_OPP] = {
	VCORE_OPP0_UV,
	VCORE_OPP1_UV,
	VCORE_OPP2_UV,
	VCORE_OPP3_UV,
	VCORE_OPP4_UV,
	VCORE_OPP5_UV,
};
#endif

#ifdef CONFIG_MTK_VCOREDVFS_SUPPORT
static int opp_type;
static unsigned int b0_en;

static unsigned int v_roundup(unsigned int x, unsigned int y)
{
	return round_up(x, y);
}

static unsigned int v_rounddown(unsigned int x, unsigned int y)
{
	return round_down(x, y);
}

static void spm_vcorefs_pwarp_cmd(uint64_t cmd, uint64_t val)
{
	if (cmd < NR_IDX_ALL)
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, cmd, val);
	else
		INFO("cmd out of range!\n");
}

static void dvfsrc_init(void)
{
	int i;
	static int dvfs_enable_done;

	if (dvfs_enable_done)
		return;

	for (i = 0; i < ARRAY_SIZE(dvfsrc_init_configs); i++)
		mmio_write_32(dvfsrc_init_configs[i].offset,
			      dvfsrc_init_configs[i].val);

#ifdef MT8196_VCORE_SUPPORT
		for (i = 0; i < ARRAY_SIZE(lp5_8533_init_configs_auto); i++)
			mmio_write_32(lp5_8533_init_configs_auto[i].offset,
				      lp5_8533_init_configs_auto[i].val);
#endif

	if (opp_type == 2) {
		for (i = 0; i < ARRAY_SIZE(lp5_7500_init_configs); i++)
			mmio_write_32(lp5_7500_init_configs[i].offset,
				      lp5_7500_init_configs[i].val);
	} else if (opp_type == 0) {
		for (i = 0; i < ARRAY_SIZE(lp5_8533_init_configs); i++)
			mmio_write_32(lp5_8533_init_configs[i].offset,
				      lp5_8533_init_configs[i].val);
	} else if (opp_type == 3) {
		for (i = 0; i < ARRAY_SIZE(lp5_10677_init_configs); i++)
			mmio_write_32(lp5_10677_init_configs[i].offset,
				      lp5_10677_init_configs[i].val);
	}
#ifdef CFG_DVFSRC_BW_ASYM_ENABLE
	if (b0_en) {
		mmio_write_32(DVFSRC_LEVEL_LABEL_210_211, 0x20100000);
		mmio_write_32(DVFSRC_LEVEL_LABEL_212_213, 0x00504030);
		mmio_write_32(DVFSRC_LEVEL_LABEL_220_221, 0x00400000);
		mmio_write_32(DVFSRC_LEVEL_LABEL_224_225, 0x00000050);
		mmio_write_32(DVFSRC_LEVEL_LABEL_226_227, 0x00300000);
		mmio_write_32(DVFSRC_LEVEL_LABEL_252_253, 0x000040F5);
		mmio_write_32(DVFSRC_LEVEL_LABEL_248_249, 0x30450000);
		mmio_write_32(DVFSRC_LEVEL_LABEL_246_247, 0x00002065);
		mmio_write_32(DVFSRC_LEVEL_LABEL_242_243, 0x10960000);
		mmio_write_32(DVFSRC_LEVEL_LABEL_202_203, 0x00010040);
		mmio_write_32(DVFSRC_LEVEL_LABEL_200_201, 0xE0420030);
	}
#endif

	mmio_write_32(DVFSRC_SW_REQ4, 0x00000030);
	/* ENABLE */
	mmio_write_32(DVFSRC_CURRENT_FORCE_4, 0x00000001);
#ifdef MT8196_VCORE_SUPPORT
	mmio_write_32(DVFSRC_BASIC_CONTROL, 0xD460213B);
	mmio_write_32(DVFSRC_BASIC_CONTROL, 0xD46001BB);
#else
	mmio_write_32(DVFSRC_BASIC_CONTROL, 0xD560213B);
	mmio_write_32(DVFSRC_BASIC_CONTROL, 0xD56001BB);
#endif
	mmio_write_32(DVFSRC_CURRENT_FORCE_4, 0x00000000);

#ifdef CONFIG_MTK_DBGTOP
	mtk_dbgtop_cfg_dvfsrc(1);
#endif
	dvfs_enable_done = 1;
}

static void spm_vcorefs_vcore_setting(void)
{
	int i;
	uint32_t dvfs_v_mode, dvfsrc_rsrv;
#ifndef MT8196_VCORE_SUPPORT
	uint32_t rsrv0, rsrv1;

	rsrv0 = mmio_read_32(DVFSRC_RSRV_0);
	rsrv1 = mmio_read_32(DVFSRC_RSRV_1);
#endif
	dvfsrc_rsrv = mmio_read_32(DVFSRC_RSRV_4);
	dvfs_v_mode = (dvfsrc_rsrv >> V_VMODE_SHIFT) & V_VMODE_MASK;

	if (dvfs_v_mode == 3) {	/* LV */
		for (i = 0; i < VCORE_MAX_OPP; i++)
			v_opp_uv[i] =
				v_rounddown((v_opp_uv[i] * 95) / 100,
					    VCORE_STEP_UV);
	} else if (dvfs_v_mode == 1) {	/* HV */
		for (i = 0; i < VCORE_MAX_OPP; i++)
			v_opp_uv[i] =
				v_roundup((v_opp_uv[i] * 105) / 100,
					   VCORE_STEP_UV);
	}
#ifndef MT8196_VCORE_SUPPORT
	else if (dvfsrc_rsrv & V_VB_EN) { /* NV & AVS1.0 */
		v_opp_uv[5] = VCORE_PMIC_TO_UV(((rsrv1 >> 24) & 0xFF) + 7);
		v_opp_uv[4] = VCORE_PMIC_TO_UV(((rsrv1 >> 16) & 0xFF) + 5);
		v_opp_uv[3] = VCORE_PMIC_TO_UV(((rsrv1 >> 8) & 0xFF) + 4);
		v_opp_uv[2] = VCORE_PMIC_TO_UV(((rsrv0 >> 24) & 0xFF) + 4);
		v_opp_uv[1] = VCORE_PMIC_TO_UV(((rsrv0 >> 16) & 0xFF) + 3);
		v_opp_uv[0] = VCORE_PMIC_TO_UV((rsrv0 & 0xFF) + 3);
		for (i = 0; i < VCORE_MAX_OPP; i++)
			v_opp_uv[i] = v_min(v_opp_uv[i], v_opp_df_uv[i]);
	}
#endif
#ifdef MT8196_VCORE_SUPPORT
	spm_vcorefs_pwarp_cmd(CMD_0, VCORE_UV_TO_PMIC(v_opp_uv[5]));
	spm_vcorefs_pwarp_cmd(CMD_1, VCORE_UV_TO_PMIC(v_opp_uv[5]));
	spm_vcorefs_pwarp_cmd(CMD_2, VCORE_UV_TO_PMIC(v_opp_uv[4]));
	spm_vcorefs_pwarp_cmd(CMD_3, VCORE_UV_TO_PMIC(v_opp_uv[3]));
	spm_vcorefs_pwarp_cmd(CMD_4, VCORE_UV_TO_PMIC(v_opp_uv[2]));
	spm_vcorefs_pwarp_cmd(CMD_5, VCORE_UV_TO_PMIC(v_opp_uv[2]));
	spm_vcorefs_pwarp_cmd(CMD_6, VCORE_UV_TO_PMIC(v_opp_uv[1]));
	spm_vcorefs_pwarp_cmd(CMD_7, VCORE_UV_TO_PMIC(v_opp_uv[1]));
	spm_vcorefs_pwarp_cmd(CMD_8, VCORE_UV_TO_PMIC(v_opp_uv[0]));

	spm_vcorefs_pwarp_cmd(CMD_9, VCORE_UV_TO_PMIC(v_opp_uv[5]));
	spm_vcorefs_pwarp_cmd(CMD_10, VCORE_UV_TO_PMIC(v_opp_uv[4]));
	spm_vcorefs_pwarp_cmd(CMD_11, VCORE_UV_TO_PMIC(v_opp_uv[3]));
	spm_vcorefs_pwarp_cmd(CMD_12, VCORE_UV_TO_PMIC(v_opp_uv[2]));
	spm_vcorefs_pwarp_cmd(CMD_13, VCORE_UV_TO_PMIC(v_opp_uv[1]));
	spm_vcorefs_pwarp_cmd(CMD_14, VCORE_UV_TO_PMIC(v_opp_uv[0]));
#else
	spm_vcorefs_pwarp_cmd(CMD_9, VCORE_UV_TO_PMIC(v_opp_uv[5]));
	spm_vcorefs_pwarp_cmd(CMD_10, VCORE_UV_TO_PMIC(v_opp_uv[4]));
	spm_vcorefs_pwarp_cmd(CMD_11, VCORE_UV_TO_PMIC(v_opp_uv[3]));
	spm_vcorefs_pwarp_cmd(CMD_12, VCORE_UV_TO_PMIC(v_opp_uv[2]));
	spm_vcorefs_pwarp_cmd(CMD_13, VCORE_UV_TO_PMIC(v_opp_uv[1]));
	spm_vcorefs_pwarp_cmd(CMD_14, VCORE_UV_TO_PMIC(v_opp_uv[0]));
#endif
}

int spm_vcorefs_plat_init(uint32_t dvfsrc_flag,
			  uint32_t dvfsrc_vmode, uint32_t *dram_type)
{
	uint64_t spm_flags = VCORE_SPM_INIT_PCM_FLAG;
	uint32_t dvfsrc_rsrv = 0;

	if (mmio_read_32(MD32PCM_PC) == 0)
		return MTK_SIP_E_NOT_SUPPORTED;
	/* set high opp */
	mmio_write_32(DVFSRC_SW_BW_9, 0x3FF);
	spm_dvfsfw_init(0, 0);
	spm_vcorefs_vcore_setting();
	dvfsrc_rsrv = mmio_read_32(DVFSRC_RSRV_4);
	opp_type = (dvfsrc_rsrv >> V_OPP_TYPE_SHIFT) & V_OPP_TYPE_SHIFT_MASK;
	b0_en = dvfsrc_rsrv & V_B0_EN_SHIFT;

	if (dvfsrc_flag & DVFSRC_DISABLE_DVS)
		spm_flags |= SPM_FLAG_DISABLE_VCORE_DVS;

	if (dvfsrc_flag & DVFSRC_DISABLE_DFS) {
		spm_flags |= (SPM_FLAG_DISABLE_DDR_DFS |
			      SPM_FLAG_DISABLE_EMI_DFS |
			      SPM_FLAG_DISABLE_BUS_DFS);
		mmio_write_32(DVFSRC_SW_BW_8, 0x3FF);
	}

	if (dvfsrc_rsrv & V_DRM_ENABLE) {
		spm_flags |= (SPM_FLAG_DISABLE_VCORE_DVS |
			      SPM_FLAG_DISABLE_DDR_DFS |
			      SPM_FLAG_DISABLE_EMI_DFS |
			      SPM_FLAG_DISABLE_BUS_DFS);
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	spm_go_to_vcorefs(spm_flags);
	dvfsrc_init();
	*dram_type = 0;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_plat_kick(void)
{
	/* release high opp if not in drm mode*/
	if (!(mmio_read_32(DVFSRC_RSRV_4) & V_DRM_ENABLE))
		mmio_write_32(DVFSRC_SW_BW_9, 0);

	mmio_write_32(DVFSRC_AVS_RETRY, 1);
	mmio_write_32(DVFSRC_AVS_RETRY, 0);

	return VCOREFS_SUCCESS;
}

void spm_vcorefs_plat_suspend(void)
{
	mmio_write_32(DVFSRC_MD_TURBO, 0x1FFF0000);
}

void spm_vcorefs_plat_resume(void)
{
	mmio_write_32(DVFSRC_MD_TURBO, 0x00000000);
}

#else
int spm_vcorefs_plat_kick(void)
{
	return MTK_SIP_E_NOT_SUPPORTED;
}

int spm_vcorefs_plat_init(uint32_t dvfsrc_flag,
			  uint32_t dvfsrc_vmode, uint32_t *dram_type)
{
	return MTK_SIP_E_NOT_SUPPORTED;
}

void spm_vcorefs_plat_suspend(void)
{
}

void spm_vcorefs_plat_resume(void)
{
}
#endif

int spm_vcorefs_get_vcore_uv(uint32_t gear, uint32_t *val)
{
	if (gear < VCORE_MAX_OPP)
		*val = v_opp_uv[VCORE_MAX_OPP - gear - 1];
	else
		*val = 0;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_dram_freq(uint32_t gear, uint32_t *val)
{
#ifdef CONFIG_MTK_DRAMC
	if (gear < DRAM_MAX_OPP)
		*val = get_dram_step_freq((DRAM_MAX_OPP - gear - 1)) * 1000;
	else
		*val = 0;
#else
	*val = 0;
#endif
	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_vcore_opp_num(uint32_t *val)
{
	*val = VCORE_MAX_OPP;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_dram_opp_num(uint32_t *val)
{
	*val = DRAM_MAX_OPP;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_opp_type(uint32_t *val)
{
	*val = 0;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_fw_type(uint32_t *val)
{
	*val = 0;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_vcore_info(uint32_t idx, uint32_t *data)
{
	if (idx == 0)
		*data = mmio_read_32(VCORE_VB_INFO0);
	else if (idx == 1)
		*data = mmio_read_32(VCORE_VB_INFO1);
	else if (idx == 2)
		*data = mmio_read_32(VCORE_VB_INFO2);
	else
		*data = 0;

	return VCOREFS_SUCCESS;
}

#define MMPC_FLAG 0x3333
int spm_vcorefs_qos_mode(uint32_t data)
{
#ifdef CFG_DVFSRC_BW_ASYM_ENABLE
	uint32_t mode, value;
	bool mm_flag;

	mm_flag = (mmio_read_32(DVFSRC_VCORE_QOS7) == 0xFFFFFE);
	mode = data >> 16;

	if ((mode == 0x3333) && b0_en && mm_flag) {
		value = data & 0xFFFF;
		if (value == 1)
			mmio_write_32(DVFSRC_QOS_EN, 0x00F3007C);
		else
			mmio_write_32(DVFSRC_QOS_EN, 0x20F1007C);
		return VCOREFS_SUCCESS;
	}
#endif

	return MTK_SIP_E_NOT_SUPPORTED;

}

int spm_vcorefs_pause_enable(uint32_t enable)
{
#ifdef CONFIG_MTK_DBGTOP
	mtk_dbgtop_dfd_pause_dvfsrc(1);
#endif

	return VCOREFS_SUCCESS;
}

#ifdef MTK_VCORE_DVFS_RES_MEM
static int vcorefs_rsc_ctrl(unsigned int rsc, bool hold)
{
	static struct mt_lp_resource_user vcorefs_res_user;
	int ret = -1;

	if (!vcorefs_res_user.uid) {
		ret = mt_lp_resource_user_register("VCOREFS",
						    &vcorefs_res_user);
		if (ret) {
			WARN("%s: register lp resource failed", __func__);
			return ret;
		}
	}

	if (hold)
		ret = vcorefs_res_user.request(&vcorefs_res_user, rsc);
	else
		ret = vcorefs_res_user.release(&vcorefs_res_user);

	if (ret)
		WARN("%s: RSC_%d %s failed",
		     __func__, rsc, hold ? "req" : "rel");

	return ret;
}

int spm_vcorefs_rsc_mem_req(bool request)
{
	vcorefs_rsc_ctrl(MT_LP_RQ_DRAM, request);

	return VCOREFS_SUCCESS;
}
#endif
