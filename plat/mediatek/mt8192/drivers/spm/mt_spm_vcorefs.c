/*
 * Copyright(C)2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <lib/utils_def.h>

#include <mtk_sip_svc.h>
#include <plat_pm.h>
#include <platform_def.h>

#include "mt_spm.h"
#include "mt_spm_internal.h"
#include "mt_spm_reg.h"
#include "mt_spm_vcorefs.h"
#include "mt_spm_pmic_wrap.h"

#define VCORE_CT_ENABLE (1U << 5)
#define SW_REQ5_INIT_VAL (6U << 12)
#define V_VMODE_SHIFT 0
#define VCORE_HV 105
#define VCORE_LV 95
#define PMIC_STEP_UV 6250

static const struct reg_config dvfsrc_init_configs[] = {
	/* Setup opp table */
	{ DVFSRC_LEVEL_LABEL_0_1, 0x50436053 },
	{ DVFSRC_LEVEL_LABEL_2_3, 0x40335042 },
	{ DVFSRC_LEVEL_LABEL_4_5, 0x40314032 },
	{ DVFSRC_LEVEL_LABEL_6_7, 0x30223023 },
	{ DVFSRC_LEVEL_LABEL_8_9, 0x20133021 },
	{ DVFSRC_LEVEL_LABEL_10_11, 0x20112012 },
	{ DVFSRC_LEVEL_LABEL_12_13, 0x10032010 },
	{ DVFSRC_LEVEL_LABEL_14_15, 0x10011002 },
	{ DVFSRC_LEVEL_LABEL_16_17, 0x00131000 },
	{ DVFSRC_LEVEL_LABEL_18_19, 0x00110012 },
	{ DVFSRC_LEVEL_LABEL_20_21, 0x00000010 },

	/* Setup hw emi qos policy */
	{ DVFSRC_DDR_REQUEST, 0x00004321 },
	{ DVFSRC_DDR_REQUEST3, 0x00000065 },

	/* Setup up for PCIe */
	{ DVFSRC_PCIE_VCORE_REQ, 0x0A298001 },

	/* Setup up HRT QOS policy */
	{ DVFSRC_HRT_BW_BASE, 0x00000004 },
	{ DVFSRC_HRT_REQ_UNIT, 0x0000001E },
	{ DVFSRC_HRT_HIGH_3, 0x18A618A6 },
	{ DVFSRC_HRT_HIGH_2, 0x18A61183 },
	{ DVFSRC_HRT_HIGH_1, 0x0D690B80 },
	{ DVFSRC_HRT_HIGH, 0x070804B0 },
	{ DVFSRC_HRT_LOW_3, 0x18A518A5 },
	{ DVFSRC_HRT_LOW_2, 0x18A51182 },
	{ DVFSRC_HRT_LOW_1, 0x0D680B7F },
	{ DVFSRC_HRT_LOW, 0x070704AF },
	{ DVFSRC_HRT_REQUEST, 0x66654321 },
	/* Setup up SRT QOS policy */
	{ DVFSRC_QOS_EN, 0x0011007C },
	{ DVFSRC_DDR_QOS0, 0x00000019 },
	{ DVFSRC_DDR_QOS1, 0x00000026 },
	{ DVFSRC_DDR_QOS2, 0x00000033 },
	{ DVFSRC_DDR_QOS3, 0x0000003B },
	{ DVFSRC_DDR_QOS4, 0x0000004C },
	{ DVFSRC_DDR_QOS5, 0x00000066 },
	{ DVFSRC_DDR_QOS6, 0x00000066 },
	{ DVFSRC_DDR_REQUEST5, 0x54321000 },
	{ DVFSRC_DDR_REQUEST7, 0x66000000 },
	/* Setup up hifi request policy */
	{ DVFSRC_DDR_REQUEST6, 0x66543210 },
	/* Setup up hw request vcore policy */
	{ DVFSRC_VCORE_USER_REQ, 0x00010A29 },

	/* Setup misc*/
	{ DVFSRC_TIMEOUT_NEXTREQ, 0x00000015 },
	{ DVFSRC_RSRV_5, 0x00000001 },
	{ DVFSRC_INT_EN, 0x00000002 },
	/* Init opp and enable dvfsrc*/
	{ DVFSRC_CURRENT_FORCE, 0x00000001 },
	{ DVFSRC_BASIC_CONTROL, 0x0298444B },
	{ DVFSRC_BASIC_CONTROL, 0x0298054B },
	{ DVFSRC_CURRENT_FORCE, 0x00000000 },
};

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src = R12_REG_CPU_WAKEUP,

	/* default VCORE DVFS is disabled */
	.pcm_flags = (SPM_FLAG_RUN_COMMON_SCENARIO |
			SPM_FLAG_DISABLE_VCORE_DVS |
			SPM_FLAG_DISABLE_VCORE_DFS),

	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	.reg_wfi_op = 0,
	.reg_wfi_type = 0,
	.reg_mp0_cputop_idle_mask = 0,
	.reg_mp1_cputop_idle_mask = 0,
	.reg_mcusys_idle_mask = 0,
	.reg_md_apsrc_1_sel = 0,
	.reg_md_apsrc_0_sel = 0,
	.reg_conn_apsrc_sel = 0,

	/* SPM_SRC_REQ */
	.reg_spm_apsrc_req = 0,
	.reg_spm_f26m_req = 0,
	.reg_spm_infra_req = 0,
	.reg_spm_vrf18_req = 0,
	.reg_spm_ddr_en_req = 1,
	.reg_spm_dvfs_req = 0,
	.reg_spm_sw_mailbox_req = 0,
	.reg_spm_sspm_mailbox_req = 0,
	.reg_spm_adsp_mailbox_req = 0,
	.reg_spm_scp_mailbox_req = 0,

	/* SPM_SRC6_MASK */
	.reg_dpmaif_srcclkena_mask_b = 1,
	.reg_dpmaif_infra_req_mask_b = 1,
	.reg_dpmaif_apsrc_req_mask_b = 1,
	.reg_dpmaif_vrf18_req_mask_b = 1,
	.reg_dpmaif_ddr_en_mask_b    = 1,

	/* SPM_SRC_MASK */
	.reg_md_srcclkena_0_mask_b = 1,
	.reg_md_srcclkena2infra_req_0_mask_b = 0,
	.reg_md_apsrc2infra_req_0_mask_b = 1,
	.reg_md_apsrc_req_0_mask_b = 1,
	.reg_md_vrf18_req_0_mask_b = 1,
	.reg_md_ddr_en_0_mask_b = 1,
	.reg_md_srcclkena_1_mask_b = 0,
	.reg_md_srcclkena2infra_req_1_mask_b = 0,
	.reg_md_apsrc2infra_req_1_mask_b = 0,
	.reg_md_apsrc_req_1_mask_b = 0,
	.reg_md_vrf18_req_1_mask_b = 0,
	.reg_md_ddr_en_1_mask_b = 0,
	.reg_conn_srcclkena_mask_b = 1,
	.reg_conn_srcclkenb_mask_b = 0,
	.reg_conn_infra_req_mask_b = 1,
	.reg_conn_apsrc_req_mask_b = 1,
	.reg_conn_vrf18_req_mask_b = 1,
	.reg_conn_ddr_en_mask_b = 1,
	.reg_conn_vfe28_mask_b = 0,
	.reg_srcclkeni0_srcclkena_mask_b = 1,
	.reg_srcclkeni0_infra_req_mask_b = 1,
	.reg_srcclkeni1_srcclkena_mask_b = 0,
	.reg_srcclkeni1_infra_req_mask_b = 0,
	.reg_srcclkeni2_srcclkena_mask_b = 0,
	.reg_srcclkeni2_infra_req_mask_b = 0,
	.reg_infrasys_apsrc_req_mask_b = 0,
	.reg_infrasys_ddr_en_mask_b = 1,
	.reg_md32_srcclkena_mask_b = 1,
	.reg_md32_infra_req_mask_b = 1,
	.reg_md32_apsrc_req_mask_b = 1,
	.reg_md32_vrf18_req_mask_b = 1,
	.reg_md32_ddr_en_mask_b = 1,

	/* SPM_SRC2_MASK */
	.reg_scp_srcclkena_mask_b = 1,
	.reg_scp_infra_req_mask_b = 1,
	.reg_scp_apsrc_req_mask_b = 1,
	.reg_scp_vrf18_req_mask_b = 1,
	.reg_scp_ddr_en_mask_b = 1,
	.reg_audio_dsp_srcclkena_mask_b = 1,
	.reg_audio_dsp_infra_req_mask_b = 1,
	.reg_audio_dsp_apsrc_req_mask_b = 1,
	.reg_audio_dsp_vrf18_req_mask_b = 1,
	.reg_audio_dsp_ddr_en_mask_b = 1,
	.reg_ufs_srcclkena_mask_b = 1,
	.reg_ufs_infra_req_mask_b = 1,
	.reg_ufs_apsrc_req_mask_b = 1,
	.reg_ufs_vrf18_req_mask_b = 1,
	.reg_ufs_ddr_en_mask_b = 1,
	.reg_disp0_apsrc_req_mask_b = 1,
	.reg_disp0_ddr_en_mask_b = 1,
	.reg_disp1_apsrc_req_mask_b = 1,
	.reg_disp1_ddr_en_mask_b = 1,
	.reg_gce_infra_req_mask_b = 1,
	.reg_gce_apsrc_req_mask_b = 1,
	.reg_gce_vrf18_req_mask_b = 1,
	.reg_gce_ddr_en_mask_b = 1,
	.reg_apu_srcclkena_mask_b = 1,
	.reg_apu_infra_req_mask_b = 1,
	.reg_apu_apsrc_req_mask_b = 1,
	.reg_apu_vrf18_req_mask_b = 1,
	.reg_apu_ddr_en_mask_b = 1,
	.reg_cg_check_srcclkena_mask_b = 0,
	.reg_cg_check_apsrc_req_mask_b = 0,
	.reg_cg_check_vrf18_req_mask_b = 0,
	.reg_cg_check_ddr_en_mask_b = 0,

	/* SPM_SRC3_MASK */
	.reg_dvfsrc_event_trigger_mask_b = 1,
	.reg_sw2spm_int0_mask_b = 0,
	.reg_sw2spm_int1_mask_b = 0,
	.reg_sw2spm_int2_mask_b = 0,
	.reg_sw2spm_int3_mask_b = 0,
	.reg_sc_adsp2spm_wakeup_mask_b = 0,
	.reg_sc_sspm2spm_wakeup_mask_b = 0,
	.reg_sc_scp2spm_wakeup_mask_b = 0,
	.reg_csyspwrreq_mask = 1,
	.reg_spm_srcclkena_reserved_mask_b = 0,
	.reg_spm_infra_req_reserved_mask_b = 0,
	.reg_spm_apsrc_req_reserved_mask_b = 0,
	.reg_spm_vrf18_req_reserved_mask_b = 0,
	.reg_spm_ddr_en_reserved_mask_b = 0,
	.reg_mcupm_srcclkena_mask_b = 1,
	.reg_mcupm_infra_req_mask_b = 1,
	.reg_mcupm_apsrc_req_mask_b = 1,
	.reg_mcupm_vrf18_req_mask_b = 1,
	.reg_mcupm_ddr_en_mask_b = 1,
	.reg_msdc0_srcclkena_mask_b = 1,
	.reg_msdc0_infra_req_mask_b = 1,
	.reg_msdc0_apsrc_req_mask_b = 1,
	.reg_msdc0_vrf18_req_mask_b = 1,
	.reg_msdc0_ddr_en_mask_b = 1,
	.reg_msdc1_srcclkena_mask_b = 1,
	.reg_msdc1_infra_req_mask_b = 1,
	.reg_msdc1_apsrc_req_mask_b = 1,
	.reg_msdc1_vrf18_req_mask_b = 1,
	.reg_msdc1_ddr_en_mask_b = 1,

	/* SPM_SRC4_MASK */
	.ccif_event_mask_b = 0xFFF,
	.reg_bak_psri_srcclkena_mask_b = 0,
	.reg_bak_psri_infra_req_mask_b = 0,
	.reg_bak_psri_apsrc_req_mask_b = 0,
	.reg_bak_psri_vrf18_req_mask_b = 0,
	.reg_bak_psri_ddr_en_mask_b = 0,
	.reg_dramc0_md32_infra_req_mask_b = 1,
	.reg_dramc0_md32_vrf18_req_mask_b = 0,
	.reg_dramc1_md32_infra_req_mask_b = 1,
	.reg_dramc1_md32_vrf18_req_mask_b = 0,
	.reg_conn_srcclkenb2pwrap_mask_b = 0,
	.reg_dramc0_md32_wakeup_mask = 1,
	.reg_dramc1_md32_wakeup_mask = 1,

	/* SPM_SRC5_MASK */
	.reg_mcusys_merge_apsrc_req_mask_b = 0x11,
	.reg_mcusys_merge_ddr_en_mask_b = 0x11,
	.reg_msdc2_srcclkena_mask_b = 1,
	.reg_msdc2_infra_req_mask_b = 1,
	.reg_msdc2_apsrc_req_mask_b = 1,
	.reg_msdc2_vrf18_req_mask_b = 1,
	.reg_msdc2_ddr_en_mask_b = 1,
	.reg_pcie_srcclkena_mask_b = 1,
	.reg_pcie_infra_req_mask_b = 1,
	.reg_pcie_apsrc_req_mask_b = 1,
	.reg_pcie_vrf18_req_mask_b = 1,
	.reg_pcie_ddr_en_mask_b = 1,

	/* SPM_WAKEUP_EVENT_MASK */
	.reg_wakeup_event_mask = 0xEFFFFFFF,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/* Auto-gen End */
};

struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl	= &vcorefs_ctrl,
};

static void spm_vcorefs_pwarp_cmd(uint64_t cmd, uint64_t val)
{
	if (cmd < NR_IDX_ALL) {
		mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, cmd, val);
	} else {
		INFO("cmd out of range!\n");
	}
}

void spm_dvfsfw_init(uint64_t boot_up_opp, uint64_t dram_issue)
{
	mmio_clrsetbits_32(SPM_DVFS_MISC, SPM_DVFS_FORCE_ENABLE_LSB,
		SPM_DVFSRC_ENABLE_LSB);

	mmio_write_32(SPM_DVFS_LEVEL, 0x00000001);
	mmio_write_32(SPM_DVS_DFS_LEVEL, 0x00010001);
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl,
					 const struct pwr_ctrl *src_pwr_ctrl)
{
	uint32_t dvfs_mask = SPM_FLAG_DISABLE_VCORE_DVS |
			     SPM_FLAG_DISABLE_VCORE_DFS |
			     SPM_FLAG_ENABLE_VOLTAGE_BIN;

	dest_pwr_ctrl->pcm_flags = (dest_pwr_ctrl->pcm_flags & (~dvfs_mask)) |
					(src_pwr_ctrl->pcm_flags & dvfs_mask);

	if (dest_pwr_ctrl->pcm_flags_cust > 0U) {
		dest_pwr_ctrl->pcm_flags_cust =
			(dest_pwr_ctrl->pcm_flags_cust & (~dvfs_mask)) |
			(src_pwr_ctrl->pcm_flags & dvfs_mask);
	}
}

static void spm_go_to_vcorefs(void)
{
	__spm_set_power_control(__spm_vcorefs.pwrctrl);
	__spm_set_wakeup_event(__spm_vcorefs.pwrctrl);
	__spm_set_pcm_flags(__spm_vcorefs.pwrctrl);
	__spm_send_cpu_wakeup_event();
}

static void dvfsrc_init(void)
{
	uint32_t i;

	for (i = 0U; i < ARRAY_SIZE(dvfsrc_init_configs); i++) {
		mmio_write_32(dvfsrc_init_configs[i].offset,
			dvfsrc_init_configs[i].val);
	}
}

static uint32_t spm_vcorefs_get_efuse_data(void)
{
	return mmio_read_32(VCORE_VB_EFUSE);
}

static uint32_t is_rising_need(void)
{
	return ((spm_vcorefs_get_efuse_data() & 0xF) == 11U) ? 1U : 0U;
}

static void spm_vcorefs_vcore_setting(uint64_t flag)
{
	uint32_t dvfs_v_mode, dvfsrc_rsrv, i;
	uint32_t opp_uv[] = {725000U, 650000U, 600000U, 575000U};

	dvfsrc_rsrv = mmio_read_32(DVFSRC_RSRV_4);

	dvfs_v_mode = (dvfsrc_rsrv >> V_VMODE_SHIFT) & 0x3;

	if (is_rising_need() != 0U) {
		opp_uv[2] = 625000U;
		opp_uv[3] = 600000U;
	}

	for (i = 0; i < ARRAY_SIZE(opp_uv); i++) {
		if (dvfs_v_mode == 3U) {
			/* LV */
			opp_uv[i] = round_down((opp_uv[i] * VCORE_LV) / 100U,
					      PMIC_STEP_UV);
		} else if (dvfs_v_mode == 1U) {
			/* HV */
			opp_uv[i] = round_up((opp_uv[i] * VCORE_HV) / 100U,
					    PMIC_STEP_UV);
		}
		spm_vcorefs_pwarp_cmd(i, __vcore_uv_to_pmic(opp_uv[i]));
	}
}

uint64_t spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3, uint64_t *x4)
{
	uint64_t cmd = x1;
	uint64_t spm_flags;

	switch (cmd) {
	case VCOREFS_SMC_CMD_INIT:
		/* vcore_dvfs init + kick */
		mmio_write_32(DVFSRC_SW_REQ5, SW_REQ5_INIT_VAL);
		spm_dvfsfw_init(0ULL, 0ULL);
		spm_vcorefs_vcore_setting(x3 & 0xF);
		spm_flags = SPM_FLAG_RUN_COMMON_SCENARIO;
		if ((x2 & 0x1) > 0U) {
			spm_flags |= SPM_FLAG_DISABLE_VCORE_DVS;
		}

		if ((x2 & 0x2) > 0U) {
			spm_flags |= SPM_FLAG_DISABLE_VCORE_DFS;
		}

		if ((mmio_read_32(DVFSRC_RSRV_4) & VCORE_CT_ENABLE) > 0U) {
			spm_flags |= SPM_FLAG_ENABLE_VOLTAGE_BIN;
		}

		set_pwrctrl_pcm_flags(__spm_vcorefs.pwrctrl, spm_flags);
		spm_go_to_vcorefs();
		dvfsrc_init();

		*x4 = 0U;
		mmio_write_32(DVFSRC_SW_REQ5, 0U);
		break;
	case VCOREFS_SMC_CMD_KICK:
		mmio_write_32(DVFSRC_SW_REQ5, 0U);
		break;
	default:
		break;
	}

	return 0ULL;
}
