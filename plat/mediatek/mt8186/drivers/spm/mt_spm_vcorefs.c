/*
 * Copyright(C)2022, MediaTek Inc. All rights reserved.
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
#include <lib/utils_def.h>
#include <plat/common/platform.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_pmic_wrap.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <mtk_sip_svc.h>
#include <plat_pm.h>
#include <platform_def.h>
#include <pmic.h>

#define VCORE_CT_ENABLE		BIT(5)
#define VCORE_DRM_ENABLE	BIT(31)
#define VCORE_PTPOD_SHIFT	(8)
#define VCORE_POWER_SHIFT	(2)

#define VCORE_MAX_OPP		(3)
#define DRAM_MAX_OPP		(6)

#define SW_REQ5_INIT_VAL	(6U << 12)
#define V_VMODE_SHIFT		(0)
#define VCORE_HV		(105)
#define VCORE_LV		(95)
#define PMIC_STEP_UV		(6250)

static int vcore_opp_0_uv = 800000;
static int vcore_opp_1_uv = 700000;
static int vcore_opp_2_uv = 650000;

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

	/* SPM_SRC6_MASK */
	.reg_ccif_event_infra_req_mask_b = 0xFFFF,
	.reg_ccif_event_apsrc_req_mask_b = 0xFFFF,

	/* SPM_SRC_REQ */
	.reg_spm_apsrc_req = 1,
	.reg_spm_f26m_req = 1,
	.reg_spm_infra_req = 1,
	.reg_spm_vrf18_req = 1,
	.reg_spm_ddren_req = 1,
	.reg_spm_dvfs_req = 0,
	.reg_spm_sw_mailbox_req = 0,
	.reg_spm_sspm_mailbox_req = 0,
	.reg_spm_adsp_mailbox_req = 0,
	.reg_spm_scp_mailbox_req = 0,

	/* SPM_SRC_MASK */
	.reg_md_0_srcclkena_mask_b = 1,
	.reg_md_0_infra_req_mask_b = 1,
	.reg_md_0_apsrc_req_mask_b = 1,
	.reg_md_0_vrf18_req_mask_b = 1,
	.reg_md_0_ddren_req_mask_b = 1,
	.reg_md_1_srcclkena_mask_b = 0,
	.reg_md_1_infra_req_mask_b = 0,
	.reg_md_1_apsrc_req_mask_b = 0,
	.reg_md_1_vrf18_req_mask_b = 0,
	.reg_md_1_ddren_req_mask_b = 0,
	.reg_conn_srcclkena_mask_b = 1,
	.reg_conn_srcclkenb_mask_b = 0,
	.reg_conn_infra_req_mask_b = 1,
	.reg_conn_apsrc_req_mask_b = 1,
	.reg_conn_vrf18_req_mask_b = 1,
	.reg_conn_ddren_req_mask_b = 1,
	.reg_conn_vfe28_mask_b = 0,
	.reg_srcclkeni_srcclkena_mask_b = 1,
	.reg_srcclkeni_infra_req_mask_b = 1,
	.reg_infrasys_apsrc_req_mask_b = 0,
	.reg_infrasys_ddren_req_mask_b = 1,
	.reg_sspm_srcclkena_mask_b = 1,
	.reg_sspm_infra_req_mask_b = 1,
	.reg_sspm_apsrc_req_mask_b = 1,
	.reg_sspm_vrf18_req_mask_b = 1,
	.reg_sspm_ddren_req_mask_b = 1,

	/* SPM_SRC2_MASK */
	.reg_scp_srcclkena_mask_b = 1,
	.reg_scp_infra_req_mask_b = 1,
	.reg_scp_apsrc_req_mask_b = 1,
	.reg_scp_vrf18_req_mask_b = 1,
	.reg_scp_ddren_req_mask_b = 1,
	.reg_audio_dsp_srcclkena_mask_b = 1,
	.reg_audio_dsp_infra_req_mask_b = 1,
	.reg_audio_dsp_apsrc_req_mask_b = 1,
	.reg_audio_dsp_vrf18_req_mask_b = 1,
	.reg_audio_dsp_ddren_req_mask_b = 1,
	.reg_ufs_srcclkena_mask_b = 1,
	.reg_ufs_infra_req_mask_b = 1,
	.reg_ufs_apsrc_req_mask_b = 1,
	.reg_ufs_vrf18_req_mask_b = 1,
	.reg_ufs_ddren_req_mask_b = 1,
	.reg_disp0_apsrc_req_mask_b = 1,
	.reg_disp0_ddren_req_mask_b = 1,
	.reg_disp1_apsrc_req_mask_b = 1,
	.reg_disp1_ddren_req_mask_b = 1,
	.reg_gce_infra_req_mask_b = 1,
	.reg_gce_apsrc_req_mask_b = 1,
	.reg_gce_vrf18_req_mask_b = 1,
	.reg_gce_ddren_req_mask_b = 1,
	.reg_apu_srcclkena_mask_b = 0,
	.reg_apu_infra_req_mask_b = 0,
	.reg_apu_apsrc_req_mask_b = 0,
	.reg_apu_vrf18_req_mask_b = 0,
	.reg_apu_ddren_req_mask_b = 0,
	.reg_cg_check_srcclkena_mask_b = 0,
	.reg_cg_check_apsrc_req_mask_b = 0,
	.reg_cg_check_vrf18_req_mask_b = 0,
	.reg_cg_check_ddren_req_mask_b = 0,

	/* SPM_SRC3_MASK */
	.reg_dvfsrc_event_trigger_mask_b = 1,
	.reg_sw2spm_wakeup_mask_b = 0,
	.reg_adsp2spm_wakeup_mask_b = 0,
	.reg_sspm2spm_wakeup_mask_b = 0,
	.reg_scp2spm_wakeup_mask_b = 0,
	.reg_csyspwrup_ack_mask = 1,
	.reg_spm_reserved_srcclkena_mask_b = 0,
	.reg_spm_reserved_infra_req_mask_b = 0,
	.reg_spm_reserved_apsrc_req_mask_b = 0,
	.reg_spm_reserved_vrf18_req_mask_b = 0,
	.reg_spm_reserved_ddren_req_mask_b = 0,
	.reg_mcupm_srcclkena_mask_b = 1,
	.reg_mcupm_infra_req_mask_b = 1,
	.reg_mcupm_apsrc_req_mask_b = 1,
	.reg_mcupm_vrf18_req_mask_b = 1,
	.reg_mcupm_ddren_req_mask_b = 1,
	.reg_msdc0_srcclkena_mask_b = 1,
	.reg_msdc0_infra_req_mask_b = 1,
	.reg_msdc0_apsrc_req_mask_b = 1,
	.reg_msdc0_vrf18_req_mask_b = 1,
	.reg_msdc0_ddren_req_mask_b = 1,
	.reg_msdc1_srcclkena_mask_b = 1,
	.reg_msdc1_infra_req_mask_b = 1,
	.reg_msdc1_apsrc_req_mask_b = 1,
	.reg_msdc1_vrf18_req_mask_b = 1,
	.reg_msdc1_ddren_req_mask_b = 1,

	/* SPM_SRC4_MASK */
	.reg_ccif_event_srcclkena_mask_b = 0x3FF,
	.reg_bak_psri_srcclkena_mask_b = 0,
	.reg_bak_psri_infra_req_mask_b = 0,
	.reg_bak_psri_apsrc_req_mask_b = 0,
	.reg_bak_psri_vrf18_req_mask_b = 0,
	.reg_bak_psri_ddren_req_mask_b = 0,
	.reg_dramc_md32_infra_req_mask_b = 1,
	.reg_dramc_md32_vrf18_req_mask_b = 0,
	.reg_conn_srcclkenb2pwrap_mask_b = 0,
	.reg_dramc_md32_apsrc_req_mask_b = 0,

	/* SPM_SRC5_MASK */
	.reg_mcusys_merge_apsrc_req_mask_b = 0x14,
	.reg_mcusys_merge_ddren_req_mask_b = 0x14,
	.reg_afe_srcclkena_mask_b = 0,
	.reg_afe_infra_req_mask_b = 0,
	.reg_afe_apsrc_req_mask_b = 0,
	.reg_afe_vrf18_req_mask_b = 0,
	.reg_afe_ddren_req_mask_b = 0,
	.reg_msdc2_srcclkena_mask_b = 0,
	.reg_msdc2_infra_req_mask_b = 0,
	.reg_msdc2_apsrc_req_mask_b = 0,
	.reg_msdc2_vrf18_req_mask_b = 0,
	.reg_msdc2_ddren_req_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.reg_wakeup_event_mask = 0xEFFFFFFF,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/* SPM_SRC7_MASK */
	.reg_pcie_srcclkena_mask_b = 1,
	.reg_pcie_infra_req_mask_b = 1,
	.reg_pcie_apsrc_req_mask_b = 1,
	.reg_pcie_vrf18_req_mask_b = 1,
	.reg_pcie_ddren_req_mask_b = 1,
	.reg_dpmaif_srcclkena_mask_b = 1,
	.reg_dpmaif_infra_req_mask_b = 1,
	.reg_dpmaif_apsrc_req_mask_b = 1,
	.reg_dpmaif_vrf18_req_mask_b = 1,
	.reg_dpmaif_ddren_req_mask_b = 1,

	/* Auto-gen End */
};

struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl	= &vcorefs_ctrl,
};

static struct reg_config dvfsrc_init_configs[] = {
	{DVFSRC_HRT_REQ_UNIT,		0x0000001E},
	{DVFSRC_DEBOUNCE_TIME,		0x00001965},
	{DVFSRC_TIMEOUT_NEXTREQ,	0x00000015},
	{DVFSRC_VCORE_REQUEST4,		0x22211100},
	{DVFSRC_DDR_QOS0,		0x00000019},
	{DVFSRC_DDR_QOS1,		0x00000026},
	{DVFSRC_DDR_QOS2,		0x00000033},
	{DVFSRC_DDR_QOS3,		0x0000004C},
	{DVFSRC_DDR_QOS4,		0x00000066},
	{DVFSRC_DDR_QOS5,		0x00000077},
	{DVFSRC_DDR_QOS6,		0x00770077},
	{DVFSRC_LEVEL_LABEL_0_1,	0x40225032},
	{DVFSRC_LEVEL_LABEL_2_3,	0x20223012},
	{DVFSRC_LEVEL_LABEL_4_5,	0x40211012},
	{DVFSRC_LEVEL_LABEL_6_7,	0x20213011},
	{DVFSRC_LEVEL_LABEL_8_9,	0x30101011},
	{DVFSRC_LEVEL_LABEL_10_11,	0x10102000},
	{DVFSRC_LEVEL_LABEL_12_13,	0x00000000},
	{DVFSRC_LEVEL_LABEL_14_15,	0x00000000},
	{DVFSRC_LEVEL_LABEL_16_17,	0x00000000},
	{DVFSRC_LEVEL_LABEL_18_19,	0x00000000},
	{DVFSRC_LEVEL_LABEL_20_21,	0x00000000},
	{DVFSRC_LEVEL_MASK,		0x00000000},
	{DVFSRC_MD_LATENCY_IMPROVE,	0x00000020},
	{DVFSRC_HRT_BW_BASE,		0x00000004},
	{DVSFRC_HRT_REQ_MD_URG,		0x000D50D5},
	{DVFSRC_HRT_REQ_MD_BW_0,	0x00200802},
	{DVFSRC_HRT_REQ_MD_BW_1,	0x00200802},
	{DVFSRC_HRT_REQ_MD_BW_2,	0x00200800},
	{DVFSRC_HRT_REQ_MD_BW_3,	0x00400802},
	{DVFSRC_HRT_REQ_MD_BW_4,	0x00601404},
	{DVFSRC_HRT_REQ_MD_BW_5,	0x00D02C09},
	{DVFSRC_HRT_REQ_MD_BW_6,	0x00000012},
	{DVFSRC_HRT_REQ_MD_BW_7,	0x00000024},
	{DVFSRC_HRT_REQ_MD_BW_8,	0x00000000},
	{DVFSRC_HRT_REQ_MD_BW_9,	0x00000000},
	{DVFSRC_HRT_REQ_MD_BW_10,	0x00035400},
	{DVFSRC_HRT1_REQ_MD_BW_0,	0x04B12C4B},
	{DVFSRC_HRT1_REQ_MD_BW_1,	0x04B12C4B},
	{DVFSRC_HRT1_REQ_MD_BW_2,	0x04B12C00},
	{DVFSRC_HRT1_REQ_MD_BW_3,	0x04B12C4B},
	{DVFSRC_HRT1_REQ_MD_BW_4,	0x04B12C4B},
	{DVFSRC_HRT1_REQ_MD_BW_5,	0x04B12C4B},
	{DVFSRC_HRT1_REQ_MD_BW_6,	0x0000004B},
	{DVFSRC_HRT1_REQ_MD_BW_7,	0x0000005C},
	{DVFSRC_HRT1_REQ_MD_BW_8,	0x00000000},
	{DVFSRC_HRT1_REQ_MD_BW_9,	0x00000000},
	{DVFSRC_HRT1_REQ_MD_BW_10,	0x00035400},
	{DVFSRC_95MD_SCEN_BW0_T,	0x22222220},
	{DVFSRC_95MD_SCEN_BW1_T,	0x22222222},
	{DVFSRC_95MD_SCEN_BW2_T,	0x22222222},
	{DVFSRC_95MD_SCEN_BW3_T,	0x52222222},
	{DVFSRC_95MD_SCEN_BW4,		0x00000005},
	{DVFSRC_RSRV_5,			0x00000001},
#ifdef DVFSRC_1600_FLOOR
	{DVFSRC_DDR_REQUEST,		0x00000022},
#else
	{DVFSRC_DDR_REQUEST,		0x00000021},
#endif
	{DVFSRC_DDR_REQUEST3,		0x00554300},
	{DVFSRC_DDR_ADD_REQUEST,	0x55543210},
#ifdef DVFSRC_1600_FLOOR
	{DVFSRC_DDR_REQUEST5,		0x54322000},
#else
	{DVFSRC_DDR_REQUEST5,		0x54321000},
#endif
	{DVFSRC_DDR_REQUEST6,		0x53143130},
	{DVFSRC_DDR_REQUEST7,		0x55000000},
	{DVFSRC_DDR_REQUEST8,		0x05000000},
	{DVFSRC_EMI_MON_DEBOUNCE_TIME,	0x4C2D0000},
	{DVFSRC_EMI_ADD_REQUEST,	0x55543210},
	{DVFSRC_VCORE_USER_REQ,		0x00010A29},
	{DVFSRC_HRT_HIGH,		0x0E100960},
	{DVFSRC_HRT_HIGH_1,		0x1AD21700},
	{DVFSRC_HRT_HIGH_2,		0x314C2306},
	{DVFSRC_HRT_HIGH_3,		0x314C314C},
	{DVFSRC_HRT_LOW,		0x0E0F095F},
	{DVFSRC_HRT_LOW_1,		0x1AD116FF},
	{DVFSRC_HRT_LOW_2,		0x314B2305},
	{DVFSRC_HRT_LOW_3,		0x314B314B},
#ifdef DVFSRC_1600_FLOOR
	{DVFSRC_HRT_REQUEST,		0x55554322},
#else
	{DVFSRC_HRT_REQUEST,		0x55554321},
#endif
	{DVFSRC_BASIC_CONTROL_3,	0x0000000E},
	{DVFSRC_INT_EN,			0x00000002},
	{DVFSRC_QOS_EN,			0x001e407C},
	{DVFSRC_CURRENT_FORCE,		0x00000001},
	{DVFSRC_BASIC_CONTROL,		0x0180004B},
	{DVFSRC_BASIC_CONTROL,		0X0180404B},
	{DVFSRC_BASIC_CONTROL,		0X0180014B},
	{DVFSRC_CURRENT_FORCE,		0x00000000},
};

#define IS_PMIC_57() ((pmic_get_hwcid() >> 8) == 0x57)

static inline unsigned int vcore_base_uv(void)
{
	static unsigned int vb;

	if (vb == 0) {
		vb = IS_PMIC_57() ? 518750 : 500000;
	}

	return vb;
}

#define _VCORE_STEP_UV	(6250)

#define __vcore_uv_to_pmic(uv)	/* pmic >= uv */	\
	((((uv) - vcore_base_uv()) + (_VCORE_STEP_UV - 1)) / _VCORE_STEP_UV)

static int devinfo_table[] = {
	3539,   492,    1038,   106,    231,    17,     46,     2179,
	4,      481,    1014,   103,    225,    17,     45,     2129,
	3,      516,    1087,   111,    242,    19,     49,     2282,
	4,      504,    1063,   108,    236,    18,     47,     2230,
	4,      448,    946,    96,     210,    15,     41,     1986,
	2,      438,    924,    93,     205,    14,     40,     1941,
	2,      470,    991,    101,    220,    16,     43,     2080,
	3,      459,    968,    98,     215,    16,     42,     2033,
	3,      594,    1250,   129,    279,    23,     57,     2621,
	6,      580,    1221,   126,    273,    22,     56,     2561,
	6,      622,    1309,   136,    293,    24,     60,     2745,
	7,      608,    1279,   132,    286,    23,     59,     2683,
	6,      541,    1139,   117,    254,    20,     51,     2390,
	5,      528,    1113,   114,    248,    19,     50,     2335,
	4,      566,    1193,   123,    266,    21,     54,     2503,
	5,      553,    1166,   120,    260,    21,     53,     2446,
	5,      338,    715,    70,     157,    9,      29,     1505,
	3153,   330,    699,    69,     153,    9,      28,     1470,
	3081,   354,    750,    74,     165,    10,     31,     1576,
	3302,   346,    732,    72,     161,    10,     30,     1540,
	3227,   307,    652,    63,     142,    8,      26,     1371,
	2875,   300,    637,    62,     139,    7,      25,     1340,
	2809,   322,    683,    67,     149,    8,      27,     1436,
	3011,   315,    667,    65,     146,    8,      26,     1404,
	2942,   408,    862,    86,     191,    13,     37,     1811,
	1,      398,    842,    84,     186,    12,     36,     1769,
	1,      428,    903,    91,     200,    14,     39,     1896,
	2,      418,    882,    89,     195,    13,     38,     1853,
	2,      371,    785,    78,     173,    11,     33,     1651,
	3458,   363,    767,    76,     169,    10,     32,     1613,
	3379,   389,    823,    82,     182,    12,     35,     1729,
	1,      380,    804,    80,     177,    11,     34,     1689,
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
	mmio_write_32(OPP0_TABLE,   0xFFFF0000);
	mmio_write_32(OPP1_TABLE,   0xFFFF0100);
	mmio_write_32(OPP2_TABLE,   0xFFFF0300);
	mmio_write_32(OPP3_TABLE,   0xFFFF0500);
	mmio_write_32(OPP4_TABLE,   0xFFFF0700);
	mmio_write_32(OPP5_TABLE,   0xFFFF0202);
	mmio_write_32(OPP6_TABLE,   0xFFFF0302);
	mmio_write_32(OPP7_TABLE,   0xFFFF0502);
	mmio_write_32(OPP8_TABLE,   0xFFFF0702);
	mmio_write_32(OPP9_TABLE,   0xFFFF0403);
	mmio_write_32(OPP10_TABLE,  0xFFFF0603);
	mmio_write_32(OPP11_TABLE,  0xFFFF0803);
	mmio_write_32(OPP12_TABLE,  0xFFFF0903);
	mmio_write_32(OPP13_TABLE,  0xFFFFFFFF);
	mmio_write_32(OPP14_TABLE,  0xFFFFFFFF);
	mmio_write_32(OPP15_TABLE,  0xFFFFFFFF);
	mmio_write_32(OPP16_TABLE,  0xFFFFFFFF);
	mmio_write_32(OPP17_TABLE,  0xFFFFFFFF);
	mmio_write_32(SHU0_ARRAY,   0xFFFFFF00);
	mmio_write_32(SHU1_ARRAY,   0xFFFFEE01);
	mmio_write_32(SHU2_ARRAY,   0xFF05EEFF);
	mmio_write_32(SHU3_ARRAY,   0xFF06EE02);
	mmio_write_32(SHU4_ARRAY,   0x0906FFFF);
	mmio_write_32(SHU5_ARRAY,   0xFF07EE03);
	mmio_write_32(SHU6_ARRAY,   0x0A07FFFF);
	mmio_write_32(SHU7_ARRAY,   0xFF08EE04);
	mmio_write_32(SHU8_ARRAY,   0x0B08FFFF);
	mmio_write_32(SHU9_ARRAY,   0x0CFFFFFF);

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
			((dest_pwr_ctrl->pcm_flags_cust) & (~dvfs_mask)) |
			((src_pwr_ctrl->pcm_flags) & (dvfs_mask));
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

static void spm_vcorefs_vcore_setting(uint64_t flag)
{
	int idx, ptpod, rsv4;
	int power = 0;

	switch (flag) {
	case 1: /*HV*/
		vcore_opp_0_uv = 840000;
		vcore_opp_1_uv = 725000;
		vcore_opp_2_uv = 682500;
		break;
	case 2: /*LV*/
		vcore_opp_0_uv = 760000;
		vcore_opp_1_uv = 665000;
		vcore_opp_2_uv = 617500;
		break;
	default:
		break;
	}

	rsv4 = mmio_read_32(DVFSRC_RSRV_4);
	ptpod = (rsv4 >> VCORE_PTPOD_SHIFT) & 0xF;
	idx = (rsv4 >> VCORE_POWER_SHIFT) & 0xFF;

	if (idx != 0) {
		power = (int)devinfo_table[idx];
	}

	if (power > 0 && power <= 40) {
		idx = ptpod & 0xF;
		if (idx == 1) {
			vcore_opp_2_uv = 700000;
		} else if (idx > 1 && idx < 10) {
			vcore_opp_2_uv = 675000;
		}
	}

	spm_vcorefs_pwarp_cmd(3, __vcore_uv_to_pmic(vcore_opp_2_uv));
	spm_vcorefs_pwarp_cmd(2, __vcore_uv_to_pmic(vcore_opp_1_uv));
	spm_vcorefs_pwarp_cmd(0, __vcore_uv_to_pmic(vcore_opp_0_uv));
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
