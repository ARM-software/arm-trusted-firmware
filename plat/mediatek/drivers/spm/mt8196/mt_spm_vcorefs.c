/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <drivers/spmi_api.h>
#include <lib/pm/mtk_pm.h>
#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <pmic_wrap/inc/mt_spm_pmic_wrap.h>

#define VCORE_BASE_UV		0
#ifdef MT8196_VCORE_SUPPORT
#define VCORE_STEP_UV		6250
#else
#define VCORE_STEP_UV		5000
#endif

#define VCORE_UV_TO_PMIC(uv)	/* pmic >= uv */	\
	((((uv) - VCORE_BASE_UV) + (VCORE_STEP_UV - 1)) / VCORE_STEP_UV)
#define VCORE_PMIC_TO_UV(pmic)	\
	(((pmic) * VCORE_STEP_UV) + VCORE_BASE_UV)

#define VSRAM_BASE_UV		0
#define VSRAM_STEP_UV		6250

#define VSRAM_UV_TO_PMIC(uv)	/* pmic >= uv */	\
	((((uv) - VSRAM_BASE_UV) + (VSRAM_STEP_UV - 1)) / VSRAM_STEP_UV)
#define VSRAM_PMIC_TO_UV(pmic)	\
	(((pmic) * VSRAM_STEP_UV) + VSRAM_BASE_UV)

static int spm_dvfs_init_done;

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= R12_CPU_WAKEUP,

	/* default VCORE DVFS is disabled */
	.pcm_flags = (SPM_FLAG_RUN_COMMON_SCENARIO |
		SPM_FLAG_DISABLE_VCORE_DVS |
		SPM_FLAG_DISABLE_DDR_DFS |
		SPM_FLAG_DISABLE_EMI_DFS | SPM_FLAG_DISABLE_BUS_DFS),

	/* SPM_SRC_REQ */
	.reg_spm_adsp_mailbox_req = 0,
	.reg_spm_apsrc_req = 0,
	.reg_spm_ddren_req = 0,
	.reg_spm_dvfs_req = 0,
	.reg_spm_emi_req = 0,
	.reg_spm_f26m_req = 0,
	.reg_spm_infra_req = 0,
	.reg_spm_pmic_req = 0,
	.reg_spm_scp_mailbox_req = 0,
	.reg_spm_sspm_mailbox_req = 0,
	.reg_spm_sw_mailbox_req = 0,
	.reg_spm_vcore_req = 1,
	.reg_spm_vrf18_req = 0,
	.adsp_mailbox_state = 0,
	.apsrc_state = 0,
	.ddren_state = 0,
	.dvfs_state = 0,
	.emi_state = 0,
	.f26m_state = 0,
	.infra_state = 0,
	.pmic_state = 0,
	.scp_mailbox_state = 0,
	.sspm_mailbox_state = 0,
	.sw_mailbox_state = 0,
	.vcore_state = 0,
	.vrf18_state = 0,

	/* SPM_SRC_MASK_0 */
	.reg_apifr_apsrc_rmb = 0,
	.reg_apifr_ddren_rmb = 0,
	.reg_apifr_emi_rmb = 0,
	.reg_apifr_infra_rmb = 0,
	.reg_apifr_pmic_rmb = 0,
	.reg_apifr_srcclkena_mb = 0,
	.reg_apifr_vcore_rmb = 0,
	.reg_apifr_vrf18_rmb = 0,
	.reg_apu_apsrc_rmb = 1,
	.reg_apu_ddren_rmb = 0,
	.reg_apu_emi_rmb = 1,
	.reg_apu_infra_rmb = 1,
	.reg_apu_pmic_rmb = 1,
	.reg_apu_srcclkena_mb = 1,
	.reg_apu_vcore_rmb = 1,
	.reg_apu_vrf18_rmb = 1,
	.reg_audio_apsrc_rmb = 1,
	.reg_audio_ddren_rmb = 0,
	.reg_audio_emi_rmb = 1,
	.reg_audio_infra_rmb = 1,
	.reg_audio_pmic_rmb = 1,
	.reg_audio_srcclkena_mb = 1,
	.reg_audio_vcore_rmb = 1,
	.reg_audio_vrf18_rmb = 1,

	/* SPM_SRC_MASK_1 */
	.reg_audio_dsp_apsrc_rmb = 1,
	.reg_audio_dsp_ddren_rmb = 0,
	.reg_audio_dsp_emi_rmb = 1,
	.reg_audio_dsp_infra_rmb = 1,
	.reg_audio_dsp_pmic_rmb = 1,
	.reg_audio_dsp_srcclkena_mb = 1,
	.reg_audio_dsp_vcore_rmb = 1,
	.reg_audio_dsp_vrf18_rmb = 1,
	.reg_cam_apsrc_rmb = 0,
	.reg_cam_ddren_rmb = 0,
	.reg_cam_emi_rmb = 0,
	.reg_cam_infra_rmb = 0,
	.reg_cam_pmic_rmb = 0,
	.reg_cam_srcclkena_mb = 0,
	.reg_cam_vrf18_rmb = 0,
	.reg_ccif_apsrc_rmb = 0xfff,

	/* SPM_SRC_MASK_2 */
	.reg_ccif_emi_rmb = 0xfff,
	.reg_ccif_infra_rmb = 0xfff,

	/* SPM_SRC_MASK_3 */
	.reg_ccif_pmic_rmb = 0xfff,
	.reg_ccif_srcclkena_mb = 0xfff,

	/* SPM_SRC_MASK_4 */
	.reg_ccif_vcore_rmb = 0xfff,
	.reg_ccif_vrf18_rmb = 0xfff,
	.reg_ccu_apsrc_rmb = 0,
	.reg_ccu_ddren_rmb = 0,
	.reg_ccu_emi_rmb = 0,
	.reg_ccu_infra_rmb = 0,
	.reg_ccu_pmic_rmb = 0,
	.reg_ccu_srcclkena_mb = 0,
	.reg_ccu_vrf18_rmb = 0,
	.reg_cg_check_apsrc_rmb = 0,

	/* SPM_SRC_MASK_5 */
	.reg_cg_check_ddren_rmb = 0,
	.reg_cg_check_emi_rmb = 0,
	.reg_cg_check_infra_rmb = 0,
	.reg_cg_check_pmic_rmb = 0,
	.reg_cg_check_srcclkena_mb = 0,
	.reg_cg_check_vcore_rmb = 1,
	.reg_cg_check_vrf18_rmb = 0,
	.reg_cksys_apsrc_rmb = 1,
	.reg_cksys_ddren_rmb = 0,
	.reg_cksys_emi_rmb = 1,
	.reg_cksys_infra_rmb = 1,
	.reg_cksys_pmic_rmb = 1,
	.reg_cksys_srcclkena_mb = 1,
	.reg_cksys_vcore_rmb = 1,
	.reg_cksys_vrf18_rmb = 1,
	.reg_cksys_1_apsrc_rmb = 1,
	.reg_cksys_1_ddren_rmb = 0,
	.reg_cksys_1_emi_rmb = 1,
	.reg_cksys_1_infra_rmb = 1,
	.reg_cksys_1_pmic_rmb = 1,
	.reg_cksys_1_srcclkena_mb = 1,
	.reg_cksys_1_vcore_rmb = 1,
	.reg_cksys_1_vrf18_rmb = 1,

	/* SPM_SRC_MASK_6 */
	.reg_cksys_2_apsrc_rmb = 1,
	.reg_cksys_2_ddren_rmb = 0,
	.reg_cksys_2_emi_rmb = 1,
	.reg_cksys_2_infra_rmb = 1,
	.reg_cksys_2_pmic_rmb = 1,
	.reg_cksys_2_srcclkena_mb = 1,
	.reg_cksys_2_vcore_rmb = 1,
	.reg_cksys_2_vrf18_rmb = 1,
	.reg_conn_apsrc_rmb = 1,
	.reg_conn_ddren_rmb = 0,
	.reg_conn_emi_rmb = 1,
	.reg_conn_infra_rmb = 1,
	.reg_conn_pmic_rmb = 1,
	.reg_conn_srcclkena_mb = 1,
	.reg_conn_srcclkenb_mb = 1,
	.reg_conn_vcore_rmb = 1,
	.reg_conn_vrf18_rmb = 1,
	.reg_corecfg_apsrc_rmb = 0,
	.reg_corecfg_ddren_rmb = 0,
	.reg_corecfg_emi_rmb = 0,
	.reg_corecfg_infra_rmb = 0,
	.reg_corecfg_pmic_rmb = 0,
	.reg_corecfg_srcclkena_mb = 0,
	.reg_corecfg_vcore_rmb = 0,
	.reg_corecfg_vrf18_rmb = 0,

	/* SPM_SRC_MASK_7 */
	.reg_cpueb_apsrc_rmb = 1,
	.reg_cpueb_ddren_rmb = 0,
	.reg_cpueb_emi_rmb = 1,
	.reg_cpueb_infra_rmb = 1,
	.reg_cpueb_pmic_rmb = 1,
	.reg_cpueb_srcclkena_mb = 1,
	.reg_cpueb_vcore_rmb = 1,
	.reg_cpueb_vrf18_rmb = 1,
	.reg_disp0_apsrc_rmb = 0,
	.reg_disp0_ddren_rmb = 0,
	.reg_disp0_emi_rmb = 0,
	.reg_disp0_infra_rmb = 0,
	.reg_disp0_pmic_rmb = 0,
	.reg_disp0_srcclkena_mb = 0,
	.reg_disp0_vrf18_rmb = 0,
	.reg_disp1_apsrc_rmb = 0,
	.reg_disp1_ddren_rmb = 0,
	.reg_disp1_emi_rmb = 0,
	.reg_disp1_infra_rmb = 0,
	.reg_disp1_pmic_rmb = 0,
	.reg_disp1_srcclkena_mb = 0,
	.reg_disp1_vrf18_rmb = 0,
	.reg_dpm_apsrc_rmb = 0xf,
	.reg_dpm_ddren_rmb = 0xf,

	/* SPM_SRC_MASK_8 */
	.reg_dpm_emi_rmb = 0xf,
	.reg_dpm_infra_rmb = 0xf,
	.reg_dpm_pmic_rmb = 0xf,
	.reg_dpm_srcclkena_mb = 0xf,
	.reg_dpm_vcore_rmb = 0xf,
	.reg_dpm_vrf18_rmb = 0xf,
	.reg_dpmaif_apsrc_rmb = 1,
	.reg_dpmaif_ddren_rmb = 0,
	.reg_dpmaif_emi_rmb = 1,
	.reg_dpmaif_infra_rmb = 1,
	.reg_dpmaif_pmic_rmb = 1,
	.reg_dpmaif_srcclkena_mb = 1,
	.reg_dpmaif_vcore_rmb = 1,
	.reg_dpmaif_vrf18_rmb = 1,

	/* SPM_SRC_MASK_9 */
	.reg_dvfsrc_level_rmb = 1,
	.reg_emisys_apsrc_rmb = 0,
	.reg_emisys_ddren_rmb = 0,
	.reg_emisys_emi_rmb = 0,
	.reg_emisys_infra_rmb = 0,
	.reg_emisys_pmic_rmb = 0,
	.reg_emisys_srcclkena_mb = 0,
	.reg_emisys_vcore_rmb = 0,
	.reg_emisys_vrf18_rmb = 0,
	.reg_gce_apsrc_rmb = 0,
	.reg_gce_ddren_rmb = 0,
	.reg_gce_emi_rmb = 0,
	.reg_gce_infra_rmb = 0,
	.reg_gce_pmic_rmb = 0,
	.reg_gce_srcclkena_mb = 0,
	.reg_gce_vcore_rmb = 0,
	.reg_gce_vrf18_rmb = 0,
	.reg_gpueb_apsrc_rmb = 1,
	.reg_gpueb_ddren_rmb = 0,
	.reg_gpueb_emi_rmb = 1,
	.reg_gpueb_infra_rmb = 1,
	.reg_gpueb_pmic_rmb = 1,
	.reg_gpueb_srcclkena_mb = 1,
	.reg_gpueb_vcore_rmb = 1,
	.reg_gpueb_vrf18_rmb = 1,
	.reg_hwccf_apsrc_rmb = 1,
	.reg_hwccf_ddren_rmb = 0,
	.reg_hwccf_emi_rmb = 1,
	.reg_hwccf_infra_rmb = 1,
	.reg_hwccf_pmic_rmb = 1,
	.reg_hwccf_srcclkena_mb = 1,
	.reg_hwccf_vcore_rmb = 1,

	/* SPM_SRC_MASK_10 */
	.reg_hwccf_vrf18_rmb = 1,
	.reg_img_apsrc_rmb = 0,
	.reg_img_ddren_rmb = 0,
	.reg_img_emi_rmb = 0,
	.reg_img_infra_rmb = 0,
	.reg_img_pmic_rmb = 0,
	.reg_img_srcclkena_mb = 0,
	.reg_img_vrf18_rmb = 0,
	.reg_infrasys_apsrc_rmb = 0,
	.reg_infrasys_ddren_rmb = 0,
	.reg_infrasys_emi_rmb = 0,
	.reg_infrasys_infra_rmb = 0,
	.reg_infrasys_pmic_rmb = 0,
	.reg_infrasys_srcclkena_mb = 0,
	.reg_infrasys_vcore_rmb = 0,
	.reg_infrasys_vrf18_rmb = 0,
	.reg_ipic_infra_rmb = 1,
	.reg_ipic_vrf18_rmb = 1,
	.reg_mcu_apsrc_rmb = 1,
	.reg_mcu_ddren_rmb = 0,
	.reg_mcu_emi_rmb = 1,
	.reg_mcu_infra_rmb = 1,
	.reg_mcu_pmic_rmb = 1,
	.reg_mcu_srcclkena_mb = 1,
	.reg_mcu_vcore_rmb = 1,
	.reg_mcu_vrf18_rmb = 1,
	.reg_md_apsrc_rmb = 1,
	.reg_md_ddren_rmb = 0,
	.reg_md_emi_rmb = 1,
	.reg_md_infra_rmb = 1,
	.reg_md_pmic_rmb = 1,
	.reg_md_srcclkena_mb = 1,

	/* SPM_SRC_MASK_11 */
	.reg_md_srcclkena1_mb = 1,
	.reg_md_vcore_rmb = 1,
	.reg_md_vrf18_rmb = 1,
	.reg_mm_proc_apsrc_rmb = 1,
	.reg_mm_proc_ddren_rmb = 0,
	.reg_mm_proc_emi_rmb = 1,
	.reg_mm_proc_infra_rmb = 1,
	.reg_mm_proc_pmic_rmb = 1,
	.reg_mm_proc_srcclkena_mb = 1,
	.reg_mm_proc_vcore_rmb = 1,
	.reg_mm_proc_vrf18_rmb = 1,
	.reg_mml0_apsrc_rmb = 0,
	.reg_mml0_ddren_rmb = 0,
	.reg_mml0_emi_rmb = 0,
	.reg_mml0_infra_rmb = 0,
	.reg_mml0_pmic_rmb = 0,
	.reg_mml0_srcclkena_mb = 0,
	.reg_mml0_vrf18_rmb = 0,
	.reg_mml1_apsrc_rmb = 0,
	.reg_mml1_ddren_rmb = 0,
	.reg_mml1_emi_rmb = 0,
	.reg_mml1_infra_rmb = 0,
	.reg_mml1_pmic_rmb = 0,
	.reg_mml1_srcclkena_mb = 0,
	.reg_mml1_vrf18_rmb = 0,
	.reg_ovl0_apsrc_rmb = 0,
	.reg_ovl0_ddren_rmb = 0,
	.reg_ovl0_emi_rmb = 0,
	.reg_ovl0_infra_rmb = 0,
	.reg_ovl0_pmic_rmb = 0,
	.reg_ovl0_srcclkena_mb = 0,
	.reg_ovl0_vrf18_rmb = 0,

	/* SPM_SRC_MASK_12 */
	.reg_ovl1_apsrc_rmb = 0,
	.reg_ovl1_ddren_rmb = 0,
	.reg_ovl1_emi_rmb = 0,
	.reg_ovl1_infra_rmb = 0,
	.reg_ovl1_pmic_rmb = 0,
	.reg_ovl1_srcclkena_mb = 0,
	.reg_ovl1_vrf18_rmb = 0,
	.reg_pcie0_apsrc_rmb = 1,
	.reg_pcie0_ddren_rmb = 0,
	.reg_pcie0_emi_rmb = 1,
	.reg_pcie0_infra_rmb = 1,
	.reg_pcie0_pmic_rmb = 1,
	.reg_pcie0_srcclkena_mb = 1,
	.reg_pcie0_vcore_rmb = 1,
	.reg_pcie0_vrf18_rmb = 1,
	.reg_pcie1_apsrc_rmb = 1,
	.reg_pcie1_ddren_rmb = 0,
	.reg_pcie1_emi_rmb = 1,
	.reg_pcie1_infra_rmb = 1,
	.reg_pcie1_pmic_rmb = 1,
	.reg_pcie1_srcclkena_mb = 1,
	.reg_pcie1_vcore_rmb = 1,
	.reg_pcie1_vrf18_rmb = 1,
	.reg_perisys_apsrc_rmb = 1,
	.reg_perisys_ddren_rmb = 0,
	.reg_perisys_emi_rmb = 1,
	.reg_perisys_infra_rmb = 1,
	.reg_perisys_pmic_rmb = 1,
	.reg_perisys_srcclkena_mb = 1,
	.reg_perisys_vcore_rmb = 1,
	.reg_perisys_vrf18_rmb = 1,
	.reg_pmsr_apsrc_rmb = 1,

	/* SPM_SRC_MASK_13 */
	.reg_pmsr_ddren_rmb = 0,
	.reg_pmsr_emi_rmb = 1,
	.reg_pmsr_infra_rmb = 1,
	.reg_pmsr_pmic_rmb = 1,
	.reg_pmsr_srcclkena_mb = 1,
	.reg_pmsr_vcore_rmb = 1,
	.reg_pmsr_vrf18_rmb = 1,
	.reg_scp_apsrc_rmb = 1,
	.reg_scp_ddren_rmb = 0,
	.reg_scp_emi_rmb = 1,
	.reg_scp_infra_rmb = 1,
	.reg_scp_pmic_rmb = 1,
	.reg_scp_srcclkena_mb = 1,
	.reg_scp_vcore_rmb = 1,
	.reg_scp_vrf18_rmb = 1,
	.reg_spu_hwr_apsrc_rmb = 1,
	.reg_spu_hwr_ddren_rmb = 0,
	.reg_spu_hwr_emi_rmb = 1,
	.reg_spu_hwr_infra_rmb = 1,
	.reg_spu_hwr_pmic_rmb = 1,
	.reg_spu_hwr_srcclkena_mb = 1,
	.reg_spu_hwr_vcore_rmb = 1,
	.reg_spu_hwr_vrf18_rmb = 1,
	.reg_spu_ise_apsrc_rmb = 1,
	.reg_spu_ise_ddren_rmb = 0,
	.reg_spu_ise_emi_rmb = 1,
	.reg_spu_ise_infra_rmb = 1,
	.reg_spu_ise_pmic_rmb = 1,
	.reg_spu_ise_srcclkena_mb = 1,
	.reg_spu_ise_vcore_rmb = 1,
	.reg_spu_ise_vrf18_rmb = 1,

	/* SPM_SRC_MASK_14 */
	.reg_srcclkeni_infra_rmb = 0x3,
	.reg_srcclkeni_pmic_rmb = 0x3,
	.reg_srcclkeni_srcclkena_mb = 0x3,
	.reg_srcclkeni_vcore_rmb = 0x3,
	.reg_sspm_apsrc_rmb = 1,
	.reg_sspm_ddren_rmb = 0,
	.reg_sspm_emi_rmb = 1,
	.reg_sspm_infra_rmb = 1,
	.reg_sspm_pmic_rmb = 1,
	.reg_sspm_srcclkena_mb = 1,
	.reg_sspm_vrf18_rmb = 1,
	.reg_ssrsys_apsrc_rmb = 1,
	.reg_ssrsys_ddren_rmb = 0,
	.reg_ssrsys_emi_rmb = 1,
	.reg_ssrsys_infra_rmb = 1,
	.reg_ssrsys_pmic_rmb = 1,
	.reg_ssrsys_srcclkena_mb = 1,
	.reg_ssrsys_vcore_rmb = 1,
	.reg_ssrsys_vrf18_rmb = 1,
	.reg_ssusb_apsrc_rmb = 1,
	.reg_ssusb_ddren_rmb = 0,
	.reg_ssusb_emi_rmb = 1,
	.reg_ssusb_infra_rmb = 1,
	.reg_ssusb_pmic_rmb = 1,
	.reg_ssusb_srcclkena_mb = 1,
	.reg_ssusb_vcore_rmb = 1,
	.reg_ssusb_vrf18_rmb = 1,
	.reg_uart_hub_infra_rmb = 1,

	/* SPM_SRC_MASK_15 */
	.reg_uart_hub_pmic_rmb = 1,
	.reg_uart_hub_srcclkena_mb = 1,
	.reg_uart_hub_vcore_rmb = 1,
	.reg_uart_hub_vrf18_rmb = 1,
	.reg_ufs_apsrc_rmb = 1,
	.reg_ufs_ddren_rmb = 0,
	.reg_ufs_emi_rmb = 1,
	.reg_ufs_infra_rmb = 1,
	.reg_ufs_pmic_rmb = 1,
	.reg_ufs_srcclkena_mb = 1,
	.reg_ufs_vcore_rmb = 1,
	.reg_ufs_vrf18_rmb = 1,
	.reg_vdec_apsrc_rmb = 0,
	.reg_vdec_ddren_rmb = 0,
	.reg_vdec_emi_rmb = 0,
	.reg_vdec_infra_rmb = 0,
	.reg_vdec_pmic_rmb = 0,
	.reg_vdec_srcclkena_mb = 0,
	.reg_vdec_vrf18_rmb = 0,
	.reg_venc_apsrc_rmb = 0,
	.reg_venc_ddren_rmb = 0,
	.reg_venc_emi_rmb = 0,
	.reg_venc_infra_rmb = 0,
	.reg_venc_pmic_rmb = 0,
	.reg_venc_srcclkena_mb = 0,
	.reg_venc_vrf18_rmb = 0,
	.reg_vlpcfg_apsrc_rmb = 1,
	.reg_vlpcfg_ddren_rmb = 0,
	.reg_vlpcfg_emi_rmb = 1,
	.reg_vlpcfg_infra_rmb = 1,
	.reg_vlpcfg_pmic_rmb = 1,
	.reg_vlpcfg_srcclkena_mb = 1,

	/* SPM_SRC_MASK_16 */
	.reg_vlpcfg_vcore_rmb = 1,
	.reg_vlpcfg_vrf18_rmb = 1,
	.reg_vlpcfg1_apsrc_rmb = 1,
	.reg_vlpcfg1_ddren_rmb = 0,
	.reg_vlpcfg1_emi_rmb = 1,
	.reg_vlpcfg1_infra_rmb = 1,
	.reg_vlpcfg1_pmic_rmb = 1,
	.reg_vlpcfg1_srcclkena_mb = 1,
	.reg_vlpcfg1_vcore_rmb = 1,
	.reg_vlpcfg1_vrf18_rmb = 1,

	/* SPM_EVENT_CON_MISC */
	.reg_srcclken_fast_resp = 0,
	.reg_csyspwrup_ack_mask = 1,

	/* SPM_SRC_MASK_17 */
	/* SPM D7X WA for wakeup source */
	.reg_spm_sw_vcore_rmb = 0x3,
	.reg_spm_sw_pmic_rmb = 0,

	/* SPM_SRC_MASK_18 */
	.reg_spm_sw_srcclkena_mb = 0,

	/* SPM_WAKE_MASK*/
#ifdef MT_SPM_COMMON_SODI_SUPPORT
#if defined(CFG_MICROTRUST_TEE_SUPPORT)
	.reg_wake_mask = 0x81302012,
#else
	.reg_wake_mask = 0x89302012,
#endif
#else
	.reg_wake_mask = 0xEFFFFFF7,
#endif

	.reg_ext_wake_mask = 0xFFFFFFFF,
};

struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl = &vcorefs_ctrl,
};

static int get_vsram_pmic_voltage(void)
{
	uint8_t vsram_pmic = 0x78;
	int spmi_ret = 0;
#ifdef CONFIG_MTK_SPMI
	struct spmi_device *spmi_dev;

	spmi_dev = get_spmi_device(SPMI_MASTER_1, SPMI_SLAVE_4);
	if (spmi_dev == NULL)
		return vsram_pmic;

	spmi_ret = spmi_ext_register_readl(spmi_dev, 0x250, &vsram_pmic, 1);
#endif
	if (spmi_ret != 0)
		vsram_pmic = 0x78;

	return vsram_pmic;
}

static int get_vcore_pmic_voltage(void)
{
	uint8_t vcore_pmic = 0x91;
	int spmi_ret = 0;
#ifdef CONFIG_MTK_SPMI
	struct spmi_device *spmi_dev;
	uint16_t vcore_addr;

	vcore_addr = (uint16_t)((mmio_read_32(SPM_PWRAP_CMD0) >> 16) & 0xFFFF);
	spmi_dev = get_spmi_device(SPMI_MASTER_P_1, SPMI_SLAVE_8);
	if (spmi_dev == NULL)
		return vcore_pmic;

	spmi_ret = spmi_ext_register_readl(spmi_dev, vcore_addr, &vcore_pmic, 1);
#endif
	if (spmi_ret != 0)
		vcore_pmic = 0x91;

	return vcore_pmic;
}

void spm_dvfsfw_init(uint64_t boot_up_opp, uint64_t dram_issue)
{
	uint32_t pmic_val;

	pmic_val = get_vsram_pmic_voltage();
	pmic_val = VCORE_UV_TO_PMIC(VSRAM_PMIC_TO_UV(pmic_val));
	mt_spm_pmic_wrap_set_cmd(PMIC_WRAP_PHASE_ALLINONE, CMD_17, pmic_val);
	pmic_val = get_vcore_pmic_voltage();
	mmio_write_32(PCM_WDT_LATCH_SPARE_5, pmic_val);

	if (spm_dvfs_init_done)
		return;

	mmio_clrsetbits_32(SPM_DVFS_CON, SPM_DVFS_FORCE_ENABLE_LSB, SPM_DVFSRC_ENABLE_LSB);

	mmio_write_32(SPM_SW_RSV_3, 0x08000000);
	mmio_write_32(SPM_DVS_DFS_LEVEL, 0x10080080);
	mmio_write_32(PCM_WDT_LATCH_SPARE_4, 0x08008002);

	spm_dvfs_init_done = 1;
}

void __spm_sync_vcore_dvfs_pcm_flags(uint32_t *dest_pcm_flags,
				     const uint32_t *src_pcm_flags)
{
	uint32_t dvfs_mask = SPM_FLAG_DISABLE_VCORE_DVS |
			     SPM_FLAG_DISABLE_DDR_DFS |
			     SPM_FLAG_DISABLE_EMI_DFS |
			     SPM_FLAG_DISABLE_BUS_DFS;

	*dest_pcm_flags = (*dest_pcm_flags & (~dvfs_mask)) |
					(*src_pcm_flags & dvfs_mask);
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl,
					 const struct pwr_ctrl *src_pwr_ctrl)
{

	__spm_sync_vcore_dvfs_pcm_flags(&dest_pwr_ctrl->pcm_flags,
					&src_pwr_ctrl->pcm_flags);

	if (dest_pwr_ctrl->pcm_flags_cust)
		__spm_sync_vcore_dvfs_pcm_flags(&dest_pwr_ctrl->pcm_flags_cust,
						&src_pwr_ctrl->pcm_flags);
}

void spm_go_to_vcorefs(uint64_t spm_flags)
{
	set_pwrctrl_pcm_flags(__spm_vcorefs.pwrctrl, spm_flags);
	__spm_set_power_control(__spm_vcorefs.pwrctrl, 0);
	__spm_set_wakeup_event(__spm_vcorefs.pwrctrl);
	__spm_set_pcm_flags(__spm_vcorefs.pwrctrl);
	__spm_send_cpu_wakeup_event();
}
