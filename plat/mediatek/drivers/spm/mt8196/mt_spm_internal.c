/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <drivers/spm/mt_spm_resource_req.h>
#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <pmic_wrap/inc/mt_spm_pmic_wrap.h>

/**************************************
 * Define and Declare
 **************************************/
#define SPM_INIT_DONE_US	20 /* Simulation result */

wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta)
{
	uint32_t i;
	wake_reason_t wr = WR_UNKNOWN;

	if (!wakesta)
		return WR_UNKNOWN;

	if (wakesta->is_abort) {
		INFO("SPM EARLY WAKE r13 = 0x%x, ", wakesta->tr.comm.r13);
#ifndef MTK_PLAT_SPM_PMIC_WRAP_DUMP_UNSUPPORT
		mt_spm_dump_pmic_warp_reg();
#endif
	}

	if (wakesta->tr.comm.r12 & R12_PCM_TIMER_B) {

		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER_EVENT)
			wr = WR_PCM_TIMER;
	}

	for (i = 2; i < 32; i++) {
		if (wakesta->tr.comm.r12 & (1U << i))
			wr = WR_WAKE_SRC;
	}

	return wr;
}

void __spm_init_pcm_register(void)
{
	/* Disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);
}

void __spm_set_power_control(const struct pwr_ctrl *pwrctrl,
			     uint32_t resource_usage)
{
	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		      ((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 0) |
		      (((pwrctrl->reg_spm_apsrc_req |
		      !!(resource_usage & MT_SPM_DRAM_S0)) & 0x1) << 1) |
		      (((pwrctrl->reg_spm_ddren_req |
		      !!(resource_usage & MT_SPM_DRAM_S1)) & 0x1) << 2) |
		      ((pwrctrl->reg_spm_dvfs_req & 0x1) << 3) |
		      (((pwrctrl->reg_spm_emi_req |
		      !!(resource_usage & MT_SPM_EMI)) & 0x1) << 4) |
		      (((pwrctrl->reg_spm_f26m_req |
		      !!(resource_usage & (MT_SPM_26M |
		      MT_SPM_XO_FPM))) & 0x1) << 5) |
		      (((pwrctrl->reg_spm_infra_req |
		      !!(resource_usage & MT_SPM_INFRA)) & 0x1) << 6) |
		      (((pwrctrl->reg_spm_pmic_req |
		      !!(resource_usage & MT_SPM_PMIC)) & 0x1) << 7) |
		      (((u32)pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 8) |
		      (((u32)pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 9) |
		      (((u32)pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 10) |
		      ((((u32)pwrctrl->reg_spm_vcore_req |
		      !!(resource_usage & MT_SPM_VCORE)) & 0x1) << 11) |
		      ((((u32)pwrctrl->reg_spm_vrf18_req |
		      !!(resource_usage & MT_SPM_SYSPLL)) & 0x1) << 12) |
		      (((u32)pwrctrl->adsp_mailbox_state & 0x1) << 16) |
		      (((u32)pwrctrl->apsrc_state & 0x1) << 17) |
		      (((u32)pwrctrl->ddren_state & 0x1) << 18) |
		      (((u32)pwrctrl->dvfs_state & 0x1) << 19) |
		      (((u32)pwrctrl->emi_state & 0x1) << 20) |
		      (((u32)pwrctrl->f26m_state & 0x1) << 21) |
		      (((u32)pwrctrl->infra_state & 0x1) << 22) |
		      (((u32)pwrctrl->pmic_state & 0x1) << 23) |
		      (((u32)pwrctrl->scp_mailbox_state & 0x1) << 24) |
		      (((u32)pwrctrl->sspm_mailbox_state & 0x1) << 25) |
		      (((u32)pwrctrl->sw_mailbox_state & 0x1) << 26) |
		      (((u32)pwrctrl->vcore_state & 0x1) << 27) |
		      (((u32)pwrctrl->vrf18_state & 0x1) << 28));

	/* SPM_SRC_MASK_0 */
	mmio_write_32(SPM_SRC_MASK_0,
		      (((u32)pwrctrl->reg_apifr_apsrc_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_apifr_ddren_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_apifr_emi_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_apifr_infra_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_apifr_pmic_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_apifr_srcclkena_mb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_apifr_vcore_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_apifr_vrf18_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_apu_apsrc_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_apu_ddren_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_apu_emi_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_apu_infra_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_apu_pmic_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_apu_srcclkena_mb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_apu_vcore_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_apu_vrf18_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_audio_apsrc_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_audio_ddren_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_audio_emi_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_audio_infra_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_audio_pmic_rmb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_audio_srcclkena_mb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_audio_vcore_rmb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_audio_vrf18_rmb & 0x1) << 23));

	/* SPM_SRC_MASK_1 */
	mmio_write_32(SPM_SRC_MASK_1,
		      (((u32)pwrctrl->reg_audio_dsp_apsrc_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_audio_dsp_ddren_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_audio_dsp_emi_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_audio_dsp_infra_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_audio_dsp_pmic_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_audio_dsp_srcclkena_mb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_audio_dsp_vcore_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_audio_dsp_vrf18_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_cam_apsrc_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_cam_ddren_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_cam_emi_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_cam_infra_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_cam_pmic_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_cam_srcclkena_mb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_cam_vrf18_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_ccif_apsrc_rmb & 0xfff) << 15));

	/* SPM_SRC_MASK_2 */
	mmio_write_32(SPM_SRC_MASK_2,
		      (((u32)pwrctrl->reg_ccif_emi_rmb & 0xfff) << 0) |
		      (((u32)pwrctrl->reg_ccif_infra_rmb & 0xfff) << 12));

	/* SPM_SRC_MASK_3 */
	mmio_write_32(SPM_SRC_MASK_3,
		      (((u32)pwrctrl->reg_ccif_pmic_rmb & 0xfff) << 0) |
		      (((u32)pwrctrl->reg_ccif_srcclkena_mb & 0xfff) << 12));

	/* SPM_SRC_MASK_4 */
	mmio_write_32(SPM_SRC_MASK_4,
		      (((u32)pwrctrl->reg_ccif_vcore_rmb & 0xfff) << 0) |
		      (((u32)pwrctrl->reg_ccif_vrf18_rmb & 0xfff) << 12) |
		      (((u32)pwrctrl->reg_ccu_apsrc_rmb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_ccu_ddren_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_ccu_emi_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_ccu_infra_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_ccu_pmic_rmb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_ccu_srcclkena_mb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_ccu_vrf18_rmb & 0x1) << 30) |
		      (((u32)pwrctrl->reg_cg_check_apsrc_rmb & 0x1) << 31));

	/* SPM_SRC_MASK_5 */
	mmio_write_32(SPM_SRC_MASK_5,
		      (((u32)pwrctrl->reg_cg_check_ddren_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_cg_check_emi_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_cg_check_infra_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_cg_check_pmic_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_cg_check_srcclkena_mb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_cg_check_vcore_rmb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_cg_check_vrf18_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_cksys_apsrc_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_cksys_ddren_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_cksys_emi_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_cksys_infra_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_cksys_pmic_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_cksys_srcclkena_mb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_cksys_vcore_rmb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_cksys_vrf18_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_cksys_1_apsrc_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_cksys_1_ddren_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_cksys_1_emi_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_cksys_1_infra_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_cksys_1_pmic_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_cksys_1_srcclkena_mb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_cksys_1_vcore_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_cksys_1_vrf18_rmb & 0x1) << 22));

	/* SPM_SRC_MASK_6 */
	mmio_write_32(SPM_SRC_MASK_6,
		      (((u32)pwrctrl->reg_cksys_2_apsrc_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_cksys_2_ddren_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_cksys_2_emi_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_cksys_2_infra_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_cksys_2_pmic_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_cksys_2_srcclkena_mb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_cksys_2_vcore_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_cksys_2_vrf18_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_conn_apsrc_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_conn_ddren_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_conn_emi_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_conn_infra_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_conn_pmic_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_conn_srcclkena_mb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_conn_srcclkenb_mb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_conn_vcore_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_conn_vrf18_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_corecfg_apsrc_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_corecfg_ddren_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_corecfg_emi_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_corecfg_infra_rmb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_corecfg_pmic_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_corecfg_srcclkena_mb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_corecfg_vcore_rmb & 0x1) << 23) |
		      (((u32)pwrctrl->reg_corecfg_vrf18_rmb & 0x1) << 24));

	/* SPM_SRC_MASK_7 */
	mmio_write_32(SPM_SRC_MASK_7,
		      (((u32)pwrctrl->reg_cpueb_apsrc_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_cpueb_ddren_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_cpueb_emi_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_cpueb_infra_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_cpueb_pmic_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_cpueb_srcclkena_mb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_cpueb_vcore_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_cpueb_vrf18_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_disp0_apsrc_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_disp0_ddren_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_disp0_emi_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_disp0_infra_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_disp0_pmic_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_disp0_srcclkena_mb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_disp0_vrf18_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_disp1_apsrc_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_disp1_ddren_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_disp1_emi_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_disp1_infra_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_disp1_pmic_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_disp1_srcclkena_mb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_disp1_vrf18_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_dpm_apsrc_rmb & 0xf) << 22) |
		      (((u32)pwrctrl->reg_dpm_ddren_rmb & 0xf) << 26));

	/* SPM_SRC_MASK_8 */
	mmio_write_32(SPM_SRC_MASK_8,
		      (((u32)pwrctrl->reg_dpm_emi_rmb & 0xf) << 0) |
		      (((u32)pwrctrl->reg_dpm_infra_rmb & 0xf) << 4) |
		      (((u32)pwrctrl->reg_dpm_pmic_rmb & 0xf) << 8) |
		      (((u32)pwrctrl->reg_dpm_srcclkena_mb & 0xf) << 12) |
		      (((u32)pwrctrl->reg_dpm_vcore_rmb & 0xf) << 16) |
		      (((u32)pwrctrl->reg_dpm_vrf18_rmb & 0xf) << 20) |
		      (((u32)pwrctrl->reg_dpmaif_apsrc_rmb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_dpmaif_ddren_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_dpmaif_emi_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_dpmaif_infra_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_dpmaif_pmic_rmb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_dpmaif_srcclkena_mb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_dpmaif_vcore_rmb & 0x1) << 30) |
		      (((u32)pwrctrl->reg_dpmaif_vrf18_rmb & 0x1) << 31));

	/* SPM_SRC_MASK_9 */
	mmio_write_32(SPM_SRC_MASK_9,
		      (((u32)pwrctrl->reg_dvfsrc_level_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_emisys_apsrc_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_emisys_ddren_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_emisys_emi_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_emisys_infra_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_emisys_pmic_rmb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_emisys_srcclkena_mb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_emisys_vcore_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_emisys_vrf18_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_gce_apsrc_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_gce_ddren_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_gce_emi_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_gce_infra_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_gce_pmic_rmb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_gce_srcclkena_mb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_gce_vcore_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_gce_vrf18_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_gpueb_apsrc_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_gpueb_ddren_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_gpueb_emi_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_gpueb_infra_rmb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_gpueb_pmic_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_gpueb_srcclkena_mb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_gpueb_vcore_rmb & 0x1) << 23) |
		      (((u32)pwrctrl->reg_gpueb_vrf18_rmb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_hwccf_apsrc_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_hwccf_ddren_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_hwccf_emi_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_hwccf_infra_rmb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_hwccf_pmic_rmb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_hwccf_srcclkena_mb & 0x1) << 30) |
		      (((u32)pwrctrl->reg_hwccf_vcore_rmb & 0x1) << 31));

	/* SPM_SRC_MASK_10 */
	mmio_write_32(SPM_SRC_MASK_10,
		      (((u32)pwrctrl->reg_hwccf_vrf18_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_img_apsrc_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_img_ddren_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_img_emi_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_img_infra_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_img_pmic_rmb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_img_srcclkena_mb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_img_vrf18_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_infrasys_apsrc_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_infrasys_ddren_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_infrasys_emi_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_infrasys_infra_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_infrasys_pmic_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_infrasys_srcclkena_mb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_infrasys_vcore_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_infrasys_vrf18_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_ipic_infra_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_ipic_vrf18_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_mcu_apsrc_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_mcu_ddren_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_mcu_emi_rmb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_mcu_infra_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_mcu_pmic_rmb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_mcu_srcclkena_mb & 0x1) << 23) |
		      (((u32)pwrctrl->reg_mcu_vcore_rmb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_mcu_vrf18_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_md_apsrc_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_md_ddren_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_md_emi_rmb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_md_infra_rmb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_md_pmic_rmb & 0x1) << 30) |
		      (((u32)pwrctrl->reg_md_srcclkena_mb & 0x1) << 31));

	/* SPM_SRC_MASK_11 */
	mmio_write_32(SPM_SRC_MASK_11,
		      (((u32)pwrctrl->reg_md_srcclkena1_mb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_md_vcore_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_md_vrf18_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_mm_proc_apsrc_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_mm_proc_ddren_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_mm_proc_emi_rmb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_mm_proc_infra_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_mm_proc_pmic_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_mm_proc_srcclkena_mb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_mm_proc_vcore_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_mm_proc_vrf18_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_mml0_apsrc_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_mml0_ddren_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_mml0_emi_rmb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_mml0_infra_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_mml0_pmic_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_mml0_srcclkena_mb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_mml0_vrf18_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_mml1_apsrc_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_mml1_ddren_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_mml1_emi_rmb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_mml1_infra_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_mml1_pmic_rmb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_mml1_srcclkena_mb & 0x1) << 23) |
		      (((u32)pwrctrl->reg_mml1_vrf18_rmb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_ovl0_apsrc_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_ovl0_ddren_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_ovl0_emi_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_ovl0_infra_rmb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_ovl0_pmic_rmb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_ovl0_srcclkena_mb & 0x1) << 30) |
		      (((u32)pwrctrl->reg_ovl0_vrf18_rmb & 0x1) << 31));

	mmio_write_32(SPM_SRC_MASK_12,
		      (((u32)pwrctrl->reg_ovl1_apsrc_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_ovl1_ddren_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_ovl1_emi_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_ovl1_infra_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_ovl1_pmic_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_ovl1_srcclkena_mb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_ovl1_vrf18_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_pcie0_apsrc_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_pcie0_ddren_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_pcie0_emi_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_pcie0_infra_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_pcie0_pmic_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_pcie0_srcclkena_mb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_pcie0_vcore_rmb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_pcie0_vrf18_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_pcie1_apsrc_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_pcie1_ddren_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_pcie1_emi_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_pcie1_infra_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_pcie1_pmic_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_pcie1_srcclkena_mb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_pcie1_vcore_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_pcie1_vrf18_rmb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_perisys_apsrc_rmb & 0x1) << 23) |
		      (((u32)pwrctrl->reg_perisys_ddren_rmb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_perisys_emi_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_perisys_infra_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_perisys_pmic_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_perisys_srcclkena_mb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_perisys_vcore_rmb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_perisys_vrf18_rmb & 0x1) << 30) |
		      (((u32)pwrctrl->reg_pmsr_apsrc_rmb & 0x1) << 31));

	/* SPM_SRC_MASK_13 */
	mmio_write_32(SPM_SRC_MASK_13,
		      (((u32)pwrctrl->reg_pmsr_ddren_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_pmsr_emi_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_pmsr_infra_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_pmsr_pmic_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_pmsr_srcclkena_mb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_pmsr_vcore_rmb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_pmsr_vrf18_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_scp_apsrc_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_scp_ddren_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_scp_emi_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_scp_infra_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_scp_pmic_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_scp_srcclkena_mb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_scp_vcore_rmb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_scp_vrf18_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_spu_hwr_apsrc_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_spu_hwr_ddren_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_spu_hwr_emi_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_spu_hwr_infra_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_spu_hwr_pmic_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_spu_hwr_srcclkena_mb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_spu_hwr_vcore_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_spu_hwr_vrf18_rmb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_spu_ise_apsrc_rmb & 0x1) << 23) |
		      (((u32)pwrctrl->reg_spu_ise_ddren_rmb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_spu_ise_emi_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_spu_ise_infra_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_spu_ise_pmic_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_spu_ise_srcclkena_mb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_spu_ise_vcore_rmb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_spu_ise_vrf18_rmb & 0x1) << 30));

	/* SPM_SRC_MASK_14 */
	mmio_write_32(SPM_SRC_MASK_14,
		      (((u32)pwrctrl->reg_srcclkeni_infra_rmb & 0x3) << 0) |
		      (((u32)pwrctrl->reg_srcclkeni_pmic_rmb & 0x3) << 2) |
		      (((u32)pwrctrl->reg_srcclkeni_srcclkena_mb & 0x3) << 4) |
		      (((u32)pwrctrl->reg_srcclkeni_vcore_rmb & 0x3) << 6) |
		      (((u32)pwrctrl->reg_sspm_apsrc_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_sspm_ddren_rmb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_sspm_emi_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_sspm_infra_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_sspm_pmic_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_sspm_srcclkena_mb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_sspm_vrf18_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_ssrsys_apsrc_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_ssrsys_ddren_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_ssrsys_emi_rmb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_ssrsys_infra_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_ssrsys_pmic_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_ssrsys_srcclkena_mb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_ssrsys_vcore_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_ssrsys_vrf18_rmb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_ssusb_apsrc_rmb & 0x1) << 23) |
		      (((u32)pwrctrl->reg_ssusb_ddren_rmb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_ssusb_emi_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_ssusb_infra_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_ssusb_pmic_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_ssusb_srcclkena_mb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_ssusb_vcore_rmb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_ssusb_vrf18_rmb & 0x1) << 30) |
		      (((u32)pwrctrl->reg_uart_hub_infra_rmb & 0x1) << 31));

	/* SPM_SRC_MASK_15 */
	mmio_write_32(SPM_SRC_MASK_15,
		      (((u32)pwrctrl->reg_uart_hub_pmic_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_uart_hub_srcclkena_mb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_uart_hub_vcore_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_uart_hub_vrf18_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_ufs_apsrc_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_ufs_ddren_rmb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_ufs_emi_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_ufs_infra_rmb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_ufs_pmic_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_ufs_srcclkena_mb & 0x1) << 9) |
		      (((u32)pwrctrl->reg_ufs_vcore_rmb & 0x1) << 10) |
		      (((u32)pwrctrl->reg_ufs_vrf18_rmb & 0x1) << 11) |
		      (((u32)pwrctrl->reg_vdec_apsrc_rmb & 0x1) << 12) |
		      (((u32)pwrctrl->reg_vdec_ddren_rmb & 0x1) << 13) |
		      (((u32)pwrctrl->reg_vdec_emi_rmb & 0x1) << 14) |
		      (((u32)pwrctrl->reg_vdec_infra_rmb & 0x1) << 15) |
		      (((u32)pwrctrl->reg_vdec_pmic_rmb & 0x1) << 16) |
		      (((u32)pwrctrl->reg_vdec_srcclkena_mb & 0x1) << 17) |
		      (((u32)pwrctrl->reg_vdec_vrf18_rmb & 0x1) << 18) |
		      (((u32)pwrctrl->reg_venc_apsrc_rmb & 0x1) << 19) |
		      (((u32)pwrctrl->reg_venc_ddren_rmb & 0x1) << 20) |
		      (((u32)pwrctrl->reg_venc_emi_rmb & 0x1) << 21) |
		      (((u32)pwrctrl->reg_venc_infra_rmb & 0x1) << 22) |
		      (((u32)pwrctrl->reg_venc_pmic_rmb & 0x1) << 23) |
		      (((u32)pwrctrl->reg_venc_srcclkena_mb & 0x1) << 24) |
		      (((u32)pwrctrl->reg_venc_vrf18_rmb & 0x1) << 25) |
		      (((u32)pwrctrl->reg_vlpcfg_apsrc_rmb & 0x1) << 26) |
		      (((u32)pwrctrl->reg_vlpcfg_ddren_rmb & 0x1) << 27) |
		      (((u32)pwrctrl->reg_vlpcfg_emi_rmb & 0x1) << 28) |
		      (((u32)pwrctrl->reg_vlpcfg_infra_rmb & 0x1) << 29) |
		      (((u32)pwrctrl->reg_vlpcfg_pmic_rmb & 0x1) << 30) |
		      (((u32)pwrctrl->reg_vlpcfg_srcclkena_mb & 0x1) << 31));

	/* SPM_SRC_MASK_16 */
	mmio_write_32(SPM_SRC_MASK_16,
		      (((u32)pwrctrl->reg_vlpcfg_vcore_rmb & 0x1) << 0) |
		      (((u32)pwrctrl->reg_vlpcfg_vrf18_rmb & 0x1) << 1) |
		      (((u32)pwrctrl->reg_vlpcfg1_apsrc_rmb & 0x1) << 2) |
		      (((u32)pwrctrl->reg_vlpcfg1_ddren_rmb & 0x1) << 3) |
		      (((u32)pwrctrl->reg_vlpcfg1_emi_rmb & 0x1) << 4) |
		      (((u32)pwrctrl->reg_vlpcfg1_infra_rmb & 0x1) << 5) |
		      (((u32)pwrctrl->reg_vlpcfg1_pmic_rmb & 0x1) << 6) |
		      (((u32)pwrctrl->reg_vlpcfg1_srcclkena_mb & 0x1) << 7) |
		      (((u32)pwrctrl->reg_vlpcfg1_vcore_rmb & 0x1) << 8) |
		      (((u32)pwrctrl->reg_vlpcfg1_vrf18_rmb & 0x1) << 9));

	/* SPM_SRC_MASK_17 */
	mmio_write_32(SPM_SRC_MASK_17,
		      (((u32)pwrctrl->reg_spm_sw_vcore_rmb & 0xffff) << 0) |
		      (((u32)pwrctrl->reg_spm_sw_pmic_rmb & 0xffff) << 16));

	/* SPM_SRC_MASK_18 */
	mmio_write_32(SPM_SRC_MASK_18,
		      (((u32)pwrctrl->reg_spm_sw_srcclkena_mb & 0xffff) << 0));

	/* SPM_EVENT_CON_MISC */
	mmio_write_32(SPM_EVENT_CON_MISC,
		      (((u32)pwrctrl->reg_srcclken_fast_resp & 0x1) << 0) |
		      (((u32)pwrctrl->reg_csyspwrup_ack_mask & 0x1) << 1));

	/* SPM_WAKE_MASK*/
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		      (((u32)pwrctrl->reg_wake_mask & 0xffffffff) << 0));

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_EXT_MASK,
		      (((u32)pwrctrl->reg_ext_wake_mask & 0xffffffff) << 0));
}

#define CHECK_ONE	0xffffffff
#define CHECK_ZERO	0x0
static int32_t __spm_check_ack(u32 reg, u32 mask, u32 check_en)
{
	u32 val;

	val = mmio_read_32(reg);
	if ((val & mask) == (mask & check_en))
		return 0;
	return -1;
}

int32_t __spm_wait_spm_request_ack(u32 spm_resource_req, u32 timeout_us)
{
	u32 spm_ctrl0_mask, spm_ctrl1_mask;
	int32_t ret, retry;

	if (spm_resource_req == 0)
		return 0;

	spm_ctrl0_mask = 0;
	spm_ctrl1_mask = 0;

	if (spm_resource_req & (MT_SPM_XO_FPM | MT_SPM_26M))
		spm_ctrl0_mask |=  CTRL0_SC_MD26M_CK_OFF;

	if (spm_resource_req & MT_SPM_VCORE)
		spm_ctrl1_mask |= CTRL1_SPM_VCORE_INTERNAL_ACK;
	if (spm_resource_req & MT_SPM_PMIC)
		spm_ctrl1_mask |= CTRL1_SPM_PMIC_INTERNAL_ACK;
	if (spm_resource_req & MT_SPM_INFRA)
		spm_ctrl1_mask |= CTRL1_SPM_INFRA_INTERNAL_ACK;
	if (spm_resource_req & MT_SPM_SYSPLL)
		spm_ctrl1_mask |= CTRL1_SPM_VRF18_INTERNAL_ACK;
	if (spm_resource_req & MT_SPM_EMI)
		spm_ctrl1_mask |= CTRL1_SPM_EMI_INTERNAL_ACK;
	if (spm_resource_req & MT_SPM_DRAM_S0)
		spm_ctrl1_mask |= CTRL1_SPM_APSRC_INTERNAL_ACK;
	if (spm_resource_req & MT_SPM_DRAM_S1)
		spm_ctrl1_mask |= CTRL1_SPM_DDREN_INTERNAL_ACK;

	retry = -1;
	ret = 0;

	while (retry++ < timeout_us) {
		udelay(1);
		if (spm_ctrl0_mask != 0) {
			ret = __spm_check_ack(MD32PCM_SCU_CTRL0,
					      spm_ctrl0_mask,
					      CHECK_ZERO);
			if (ret)
				continue;
		}
		if (spm_ctrl1_mask != 0) {
			ret = __spm_check_ack(MD32PCM_SCU_CTRL1,
					      spm_ctrl1_mask,
					      CHECK_ONE);
			if (ret)
				continue;
		}
		break;
	}

	return ret;
}

void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	u32 val, mask;

	/* Toggle event counter clear */
	mmio_write_32(SPM_EVENT_COUNTER_CLEAR, REG_SPM_EVENT_COUNTER_CLR_LSB);
	/* Toggle for reset SYS TIMER start point */
	mmio_setbits_32(SYS_TIMER_CON, SYS_TIMER_START_EN_LSB);

	if (pwrctrl->timer_val_cust == 0)
		val = pwrctrl->timer_val ? pwrctrl->timer_val : PCM_TIMER_MAX;
	else
		val = pwrctrl->timer_val_cust;

	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_setbits_32(PCM_CON1, SPM_REGWR_CFG_KEY | REG_PCM_TIMER_EN_LSB);

	/* Unmask AP wakeup source */
	if (pwrctrl->wake_src_cust == 0)
		mask = pwrctrl->wake_src;
	else
		mask = pwrctrl->wake_src_cust;

	if (pwrctrl->reg_csyspwrup_ack_mask)
		mask &= ~R12_CSYSPWREQ_B;
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	/* Unmask SPM ISR (keep TWAM setting) */
	mmio_setbits_32(SPM_IRQ_MASK, ISRM_RET_IRQ_AUX);

	/* Toggle event counter clear */
	mmio_write_32(SPM_EVENT_COUNTER_CLEAR, 0);
	/* Toggle for reset SYS TIMER start point */
	mmio_clrbits_32(SYS_TIMER_CON, SYS_TIMER_START_EN_LSB);
}

void __spm_set_fw_resume_option(struct pwr_ctrl *pwrctrl)
{
#if SPM_FW_NO_RESUME
    /* Do Nothing */
#else
	pwrctrl->pcm_flags1 |= SPM_FLAG1_DISABLE_NO_RESUME;
#endif
}

void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl)
{
	/* Set PCM flags and data */
	if (pwrctrl->pcm_flags_cust_clr != 0)
		pwrctrl->pcm_flags &= ~pwrctrl->pcm_flags_cust_clr;
	if (pwrctrl->pcm_flags_cust_set != 0)
		pwrctrl->pcm_flags |= pwrctrl->pcm_flags_cust_set;
	if (pwrctrl->pcm_flags1_cust_clr != 0)
		pwrctrl->pcm_flags1 &= ~pwrctrl->pcm_flags1_cust_clr;
	if (pwrctrl->pcm_flags1_cust_set != 0)
		pwrctrl->pcm_flags1 |= pwrctrl->pcm_flags1_cust_set;

	mmio_write_32(SPM_SW_FLAG_0, pwrctrl->pcm_flags);

	mmio_write_32(SPM_SW_FLAG_1, pwrctrl->pcm_flags1);

	mmio_write_32(SPM_SW_RSV_7, pwrctrl->pcm_flags);

	mmio_write_32(SPM_SW_RSV_8, pwrctrl->pcm_flags1);
}

void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	/* Waiting for loading SPMFW done*/
	while (mmio_read_32(MD32PCM_DMA0_RLCT) != 0x0)
		;

	__spm_set_pcm_flags(pwrctrl);

	udelay(SPM_INIT_DONE_US);
}

void __spm_get_wakeup_status(struct wake_status *wakesta,
			     uint32_t ext_status)
{
	/* Get wakeup event */
	wakesta->tr.comm.r12 = mmio_read_32(SPM_BK_WAKE_EVENT);
	wakesta->r12_ext = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->tr.comm.raw_sta = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->md32pcm_wakeup_sta = mmio_read_32(MD32PCM_WAKEUP_STA);
	wakesta->md32pcm_event_sta = mmio_read_32(MD32PCM_EVENT_STA);
	wakesta->wake_misc = mmio_read_32(SPM_BK_WAKE_MISC);

	/* Get sleep time */
	wakesta->tr.comm.timer_out = mmio_read_32(SPM_BK_PCM_TIMER);
	wakesta->tr.comm.r13 = mmio_read_32(MD32PCM_SCU_STA0);
	wakesta->tr.comm.req_sta0 = mmio_read_32(SPM_REQ_STA_0);
	wakesta->tr.comm.req_sta1 = mmio_read_32(SPM_REQ_STA_1);
	wakesta->tr.comm.req_sta2 = mmio_read_32(SPM_REQ_STA_2);
	wakesta->tr.comm.req_sta3 = mmio_read_32(SPM_REQ_STA_3);
	wakesta->tr.comm.req_sta4 = mmio_read_32(SPM_REQ_STA_4);
	wakesta->tr.comm.req_sta5 = mmio_read_32(SPM_REQ_STA_5);
	wakesta->tr.comm.req_sta6 = mmio_read_32(SPM_REQ_STA_6);
	wakesta->tr.comm.req_sta7 = mmio_read_32(SPM_REQ_STA_7);
	wakesta->tr.comm.req_sta8 = mmio_read_32(SPM_REQ_STA_8);
	wakesta->tr.comm.req_sta9 = mmio_read_32(SPM_REQ_STA_9);
	wakesta->tr.comm.req_sta10 = mmio_read_32(SPM_REQ_STA_10);
	wakesta->tr.comm.req_sta11 = mmio_read_32(SPM_REQ_STA_11);
	wakesta->tr.comm.req_sta12 = mmio_read_32(SPM_REQ_STA_12);
	wakesta->tr.comm.req_sta13 = mmio_read_32(SPM_REQ_STA_13);
	wakesta->tr.comm.req_sta14 = mmio_read_32(SPM_REQ_STA_14);
	wakesta->tr.comm.req_sta15 = mmio_read_32(SPM_REQ_STA_15);
	wakesta->tr.comm.req_sta16 = mmio_read_32(SPM_REQ_STA_16);

	/* Get debug flag for PCM execution check */
	wakesta->tr.comm.debug_flag = mmio_read_32(PCM_WDT_LATCH_SPARE_0);
	wakesta->tr.comm.debug_flag1 = mmio_read_32(PCM_WDT_LATCH_SPARE_1);

	/* Get backup SW flag status */
	wakesta->tr.comm.b_sw_flag0 = mmio_read_32(SPM_SW_RSV_7);
	wakesta->tr.comm.b_sw_flag1 = mmio_read_32(SPM_SW_RSV_8);

	/* Get ISR status */
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);

	/* Get SW flag status */
	wakesta->sw_flag0 = mmio_read_32(SPM_SW_FLAG_0);
	wakesta->sw_flag1 = mmio_read_32(SPM_SW_FLAG_1);

	/* Check abort */
	wakesta->is_abort = wakesta->tr.comm.debug_flag1 & DEBUG_ABORT_MASK_1;
}

void __spm_clean_after_wakeup(void)
{
	/*
	 * Copy SPM_WAKEUP_STA to SPM_BK_WAKE_EVENT
	 * before clear SPM_WAKEUP_STA
	 *
	 * CPU dormant driver @kernel will copy  edge-trig IRQ pending
	 * (recorded @SPM_BK_WAKE_EVENT) to GIC
	 */
	mmio_write_32(SPM_BK_WAKE_EVENT, mmio_read_32(SPM_WAKEUP_STA) |
		mmio_read_32(SPM_BK_WAKE_EVENT));

	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);

	/* Clean wakeup event raw status (for edge trigger event) */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, 0xefffffff);

	/* Clean ISR status (except TWAM) */
	mmio_setbits_32(SPM_IRQ_MASK,  ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL_EXC_TWAM);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);
}

void __spm_set_pcm_wdt(int en)
{
	/* Enable PCM WDT (normal mode) to start count if needed */
	if (en) {
		mmio_clrsetbits_32(PCM_CON1, REG_PCM_WDT_WAKE_LSB,
				   SPM_REGWR_CFG_KEY);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX)
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		mmio_write_32(PCM_WDT_VAL, mmio_read_32(PCM_TIMER_VAL) +
			      PCM_WDT_TIMEOUT);
		mmio_setbits_32(PCM_CON1, SPM_REGWR_CFG_KEY |
				REG_PCM_WDT_EN_LSB);
	} else {
		mmio_clrsetbits_32(PCM_CON1, REG_PCM_WDT_EN_LSB,
				   SPM_REGWR_CFG_KEY);
	}
}

u32 __spm_get_pcm_timer_val(void)
{
	return mmio_read_32(PCM_TIMER_VAL) >> 15;
}

void __spm_send_cpu_wakeup_event(void)
{
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
}

void __spm_ext_int_wakeup_req_clr(void)
{
	u32 cpu = plat_my_core_pos();

	mmio_write_32(EXT_INT_WAKEUP_REQ_CLR, (1U << cpu));

	/* Clear spm2mcupm wakeup interrupt status */
	mmio_clrbits_32(SPM2MCUPM_CON, SPM2MCUPM_SW_INT_LSB);
}

void __spm_hw_s1_state_monitor(int en, uint32_t *status)
{
	uint32_t reg;

	if (en) {
		mmio_clrsetbits_32(SPM_ACK_CHK_CON_3,
				   SPM_ACK_CHK_3_CON_CLR_ALL,
				   SPM_ACK_CHK_3_CON_EN);
	} else {

		reg = mmio_read_32(SPM_ACK_CHK_CON_3);

		if (reg & SPM_ACK_CHK_3_CON_RESULT) {
			if (status)
				*status |= SPM_INTERNAL_STATUS_HW_S1;
		}
		mmio_clrsetbits_32(SPM_ACK_CHK_CON_3, SPM_ACK_CHK_3_CON_EN,
				   (SPM_ACK_CHK_3_CON_HW_MODE_TRIG |
				   SPM_ACK_CHK_3_CON_CLR_ALL));
	}
}
