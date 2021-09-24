/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>
#include <string.h>
#include <common/debug.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_pmic_wrap.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <mtk_plat_common.h>
#include <mtk_sip_svc.h>
#include <platform_def.h>

#define VCORE_MAX_OPP 4
#define DRAM_MAX_OPP 7

static bool spm_dvfs_init_done;
static bool dvfs_enable_done;
static int vcore_opp_0_uv = 750000;
static int vcore_opp_1_uv = 650000;
static int vcore_opp_2_uv = 600000;
static int vcore_opp_3_uv = 550000;

static struct reg_config dvfsrc_init_configs[] = {
	{ DVFSRC_HRT_REQ_UNIT,       0x0000001E },
	{ DVFSRC_DEBOUNCE_TIME,      0x19651965 },
	{ DVFSRC_TIMEOUT_NEXTREQ,    0x00000015 },
	{ DVFSRC_LEVEL_MASK,         0x000EE000 },
	{ DVFSRC_DDR_QOS0,           0x00000019 },
	{ DVFSRC_DDR_QOS1,           0x00000026 },
	{ DVFSRC_DDR_QOS2,           0x00000033 },
	{ DVFSRC_DDR_QOS3,           0x0000003B },
	{ DVFSRC_DDR_QOS4,           0x0000004C },
	{ DVFSRC_DDR_QOS5,           0x00000066 },
	{ DVFSRC_DDR_QOS6,           0x00660066 },
	{ DVFSRC_LEVEL_LABEL_0_1,    0x50436053 },
	{ DVFSRC_LEVEL_LABEL_2_3,    0x40335042 },
	{ DVFSRC_LEVEL_LABEL_4_5,    0x40314032 },
	{ DVFSRC_LEVEL_LABEL_6_7,    0x30223023 },
	{ DVFSRC_LEVEL_LABEL_8_9,    0x20133021 },
	{ DVFSRC_LEVEL_LABEL_10_11,  0x20112012 },
	{ DVFSRC_LEVEL_LABEL_12_13,  0x10032010 },
	{ DVFSRC_LEVEL_LABEL_14_15,  0x10011002 },
	{ DVFSRC_LEVEL_LABEL_16_17,  0x00131000 },
	{ DVFSRC_LEVEL_LABEL_18_19,  0x00110012 },
	{ DVFSRC_LEVEL_LABEL_20_21,  0x00000010 },
	{ DVFSRC_MD_LATENCY_IMPROVE, 0x00000040 },
	{ DVFSRC_DDR_REQUEST,        0x00004321 },
	{ DVFSRC_DDR_REQUEST3,       0x00000065 },
	{ DVFSRC_DDR_ADD_REQUEST,    0x66543210 },
	{ DVFSRC_HRT_REQUEST,        0x66654321 },
	{ DVFSRC_DDR_REQUEST5,       0x54321000 },
	{ DVFSRC_DDR_REQUEST7,       0x66000000 },
	{ DVFSRC_VCORE_USER_REQ,     0x00010A29 },
	{ DVFSRC_HRT_HIGH_3,         0x18A618A6 },
	{ DVFSRC_HRT_HIGH_2,         0x18A61183 },
	{ DVFSRC_HRT_HIGH_1,         0x0D690B80 },
	{ DVFSRC_HRT_HIGH,           0x070804B0 },
	{ DVFSRC_HRT_LOW_3,          0x18A518A5 },
	{ DVFSRC_HRT_LOW_2,          0x18A51182 },
	{ DVFSRC_HRT_LOW_1,          0x0D680B7F },
	{ DVFSRC_HRT_LOW,            0x070704AF },
	{ DVFSRC_BASIC_CONTROL_3,    0x00000006 },
	{ DVFSRC_INT_EN,             0x00000002 },
	{ DVFSRC_QOS_EN,             0x0000407C },
	{ DVFSRC_HRT_BW_BASE,        0x00000004 },
	{ DVFSRC_PCIE_VCORE_REQ,     0x65908101 },
	{ DVFSRC_CURRENT_FORCE,      0x00000001 },
	{ DVFSRC_BASIC_CONTROL,      0x6698444B },
	{ DVFSRC_BASIC_CONTROL,      0x6698054B },
	{ DVFSRC_CURRENT_FORCE,      0x00000000 },
};

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_REG_CPU_WAKEUP,

	/* default VCORE DVFS is disabled */
	.pcm_flags = (SPM_FLAG_RUN_COMMON_SCENARIO |
			SPM_FLAG_DISABLE_VCORE_DVS | SPM_FLAG_DISABLE_VCORE_DFS),

	/* SPM_AP_STANDBY_CON */
	/* [0] */
	.reg_wfi_op = 0,
	/* [1] */
	.reg_wfi_type = 0,
	/* [2] */
	.reg_mp0_cputop_idle_mask = 0,
	/* [3] */
	.reg_mp1_cputop_idle_mask = 0,
	/* [4] */
	.reg_mcusys_idle_mask = 0,
	/* [25] */
	.reg_md_apsrc_1_sel = 0,
	/* [26] */
	.reg_md_apsrc_0_sel = 0,
	/* [29] */
	.reg_conn_apsrc_sel = 0,

	/* SPM_SRC_REQ */
	/* [0] */
	.reg_spm_apsrc_req = 0,
	/* [1] */
	.reg_spm_f26m_req = 0,
	/* [3] */
	.reg_spm_infra_req = 0,
	/* [4] */
	.reg_spm_vrf18_req = 0,
	/* [7] FIXME: default disable HW Auto S1*/
	.reg_spm_ddr_en_req = 1,
	/* [8] */
	.reg_spm_dvfs_req = 0,
	/* [9] */
	.reg_spm_sw_mailbox_req = 0,
	/* [10] */
	.reg_spm_sspm_mailbox_req = 0,
	/* [11] */
	.reg_spm_adsp_mailbox_req = 0,
	/* [12] */
	.reg_spm_scp_mailbox_req = 0,

	/* SPM_SRC_MASK */
	/* [0] */
	.reg_sspm_srcclkena_0_mask_b = 1,
	/* [1] */
	.reg_sspm_infra_req_0_mask_b = 1,
	/* [2] */
	.reg_sspm_apsrc_req_0_mask_b = 1,
	/* [3] */
	.reg_sspm_vrf18_req_0_mask_b = 1,
	/* [4] */
	.reg_sspm_ddr_en_0_mask_b = 1,
	/* [5] */
	.reg_scp_srcclkena_mask_b = 1,
	/* [6] */
	.reg_scp_infra_req_mask_b = 1,
	/* [7] */
	.reg_scp_apsrc_req_mask_b = 1,
	/* [8] */
	.reg_scp_vrf18_req_mask_b = 1,
	/* [9] */
	.reg_scp_ddr_en_mask_b = 1,
	/* [10] */
	.reg_audio_dsp_srcclkena_mask_b = 1,
	/* [11] */
	.reg_audio_dsp_infra_req_mask_b = 1,
	/* [12] */
	.reg_audio_dsp_apsrc_req_mask_b = 1,
	/* [13] */
	.reg_audio_dsp_vrf18_req_mask_b = 1,
	/* [14] */
	.reg_audio_dsp_ddr_en_mask_b = 1,
	/* [15] */
	.reg_apu_srcclkena_mask_b = 1,
	/* [16] */
	.reg_apu_infra_req_mask_b = 1,
	/* [17] */
	.reg_apu_apsrc_req_mask_b = 1,
	/* [18] */
	.reg_apu_vrf18_req_mask_b = 1,
	/* [19] */
	.reg_apu_ddr_en_mask_b = 1,
	/* [20] */
	.reg_cpueb_srcclkena_mask_b = 1,
	/* [21] */
	.reg_cpueb_infra_req_mask_b = 1,
	/* [22] */
	.reg_cpueb_apsrc_req_mask_b = 1,
	/* [23] */
	.reg_cpueb_vrf18_req_mask_b = 1,
	/* [24] */
	.reg_cpueb_ddr_en_mask_b = 1,
	/* [25] */
	.reg_bak_psri_srcclkena_mask_b = 0,
	/* [26] */
	.reg_bak_psri_infra_req_mask_b = 0,
	/* [27] */
	.reg_bak_psri_apsrc_req_mask_b = 0,
	/* [28] */
	.reg_bak_psri_vrf18_req_mask_b = 0,
	/* [29] */
	.reg_bak_psri_ddr_en_mask_b = 0,

	/* SPM_SRC2_MASK */
	/* [0] */
	.reg_msdc0_srcclkena_mask_b = 1,
	/* [1] */
	.reg_msdc0_infra_req_mask_b = 1,
	/* [2] */
	.reg_msdc0_apsrc_req_mask_b = 1,
	/* [3] */
	.reg_msdc0_vrf18_req_mask_b = 1,
	/* [4] */
	.reg_msdc0_ddr_en_mask_b = 1,
	/* [5] */
	.reg_msdc1_srcclkena_mask_b = 1,
	/* [6] */
	.reg_msdc1_infra_req_mask_b = 1,
	/* [7] */
	.reg_msdc1_apsrc_req_mask_b = 1,
	/* [8] */
	.reg_msdc1_vrf18_req_mask_b = 1,
	/* [9] */
	.reg_msdc1_ddr_en_mask_b = 1,
	/* [10] */
	.reg_msdc2_srcclkena_mask_b = 1,
	/* [11] */
	.reg_msdc2_infra_req_mask_b = 1,
	/* [12] */
	.reg_msdc2_apsrc_req_mask_b = 1,
	/* [13] */
	.reg_msdc2_vrf18_req_mask_b = 1,
	/* [14] */
	.reg_msdc2_ddr_en_mask_b = 1,
	/* [15] */
	.reg_ufs_srcclkena_mask_b = 1,
	/* [16] */
	.reg_ufs_infra_req_mask_b = 1,
	/* [17] */
	.reg_ufs_apsrc_req_mask_b = 1,
	/* [18] */
	.reg_ufs_vrf18_req_mask_b = 1,
	/* [19] */
	.reg_ufs_ddr_en_mask_b = 1,
	/* [20] */
	.reg_usb_srcclkena_mask_b = 1,
	/* [21] */
	.reg_usb_infra_req_mask_b = 1,
	/* [22] */
	.reg_usb_apsrc_req_mask_b = 1,
	/* [23] */
	.reg_usb_vrf18_req_mask_b = 1,
	/* [24] */
	.reg_usb_ddr_en_mask_b = 1,
	/* [25] */
	.reg_pextp_p0_srcclkena_mask_b = 1,
	/* [26] */
	.reg_pextp_p0_infra_req_mask_b = 1,
	/* [27] */
	.reg_pextp_p0_apsrc_req_mask_b = 1,
	/* [28] */
	.reg_pextp_p0_vrf18_req_mask_b = 1,
	/* [29] */
	.reg_pextp_p0_ddr_en_mask_b = 1,

	/* SPM_SRC3_MASK */
	/* [0] */
	.reg_pextp_p1_srcclkena_mask_b = 1,
	/* [1] */
	.reg_pextp_p1_infra_req_mask_b = 1,
	/* [2] */
	.reg_pextp_p1_apsrc_req_mask_b = 1,
	/* [3] */
	.reg_pextp_p1_vrf18_req_mask_b = 1,
	/* [4] */
	.reg_pextp_p1_ddr_en_mask_b = 1,
	/* [5] */
	.reg_gce0_infra_req_mask_b = 1,
	/* [6] */
	.reg_gce0_apsrc_req_mask_b = 1,
	/* [7] */
	.reg_gce0_vrf18_req_mask_b = 1,
	/* [8] */
	.reg_gce0_ddr_en_mask_b = 1,
	/* [9] */
	.reg_gce1_infra_req_mask_b = 1,
	/* [10] */
	.reg_gce1_apsrc_req_mask_b = 1,
	/* [11] */
	.reg_gce1_vrf18_req_mask_b = 1,
	/* [12] */
	.reg_gce1_ddr_en_mask_b = 1,
	/* [13] */
	.reg_spm_srcclkena_reserved_mask_b = 1,
	/* [14] */
	.reg_spm_infra_req_reserved_mask_b = 1,
	/* [15] */
	.reg_spm_apsrc_req_reserved_mask_b = 1,
	/* [16] */
	.reg_spm_vrf18_req_reserved_mask_b = 1,
	/* [17] */
	.reg_spm_ddr_en_reserved_mask_b = 1,
	/* [18] */
	.reg_disp0_apsrc_req_mask_b = 1,
	/* [19] */
	.reg_disp0_ddr_en_mask_b = 1,
	/* [20] */
	.reg_disp1_apsrc_req_mask_b = 1,
	/* [21] */
	.reg_disp1_ddr_en_mask_b = 1,
	/* [22] */
	.reg_disp2_apsrc_req_mask_b = 1,
	/* [23] */
	.reg_disp2_ddr_en_mask_b = 1,
	/* [24] */
	.reg_disp3_apsrc_req_mask_b = 1,
	/* [25] */
	.reg_disp3_ddr_en_mask_b = 1,
	/* [26] */
	.reg_infrasys_apsrc_req_mask_b = 0,
	/* [27] */
	.reg_infrasys_ddr_en_mask_b = 1,

	/* [28] */
	.reg_cg_check_srcclkena_mask_b = 1,
	/* [29] */
	.reg_cg_check_apsrc_req_mask_b = 1,
	/* [30] */
	.reg_cg_check_vrf18_req_mask_b = 1,
	/* [31] */
	.reg_cg_check_ddr_en_mask_b = 1,

	/* SPM_SRC4_MASK */
	/* [8:0] */
	.reg_mcusys_merge_apsrc_req_mask_b = 0x11,
	/* [17:9] */
	.reg_mcusys_merge_ddr_en_mask_b = 0x11,
	/* [19:18] */
	.reg_dramc_md32_infra_req_mask_b = 0,
	/* [21:20] */
	.reg_dramc_md32_vrf18_req_mask_b = 0,
	/* [23:22] */
	.reg_dramc_md32_ddr_en_mask_b = 0,
	/* [24] */
	.reg_dvfsrc_event_trigger_mask_b = 1,

	/* SPM_WAKEUP_EVENT_MASK2 */
	/* [3:0] */
	.reg_sc_sw2spm_wakeup_mask_b = 0,
	/* [4] */
	.reg_sc_adsp2spm_wakeup_mask_b = 0,
	/* [8:5] */
	.reg_sc_sspm2spm_wakeup_mask_b = 0,
	/* [9] */
	.reg_sc_scp2spm_wakeup_mask_b = 0,
	/* [10] */
	.reg_csyspwrup_ack_mask = 0,
	/* [11] */
	.reg_csyspwrup_req_mask = 1,

	/* SPM_WAKEUP_EVENT_MASK */
	/* [31:0] */
	.reg_wakeup_event_mask = 0xEFFFFFFF,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	/* [31:0] */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,
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
	if (spm_dvfs_init_done == false) {
		mmio_write_32(SPM_DVFS_MISC, (mmio_read_32(SPM_DVFS_MISC) &
				~(SPM_DVFS_FORCE_ENABLE_LSB)) | (SPM_DVFSRC_ENABLE_LSB));

		mmio_write_32(SPM_DVFS_LEVEL, 0x00000001);
		mmio_write_32(SPM_DVS_DFS_LEVEL, 0x00010001);

		spm_dvfs_init_done = true;
	}
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl,
						const struct pwr_ctrl *src_pwr_ctrl)
{
	uint32_t dvfs_mask = SPM_FLAG_DISABLE_VCORE_DVS |
			     SPM_FLAG_DISABLE_VCORE_DFS |
			     SPM_FLAG_ENABLE_VOLTAGE_BIN;

	dest_pwr_ctrl->pcm_flags = (dest_pwr_ctrl->pcm_flags & (~dvfs_mask)) |
					(src_pwr_ctrl->pcm_flags & dvfs_mask);

	if (dest_pwr_ctrl->pcm_flags_cust) {
		dest_pwr_ctrl->pcm_flags_cust = (dest_pwr_ctrl->pcm_flags_cust & (~dvfs_mask)) |
						(src_pwr_ctrl->pcm_flags & dvfs_mask);
	}
}

void spm_go_to_vcorefs(uint64_t spm_flags)
{
	__spm_set_power_control(__spm_vcorefs.pwrctrl);
	__spm_set_wakeup_event(__spm_vcorefs.pwrctrl);
	__spm_set_pcm_flags(__spm_vcorefs.pwrctrl);
	__spm_send_cpu_wakeup_event();
}

uint64_t spm_vcorefs_args(uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint64_t ret = 0U;
	uint64_t cmd = x1;
	uint64_t spm_flags;

	switch (cmd) {
	case VCOREFS_SMC_CMD_0:
		spm_dvfsfw_init(x2, x3);
		break;
	case VCOREFS_SMC_CMD_1:
		spm_flags = SPM_FLAG_RUN_COMMON_SCENARIO;
		if (x2 & SPM_FLAG_DISABLE_VCORE_DVS)
			spm_flags |= SPM_FLAG_DISABLE_VCORE_DVS;
		if (x2 & SPM_FLAG_DISABLE_VCORE_DFS)
			spm_flags |= SPM_FLAG_DISABLE_VCORE_DFS;
		spm_go_to_vcorefs(spm_flags);
		break;
	case VCOREFS_SMC_CMD_3:
		spm_vcorefs_pwarp_cmd(x2, x3);
		break;
	case VCOREFS_SMC_CMD_2:
	case VCOREFS_SMC_CMD_4:
	case VCOREFS_SMC_CMD_5:
	case VCOREFS_SMC_CMD_7:
	default:
		break;
	}
	return ret;
}

static void dvfsrc_init(void)
{
	int i;
	int count = ARRAY_SIZE(dvfsrc_init_configs);

	if (dvfs_enable_done == false) {
		for (i = 0; i < count; i++) {
			mmio_write_32(dvfsrc_init_configs[i].offset,
				dvfsrc_init_configs[i].val);
		}

		mmio_write_32(DVFSRC_QOS_EN, 0x0011007C);

		dvfs_enable_done = true;
	}
}

static void spm_vcorefs_vcore_setting(uint64_t flag)
{
	spm_vcorefs_pwarp_cmd(3, __vcore_uv_to_pmic(vcore_opp_3_uv));
	spm_vcorefs_pwarp_cmd(2, __vcore_uv_to_pmic(vcore_opp_2_uv));
	spm_vcorefs_pwarp_cmd(1, __vcore_uv_to_pmic(vcore_opp_1_uv));
	spm_vcorefs_pwarp_cmd(0, __vcore_uv_to_pmic(vcore_opp_0_uv));
}

int spm_vcorefs_get_vcore(unsigned int gear)
{
	int ret_val;

	switch (gear) {
	case 3:
		ret_val = vcore_opp_0_uv;
		break;
	case 2:
		ret_val = vcore_opp_1_uv;
		break;
	case 1:
		ret_val = vcore_opp_2_uv;
		break;
	case 0:
	default:
		ret_val = vcore_opp_3_uv;
		break;
	}
	return ret_val;
}

uint64_t spm_vcorefs_v2_args(u_register_t x1, u_register_t x2, u_register_t x3, u_register_t *x4)
{
	uint64_t ret = 0U;
	uint64_t cmd = x1;
	uint64_t spm_flags;

	switch (cmd) {
	case VCOREFS_SMC_CMD_INIT:
		/* vcore_dvfs init + kick */
		spm_dvfsfw_init(0, 0);
		spm_vcorefs_vcore_setting(x3 & 0xF);
		spm_flags = SPM_FLAG_RUN_COMMON_SCENARIO;
		if (x2 & 0x1) {
			spm_flags |= SPM_FLAG_DISABLE_VCORE_DVS;
		}
		if (x2 & 0x2) {
			spm_flags |= SPM_FLAG_DISABLE_VCORE_DFS;
		}
		spm_go_to_vcorefs(spm_flags);
		dvfsrc_init();
		*x4 = 0U;
		break;
	case VCOREFS_SMC_CMD_OPP_TYPE:
		/* get dram type */
		*x4 = 0U;
		break;
	case VCOREFS_SMC_CMD_FW_TYPE:
		*x4 = 0U;
		break;
	case VCOREFS_SMC_CMD_GET_UV:
		*x4 = spm_vcorefs_get_vcore(x2);
		break;
	case VCOREFS_SMC_CMD_GET_NUM_V:
		*x4 = VCORE_MAX_OPP;
		break;
	case VCOREFS_SMC_CMD_GET_NUM_F:
		*x4 = DRAM_MAX_OPP;
		break;
	default:
		break;
	}

	return ret;
}
