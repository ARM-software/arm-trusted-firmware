/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_INTERNAL_H
#define MT_SPM_INTERNAL_H

#include <dbg_ctrl.h>
#include <inc/mt_spm_ver.h>
#include <mt_spm.h>
#include <mt_spm_stats.h>

/**************************************
 * Config and Parameter
 **************************************/
#define POWER_ON_VAL0_DEF 0x0000F100
/* SPM_POWER_ON_VAL1 */
#define POWER_ON_VAL1_DEF 0x003FFE20
/* SPM_WAKEUP_EVENT_MASK */
#define SPM_WAKEUP_EVENT_MASK_DEF 0xF97FFCFF

/* PCM_WDT_VAL */
#define PCM_WDT_TIMEOUT (30 * 32768) /* 30s */
/* AP WDT setting */
#define AP_WDT_TIMEOUT (31) /* 31s */
#define AP_WDT_TIMEOUT_SUSPEND (5400) /* 90min */
/* PCM_TIMER_VAL */
#define PCM_TIMER_SUSPEND \
	((AP_WDT_TIMEOUT_SUSPEND - 30) * 32768) /* 90min - 30sec */
#define PCM_TIMER_MAX (PCM_TIMER_SUSPEND)

/**************************************
 * Define and Declare
 **************************************/
/* MD32PCM ADDR for SPM code fetch */
#define MD32PCM_BASE (SPM_BASE + 0x0A00)
#define MD32PCM_CFGREG_SW_RSTN (MD32PCM_BASE + 0x0000)
#define MD32PCM_DMA0_SRC (MD32PCM_BASE + 0x0200)
#define MD32PCM_DMA0_DST (MD32PCM_BASE + 0x0204)
#define MD32PCM_DMA0_WPPT (MD32PCM_BASE + 0x0208)
#define MD32PCM_DMA0_WPTO (MD32PCM_BASE + 0x020C)
#define MD32PCM_DMA0_COUNT (MD32PCM_BASE + 0x0210)
#define MD32PCM_DMA0_CON (MD32PCM_BASE + 0x0214)
#define MD32PCM_DMA0_START (MD32PCM_BASE + 0x0218)
#define MD32PCM_DMA0_RLCT (MD32PCM_BASE + 0x0224)
#define MD32PCM_INTC_IRQ_RAW_STA (MD32PCM_BASE + 0x033C)

/* ABORT MASK for DEBUG FOORTPRINT */
#define DEBUG_ABORT_MASK                              \
	(SPM_DBG_DEBUG_IDX_DRAM_SREF_ABORT_IN_APSRC | \
	 SPM_DBG_DEBUG_IDX_DRAM_SREF_ABORT_IN_DDREN)

#define DEBUG_ABORT_MASK_1                               \
	(SPM_DBG1_DEBUG_IDX_VTCXO_SLEEP_ABORT_0 |        \
	 SPM_DBG1_DEBUG_IDX_VTCXO_SLEEP_ABORT_1 |        \
	 SPM_DBG1_DEBUG_IDX_PMIC_IRQ_ACK_LOW_ABORT |     \
	 SPM_DBG1_DEBUG_IDX_PMIC_IRQ_ACK_HIGH_ABORT |    \
	 SPM_DBG1_DEBUG_IDX_PWRAP_SLEEP_ACK_LOW_ABORT |  \
	 SPM_DBG1_DEBUG_IDX_PWRAP_SLEEP_ACK_HIGH_ABORT | \
	 SPM_DBG1_DEBUG_IDX_SCP_SLP_ACK_LOW_ABORT |      \
	 SPM_DBG1_DEBUG_IDX_SCP_SLP_ACK_HIGH_ABORT |     \
	 SPM_DBG1_DEBUG_IDX_SPM_PMIF_CMD_RDY_ABORT)

struct pwr_ctrl {
	/* for SPM */
	uint32_t pcm_flags;
	uint32_t pcm_flags_cust;
	uint32_t pcm_flags_cust_set;
	uint32_t pcm_flags_cust_clr;
	uint32_t pcm_flags1;
	uint32_t pcm_flags1_cust;
	uint32_t pcm_flags1_cust_set;
	uint32_t pcm_flags1_cust_clr;
	uint32_t timer_val;
	uint32_t timer_val_cust;
	uint32_t timer_val_ramp_en;
	uint32_t timer_val_ramp_en_sec;
	uint32_t wake_src;
	uint32_t wake_src_cust;
	uint32_t wakelock_timer_val;
	uint8_t wdt_disable;
	/* Auto-gen Start */

	/* SPM_CLK_CON */
	uint8_t reg_spm_lock_infra_dcm_lsb;
	uint8_t reg_cxo32k_remove_en_lsb;
	uint8_t reg_spm_leave_suspend_merge_mask_lsb;
	uint8_t reg_sysclk0_src_mask_b_lsb;
	uint8_t reg_sysclk1_src_mask_b_lsb;
	uint8_t reg_sysclk2_src_mask_b_lsb;

	/* SPM_AP_STANDBY_CON */
	uint8_t reg_wfi_op;
	uint8_t reg_wfi_type;
	uint8_t reg_mp0_cputop_idle_mask;
	uint8_t reg_mp1_cputop_idle_mask;
	uint8_t reg_mcusys_idle_mask;
	uint8_t reg_csyspwrup_req_mask_lsb;
	uint8_t reg_wfi_af_sel;
	uint8_t reg_cpu_sleep_wfi;

	/* SPM_SRC_REQ */
	uint8_t reg_spm_adsp_mailbox_req;
	uint8_t reg_spm_apsrc_req;
	uint8_t reg_spm_ddren_req;
	uint8_t reg_spm_dvfs_req;
	uint8_t reg_spm_emi_req;
	uint8_t reg_spm_f26m_req;
	uint8_t reg_spm_infra_req;
	uint8_t reg_spm_pmic_req;
	uint8_t reg_spm_scp_mailbox_req;
	uint8_t reg_spm_sspm_mailbox_req;
	uint8_t reg_spm_sw_mailbox_req;
	uint8_t reg_spm_vcore_req;
	uint8_t reg_spm_vrf18_req;
	uint8_t adsp_mailbox_state;
	uint8_t apsrc_state;
	uint8_t ddren_state;
	uint8_t dvfs_state;
	uint8_t emi_state;
	uint8_t f26m_state;
	uint8_t infra_state;
	uint8_t pmic_state;
	uint8_t scp_mailbox_state;
	uint8_t sspm_mailbox_state;
	uint8_t sw_mailbox_state;
	uint8_t vcore_state;
	uint8_t vrf18_state;

	/* SPM_SRC_MASK_0 */
	uint8_t reg_apu_apsrc_req_mask_b;
	uint8_t reg_apu_ddren_req_mask_b;
	uint8_t reg_apu_emi_req_mask_b;
	uint8_t reg_apu_infra_req_mask_b;
	uint8_t reg_apu_pmic_req_mask_b;
	uint8_t reg_apu_srcclkena_mask_b;
	uint8_t reg_apu_vrf18_req_mask_b;
	uint8_t reg_audio_dsp_apsrc_req_mask_b;
	uint8_t reg_audio_dsp_ddren_req_mask_b;
	uint8_t reg_audio_dsp_emi_req_mask_b;
	uint8_t reg_audio_dsp_infra_req_mask_b;
	uint8_t reg_audio_dsp_pmic_req_mask_b;
	uint8_t reg_audio_dsp_srcclkena_mask_b;
	uint8_t reg_audio_dsp_vcore_req_mask_b;
	uint8_t reg_audio_dsp_vrf18_req_mask_b;
	uint8_t reg_cam_apsrc_req_mask_b;
	uint8_t reg_cam_ddren_req_mask_b;
	uint8_t reg_cam_emi_req_mask_b;
	uint8_t reg_cam_infra_req_mask_b;
	uint8_t reg_cam_pmic_req_mask_b;
	uint8_t reg_cam_srcclkena_mask_b;
	uint8_t reg_cam_vrf18_req_mask_b;
	uint8_t reg_mdp_emi_req_mask_b;

	/* SPM_SRC_MASK_1 */
	uint32_t reg_ccif_apsrc_req_mask_b;
	uint32_t reg_ccif_emi_req_mask_b;

	/* SPM_SRC_MASK_2 */
	uint32_t reg_ccif_infra_req_mask_b;
	uint32_t reg_ccif_pmic_req_mask_b;

	/* SPM_SRC_MASK_3 */
	uint32_t reg_ccif_srcclkena_mask_b;
	uint32_t reg_ccif_vrf18_req_mask_b;
	uint8_t reg_ccu_apsrc_req_mask_b;
	uint8_t reg_ccu_ddren_req_mask_b;
	uint8_t reg_ccu_emi_req_mask_b;
	uint8_t reg_ccu_infra_req_mask_b;
	uint8_t reg_ccu_pmic_req_mask_b;
	uint8_t reg_ccu_srcclkena_mask_b;
	uint8_t reg_ccu_vrf18_req_mask_b;
	uint8_t reg_cg_check_apsrc_req_mask_b;

	/* SPM_SRC_MASK_4 */
	uint8_t reg_cg_check_ddren_req_mask_b;
	uint8_t reg_cg_check_emi_req_mask_b;
	uint8_t reg_cg_check_infra_req_mask_b;
	uint8_t reg_cg_check_pmic_req_mask_b;
	uint8_t reg_cg_check_srcclkena_mask_b;
	uint8_t reg_cg_check_vcore_req_mask_b;
	uint8_t reg_cg_check_vrf18_req_mask_b;
	uint8_t reg_conn_apsrc_req_mask_b;
	uint8_t reg_conn_ddren_req_mask_b;
	uint8_t reg_conn_emi_req_mask_b;
	uint8_t reg_conn_infra_req_mask_b;
	uint8_t reg_conn_pmic_req_mask_b;
	uint8_t reg_conn_srcclkena_mask_b;
	uint8_t reg_conn_srcclkenb_mask_b;
	uint8_t reg_conn_vcore_req_mask_b;
	uint8_t reg_conn_vrf18_req_mask_b;
	uint8_t reg_cpueb_apsrc_req_mask_b;
	uint8_t reg_cpueb_ddren_req_mask_b;
	uint8_t reg_cpueb_emi_req_mask_b;
	uint8_t reg_cpueb_infra_req_mask_b;
	uint8_t reg_cpueb_pmic_req_mask_b;
	uint8_t reg_cpueb_srcclkena_mask_b;
	uint8_t reg_cpueb_vrf18_req_mask_b;
	uint8_t reg_disp0_apsrc_req_mask_b;
	uint8_t reg_disp0_ddren_req_mask_b;
	uint8_t reg_disp0_emi_req_mask_b;
	uint8_t reg_disp0_infra_req_mask_b;
	uint8_t reg_disp0_pmic_req_mask_b;
	uint8_t reg_disp0_srcclkena_mask_b;
	uint8_t reg_disp0_vrf18_req_mask_b;
	uint8_t reg_disp1_apsrc_req_mask_b;
	uint8_t reg_disp1_ddren_req_mask_b;

	/* SPM_SRC_MASK_5 */
	uint8_t reg_disp1_emi_req_mask_b;
	uint8_t reg_disp1_infra_req_mask_b;
	uint8_t reg_disp1_pmic_req_mask_b;
	uint8_t reg_disp1_srcclkena_mask_b;
	uint8_t reg_disp1_vrf18_req_mask_b;
	uint8_t reg_dpm_apsrc_req_mask_b;
	uint8_t reg_dpm_ddren_req_mask_b;
	uint8_t reg_dpm_emi_req_mask_b;
	uint8_t reg_dpm_infra_req_mask_b;
	uint8_t reg_dpm_pmic_req_mask_b;
	uint8_t reg_dpm_srcclkena_mask_b;

	/* SPM_SRC_MASK_6 */
	uint8_t reg_dpm_vcore_req_mask_b;
	uint8_t reg_dpm_vrf18_req_mask_b;
	uint8_t reg_dpmaif_apsrc_req_mask_b;
	uint8_t reg_dpmaif_ddren_req_mask_b;
	uint8_t reg_dpmaif_emi_req_mask_b;
	uint8_t reg_dpmaif_infra_req_mask_b;
	uint8_t reg_dpmaif_pmic_req_mask_b;
	uint8_t reg_dpmaif_srcclkena_mask_b;
	uint8_t reg_dpmaif_vrf18_req_mask_b;
	uint8_t reg_dvfsrc_level_req_mask_b;
	uint8_t reg_emisys_apsrc_req_mask_b;
	uint8_t reg_emisys_ddren_req_mask_b;
	uint8_t reg_emisys_emi_req_mask_b;
	uint8_t reg_gce_d_apsrc_req_mask_b;
	uint8_t reg_gce_d_ddren_req_mask_b;
	uint8_t reg_gce_d_emi_req_mask_b;
	uint8_t reg_gce_d_infra_req_mask_b;
	uint8_t reg_gce_d_pmic_req_mask_b;
	uint8_t reg_gce_d_srcclkena_mask_b;
	uint8_t reg_gce_d_vrf18_req_mask_b;
	uint8_t reg_gce_m_apsrc_req_mask_b;
	uint8_t reg_gce_m_ddren_req_mask_b;
	uint8_t reg_gce_m_emi_req_mask_b;
	uint8_t reg_gce_m_infra_req_mask_b;
	uint8_t reg_gce_m_pmic_req_mask_b;
	uint8_t reg_gce_m_srcclkena_mask_b;

	/* SPM_SRC_MASK_7 */
	uint8_t reg_gce_m_vrf18_req_mask_b;
	uint8_t reg_gpueb_apsrc_req_mask_b;
	uint8_t reg_gpueb_ddren_req_mask_b;
	uint8_t reg_gpueb_emi_req_mask_b;
	uint8_t reg_gpueb_infra_req_mask_b;
	uint8_t reg_gpueb_pmic_req_mask_b;
	uint8_t reg_gpueb_srcclkena_mask_b;
	uint8_t reg_gpueb_vrf18_req_mask_b;
	uint8_t reg_hwccf_apsrc_req_mask_b;
	uint8_t reg_hwccf_ddren_req_mask_b;
	uint8_t reg_hwccf_emi_req_mask_b;
	uint8_t reg_hwccf_infra_req_mask_b;
	uint8_t reg_hwccf_pmic_req_mask_b;
	uint8_t reg_hwccf_srcclkena_mask_b;
	uint8_t reg_hwccf_vcore_req_mask_b;
	uint8_t reg_hwccf_vrf18_req_mask_b;
	uint8_t reg_img_apsrc_req_mask_b;
	uint8_t reg_img_ddren_req_mask_b;
	uint8_t reg_img_emi_req_mask_b;
	uint8_t reg_img_infra_req_mask_b;
	uint8_t reg_img_pmic_req_mask_b;
	uint8_t reg_img_srcclkena_mask_b;
	uint8_t reg_img_vrf18_req_mask_b;
	uint8_t reg_infrasys_apsrc_req_mask_b;
	uint8_t reg_infrasys_ddren_req_mask_b;
	uint8_t reg_infrasys_emi_req_mask_b;
	uint8_t reg_ipic_infra_req_mask_b;
	uint8_t reg_ipic_vrf18_req_mask_b;
	uint8_t reg_mcu_apsrc_req_mask_b;
	uint8_t reg_mcu_ddren_req_mask_b;
	uint8_t reg_mcu_emi_req_mask_b;

	/* SPM_SRC_MASK_8 */
	uint8_t reg_mcusys_apsrc_req_mask_b;
	uint8_t reg_mcusys_ddren_req_mask_b;
	uint8_t reg_mcusys_emi_req_mask_b;
	uint8_t reg_mcusys_infra_req_mask_b;

	/* SPM_SRC_MASK_9 */
	uint8_t reg_mcusys_pmic_req_mask_b;
	uint8_t reg_mcusys_srcclkena_mask_b;
	uint8_t reg_mcusys_vrf18_req_mask_b;
	uint8_t reg_md_apsrc_req_mask_b;
	uint8_t reg_md_ddren_req_mask_b;
	uint8_t reg_md_emi_req_mask_b;
	uint8_t reg_md_infra_req_mask_b;
	uint8_t reg_md_pmic_req_mask_b;
	uint8_t reg_md_srcclkena_mask_b;
	uint8_t reg_md_srcclkena1_mask_b;
	uint8_t reg_md_vcore_req_mask_b;

	/* SPM_SRC_MASK_10 */
	uint8_t reg_md_vrf18_req_mask_b;
	uint8_t reg_mdp_apsrc_req_mask_b;
	uint8_t reg_mdp_ddren_req_mask_b;
	uint8_t reg_mm_proc_apsrc_req_mask_b;
	uint8_t reg_mm_proc_ddren_req_mask_b;
	uint8_t reg_mm_proc_emi_req_mask_b;
	uint8_t reg_mm_proc_infra_req_mask_b;
	uint8_t reg_mm_proc_pmic_req_mask_b;
	uint8_t reg_mm_proc_srcclkena_mask_b;
	uint8_t reg_mm_proc_vrf18_req_mask_b;
	uint8_t reg_mmsys_apsrc_req_mask_b;
	uint8_t reg_mmsys_ddren_req_mask_b;
	uint8_t reg_mmsys_vrf18_req_mask_b;
	uint8_t reg_pcie0_apsrc_req_mask_b;
	uint8_t reg_pcie0_ddren_req_mask_b;
	uint8_t reg_pcie0_infra_req_mask_b;
	uint8_t reg_pcie0_srcclkena_mask_b;
	uint8_t reg_pcie0_vrf18_req_mask_b;
	uint8_t reg_pcie1_apsrc_req_mask_b;
	uint8_t reg_pcie1_ddren_req_mask_b;
	uint8_t reg_pcie1_infra_req_mask_b;
	uint8_t reg_pcie1_srcclkena_mask_b;
	uint8_t reg_pcie1_vrf18_req_mask_b;
	uint8_t reg_perisys_apsrc_req_mask_b;
	uint8_t reg_perisys_ddren_req_mask_b;
	uint8_t reg_perisys_emi_req_mask_b;
	uint8_t reg_perisys_infra_req_mask_b;
	uint8_t reg_perisys_pmic_req_mask_b;
	uint8_t reg_perisys_srcclkena_mask_b;
	uint8_t reg_perisys_vcore_req_mask_b;
	uint8_t reg_perisys_vrf18_req_mask_b;
	uint8_t reg_scp_apsrc_req_mask_b;

	/* SPM_SRC_MASK_11 */
	uint8_t reg_scp_ddren_req_mask_b;
	uint8_t reg_scp_emi_req_mask_b;
	uint8_t reg_scp_infra_req_mask_b;
	uint8_t reg_scp_pmic_req_mask_b;
	uint8_t reg_scp_srcclkena_mask_b;
	uint8_t reg_scp_vcore_req_mask_b;
	uint8_t reg_scp_vrf18_req_mask_b;
	uint8_t reg_srcclkeni_infra_req_mask_b;
	uint8_t reg_srcclkeni_pmic_req_mask_b;
	uint8_t reg_srcclkeni_srcclkena_mask_b;
	uint8_t reg_sspm_apsrc_req_mask_b;
	uint8_t reg_sspm_ddren_req_mask_b;
	uint8_t reg_sspm_emi_req_mask_b;
	uint8_t reg_sspm_infra_req_mask_b;
	uint8_t reg_sspm_pmic_req_mask_b;
	uint8_t reg_sspm_srcclkena_mask_b;
	uint8_t reg_sspm_vrf18_req_mask_b;
	uint8_t reg_ssr_apsrc_req_mask_b;
	uint8_t reg_ssr_ddren_req_mask_b;
	uint8_t reg_ssr_emi_req_mask_b;
	uint8_t reg_ssr_infra_req_mask_b;
	uint8_t reg_ssr_pmic_req_mask_b;
	uint8_t reg_ssr_srcclkena_mask_b;
	uint8_t reg_ssr_vrf18_req_mask_b;
	uint8_t reg_ufs_apsrc_req_mask_b;
	uint8_t reg_ufs_ddren_req_mask_b;
	uint8_t reg_ufs_emi_req_mask_b;
	uint8_t reg_ufs_infra_req_mask_b;
	uint8_t reg_ufs_pmic_req_mask_b;

	/* SPM_SRC_MASK_12 */
	uint8_t reg_ufs_srcclkena_mask_b;
	uint8_t reg_ufs_vrf18_req_mask_b;
	uint8_t reg_vdec_apsrc_req_mask_b;
	uint8_t reg_vdec_ddren_req_mask_b;
	uint8_t reg_vdec_emi_req_mask_b;
	uint8_t reg_vdec_infra_req_mask_b;
	uint8_t reg_vdec_pmic_req_mask_b;
	uint8_t reg_vdec_srcclkena_mask_b;
	uint8_t reg_vdec_vrf18_req_mask_b;
	uint8_t reg_venc_apsrc_req_mask_b;
	uint8_t reg_venc_ddren_req_mask_b;
	uint8_t reg_venc_emi_req_mask_b;
	uint8_t reg_venc_infra_req_mask_b;
	uint8_t reg_venc_pmic_req_mask_b;
	uint8_t reg_venc_srcclkena_mask_b;
	uint8_t reg_venc_vrf18_req_mask_b;
	uint8_t reg_ipe_apsrc_req_mask_b;
	uint8_t reg_ipe_ddren_req_mask_b;
	uint8_t reg_ipe_emi_req_mask_b;
	uint8_t reg_ipe_infra_req_mask_b;
	uint8_t reg_ipe_pmic_req_mask_b;
	uint8_t reg_ipe_srcclkena_mask_b;
	uint8_t reg_ipe_vrf18_req_mask_b;
	uint8_t reg_ufs_vcore_req_mask_b;

	/* SPM_EVENT_CON_MISC */
	uint8_t reg_srcclken_fast_resp;
	uint8_t reg_csyspwrup_ack_mask;

	/* SPM_WAKEUP_EVENT_MASK */
	uint32_t reg_wakeup_event_mask;

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	uint32_t reg_ext_wakeup_event_mask;

	/* Auto-gen End */
};

/* code gen by spm_pwr_ctrl_atf.pl, need struct pwr_ctrl */
enum pwr_ctrl_enum {
	PW_PCM_FLAGS,
	PW_PCM_FLAGS_CUST,
	PW_PCM_FLAGS_CUST_SET,
	PW_PCM_FLAGS_CUST_CLR,
	PW_PCM_FLAGS1,
	PW_PCM_FLAGS1_CUST,
	PW_PCM_FLAGS1_CUST_SET,
	PW_PCM_FLAGS1_CUST_CLR,
	PW_TIMER_VAL,
	PW_TIMER_VAL_CUST,
	PW_TIMER_VAL_RAMP_EN,
	PW_TIMER_VAL_RAMP_EN_SEC,
	PW_WAKE_SRC,
	PW_WAKE_SRC_CUST,
	PW_WAKELOCK_TIMER_VAL,
	PW_WDT_DISABLE,

	/* SPM_SRC_REQ */
	PW_REG_SPM_ADSP_MAILBOX_REQ,
	PW_REG_SPM_APSRC_REQ,
	PW_REG_SPM_DDREN_REQ,
	PW_REG_SPM_DVFS_REQ,
	PW_REG_SPM_EMI_REQ,
	PW_REG_SPM_F26M_REQ,
	PW_REG_SPM_INFRA_REQ,
	PW_REG_SPM_PMIC_REQ,
	PW_REG_SPM_SCP_MAILBOX_REQ,
	PW_REG_SPM_SSPM_MAILBOX_REQ,
	PW_REG_SPM_SW_MAILBOX_REQ,
	PW_REG_SPM_VCORE_REQ,
	PW_REG_SPM_VRF18_REQ,

	/* SPM_SRC_MASK_0 */
	PW_REG_APU_APSRC_REQ_MASK_B,
	PW_REG_APU_DDREN_REQ_MASK_B,
	PW_REG_APU_EMI_REQ_MASK_B,
	PW_REG_APU_INFRA_REQ_MASK_B,
	PW_REG_APU_PMIC_REQ_MASK_B,
	PW_REG_APU_SRCCLKENA_MASK_B,
	PW_REG_APU_VRF18_REQ_MASK_B,
	PW_REG_AUDIO_DSP_APSRC_REQ_MASK_B,
	PW_REG_AUDIO_DSP_DDREN_REQ_MASK_B,
	PW_REG_AUDIO_DSP_EMI_REQ_MASK_B,
	PW_REG_AUDIO_DSP_INFRA_REQ_MASK_B,
	PW_REG_AUDIO_DSP_PMIC_REQ_MASK_B,
	PW_REG_AUDIO_DSP_SRCCLKENA_MASK_B,
	PW_REG_AUDIO_DSP_VCORE_REQ_MASK_B,
	PW_REG_AUDIO_DSP_VRF18_REQ_MASK_B,
	PW_REG_CAM_APSRC_REQ_MASK_B,
	PW_REG_CAM_DDREN_REQ_MASK_B,
	PW_REG_CAM_EMI_REQ_MASK_B,
	PW_REG_CAM_INFRA_REQ_MASK_B,
	PW_REG_CAM_PMIC_REQ_MASK_B,
	PW_REG_CAM_SRCCLKENA_MASK_B,
	PW_REG_CAM_VRF18_REQ_MASK_B,
	PW_REG_MDP_EMI_REQ_MASK_B,

	/* SPM_SRC_MASK_1 */
	PW_REG_CCIF_APSRC_REQ_MASK_B,
	PW_REG_CCIF_EMI_REQ_MASK_B,

	/* SPM_SRC_MASK_2 */
	PW_REG_CCIF_INFRA_REQ_MASK_B,
	PW_REG_CCIF_PMIC_REQ_MASK_B,

	/* SPM_SRC_MASK_3 */
	PW_REG_CCIF_SRCCLKENA_MASK_B,
	PW_REG_CCIF_VRF18_REQ_MASK_B,
	PW_REG_CCU_APSRC_REQ_MASK_B,
	PW_REG_CCU_DDREN_REQ_MASK_B,
	PW_REG_CCU_EMI_REQ_MASK_B,
	PW_REG_CCU_INFRA_REQ_MASK_B,
	PW_REG_CCU_PMIC_REQ_MASK_B,
	PW_REG_CCU_SRCCLKENA_MASK_B,
	PW_REG_CCU_VRF18_REQ_MASK_B,
	PW_REG_CG_CHECK_APSRC_REQ_MASK_B,

	/* SPM_SRC_MASK_4 */
	PW_REG_CG_CHECK_DDREN_REQ_MASK_B,
	PW_REG_CG_CHECK_EMI_REQ_MASK_B,
	PW_REG_CG_CHECK_INFRA_REQ_MASK_B,
	PW_REG_CG_CHECK_PMIC_REQ_MASK_B,
	PW_REG_CG_CHECK_SRCCLKENA_MASK_B,
	PW_REG_CG_CHECK_VCORE_REQ_MASK_B,
	PW_REG_CG_CHECK_VRF18_REQ_MASK_B,
	PW_REG_CONN_APSRC_REQ_MASK_B,
	PW_REG_CONN_DDREN_REQ_MASK_B,
	PW_REG_CONN_EMI_REQ_MASK_B,
	PW_REG_CONN_INFRA_REQ_MASK_B,
	PW_REG_CONN_PMIC_REQ_MASK_B,
	PW_REG_CONN_SRCCLKENA_MASK_B,
	PW_REG_CONN_SRCCLKENB_MASK_B,
	PW_REG_CONN_VCORE_REQ_MASK_B,
	PW_REG_CONN_VRF18_REQ_MASK_B,
	PW_REG_CPUEB_APSRC_REQ_MASK_B,
	PW_REG_CPUEB_DDREN_REQ_MASK_B,
	PW_REG_CPUEB_EMI_REQ_MASK_B,
	PW_REG_CPUEB_INFRA_REQ_MASK_B,
	PW_REG_CPUEB_PMIC_REQ_MASK_B,
	PW_REG_CPUEB_SRCCLKENA_MASK_B,
	PW_REG_CPUEB_VRF18_REQ_MASK_B,
	PW_REG_DISP0_APSRC_REQ_MASK_B,
	PW_REG_DISP0_DDREN_REQ_MASK_B,
	PW_REG_DISP0_EMI_REQ_MASK_B,
	PW_REG_DISP0_INFRA_REQ_MASK_B,
	PW_REG_DISP0_PMIC_REQ_MASK_B,
	PW_REG_DISP0_SRCCLKENA_MASK_B,
	PW_REG_DISP0_VRF18_REQ_MASK_B,
	PW_REG_DISP1_APSRC_REQ_MASK_B,
	PW_REG_DISP1_DDREN_REQ_MASK_B,

	/* SPM_SRC_MASK_5 */
	PW_REG_DISP1_EMI_REQ_MASK_B,
	PW_REG_DISP1_INFRA_REQ_MASK_B,
	PW_REG_DISP1_PMIC_REQ_MASK_B,
	PW_REG_DISP1_SRCCLKENA_MASK_B,
	PW_REG_DISP1_VRF18_REQ_MASK_B,
	PW_REG_DPM_APSRC_REQ_MASK_B,
	PW_REG_DPM_DDREN_REQ_MASK_B,
	PW_REG_DPM_EMI_REQ_MASK_B,
	PW_REG_DPM_INFRA_REQ_MASK_B,
	PW_REG_DPM_PMIC_REQ_MASK_B,
	PW_REG_DPM_SRCCLKENA_MASK_B,

	/* SPM_SRC_MASK_6 */
	PW_REG_DPM_VCORE_REQ_MASK_B,
	PW_REG_DPM_VRF18_REQ_MASK_B,
	PW_REG_DPMAIF_APSRC_REQ_MASK_B,
	PW_REG_DPMAIF_DDREN_REQ_MASK_B,
	PW_REG_DPMAIF_EMI_REQ_MASK_B,
	PW_REG_DPMAIF_INFRA_REQ_MASK_B,
	PW_REG_DPMAIF_PMIC_REQ_MASK_B,
	PW_REG_DPMAIF_SRCCLKENA_MASK_B,
	PW_REG_DPMAIF_VRF18_REQ_MASK_B,
	PW_REG_DVFSRC_LEVEL_REQ_MASK_B,
	PW_REG_EMISYS_APSRC_REQ_MASK_B,
	PW_REG_EMISYS_DDREN_REQ_MASK_B,
	PW_REG_EMISYS_EMI_REQ_MASK_B,
	PW_REG_GCE_D_APSRC_REQ_MASK_B,
	PW_REG_GCE_D_DDREN_REQ_MASK_B,
	PW_REG_GCE_D_EMI_REQ_MASK_B,
	PW_REG_GCE_D_INFRA_REQ_MASK_B,
	PW_REG_GCE_D_PMIC_REQ_MASK_B,
	PW_REG_GCE_D_SRCCLKENA_MASK_B,
	PW_REG_GCE_D_VRF18_REQ_MASK_B,
	PW_REG_GCE_M_APSRC_REQ_MASK_B,
	PW_REG_GCE_M_DDREN_REQ_MASK_B,
	PW_REG_GCE_M_EMI_REQ_MASK_B,
	PW_REG_GCE_M_INFRA_REQ_MASK_B,
	PW_REG_GCE_M_PMIC_REQ_MASK_B,
	PW_REG_GCE_M_SRCCLKENA_MASK_B,

	/* SPM_SRC_MASK_7 */
	PW_REG_GCE_M_VRF18_REQ_MASK_B,
	PW_REG_GPUEB_APSRC_REQ_MASK_B,
	PW_REG_GPUEB_DDREN_REQ_MASK_B,
	PW_REG_GPUEB_EMI_REQ_MASK_B,
	PW_REG_GPUEB_INFRA_REQ_MASK_B,
	PW_REG_GPUEB_PMIC_REQ_MASK_B,
	PW_REG_GPUEB_SRCCLKENA_MASK_B,
	PW_REG_GPUEB_VRF18_REQ_MASK_B,
	PW_REG_HWCCF_APSRC_REQ_MASK_B,
	PW_REG_HWCCF_DDREN_REQ_MASK_B,
	PW_REG_HWCCF_EMI_REQ_MASK_B,
	PW_REG_HWCCF_INFRA_REQ_MASK_B,
	PW_REG_HWCCF_PMIC_REQ_MASK_B,
	PW_REG_HWCCF_SRCCLKENA_MASK_B,
	PW_REG_HWCCF_VCORE_REQ_MASK_B,
	PW_REG_HWCCF_VRF18_REQ_MASK_B,
	PW_REG_IMG_APSRC_REQ_MASK_B,
	PW_REG_IMG_DDREN_REQ_MASK_B,
	PW_REG_IMG_EMI_REQ_MASK_B,
	PW_REG_IMG_INFRA_REQ_MASK_B,
	PW_REG_IMG_PMIC_REQ_MASK_B,
	PW_REG_IMG_SRCCLKENA_MASK_B,
	PW_REG_IMG_VRF18_REQ_MASK_B,
	PW_REG_INFRASYS_APSRC_REQ_MASK_B,
	PW_REG_INFRASYS_DDREN_REQ_MASK_B,
	PW_REG_INFRASYS_EMI_REQ_MASK_B,
	PW_REG_IPIC_INFRA_REQ_MASK_B,
	PW_REG_IPIC_VRF18_REQ_MASK_B,
	PW_REG_MCU_APSRC_REQ_MASK_B,
	PW_REG_MCU_DDREN_REQ_MASK_B,
	PW_REG_MCU_EMI_REQ_MASK_B,

	/* SPM_SRC_MASK_8 */
	PW_REG_MCUSYS_APSRC_REQ_MASK_B,
	PW_REG_MCUSYS_DDREN_REQ_MASK_B,
	PW_REG_MCUSYS_EMI_REQ_MASK_B,
	PW_REG_MCUSYS_INFRA_REQ_MASK_B,

	/* SPM_SRC_MASK_9 */
	PW_REG_MCUSYS_PMIC_REQ_MASK_B,
	PW_REG_MCUSYS_SRCCLKENA_MASK_B,
	PW_REG_MCUSYS_VRF18_REQ_MASK_B,
	PW_REG_MD_APSRC_REQ_MASK_B,
	PW_REG_MD_DDREN_REQ_MASK_B,
	PW_REG_MD_EMI_REQ_MASK_B,
	PW_REG_MD_INFRA_REQ_MASK_B,
	PW_REG_MD_PMIC_REQ_MASK_B,
	PW_REG_MD_SRCCLKENA_MASK_B,
	PW_REG_MD_SRCCLKENA1_MASK_B,
	PW_REG_MD_VCORE_REQ_MASK_B,

	/* SPM_SRC_MASK_10 */
	PW_REG_MD_VRF18_REQ_MASK_B,
	PW_REG_MDP_APSRC_REQ_MASK_B,
	PW_REG_MDP_DDREN_REQ_MASK_B,
	PW_REG_MM_PROC_APSRC_REQ_MASK_B,
	PW_REG_MM_PROC_DDREN_REQ_MASK_B,
	PW_REG_MM_PROC_EMI_REQ_MASK_B,
	PW_REG_MM_PROC_INFRA_REQ_MASK_B,
	PW_REG_MM_PROC_PMIC_REQ_MASK_B,
	PW_REG_MM_PROC_SRCCLKENA_MASK_B,
	PW_REG_MM_PROC_VRF18_REQ_MASK_B,
	PW_REG_MMSYS_APSRC_REQ_MASK_B,
	PW_REG_MMSYS_DDREN_REQ_MASK_B,
	PW_REG_MMSYS_VRF18_REQ_MASK_B,
	PW_REG_PCIE0_APSRC_REQ_MASK_B,
	PW_REG_PCIE0_DDREN_REQ_MASK_B,
	PW_REG_PCIE0_INFRA_REQ_MASK_B,
	PW_REG_PCIE0_SRCCLKENA_MASK_B,
	PW_REG_PCIE0_VRF18_REQ_MASK_B,
	PW_REG_PCIE1_APSRC_REQ_MASK_B,
	PW_REG_PCIE1_DDREN_REQ_MASK_B,
	PW_REG_PCIE1_INFRA_REQ_MASK_B,
	PW_REG_PCIE1_SRCCLKENA_MASK_B,
	PW_REG_PCIE1_VRF18_REQ_MASK_B,
	PW_REG_PERISYS_APSRC_REQ_MASK_B,
	PW_REG_PERISYS_DDREN_REQ_MASK_B,
	PW_REG_PERISYS_EMI_REQ_MASK_B,
	PW_REG_PERISYS_INFRA_REQ_MASK_B,
	PW_REG_PERISYS_PMIC_REQ_MASK_B,
	PW_REG_PERISYS_SRCCLKENA_MASK_B,
	PW_REG_PERISYS_VCORE_REQ_MASK_B,
	PW_REG_PERISYS_VRF18_REQ_MASK_B,
	PW_REG_SCP_APSRC_REQ_MASK_B,

	/* SPM_SRC_MASK_11 */
	PW_REG_SCP_DDREN_REQ_MASK_B,
	PW_REG_SCP_EMI_REQ_MASK_B,
	PW_REG_SCP_INFRA_REQ_MASK_B,
	PW_REG_SCP_PMIC_REQ_MASK_B,
	PW_REG_SCP_SRCCLKENA_MASK_B,
	PW_REG_SCP_VCORE_REQ_MASK_B,
	PW_REG_SCP_VRF18_REQ_MASK_B,
	PW_REG_SRCCLKENI_INFRA_REQ_MASK_B,
	PW_REG_SRCCLKENI_PMIC_REQ_MASK_B,
	PW_REG_SRCCLKENI_SRCCLKENA_MASK_B,
	PW_REG_SSPM_APSRC_REQ_MASK_B,
	PW_REG_SSPM_DDREN_REQ_MASK_B,
	PW_REG_SSPM_EMI_REQ_MASK_B,
	PW_REG_SSPM_INFRA_REQ_MASK_B,
	PW_REG_SSPM_PMIC_REQ_MASK_B,
	PW_REG_SSPM_SRCCLKENA_MASK_B,
	PW_REG_SSPM_VRF18_REQ_MASK_B,
	PW_REG_SSR_APSRC_REQ_MASK_B,
	PW_REG_SSR_DDREN_REQ_MASK_B,
	PW_REG_SSR_EMI_REQ_MASK_B,
	PW_REG_SSR_INFRA_REQ_MASK_B,
	PW_REG_SSR_PMIC_REQ_MASK_B,
	PW_REG_SSR_SRCCLKENA_MASK_B,
	PW_REG_SSR_VRF18_REQ_MASK_B,
	PW_REG_UFS_APSRC_REQ_MASK_B,
	PW_REG_UFS_DDREN_REQ_MASK_B,
	PW_REG_UFS_EMI_REQ_MASK_B,
	PW_REG_UFS_INFRA_REQ_MASK_B,
	PW_REG_UFS_PMIC_REQ_MASK_B,

	/* SPM_SRC_MASK_12 */
	PW_REG_UFS_SRCCLKENA_MASK_B,
	PW_REG_UFS_VRF18_REQ_MASK_B,
	PW_REG_VDEC_APSRC_REQ_MASK_B,
	PW_REG_VDEC_DDREN_REQ_MASK_B,
	PW_REG_VDEC_EMI_REQ_MASK_B,
	PW_REG_VDEC_INFRA_REQ_MASK_B,
	PW_REG_VDEC_PMIC_REQ_MASK_B,
	PW_REG_VDEC_SRCCLKENA_MASK_B,
	PW_REG_VDEC_VRF18_REQ_MASK_B,
	PW_REG_VENC_APSRC_REQ_MASK_B,
	PW_REG_VENC_DDREN_REQ_MASK_B,
	PW_REG_VENC_EMI_REQ_MASK_B,
	PW_REG_VENC_INFRA_REQ_MASK_B,
	PW_REG_VENC_PMIC_REQ_MASK_B,
	PW_REG_VENC_SRCCLKENA_MASK_B,
	PW_REG_VENC_VRF18_REQ_MASK_B,
	PW_REG_IPE_APSRC_REQ_MASK_B,
	PW_REG_IPE_DDREN_REQ_MASK_B,
	PW_REG_IPE_EMI_REQ_MASK_B,
	PW_REG_IPE_INFRA_REQ_MASK_B,
	PW_REG_IPE_PMIC_REQ_MASK_B,
	PW_REG_IPE_SRCCLKENA_MASK_B,
	PW_REG_IPE_VRF18_REQ_MASK_B,
	PW_REG_UFS_VCORE_REQ_MASK_B,

	/* SPM_EVENT_CON_MISC */
	PW_REG_SRCCLKEN_FAST_RESP,
	PW_REG_CSYSPWRUP_ACK_MASK,

	/* SPM_WAKEUP_EVENT_MASK */
	PW_REG_WAKEUP_EVENT_MASK,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	PW_REG_EXT_WAKEUP_EVENT_MASK,

	PW_MAX_COUNT,
};

/*
 * HW_TARG_GROUP_SEL_3		: 3b'1 (pcm_reg_13)
 * HW_TARG_SIGNAL_SEL_3		: 5b'10101 (pcm_reg_13[21]=sc_emi_clk_off_ack_all)
 * HW_TRIG_GROUP_SEL_3		: 3'b100 (trig_reserve)
 * HW_TRIG_SIGNAL_SEL_3		: 5'b1100 (trig_reserve[24]=sc_hw_s1_req)
 */
#define SPM_ACK_CHK_3_SEL_HW_S1 (0x00350098)
#define SPM_ACK_CHK_3_HW_S1_CNT (1)

#define SPM_ACK_CHK_3_CON_HW_MODE_TRIG (0x800)
/* BIT[0]: SW_EN, BIT[4]: STA_EN, BIT[8]: HW_EN */
#define SPM_ACK_CHK_3_CON_EN (0x110)
#define SPM_ACK_CHK_3_CON_CLR_ALL (0x2)
/* BIT[15]: RESULT */
#define SPM_ACK_CHK_3_CON_RESULT (0x8000)

struct wake_status_trace_comm {
	uint32_t debug_flag; /* PCM_WDT_LATCH_SPARE_0 */
	uint32_t debug_flag1; /* PCM_WDT_LATCH_SPARE_1 */
	uint32_t timer_out; /* SPM_SW_RSV_6*/
	uint32_t b_sw_flag0; /* PCM_WDT_LATCH_SPARE_7 */
	uint32_t b_sw_flag1; /* PCM_WDT_LATCH_SPARE_5 */
	uint32_t r12; /* SPM_SW_RSV_0 */
	uint32_t r13; /* PCM_REG13_DATA */
	uint32_t req_sta0; /* SRC_REQ_STA_0 */
	uint32_t req_sta1; /* SRC_REQ_STA_1 */
	uint32_t req_sta2; /* SRC_REQ_STA_2 */
	uint32_t req_sta3; /* SRC_REQ_STA_3 */
	uint32_t req_sta4; /* SRC_REQ_STA_4 */
	uint32_t req_sta5; /* SRC_REQ_STA_5 */
	uint32_t req_sta6; /* SRC_REQ_STA_6 */
	uint32_t req_sta7; /* SRC_REQ_STA_7 */
	uint32_t req_sta8; /* SRC_REQ_STA_8 */
	uint32_t req_sta9; /* SRC_REQ_STA_9 */
	uint32_t req_sta10; /* SRC_REQ_STA_10 */
	uint32_t req_sta11; /* SRC_REQ_STA_11 */
	uint32_t req_sta12; /* SRC_REQ_STA_12 */
	uint32_t raw_sta; /* SPM_WAKEUP_STA */
	uint32_t times_h; /* timestamp high bits */
	uint32_t times_l; /* timestamp low bits */
	uint32_t resumetime; /* timestamp low bits */
};

struct wake_status_trace {
	/* Common part */
	struct wake_status_trace_comm comm;
	/* Add suspend or idle part bellow */
};

struct wake_status {
	struct wake_status_trace tr;
	uint32_t r12_ext; /* SPM_WAKEUP_EXT_STA */
	uint32_t raw_ext_sta; /* SPM_WAKEUP_EXT_STA */
	uint32_t md32pcm_wakeup_sta; /* MD32PCM_WAKEUP_STA */
	uint32_t md32pcm_event_sta; /* MD32PCM_EVENT_STA */
	uint32_t wake_misc; /* SPM_SW_RSV_5 */
	uint32_t sw_flag0; /* SPM_SW_FLAG_0 */
	uint32_t sw_flag1; /* SPM_SW_FLAG_1 */
	uint32_t isr; /* SPM_IRQ_STA */
	uint32_t log_index;
	uint32_t is_abort;
};

struct spm_lp_scen {
	struct pcm_desc *pcmdesc;
	struct pwr_ctrl *pwrctrl;
	struct dbg_ctrl *dbgctrl;
	struct spm_lp_stat *lpstat;
};

extern struct spm_lp_scen __spm_vcorefs;

void __spm_set_cpu_status(int cpu);
void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc);
void __spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc);

void __spm_init_pcm_register(void); /* init r0 and r7 */
void __spm_set_power_control(const struct pwr_ctrl *pwrctrl,
			     uint32_t resource_usage);
void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);
void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl);
void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl);
void __spm_send_cpu_wakeup_event(void);

void __spm_get_wakeup_status(struct wake_status *wakesta, uint32_t ext_status);
void __spm_clean_after_wakeup(void);
wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta);

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl,
					 const struct pwr_ctrl *src_pwr_ctrl);

void __spm_set_pcm_wdt(int en);
uint32_t __spm_get_pcm_timer_val(void);
uint32_t _spm_get_wake_period(int pwake_time, wake_reason_t last_wr);
void __spm_set_fw_resume_option(struct pwr_ctrl *pwrctrl);
void __spm_ext_int_wakeup_req_clr(void);

static inline void set_pwrctrl_pcm_flags(struct pwr_ctrl *pwrctrl,
					 uint32_t flags)
{
	if (!pwrctrl)
		return;

	if (pwrctrl->pcm_flags_cust == 0)
		pwrctrl->pcm_flags = flags;
	else
		pwrctrl->pcm_flags = pwrctrl->pcm_flags_cust;
}

static inline void set_pwrctrl_pcm_flags1(struct pwr_ctrl *pwrctrl,
					  uint32_t flags)
{
	if (!pwrctrl)
		return;

	if (pwrctrl->pcm_flags1_cust == 0)
		pwrctrl->pcm_flags1 = flags;
	else
		pwrctrl->pcm_flags1 = pwrctrl->pcm_flags1_cust;
}

void __spm_hw_s1_state_monitor(int en, uint32_t *status);

static inline void spm_hw_s1_state_monitor_resume(void)
{
	__spm_hw_s1_state_monitor(1, NULL);
}
static inline void spm_hw_s1_state_monitor_pause(uint32_t *status)
{
	__spm_hw_s1_state_monitor(0, status);
}

void __spm_clean_before_wfi(void);
int32_t __spm_wait_spm_request_ack(uint32_t spm_resource_req,
				   uint32_t timeout_us);
#endif /* MT_SPM_INTERNAL */
