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
#include <mt_spm_vcorefs_reg.h>
#include <mt_vcore_dvfsrc_plat_def.h>
#include <mtk_mmap_pool.h>
#include <mtk_sip_svc.h>
#include <pmic_wrap/inc/mt_spm_pmic_wrap.h>
#include <sleep_def.h>

#define V_VB_TEST_EN	BIT(6)
#define AVS_THERMAL_ENABLE	BIT(30)
#define V_DRM_ENABLE	BIT(31)

#define V_VMODE_SHIFT	0
#define V_VMODE_MASK	0x3
#define V_OPP_TYPE_SHIFT	20
#define V_OPP_TYPE_SHIFT_MASK	0x3
#define SLT2_AVSQ_SHIFT	28
#define SLT2_SHIFT	29

#define DVFSRC_DISABLE_DVS	0x1
#define DVFSRC_DISABLE_DFS	0x2

#define VCORE_MAX_OPP	5
#define DRAM_MAX_OPP	5
#define VCORE_MAX_OPP_LP4X	3
#define DRAM_MAX_OPP_LP4X	3
#define VCORE_OPP0_UV	800000
#define VCORE_OPP1_UV	725000
#define VCORE_OPP2_UV	650000
#define VCORE_OPP3_UV	600000
#define VCORE_OPP4_UV	550000

static unsigned int v_opp_uv[VCORE_MAX_OPP] = {
	VCORE_OPP0_UV, VCORE_OPP1_UV, VCORE_OPP2_UV,
	VCORE_OPP3_UV, VCORE_OPP4_UV,
};

static const mmap_region_t dvfsrc_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(DVFSRC_BASE, DVFSRC_REG_SIZE, MT_DEVICE | MT_RW),
	{ 0 }
};
DECLARE_MTK_MMAP_REGIONS(dvfsrc_mmap);

#ifdef CONFIG_MTK_VCOREDVFS_SUPPORT
static unsigned int v_opp_df_uv[VCORE_MAX_OPP] = {
	VCORE_OPP0_UV, VCORE_OPP1_UV, VCORE_OPP2_UV,
	VCORE_OPP3_UV, VCORE_OPP4_UV,
};

static const unsigned int v_aging_margin[VCORE_MAX_OPP] = { 12500, 12500, 12500,
							    12500, 12500 };
static uint32_t opp_type;

void spm_vcorefs_pwarp_cmd(uint64_t cmd, uint64_t val)
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
	uint32_t dvfsrc_rsrv;

	if (dvfs_enable_done)
		return;

	dvfsrc_rsrv = mmio_read_32(DVFSRC_RSRV_4);
	opp_type = (dvfsrc_rsrv >> V_OPP_TYPE_SHIFT) & V_OPP_TYPE_SHIFT_MASK;

	for (i = 0; i < ARRAY_SIZE(dvfsrc_init_configs); i++)
		mmio_write_32(dvfsrc_init_configs[i].offset,
			      dvfsrc_init_configs[i].val);

	if (opp_type == 1) {
		for (i = 0; i < ARRAY_SIZE(lp4_init_configs); i++)
			mmio_write_32(lp4_init_configs[i].offset,
				      lp4_init_configs[i].val);
	}

#ifdef CONFIG_MTK_DBGTOP
	mtk_dbgtop_cfg_dvfsrc(1);
#endif

	dvfs_enable_done = 1;
}

static unsigned int spm_vcore_get_vb_voltage(unsigned int index)
{
	unsigned int volt = 0;

	volt = mmio_read_32(DVFSRC_RSRV_0);
	volt = (volt >> (index * 8)) & 0xff;
	return VCORE_PMIC_TO_UV(volt);
}

static void spm_vcorefs_vcore_setting(void)
{
	int i;
	unsigned int v_slt2_margin = 0;
	unsigned int v_slt2_avsq_margin = 0;
	uint32_t dvfs_v_mode, dvfsrc_rsrv, is_vb_en;

	dvfsrc_rsrv = mmio_read_32(DVFSRC_RSRV_4);
	dvfs_v_mode = (dvfsrc_rsrv >> V_VMODE_SHIFT) & V_VMODE_MASK;
	is_vb_en = dvfsrc_rsrv & V_VB_EN;

	if ((dvfsrc_rsrv >> SLT2_SHIFT) & 0x1)
		v_slt2_margin = VCORE_STEP_UV;

	if ((dvfsrc_rsrv >> SLT2_AVSQ_SHIFT) & 0x1)
		v_slt2_avsq_margin = VCORE_STEP_UV * 2;

	if (is_vb_en) {
		for (i = 0; i < VCORE_MAX_OPP; i++)
			v_opp_uv[i] = spm_vcore_get_vb_voltage(i);
	}

	if (dvfs_v_mode == 3) { /* LV */
		for (i = 0; i < VCORE_MAX_OPP; i++) {
			v_opp_uv[i] = v_opp_uv[i] - v_slt2_margin -
				      v_slt2_avsq_margin;
			v_opp_uv[i] = round_up(v_opp_uv[i] - v_aging_margin[i] -
						       VCORE_STEP_UV,
					       VCORE_STEP_UV);
			v_opp_df_uv[i] = round_down((v_opp_df_uv[i] * 95) / 100,
						    VCORE_STEP_UV);
			if (v_opp_uv[i] > v_opp_df_uv[i])
				v_opp_uv[i] = v_opp_df_uv[i];
		}
	} else if (dvfs_v_mode == 1) { /* HV */
		for (i = 0; i < VCORE_MAX_OPP; i++) {
			v_opp_uv[i] = round_up((v_opp_uv[i] * 105) / 100,
					       VCORE_STEP_UV);
			v_opp_df_uv[i] = v_opp_uv[i];
		}
	}

	spm_vcorefs_pwarp_cmd(CMD_0, VCORE_UV_TO_PMIC(v_opp_df_uv[4]));
	spm_vcorefs_pwarp_cmd(CMD_1, VCORE_UV_TO_PMIC(v_opp_df_uv[3]));
	spm_vcorefs_pwarp_cmd(CMD_2, VCORE_UV_TO_PMIC(v_opp_df_uv[2]));
	spm_vcorefs_pwarp_cmd(CMD_3, VCORE_UV_TO_PMIC(v_opp_df_uv[1]));
	spm_vcorefs_pwarp_cmd(CMD_4, VCORE_UV_TO_PMIC(v_opp_df_uv[0]));
	spm_vcorefs_pwarp_cmd(CMD_20, VCORE_UV_TO_PMIC(v_opp_uv[4]));
	spm_vcorefs_pwarp_cmd(CMD_21, VCORE_UV_TO_PMIC(v_opp_uv[3]));
	spm_vcorefs_pwarp_cmd(CMD_22, VCORE_UV_TO_PMIC(v_opp_uv[2]));
	spm_vcorefs_pwarp_cmd(CMD_23, VCORE_UV_TO_PMIC(v_opp_uv[1]));
	spm_vcorefs_pwarp_cmd(CMD_24, VCORE_UV_TO_PMIC(v_opp_uv[0]));
}

int spm_vcorefs_plat_init(uint32_t dvfsrc_flag, uint32_t dvfsrc_vmode,
			  uint32_t *dram_type)
{
	uint64_t spm_flags = SPM_FLAG_RUN_COMMON_SCENARIO;

	if (dram_type == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	if (!spm_load_firmware_status())
		return MTK_SIP_E_NOT_SUPPORTED;

	/* set high opp */
	mmio_write_32(DVFSRC_SW_BW_9, 0x3FF);
	spm_dvfsfw_init(0, 0);
	spm_vcorefs_vcore_setting();

	if (dvfsrc_flag & DVFSRC_DISABLE_DVS)
		spm_flags |= SPM_FLAG_DISABLE_VCORE_DVS;

	if (dvfsrc_flag & DVFSRC_DISABLE_DFS) {
		spm_flags |=
			(SPM_FLAG_DISABLE_DDR_DFS | SPM_FLAG_DISABLE_EMI_DFS |
			 SPM_FLAG_DISABLE_BUS_DFS);
		mmio_write_32(DVFSRC_SW_BW_8, 0x3FF);
	}

	if (mmio_read_32(DVFSRC_RSRV_4) & V_DRM_ENABLE) {
		spm_flags |=
			(SPM_FLAG_DISABLE_VCORE_DVS | SPM_FLAG_DISABLE_DDR_DFS |
			 SPM_FLAG_DISABLE_EMI_DFS | SPM_FLAG_DISABLE_BUS_DFS);
		return MTK_SIP_E_NOT_SUPPORTED;
	}

	spm_go_to_vcorefs(spm_flags);
	dvfsrc_init();
	*dram_type = opp_type;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_plat_kick(void)
{
	if (!spm_load_firmware_status())
		return MTK_SIP_E_NOT_SUPPORTED;
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
	mmio_write_32(SPM_SW_RSV_2, BIT(0));
	mmio_write_32(DVFSRC_AVS_RETRY, 1);
	mmio_write_32(DVFSRC_AVS_RETRY, 0);
}

void spm_vcorefs_plat_resume(void)
{
	mmio_write_32(DVFSRC_MD_TURBO, 0);
}

#else
int spm_vcorefs_plat_kick(void)
{
	return MTK_SIP_E_NOT_SUPPORTED;
}

int spm_vcorefs_plat_init(uint32_t dvfsrc_flag, uint32_t dvfsrc_vmode,
			  uint32_t *dram_type)
{
	return MTK_SIP_E_NOT_SUPPORTED;
}

void spm_vcorefs_plat_suspend(void)
{
}

void spm_vcorefs_plat_resume(void)
{
}
#endif /*CONFIG_MTK_VCOREDVFS_SUPPORT*/

int spm_vcorefs_get_vcore_uv(uint32_t gear, uint32_t *val)
{
	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	if (gear < VCORE_MAX_OPP)
		*val = v_opp_uv[VCORE_MAX_OPP - gear - 1];
	else
		*val = 0;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_dram_freq(uint32_t gear, uint32_t *val)
{
	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;

#ifdef CONFIG_MTK_DRAMC
	if (opp_type == 0)
		*val = get_dram_step_freq((DRAM_MAX_OPP - gear - 1)) * 1000;
	else
		*val = get_dram_step_freq((DRAM_MAX_OPP_LP4X - gear - 1)) *
		       1000;
#else
	*val = 0;
#endif
	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_vcore_opp_num(uint32_t *val)
{
	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	*val = VCORE_MAX_OPP;
	if (opp_type == 1)
		*val = VCORE_MAX_OPP_LP4X;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_dram_opp_num(uint32_t *val)
{
	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	*val = DRAM_MAX_OPP;
	if (opp_type == 1)
		*val = DRAM_MAX_OPP_LP4X;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_opp_type(uint32_t *val)
{
	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	*val = opp_type;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_fw_type(uint32_t *val)
{
	if (val == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	*val = 0;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_get_vcore_info(uint32_t idx, uint32_t *data)
{
	if (data == NULL)
		return MTK_SIP_E_INVALID_PARAM;

	*data = 0;

	return VCOREFS_SUCCESS;
}

int spm_vcorefs_pause_enable(uint32_t enable)
{
	mtk_dbgtop_dfd_pause_dvfsrc(1);

	return VCOREFS_SUCCESS;
}
