/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_INTERNAL_H
#define MT_SPM_INTERNAL_H

#include <dbg_ctrl.h>
#include <mt_spm.h>
#include <mt_spm_stats.h>

/**************************************
 * Config and Parameter
 **************************************/
#define POWER_ON_VAL0_DEF	0x0000F100
/* SPM_POWER_ON_VAL1 */
#define POWER_ON_VAL1_DEF	0x003FFE20
/* SPM_WAKE_MASK*/
#define SPM_WAKEUP_EVENT_MASK_DEF	0xEFFFFFFF

#define PCM_WDT_TIMEOUT		(30 * 32768)    /* 30s */
#define PCM_TIMER_MAX		(0xFFFFFFFF)
/**************************************
 * Define and Declare
 **************************************/
/* MD32PCM ADDR for SPM code fetch */
#define MD32PCM_BASE				(SPM_BASE + 0x0A00)
#define MD32PCM_CFGREG_SW_RSTN			(MD32PCM_BASE + 0x0000)
#define MD32PCM_DMA0_SRC			(MD32PCM_BASE + 0x0200)
#define MD32PCM_DMA0_DST			(MD32PCM_BASE + 0x0204)
#define MD32PCM_DMA0_WPPT			(MD32PCM_BASE + 0x0208)
#define MD32PCM_DMA0_WPTO			(MD32PCM_BASE + 0x020C)
#define MD32PCM_DMA0_COUNT			(MD32PCM_BASE + 0x0210)
#define MD32PCM_DMA0_CON			(MD32PCM_BASE + 0x0214)
#define MD32PCM_DMA0_START			(MD32PCM_BASE + 0x0218)
#define MD32PCM_DMA0_RLCT			(MD32PCM_BASE + 0x0224)
#define MD32PCM_INTC_IRQ_RAW_STA		(MD32PCM_BASE + 0x033C)

/* ABORT MASK for DEBUG FOORTPRINT */
#define DEBUG_ABORT_MASK (SPM_DBG_DEBUG_IDX_DRAM_SREF_ABORT_IN_APSRC | \
			  SPM_DBG_DEBUG_IDX_DRAM_SREF_ABORT_IN_DDREN)

#define DEBUG_ABORT_MASK_1 (SPM_DBG1_DEBUG_IDX_VTCXO_SLEEP_ABORT_0 | \
			    SPM_DBG1_DEBUG_IDX_VTCXO_SLEEP_ABORT_1 | \
			    SPM_DBG1_DEBUG_IDX_VCORE_SLEEP_ABORT_0 | \
			    SPM_DBG1_DEBUG_IDX_VCORE_SLEEP_ABORT_1 | \
			    SPM_DBG1_DEBUG_IDX_PMIC_IRQ_ACK_LOW_ABORT | \
			    SPM_DBG1_DEBUG_IDX_PMIC_IRQ_ACK_HIGH_ABORT | \
			    SPM_DBG1_DEBUG_IDX_PWRAP_SLEEP_ACK_LOW_ABORT | \
			    SPM_DBG1_DEBUG_IDX_PWRAP_SLEEP_ACK_HIGH_ABORT | \
			    SPM_DBG1_DEBUG_IDX_SCP_SLP_ACK_LOW_ABORT | \
			    SPM_DBG1_DEBUG_IDX_SCP_SLP_ACK_HIGH_ABORT | \
			    SPM_DBG1_DEBUG_IDX_SPM_PMIF_CMD_RDY_ABORT)

struct pwr_ctrl {

	/* For SPM */
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
	uint8_t reg_sysclk0_src_mb_lsb;
	uint8_t reg_sysclk1_src_mb_lsb;
	uint8_t reg_sysclk2_src_mb_lsb;

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
	uint8_t reg_apifr_apsrc_rmb;
	uint8_t reg_apifr_ddren_rmb;
	uint8_t reg_apifr_emi_rmb;
	uint8_t reg_apifr_infra_rmb;
	uint8_t reg_apifr_pmic_rmb;
	uint8_t reg_apifr_srcclkena_mb;
	uint8_t reg_apifr_vcore_rmb;
	uint8_t reg_apifr_vrf18_rmb;
	uint8_t reg_apu_apsrc_rmb;
	uint8_t reg_apu_ddren_rmb;
	uint8_t reg_apu_emi_rmb;
	uint8_t reg_apu_infra_rmb;
	uint8_t reg_apu_pmic_rmb;
	uint8_t reg_apu_srcclkena_mb;
	uint8_t reg_apu_vcore_rmb;
	uint8_t reg_apu_vrf18_rmb;
	uint8_t reg_audio_apsrc_rmb;
	uint8_t reg_audio_ddren_rmb;
	uint8_t reg_audio_emi_rmb;
	uint8_t reg_audio_infra_rmb;
	uint8_t reg_audio_pmic_rmb;
	uint8_t reg_audio_srcclkena_mb;
	uint8_t reg_audio_vcore_rmb;
	uint8_t reg_audio_vrf18_rmb;

	/* SPM_SRC_MASK_1 */
	uint8_t reg_audio_dsp_apsrc_rmb;
	uint8_t reg_audio_dsp_ddren_rmb;
	uint8_t reg_audio_dsp_emi_rmb;
	uint8_t reg_audio_dsp_infra_rmb;
	uint8_t reg_audio_dsp_pmic_rmb;
	uint8_t reg_audio_dsp_srcclkena_mb;
	uint8_t reg_audio_dsp_vcore_rmb;
	uint8_t reg_audio_dsp_vrf18_rmb;
	uint8_t reg_cam_apsrc_rmb;
	uint8_t reg_cam_ddren_rmb;
	uint8_t reg_cam_emi_rmb;
	uint8_t reg_cam_infra_rmb;
	uint8_t reg_cam_pmic_rmb;
	uint8_t reg_cam_srcclkena_mb;
	uint8_t reg_cam_vrf18_rmb;
	uint32_t reg_ccif_apsrc_rmb;

	/* SPM_SRC_MASK_2 */
	uint32_t reg_ccif_emi_rmb;
	uint32_t reg_ccif_infra_rmb;

	/* SPM_SRC_MASK_3 */
	uint32_t reg_ccif_pmic_rmb;
	uint32_t reg_ccif_srcclkena_mb;

	/* SPM_SRC_MASK_4 */
	uint32_t reg_ccif_vcore_rmb;
	uint32_t reg_ccif_vrf18_rmb;
	uint8_t reg_ccu_apsrc_rmb;
	uint8_t reg_ccu_ddren_rmb;
	uint8_t reg_ccu_emi_rmb;
	uint8_t reg_ccu_infra_rmb;
	uint8_t reg_ccu_pmic_rmb;
	uint8_t reg_ccu_srcclkena_mb;
	uint8_t reg_ccu_vrf18_rmb;
	uint8_t reg_cg_check_apsrc_rmb;

	/* SPM_SRC_MASK_5 */
	uint8_t reg_cg_check_ddren_rmb;
	uint8_t reg_cg_check_emi_rmb;
	uint8_t reg_cg_check_infra_rmb;
	uint8_t reg_cg_check_pmic_rmb;
	uint8_t reg_cg_check_srcclkena_mb;
	uint8_t reg_cg_check_vcore_rmb;
	uint8_t reg_cg_check_vrf18_rmb;
	uint8_t reg_cksys_apsrc_rmb;
	uint8_t reg_cksys_ddren_rmb;
	uint8_t reg_cksys_emi_rmb;
	uint8_t reg_cksys_infra_rmb;
	uint8_t reg_cksys_pmic_rmb;
	uint8_t reg_cksys_srcclkena_mb;
	uint8_t reg_cksys_vcore_rmb;
	uint8_t reg_cksys_vrf18_rmb;
	uint8_t reg_cksys_1_apsrc_rmb;
	uint8_t reg_cksys_1_ddren_rmb;
	uint8_t reg_cksys_1_emi_rmb;
	uint8_t reg_cksys_1_infra_rmb;
	uint8_t reg_cksys_1_pmic_rmb;
	uint8_t reg_cksys_1_srcclkena_mb;
	uint8_t reg_cksys_1_vcore_rmb;
	uint8_t reg_cksys_1_vrf18_rmb;

	/* SPM_SRC_MASK_6 */
	uint8_t reg_cksys_2_apsrc_rmb;
	uint8_t reg_cksys_2_ddren_rmb;
	uint8_t reg_cksys_2_emi_rmb;
	uint8_t reg_cksys_2_infra_rmb;
	uint8_t reg_cksys_2_pmic_rmb;
	uint8_t reg_cksys_2_srcclkena_mb;
	uint8_t reg_cksys_2_vcore_rmb;
	uint8_t reg_cksys_2_vrf18_rmb;
	uint8_t reg_conn_apsrc_rmb;
	uint8_t reg_conn_ddren_rmb;
	uint8_t reg_conn_emi_rmb;
	uint8_t reg_conn_infra_rmb;
	uint8_t reg_conn_pmic_rmb;
	uint8_t reg_conn_srcclkena_mb;
	uint8_t reg_conn_srcclkenb_mb;
	uint8_t reg_conn_vcore_rmb;
	uint8_t reg_conn_vrf18_rmb;
	uint8_t reg_corecfg_apsrc_rmb;
	uint8_t reg_corecfg_ddren_rmb;
	uint8_t reg_corecfg_emi_rmb;
	uint8_t reg_corecfg_infra_rmb;
	uint8_t reg_corecfg_pmic_rmb;
	uint8_t reg_corecfg_srcclkena_mb;
	uint8_t reg_corecfg_vcore_rmb;
	uint8_t reg_corecfg_vrf18_rmb;

	/* SPM_SRC_MASK_7 */
	uint8_t reg_cpueb_apsrc_rmb;
	uint8_t reg_cpueb_ddren_rmb;
	uint8_t reg_cpueb_emi_rmb;
	uint8_t reg_cpueb_infra_rmb;
	uint8_t reg_cpueb_pmic_rmb;
	uint8_t reg_cpueb_srcclkena_mb;
	uint8_t reg_cpueb_vcore_rmb;
	uint8_t reg_cpueb_vrf18_rmb;
	uint8_t reg_disp0_apsrc_rmb;
	uint8_t reg_disp0_ddren_rmb;
	uint8_t reg_disp0_emi_rmb;
	uint8_t reg_disp0_infra_rmb;
	uint8_t reg_disp0_pmic_rmb;
	uint8_t reg_disp0_srcclkena_mb;
	uint8_t reg_disp0_vrf18_rmb;
	uint8_t reg_disp1_apsrc_rmb;
	uint8_t reg_disp1_ddren_rmb;
	uint8_t reg_disp1_emi_rmb;
	uint8_t reg_disp1_infra_rmb;
	uint8_t reg_disp1_pmic_rmb;
	uint8_t reg_disp1_srcclkena_mb;
	uint8_t reg_disp1_vrf18_rmb;
	uint8_t reg_dpm_apsrc_rmb;
	uint8_t reg_dpm_ddren_rmb;

	/* SPM_SRC_MASK_8 */
	uint8_t reg_dpm_emi_rmb;
	uint8_t reg_dpm_infra_rmb;
	uint8_t reg_dpm_pmic_rmb;
	uint8_t reg_dpm_srcclkena_mb;
	uint8_t reg_dpm_vcore_rmb;
	uint8_t reg_dpm_vrf18_rmb;
	uint8_t reg_dpmaif_apsrc_rmb;
	uint8_t reg_dpmaif_ddren_rmb;
	uint8_t reg_dpmaif_emi_rmb;
	uint8_t reg_dpmaif_infra_rmb;
	uint8_t reg_dpmaif_pmic_rmb;
	uint8_t reg_dpmaif_srcclkena_mb;
	uint8_t reg_dpmaif_vcore_rmb;
	uint8_t reg_dpmaif_vrf18_rmb;

	/* SPM_SRC_MASK_9 */
	uint8_t reg_dvfsrc_level_rmb;
	uint8_t reg_emisys_apsrc_rmb;
	uint8_t reg_emisys_ddren_rmb;
	uint8_t reg_emisys_emi_rmb;
	uint8_t reg_emisys_infra_rmb;
	uint8_t reg_emisys_pmic_rmb;
	uint8_t reg_emisys_srcclkena_mb;
	uint8_t reg_emisys_vcore_rmb;
	uint8_t reg_emisys_vrf18_rmb;
	uint8_t reg_gce_apsrc_rmb;
	uint8_t reg_gce_ddren_rmb;
	uint8_t reg_gce_emi_rmb;
	uint8_t reg_gce_infra_rmb;
	uint8_t reg_gce_pmic_rmb;
	uint8_t reg_gce_srcclkena_mb;
	uint8_t reg_gce_vcore_rmb;
	uint8_t reg_gce_vrf18_rmb;
	uint8_t reg_gpueb_apsrc_rmb;
	uint8_t reg_gpueb_ddren_rmb;
	uint8_t reg_gpueb_emi_rmb;
	uint8_t reg_gpueb_infra_rmb;
	uint8_t reg_gpueb_pmic_rmb;
	uint8_t reg_gpueb_srcclkena_mb;
	uint8_t reg_gpueb_vcore_rmb;
	uint8_t reg_gpueb_vrf18_rmb;
	uint8_t reg_hwccf_apsrc_rmb;
	uint8_t reg_hwccf_ddren_rmb;
	uint8_t reg_hwccf_emi_rmb;
	uint8_t reg_hwccf_infra_rmb;
	uint8_t reg_hwccf_pmic_rmb;
	uint8_t reg_hwccf_srcclkena_mb;
	uint8_t reg_hwccf_vcore_rmb;

	/* SPM_SRC_MASK_10 */
	uint8_t reg_hwccf_vrf18_rmb;
	uint8_t reg_img_apsrc_rmb;
	uint8_t reg_img_ddren_rmb;
	uint8_t reg_img_emi_rmb;
	uint8_t reg_img_infra_rmb;
	uint8_t reg_img_pmic_rmb;
	uint8_t reg_img_srcclkena_mb;
	uint8_t reg_img_vrf18_rmb;
	uint8_t reg_infrasys_apsrc_rmb;
	uint8_t reg_infrasys_ddren_rmb;
	uint8_t reg_infrasys_emi_rmb;
	uint8_t reg_infrasys_infra_rmb;
	uint8_t reg_infrasys_pmic_rmb;
	uint8_t reg_infrasys_srcclkena_mb;
	uint8_t reg_infrasys_vcore_rmb;
	uint8_t reg_infrasys_vrf18_rmb;
	uint8_t reg_ipic_infra_rmb;
	uint8_t reg_ipic_vrf18_rmb;
	uint8_t reg_mcu_apsrc_rmb;
	uint8_t reg_mcu_ddren_rmb;
	uint8_t reg_mcu_emi_rmb;
	uint8_t reg_mcu_infra_rmb;
	uint8_t reg_mcu_pmic_rmb;
	uint8_t reg_mcu_srcclkena_mb;
	uint8_t reg_mcu_vcore_rmb;
	uint8_t reg_mcu_vrf18_rmb;
	uint8_t reg_md_apsrc_rmb;
	uint8_t reg_md_ddren_rmb;
	uint8_t reg_md_emi_rmb;
	uint8_t reg_md_infra_rmb;
	uint8_t reg_md_pmic_rmb;
	uint8_t reg_md_srcclkena_mb;

	/* SPM_SRC_MASK_11 */
	uint8_t reg_md_srcclkena1_mb;
	uint8_t reg_md_vcore_rmb;
	uint8_t reg_md_vrf18_rmb;
	uint8_t reg_mm_proc_apsrc_rmb;
	uint8_t reg_mm_proc_ddren_rmb;
	uint8_t reg_mm_proc_emi_rmb;
	uint8_t reg_mm_proc_infra_rmb;
	uint8_t reg_mm_proc_pmic_rmb;
	uint8_t reg_mm_proc_srcclkena_mb;
	uint8_t reg_mm_proc_vcore_rmb;
	uint8_t reg_mm_proc_vrf18_rmb;
	uint8_t reg_mml0_apsrc_rmb;
	uint8_t reg_mml0_ddren_rmb;
	uint8_t reg_mml0_emi_rmb;
	uint8_t reg_mml0_infra_rmb;
	uint8_t reg_mml0_pmic_rmb;
	uint8_t reg_mml0_srcclkena_mb;
	uint8_t reg_mml0_vrf18_rmb;
	uint8_t reg_mml1_apsrc_rmb;
	uint8_t reg_mml1_ddren_rmb;
	uint8_t reg_mml1_emi_rmb;
	uint8_t reg_mml1_infra_rmb;
	uint8_t reg_mml1_pmic_rmb;
	uint8_t reg_mml1_srcclkena_mb;
	uint8_t reg_mml1_vrf18_rmb;
	uint8_t reg_ovl0_apsrc_rmb;
	uint8_t reg_ovl0_ddren_rmb;
	uint8_t reg_ovl0_emi_rmb;
	uint8_t reg_ovl0_infra_rmb;
	uint8_t reg_ovl0_pmic_rmb;
	uint8_t reg_ovl0_srcclkena_mb;
	uint8_t reg_ovl0_vrf18_rmb;

	/* SPM_SRC_MASK_12 */
	uint8_t reg_ovl1_apsrc_rmb;
	uint8_t reg_ovl1_ddren_rmb;
	uint8_t reg_ovl1_emi_rmb;
	uint8_t reg_ovl1_infra_rmb;
	uint8_t reg_ovl1_pmic_rmb;
	uint8_t reg_ovl1_srcclkena_mb;
	uint8_t reg_ovl1_vrf18_rmb;
	uint8_t reg_pcie0_apsrc_rmb;
	uint8_t reg_pcie0_ddren_rmb;
	uint8_t reg_pcie0_emi_rmb;
	uint8_t reg_pcie0_infra_rmb;
	uint8_t reg_pcie0_pmic_rmb;
	uint8_t reg_pcie0_srcclkena_mb;
	uint8_t reg_pcie0_vcore_rmb;
	uint8_t reg_pcie0_vrf18_rmb;
	uint8_t reg_pcie1_apsrc_rmb;
	uint8_t reg_pcie1_ddren_rmb;
	uint8_t reg_pcie1_emi_rmb;
	uint8_t reg_pcie1_infra_rmb;
	uint8_t reg_pcie1_pmic_rmb;
	uint8_t reg_pcie1_srcclkena_mb;
	uint8_t reg_pcie1_vcore_rmb;
	uint8_t reg_pcie1_vrf18_rmb;
	uint8_t reg_perisys_apsrc_rmb;
	uint8_t reg_perisys_ddren_rmb;
	uint8_t reg_perisys_emi_rmb;
	uint8_t reg_perisys_infra_rmb;
	uint8_t reg_perisys_pmic_rmb;
	uint8_t reg_perisys_srcclkena_mb;
	uint8_t reg_perisys_vcore_rmb;
	uint8_t reg_perisys_vrf18_rmb;
	uint8_t reg_pmsr_apsrc_rmb;

	/* SPM_SRC_MASK_13 */
	uint8_t reg_pmsr_ddren_rmb;
	uint8_t reg_pmsr_emi_rmb;
	uint8_t reg_pmsr_infra_rmb;
	uint8_t reg_pmsr_pmic_rmb;
	uint8_t reg_pmsr_srcclkena_mb;
	uint8_t reg_pmsr_vcore_rmb;
	uint8_t reg_pmsr_vrf18_rmb;
	uint8_t reg_scp_apsrc_rmb;
	uint8_t reg_scp_ddren_rmb;
	uint8_t reg_scp_emi_rmb;
	uint8_t reg_scp_infra_rmb;
	uint8_t reg_scp_pmic_rmb;
	uint8_t reg_scp_srcclkena_mb;
	uint8_t reg_scp_vcore_rmb;
	uint8_t reg_scp_vrf18_rmb;
	uint8_t reg_spu_hwr_apsrc_rmb;
	uint8_t reg_spu_hwr_ddren_rmb;
	uint8_t reg_spu_hwr_emi_rmb;
	uint8_t reg_spu_hwr_infra_rmb;
	uint8_t reg_spu_hwr_pmic_rmb;
	uint8_t reg_spu_hwr_srcclkena_mb;
	uint8_t reg_spu_hwr_vcore_rmb;
	uint8_t reg_spu_hwr_vrf18_rmb;
	uint8_t reg_spu_ise_apsrc_rmb;
	uint8_t reg_spu_ise_ddren_rmb;
	uint8_t reg_spu_ise_emi_rmb;
	uint8_t reg_spu_ise_infra_rmb;
	uint8_t reg_spu_ise_pmic_rmb;
	uint8_t reg_spu_ise_srcclkena_mb;
	uint8_t reg_spu_ise_vcore_rmb;
	uint8_t reg_spu_ise_vrf18_rmb;

	/* SPM_SRC_MASK_14 */
	uint8_t reg_srcclkeni_infra_rmb;
	uint8_t reg_srcclkeni_pmic_rmb;
	uint8_t reg_srcclkeni_srcclkena_mb;
	uint8_t reg_srcclkeni_vcore_rmb;
	uint8_t reg_sspm_apsrc_rmb;
	uint8_t reg_sspm_ddren_rmb;
	uint8_t reg_sspm_emi_rmb;
	uint8_t reg_sspm_infra_rmb;
	uint8_t reg_sspm_pmic_rmb;
	uint8_t reg_sspm_srcclkena_mb;
	uint8_t reg_sspm_vrf18_rmb;
	uint8_t reg_ssrsys_apsrc_rmb;
	uint8_t reg_ssrsys_ddren_rmb;
	uint8_t reg_ssrsys_emi_rmb;
	uint8_t reg_ssrsys_infra_rmb;
	uint8_t reg_ssrsys_pmic_rmb;
	uint8_t reg_ssrsys_srcclkena_mb;
	uint8_t reg_ssrsys_vcore_rmb;
	uint8_t reg_ssrsys_vrf18_rmb;
	uint8_t reg_ssusb_apsrc_rmb;
	uint8_t reg_ssusb_ddren_rmb;
	uint8_t reg_ssusb_emi_rmb;
	uint8_t reg_ssusb_infra_rmb;
	uint8_t reg_ssusb_pmic_rmb;
	uint8_t reg_ssusb_srcclkena_mb;
	uint8_t reg_ssusb_vcore_rmb;
	uint8_t reg_ssusb_vrf18_rmb;
	uint8_t reg_uart_hub_infra_rmb;

	/* SPM_SRC_MASK_15 */
	uint8_t reg_uart_hub_pmic_rmb;
	uint8_t reg_uart_hub_srcclkena_mb;
	uint8_t reg_uart_hub_vcore_rmb;
	uint8_t reg_uart_hub_vrf18_rmb;
	uint8_t reg_ufs_apsrc_rmb;
	uint8_t reg_ufs_ddren_rmb;
	uint8_t reg_ufs_emi_rmb;
	uint8_t reg_ufs_infra_rmb;
	uint8_t reg_ufs_pmic_rmb;
	uint8_t reg_ufs_srcclkena_mb;
	uint8_t reg_ufs_vcore_rmb;
	uint8_t reg_ufs_vrf18_rmb;
	uint8_t reg_vdec_apsrc_rmb;
	uint8_t reg_vdec_ddren_rmb;
	uint8_t reg_vdec_emi_rmb;
	uint8_t reg_vdec_infra_rmb;
	uint8_t reg_vdec_pmic_rmb;
	uint8_t reg_vdec_srcclkena_mb;
	uint8_t reg_vdec_vrf18_rmb;
	uint8_t reg_venc_apsrc_rmb;
	uint8_t reg_venc_ddren_rmb;
	uint8_t reg_venc_emi_rmb;
	uint8_t reg_venc_infra_rmb;
	uint8_t reg_venc_pmic_rmb;
	uint8_t reg_venc_srcclkena_mb;
	uint8_t reg_venc_vrf18_rmb;
	uint8_t reg_vlpcfg_apsrc_rmb;
	uint8_t reg_vlpcfg_ddren_rmb;
	uint8_t reg_vlpcfg_emi_rmb;
	uint8_t reg_vlpcfg_infra_rmb;
	uint8_t reg_vlpcfg_pmic_rmb;
	uint8_t reg_vlpcfg_srcclkena_mb;

	/* SPM_SRC_MASK_16 */
	uint8_t reg_vlpcfg_vcore_rmb;
	uint8_t reg_vlpcfg_vrf18_rmb;
	uint8_t reg_vlpcfg1_apsrc_rmb;
	uint8_t reg_vlpcfg1_ddren_rmb;
	uint8_t reg_vlpcfg1_emi_rmb;
	uint8_t reg_vlpcfg1_infra_rmb;
	uint8_t reg_vlpcfg1_pmic_rmb;
	uint8_t reg_vlpcfg1_srcclkena_mb;
	uint8_t reg_vlpcfg1_vcore_rmb;
	uint8_t reg_vlpcfg1_vrf18_rmb;

	/* SPM_EVENT_CON_MISC */
	uint8_t reg_srcclken_fast_resp;
	uint8_t reg_csyspwrup_ack_mask;

	/* SPM_SRC_MASK_17 */
	uint32_t reg_spm_sw_vcore_rmb;
	uint32_t reg_spm_sw_pmic_rmb;

	/* SPM_SRC_MASK_18 */
	uint32_t reg_spm_sw_srcclkena_mb;

	/* SPM_WAKE_MASK*/
	uint32_t reg_wake_mask;

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	uint32_t reg_ext_wake_mask;
};

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
	PW_REG_APIFR_APSRC_RMB,
	PW_REG_APIFR_DDREN_RMB,
	PW_REG_APIFR_EMI_RMB,
	PW_REG_APIFR_INFRA_RMB,
	PW_REG_APIFR_PMIC_RMB,
	PW_REG_APIFR_SRCCLKENA_MB,
	PW_REG_APIFR_VCORE_RMB,
	PW_REG_APIFR_VRF18_RMB,
	PW_REG_APU_APSRC_RMB,
	PW_REG_APU_DDREN_RMB,
	PW_REG_APU_EMI_RMB,
	PW_REG_APU_INFRA_RMB,
	PW_REG_APU_PMIC_RMB,
	PW_REG_APU_SRCCLKENA_MB,
	PW_REG_APU_VCORE_RMB,
	PW_REG_APU_VRF18_RMB,
	PW_REG_AUDIO_APSRC_RMB,
	PW_REG_AUDIO_DDREN_RMB,
	PW_REG_AUDIO_EMI_RMB,
	PW_REG_AUDIO_INFRA_RMB,
	PW_REG_AUDIO_PMIC_RMB,
	PW_REG_AUDIO_SRCCLKENA_MB,
	PW_REG_AUDIO_VCORE_RMB,
	PW_REG_AUDIO_VRF18_RMB,

	/* SPM_SRC_MASK_1 */
	PW_REG_AUDIO_DSP_APSRC_RMB,
	PW_REG_AUDIO_DSP_DDREN_RMB,
	PW_REG_AUDIO_DSP_EMI_RMB,
	PW_REG_AUDIO_DSP_INFRA_RMB,
	PW_REG_AUDIO_DSP_PMIC_RMB,
	PW_REG_AUDIO_DSP_SRCCLKENA_MB,
	PW_REG_AUDIO_DSP_VCORE_RMB,
	PW_REG_AUDIO_DSP_VRF18_RMB,
	PW_REG_CAM_APSRC_RMB,
	PW_REG_CAM_DDREN_RMB,
	PW_REG_CAM_EMI_RMB,
	PW_REG_CAM_INFRA_RMB,
	PW_REG_CAM_PMIC_RMB,
	PW_REG_CAM_SRCCLKENA_MB,
	PW_REG_CAM_VRF18_RMB,
	PW_REG_CCIF_APSRC_RMB,

	/* SPM_SRC_MASK_2 */
	PW_REG_CCIF_EMI_RMB,
	PW_REG_CCIF_INFRA_RMB,

	/* SPM_SRC_MASK_3 */
	PW_REG_CCIF_PMIC_RMB,
	PW_REG_CCIF_SRCCLKENA_MB,

	/* SPM_SRC_MASK_4 */
	PW_REG_CCIF_VCORE_RMB,
	PW_REG_CCIF_VRF18_RMB,
	PW_REG_CCU_APSRC_RMB,
	PW_REG_CCU_DDREN_RMB,
	PW_REG_CCU_EMI_RMB,
	PW_REG_CCU_INFRA_RMB,
	PW_REG_CCU_PMIC_RMB,
	PW_REG_CCU_SRCCLKENA_MB,
	PW_REG_CCU_VRF18_RMB,
	PW_REG_CG_CHECK_APSRC_RMB,

	/* SPM_SRC_MASK_5 */
	PW_REG_CG_CHECK_DDREN_RMB,
	PW_REG_CG_CHECK_EMI_RMB,
	PW_REG_CG_CHECK_INFRA_RMB,
	PW_REG_CG_CHECK_PMIC_RMB,
	PW_REG_CG_CHECK_SRCCLKENA_MB,
	PW_REG_CG_CHECK_VCORE_RMB,
	PW_REG_CG_CHECK_VRF18_RMB,
	PW_REG_CKSYS_APSRC_RMB,
	PW_REG_CKSYS_DDREN_RMB,
	PW_REG_CKSYS_EMI_RMB,
	PW_REG_CKSYS_INFRA_RMB,
	PW_REG_CKSYS_PMIC_RMB,
	PW_REG_CKSYS_SRCCLKENA_MB,
	PW_REG_CKSYS_VCORE_RMB,
	PW_REG_CKSYS_VRF18_RMB,
	PW_REG_CKSYS_1_APSRC_RMB,
	PW_REG_CKSYS_1_DDREN_RMB,
	PW_REG_CKSYS_1_EMI_RMB,
	PW_REG_CKSYS_1_INFRA_RMB,
	PW_REG_CKSYS_1_PMIC_RMB,
	PW_REG_CKSYS_1_SRCCLKENA_MB,
	PW_REG_CKSYS_1_VCORE_RMB,
	PW_REG_CKSYS_1_VRF18_RMB,

	/* SPM_SRC_MASK_6 */
	PW_REG_CKSYS_2_APSRC_RMB,
	PW_REG_CKSYS_2_DDREN_RMB,
	PW_REG_CKSYS_2_EMI_RMB,
	PW_REG_CKSYS_2_INFRA_RMB,
	PW_REG_CKSYS_2_PMIC_RMB,
	PW_REG_CKSYS_2_SRCCLKENA_MB,
	PW_REG_CKSYS_2_VCORE_RMB,
	PW_REG_CKSYS_2_VRF18_RMB,
	PW_REG_CONN_APSRC_RMB,
	PW_REG_CONN_DDREN_RMB,
	PW_REG_CONN_EMI_RMB,
	PW_REG_CONN_INFRA_RMB,
	PW_REG_CONN_PMIC_RMB,
	PW_REG_CONN_SRCCLKENA_MB,
	PW_REG_CONN_SRCCLKENB_MB,
	PW_REG_CONN_VCORE_RMB,
	PW_REG_CONN_VRF18_RMB,
	PW_REG_CORECFG_APSRC_RMB,
	PW_REG_CORECFG_DDREN_RMB,
	PW_REG_CORECFG_EMI_RMB,
	PW_REG_CORECFG_INFRA_RMB,
	PW_REG_CORECFG_PMIC_RMB,
	PW_REG_CORECFG_SRCCLKENA_MB,
	PW_REG_CORECFG_VCORE_RMB,
	PW_REG_CORECFG_VRF18_RMB,

	/* SPM_SRC_MASK_7 */
	PW_REG_CPUEB_APSRC_RMB,
	PW_REG_CPUEB_DDREN_RMB,
	PW_REG_CPUEB_EMI_RMB,
	PW_REG_CPUEB_INFRA_RMB,
	PW_REG_CPUEB_PMIC_RMB,
	PW_REG_CPUEB_SRCCLKENA_MB,
	PW_REG_CPUEB_VCORE_RMB,
	PW_REG_CPUEB_VRF18_RMB,
	PW_REG_DISP0_APSRC_RMB,
	PW_REG_DISP0_DDREN_RMB,
	PW_REG_DISP0_EMI_RMB,
	PW_REG_DISP0_INFRA_RMB,
	PW_REG_DISP0_PMIC_RMB,
	PW_REG_DISP0_SRCCLKENA_MB,
	PW_REG_DISP0_VRF18_RMB,
	PW_REG_DISP1_APSRC_RMB,
	PW_REG_DISP1_DDREN_RMB,
	PW_REG_DISP1_EMI_RMB,
	PW_REG_DISP1_INFRA_RMB,
	PW_REG_DISP1_PMIC_RMB,
	PW_REG_DISP1_SRCCLKENA_MB,
	PW_REG_DISP1_VRF18_RMB,
	PW_REG_DPM_APSRC_RMB,
	PW_REG_DPM_DDREN_RMB,

	/* SPM_SRC_MASK_8 */
	PW_REG_DPM_EMI_RMB,
	PW_REG_DPM_INFRA_RMB,
	PW_REG_DPM_PMIC_RMB,
	PW_REG_DPM_SRCCLKENA_MB,
	PW_REG_DPM_VCORE_RMB,
	PW_REG_DPM_VRF18_RMB,
	PW_REG_DPMAIF_APSRC_RMB,
	PW_REG_DPMAIF_DDREN_RMB,
	PW_REG_DPMAIF_EMI_RMB,
	PW_REG_DPMAIF_INFRA_RMB,
	PW_REG_DPMAIF_PMIC_RMB,
	PW_REG_DPMAIF_SRCCLKENA_MB,
	PW_REG_DPMAIF_VCORE_RMB,
	PW_REG_DPMAIF_VRF18_RMB,

	/* SPM_SRC_MASK_9 */
	PW_REG_DVFSRC_LEVEL_RMB,
	PW_REG_EMISYS_APSRC_RMB,
	PW_REG_EMISYS_DDREN_RMB,
	PW_REG_EMISYS_EMI_RMB,
	PW_REG_EMISYS_INFRA_RMB,
	PW_REG_EMISYS_PMIC_RMB,
	PW_REG_EMISYS_SRCCLKENA_MB,
	PW_REG_EMISYS_VCORE_RMB,
	PW_REG_EMISYS_VRF18_RMB,
	PW_REG_GCE_APSRC_RMB,
	PW_REG_GCE_DDREN_RMB,
	PW_REG_GCE_EMI_RMB,
	PW_REG_GCE_INFRA_RMB,
	PW_REG_GCE_PMIC_RMB,
	PW_REG_GCE_SRCCLKENA_MB,
	PW_REG_GCE_VCORE_RMB,
	PW_REG_GCE_VRF18_RMB,
	PW_REG_GPUEB_APSRC_RMB,
	PW_REG_GPUEB_DDREN_RMB,
	PW_REG_GPUEB_EMI_RMB,
	PW_REG_GPUEB_INFRA_RMB,
	PW_REG_GPUEB_PMIC_RMB,
	PW_REG_GPUEB_SRCCLKENA_MB,
	PW_REG_GPUEB_VCORE_RMB,
	PW_REG_GPUEB_VRF18_RMB,
	PW_REG_HWCCF_APSRC_RMB,
	PW_REG_HWCCF_DDREN_RMB,
	PW_REG_HWCCF_EMI_RMB,
	PW_REG_HWCCF_INFRA_RMB,
	PW_REG_HWCCF_PMIC_RMB,
	PW_REG_HWCCF_SRCCLKENA_MB,
	PW_REG_HWCCF_VCORE_RMB,

	/* SPM_SRC_MASK_10 */
	PW_REG_HWCCF_VRF18_RMB,
	PW_REG_IMG_APSRC_RMB,
	PW_REG_IMG_DDREN_RMB,
	PW_REG_IMG_EMI_RMB,
	PW_REG_IMG_INFRA_RMB,
	PW_REG_IMG_PMIC_RMB,
	PW_REG_IMG_SRCCLKENA_MB,
	PW_REG_IMG_VRF18_RMB,
	PW_REG_INFRASYS_APSRC_RMB,
	PW_REG_INFRASYS_DDREN_RMB,
	PW_REG_INFRASYS_EMI_RMB,
	PW_REG_INFRASYS_INFRA_RMB,
	PW_REG_INFRASYS_PMIC_RMB,
	PW_REG_INFRASYS_SRCCLKENA_MB,
	PW_REG_INFRASYS_VCORE_RMB,
	PW_REG_INFRASYS_VRF18_RMB,
	PW_REG_IPIC_INFRA_RMB,
	PW_REG_IPIC_VRF18_RMB,
	PW_REG_MCU_APSRC_RMB,
	PW_REG_MCU_DDREN_RMB,
	PW_REG_MCU_EMI_RMB,
	PW_REG_MCU_INFRA_RMB,
	PW_REG_MCU_PMIC_RMB,
	PW_REG_MCU_SRCCLKENA_MB,
	PW_REG_MCU_VCORE_RMB,
	PW_REG_MCU_VRF18_RMB,
	PW_REG_MD_APSRC_RMB,
	PW_REG_MD_DDREN_RMB,
	PW_REG_MD_EMI_RMB,
	PW_REG_MD_INFRA_RMB,
	PW_REG_MD_PMIC_RMB,
	PW_REG_MD_SRCCLKENA_MB,

	/* SPM_SRC_MASK_11 */
	PW_REG_MD_SRCCLKENA1_MB,
	PW_REG_MD_VCORE_RMB,
	PW_REG_MD_VRF18_RMB,
	PW_REG_MM_PROC_APSRC_RMB,
	PW_REG_MM_PROC_DDREN_RMB,
	PW_REG_MM_PROC_EMI_RMB,
	PW_REG_MM_PROC_INFRA_RMB,
	PW_REG_MM_PROC_PMIC_RMB,
	PW_REG_MM_PROC_SRCCLKENA_MB,
	PW_REG_MM_PROC_VCORE_RMB,
	PW_REG_MM_PROC_VRF18_RMB,
	PW_REG_MML0_APSRC_RMB,
	PW_REG_MML0_DDREN_RMB,
	PW_REG_MML0_EMI_RMB,
	PW_REG_MML0_INFRA_RMB,
	PW_REG_MML0_PMIC_RMB,
	PW_REG_MML0_SRCCLKENA_MB,
	PW_REG_MML0_VRF18_RMB,
	PW_REG_MML1_APSRC_RMB,
	PW_REG_MML1_DDREN_RMB,
	PW_REG_MML1_EMI_RMB,
	PW_REG_MML1_INFRA_RMB,
	PW_REG_MML1_PMIC_RMB,
	PW_REG_MML1_SRCCLKENA_MB,
	PW_REG_MML1_VRF18_RMB,
	PW_REG_OVL0_APSRC_RMB,
	PW_REG_OVL0_DDREN_RMB,
	PW_REG_OVL0_EMI_RMB,
	PW_REG_OVL0_INFRA_RMB,
	PW_REG_OVL0_PMIC_RMB,
	PW_REG_OVL0_SRCCLKENA_MB,
	PW_REG_OVL0_VRF18_RMB,

	/* SPM_SRC_MASK_12 */
	PW_REG_OVL1_APSRC_RMB,
	PW_REG_OVL1_DDREN_RMB,
	PW_REG_OVL1_EMI_RMB,
	PW_REG_OVL1_INFRA_RMB,
	PW_REG_OVL1_PMIC_RMB,
	PW_REG_OVL1_SRCCLKENA_MB,
	PW_REG_OVL1_VRF18_RMB,
	PW_REG_PCIE0_APSRC_RMB,
	PW_REG_PCIE0_DDREN_RMB,
	PW_REG_PCIE0_EMI_RMB,
	PW_REG_PCIE0_INFRA_RMB,
	PW_REG_PCIE0_PMIC_RMB,
	PW_REG_PCIE0_SRCCLKENA_MB,
	PW_REG_PCIE0_VCORE_RMB,
	PW_REG_PCIE0_VRF18_RMB,
	PW_REG_PCIE1_APSRC_RMB,
	PW_REG_PCIE1_DDREN_RMB,
	PW_REG_PCIE1_EMI_RMB,
	PW_REG_PCIE1_INFRA_RMB,
	PW_REG_PCIE1_PMIC_RMB,
	PW_REG_PCIE1_SRCCLKENA_MB,
	PW_REG_PCIE1_VCORE_RMB,
	PW_REG_PCIE1_VRF18_RMB,
	PW_REG_PERISYS_APSRC_RMB,
	PW_REG_PERISYS_DDREN_RMB,
	PW_REG_PERISYS_EMI_RMB,
	PW_REG_PERISYS_INFRA_RMB,
	PW_REG_PERISYS_PMIC_RMB,
	PW_REG_PERISYS_SRCCLKENA_MB,
	PW_REG_PERISYS_VCORE_RMB,
	PW_REG_PERISYS_VRF18_RMB,
	PW_REG_PMSR_APSRC_RMB,

	/* SPM_SRC_MASK_13 */
	PW_REG_PMSR_DDREN_RMB,
	PW_REG_PMSR_EMI_RMB,
	PW_REG_PMSR_INFRA_RMB,
	PW_REG_PMSR_PMIC_RMB,
	PW_REG_PMSR_SRCCLKENA_MB,
	PW_REG_PMSR_VCORE_RMB,
	PW_REG_PMSR_VRF18_RMB,
	PW_REG_SCP_APSRC_RMB,
	PW_REG_SCP_DDREN_RMB,
	PW_REG_SCP_EMI_RMB,
	PW_REG_SCP_INFRA_RMB,
	PW_REG_SCP_PMIC_RMB,
	PW_REG_SCP_SRCCLKENA_MB,
	PW_REG_SCP_VCORE_RMB,
	PW_REG_SCP_VRF18_RMB,
	PW_REG_SPU_HWR_APSRC_RMB,
	PW_REG_SPU_HWR_DDREN_RMB,
	PW_REG_SPU_HWR_EMI_RMB,
	PW_REG_SPU_HWR_INFRA_RMB,
	PW_REG_SPU_HWR_PMIC_RMB,
	PW_REG_SPU_HWR_SRCCLKENA_MB,
	PW_REG_SPU_HWR_VCORE_RMB,
	PW_REG_SPU_HWR_VRF18_RMB,
	PW_REG_SPU_ISE_APSRC_RMB,
	PW_REG_SPU_ISE_DDREN_RMB,
	PW_REG_SPU_ISE_EMI_RMB,
	PW_REG_SPU_ISE_INFRA_RMB,
	PW_REG_SPU_ISE_PMIC_RMB,
	PW_REG_SPU_ISE_SRCCLKENA_MB,
	PW_REG_SPU_ISE_VCORE_RMB,
	PW_REG_SPU_ISE_VRF18_RMB,

	/* SPM_SRC_MASK_14 */
	PW_REG_SRCCLKENI_INFRA_RMB,
	PW_REG_SRCCLKENI_PMIC_RMB,
	PW_REG_SRCCLKENI_SRCCLKENA_MB,
	PW_REG_SRCCLKENI_VCORE_RMB,
	PW_REG_SSPM_APSRC_RMB,
	PW_REG_SSPM_DDREN_RMB,
	PW_REG_SSPM_EMI_RMB,
	PW_REG_SSPM_INFRA_RMB,
	PW_REG_SSPM_PMIC_RMB,
	PW_REG_SSPM_SRCCLKENA_MB,
	PW_REG_SSPM_VRF18_RMB,
	PW_REG_SSRSYS_APSRC_RMB,
	PW_REG_SSRSYS_DDREN_RMB,
	PW_REG_SSRSYS_EMI_RMB,
	PW_REG_SSRSYS_INFRA_RMB,
	PW_REG_SSRSYS_PMIC_RMB,
	PW_REG_SSRSYS_SRCCLKENA_MB,
	PW_REG_SSRSYS_VCORE_RMB,
	PW_REG_SSRSYS_VRF18_RMB,
	PW_REG_SSUSB_APSRC_RMB,
	PW_REG_SSUSB_DDREN_RMB,
	PW_REG_SSUSB_EMI_RMB,
	PW_REG_SSUSB_INFRA_RMB,
	PW_REG_SSUSB_PMIC_RMB,
	PW_REG_SSUSB_SRCCLKENA_MB,
	PW_REG_SSUSB_VCORE_RMB,
	PW_REG_SSUSB_VRF18_RMB,
	PW_REG_UART_HUB_INFRA_RMB,

	/* SPM_SRC_MASK_15 */
	PW_REG_UART_HUB_PMIC_RMB,
	PW_REG_UART_HUB_SRCCLKENA_MB,
	PW_REG_UART_HUB_VCORE_RMB,
	PW_REG_UART_HUB_VRF18_RMB,
	PW_REG_UFS_APSRC_RMB,
	PW_REG_UFS_DDREN_RMB,
	PW_REG_UFS_EMI_RMB,
	PW_REG_UFS_INFRA_RMB,
	PW_REG_UFS_PMIC_RMB,
	PW_REG_UFS_SRCCLKENA_MB,
	PW_REG_UFS_VCORE_RMB,
	PW_REG_UFS_VRF18_RMB,
	PW_REG_VDEC_APSRC_RMB,
	PW_REG_VDEC_DDREN_RMB,
	PW_REG_VDEC_EMI_RMB,
	PW_REG_VDEC_INFRA_RMB,
	PW_REG_VDEC_PMIC_RMB,
	PW_REG_VDEC_SRCCLKENA_MB,
	PW_REG_VDEC_VRF18_RMB,
	PW_REG_VENC_APSRC_RMB,
	PW_REG_VENC_DDREN_RMB,
	PW_REG_VENC_EMI_RMB,
	PW_REG_VENC_INFRA_RMB,
	PW_REG_VENC_PMIC_RMB,
	PW_REG_VENC_SRCCLKENA_MB,
	PW_REG_VENC_VRF18_RMB,
	PW_REG_VLPCFG_APSRC_RMB,
	PW_REG_VLPCFG_DDREN_RMB,
	PW_REG_VLPCFG_EMI_RMB,
	PW_REG_VLPCFG_INFRA_RMB,
	PW_REG_VLPCFG_PMIC_RMB,
	PW_REG_VLPCFG_SRCCLKENA_MB,

	/* SPM_SRC_MASK_16 */
	PW_REG_VLPCFG_VCORE_RMB,
	PW_REG_VLPCFG_VRF18_RMB,
	PW_REG_VLPCFG1_APSRC_RMB,
	PW_REG_VLPCFG1_DDREN_RMB,
	PW_REG_VLPCFG1_EMI_RMB,
	PW_REG_VLPCFG1_INFRA_RMB,
	PW_REG_VLPCFG1_PMIC_RMB,
	PW_REG_VLPCFG1_SRCCLKENA_MB,
	PW_REG_VLPCFG1_VCORE_RMB,
	PW_REG_VLPCFG1_VRF18_RMB,

	/* SPM_EVENT_CON_MISC */
	PW_REG_SRCCLKEN_FAST_RESP,
	PW_REG_CSYSPWRUP_ACK_MASK,

	/* SPM_SRC_MASK_17 */
	PW_REG_SPM_SW_VCORE_RMB,
	PW_REG_SPM_SW_PMIC_RMB,

	/* SPM_SRC_MASK_18 */
	PW_REG_SPM_SW_SRCCLKENA_MB,

	/* SPM_WAKE_MASK*/
	PW_REG_WAKEUP_EVENT_MASK,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	PW_REG_EXT_WAKEUP_EVENT_MASK,

	PW_MAX_COUNT,
};

/*
 * HW_TARG_GROUP_SEL_3		: 3b'1 (pcm_reg_13)
 * HW_TARG_SIGNAL_SEL_3		: 5b'10101
 * HW_TRIG_GROUP_SEL_3		: 3'b100 (trig_reserve)
 * HW_TRIG_SIGNAL_SEL_3		: 5'b1100 (trig_reserve[24]=sc_hw_s1_req)
 */
#define SPM_ACK_CHK_3_SEL_HW_S1		(0x00350098)
#define SPM_ACK_CHK_3_HW_S1_CNT		(1)

#define SPM_ACK_CHK_3_CON_HW_MODE_TRIG	(0x800)
/* BIT[0]: SW_EN, BIT[4]: STA_EN, BIT[8]: HW_EN */
#define SPM_ACK_CHK_3_CON_EN		(0x110)
#define SPM_ACK_CHK_3_CON_CLR_ALL	(0x2)
/* BIT[15]: RESULT */
#define SPM_ACK_CHK_3_CON_RESULT	(0x8000)

struct wake_status_trace_comm {
	uint32_t debug_flag;			/* PCM_WDT_LATCH_SPARE_0 */
	uint32_t debug_flag1;			/* PCM_WDT_LATCH_SPARE_1 */
	uint32_t timer_out;			/* SPM_SW_RSV_6*/
	uint32_t b_sw_flag0;			/* SPM_SW_RSV_7 */
	uint32_t b_sw_flag1;			/* SPM_SW_RSV_7 */
	uint32_t r12;				/* SPM_SW_RSV_0 */
	uint32_t r13;				/* PCM_REG13_DATA */
	uint32_t req_sta0;			/* SRC_REQ_STA_0 */
	uint32_t req_sta1;			/* SRC_REQ_STA_1 */
	uint32_t req_sta2;			/* SRC_REQ_STA_2 */
	uint32_t req_sta3;			/* SRC_REQ_STA_3 */
	uint32_t req_sta4;			/* SRC_REQ_STA_4 */
	uint32_t req_sta5;			/* SRC_REQ_STA_5 */
	uint32_t req_sta6;			/* SRC_REQ_STA_6 */
	uint32_t req_sta7;			/* SRC_REQ_STA_7 */
	uint32_t req_sta8;			/* SRC_REQ_STA_8 */
	uint32_t req_sta9;			/* SRC_REQ_STA_9 */
	uint32_t req_sta10;			/* SRC_REQ_STA_10 */
	uint32_t req_sta11;			/* SRC_REQ_STA_11 */
	uint32_t req_sta12;			/* SRC_REQ_STA_12 */
	uint32_t req_sta13;			/* SRC_REQ_STA_13 */
	uint32_t req_sta14;			/* SRC_REQ_STA_14 */
	uint32_t req_sta15;			/* SRC_REQ_STA_15 */
	uint32_t req_sta16;			/* SRC_REQ_STA_16 */
	uint32_t raw_sta;			/* SPM_WAKEUP_STA */
	uint32_t times_h;			/* Timestamp high bits */
	uint32_t times_l;			/* Timestamp low bits */
	uint32_t resumetime;			/* Timestamp low bits */
};

struct wake_status_trace {
	struct wake_status_trace_comm comm;
	/* Add suspend or idle part bellow */
};

struct wake_status {
	struct wake_status_trace tr;
	uint32_t r12_ext;			/* SPM_WAKEUP_EXT_STA */
	uint32_t raw_ext_sta;			/* SPM_WAKEUP_EXT_STA */
	uint32_t md32pcm_wakeup_sta;		/* MD32PCM_WAKEUP_STA */
	uint32_t md32pcm_event_sta;		/* MD32PCM_EVENT_STA */
	uint32_t wake_misc;			/* SPM_SW_RSV_5 */
	uint32_t sw_flag0;			/* SPM_SW_FLAG_0 */
	uint32_t sw_flag1;			/* SPM_SW_FLAG_1 */
	uint32_t isr;				/* SPM_IRQ_STA */
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
typedef uint32_t u32;

void __spm_init_pcm_register(void);	/* init r0 and r7 */
void __spm_set_power_control(const struct pwr_ctrl *pwrctrl,
			     uint32_t resource_usage);
void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);
void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl);
void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl);
void __spm_send_cpu_wakeup_event(void);

void __spm_get_wakeup_status(struct wake_status *wakesta,
			     uint32_t ext_status);
void __spm_clean_after_wakeup(void);
wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta);

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl,
					 const struct pwr_ctrl *src_pwr_ctrl);
void __spm_sync_vcore_dvfs_pcm_flags(uint32_t *dest_pcm_flags,
				     const uint32_t *src_pcm_flags);

void __spm_set_pcm_wdt(int en);
uint32_t __spm_get_pcm_timer_val(void);
uint32_t _spm_get_wake_period(int pwake_time, wake_reason_t last_wr);
void __spm_set_fw_resume_option(struct pwr_ctrl *pwrctrl);
void __spm_ext_int_wakeup_req_clr(void);

static inline void set_pwrctrl_pcm_flags(struct pwr_ctrl *pwrctrl,
					 uint32_t flags)
{
	if (pwrctrl->pcm_flags_cust == 0)
		pwrctrl->pcm_flags = flags;
	else
		pwrctrl->pcm_flags = pwrctrl->pcm_flags_cust;
}

static inline void set_pwrctrl_pcm_flags1(struct pwr_ctrl *pwrctrl,
					  uint32_t flags)
{
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

int32_t __spm_wait_spm_request_ack(uint32_t spm_resource_req,
				   uint32_t timeout_us);

#endif /* MT_SPM_INTERNAL */
