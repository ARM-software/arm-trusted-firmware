/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>

#include <common/debug.h>
#include <drivers/gpio.h>
#include <lib/mmio.h>

#include <constraints/mt_spm_rc_internal.h>
#include <drivers/spm/mt_spm_resource_req.h>
#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lp_api.h>
#include <lpm_v2/mt_lp_rqm.h>
#include <mt_spm.h>
#include <mt_spm_conservation.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_stats.h>
#include <mt_spm_suspend.h>
#if defined(CONFIG_MTK_VCOREDVFS_SUPPORT)
#include <mt_spm_vcorefs_exp.h>
#endif

#define SPM_SUSPEND_SLEEP_PCM_FLAG                                  \
	(SPM_FLAG_DISABLE_INFRA_PDN | SPM_FLAG_DISABLE_DPM_PDN |    \
	 SPM_FLAG_DISABLE_MCUPM_PDN | SPM_FLAG_DISABLE_VCORE_DVS |  \
	 SPM_FLAG_DISABLE_DDR_DFS | SPM_FLAG_DISABLE_EMI_DFS |      \
	 SPM_FLAG_DISABLE_BUS_DFS | SPM_FLAG_KEEP_CSYSPWRACK_HIGH | \
	 SPM_FLAG_SRAM_SLEEP_CTRL)

#define SPM_SUSPEND_SLEEP_PCM_FLAG1 (SPM_FLAG1_ENABLE_VS2_VS3_VOTER)

#define SPM_SUSPEND_PCM_FLAG                                     \
	(SPM_FLAG_DISABLE_VCORE_DVS | SPM_FLAG_DISABLE_DDR_DFS | \
	 SPM_FLAG_DISABLE_EMI_DFS | SPM_FLAG_DISABLE_BUS_DFS |   \
	 SPM_FLAG_SRAM_SLEEP_CTRL)

#define SPM_SUSPEND_PCM_FLAG1                                           \
	(SPM_FLAG1_ENABLE_VS2_VS3_VOTER	| SPM_FLAG1_DISABLE_PERI_OFF |  \
	 SPM_FLAG1_ENABLE_MCU_INFRA_PARITY)

/* Suspend spm power control */
#define __WAKE_SRC_FOR_SUSPEND_COMMON__                                        \
	((R12_PCM_TIMER_B) | (R12_KP_IRQ_B) | (R12_CONN2AP_WAKEUP_B) |         \
	 (R12_EINT_EVENT_B) | (R12_CONN_WDT_IRQ_B) | (R12_SSPM2SPM_WAKEUP_B) | \
	 (R12_SCP2SPM_WAKEUP_B) | (R12_VADSP2SPM_WAKEUP_B) |                   \
	 (R12_USB0_CDSC_B) | (R12_USB0_POWERDWN_B) | (R12_SBD_INTR_B) |        \
	 (R12_UART2SPM_IRQ_B) | (R12_SYS_TIMER_EVENT_B) |                      \
	 (R12_EINT_EVENT_SECURE_B) | (R12_SYS_CIRQ_IRQ_B) | (R12_CPU_WAKEUP) | \
	 (R12_APUSYS_WAKE_HOST_B) | (R12_PCIE_MAC_IRQ_WAKE_B) |                \
	 (R12_MSDC_WAKEUP_EVENT_B))

#if defined(CFG_MICROTRUST_TEE_SUPPORT)
#define WAKE_SRC_FOR_SUSPEND (__WAKE_SRC_FOR_SUSPEND_COMMON__)
#else
#define WAKE_SRC_FOR_SUSPEND (__WAKE_SRC_FOR_SUSPEND_COMMON__ | R12_SEJ_B)
#endif

static struct pwr_ctrl suspend_ctrl = {
	.wake_src = WAKE_SRC_FOR_SUSPEND,

	/* Auto-gen Start */

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
	.reg_apu_apsrc_req_mask_b = 0x1,
	.reg_apu_ddren_req_mask_b = 0x1,
	.reg_apu_emi_req_mask_b = 0x1,
	.reg_apu_infra_req_mask_b = 0x1,
	.reg_apu_pmic_req_mask_b = 0x1,
	.reg_apu_srcclkena_mask_b = 0x1,
	.reg_apu_vrf18_req_mask_b = 0x1,
	.reg_audio_dsp_apsrc_req_mask_b = 0x0,
	.reg_audio_dsp_ddren_req_mask_b = 0x0,
	.reg_audio_dsp_emi_req_mask_b = 0x0,
	.reg_audio_dsp_infra_req_mask_b = 0x0,
	.reg_audio_dsp_pmic_req_mask_b = 0x0,
	.reg_audio_dsp_srcclkena_mask_b = 0x0,
	.reg_audio_dsp_vcore_req_mask_b = 0x0,
	.reg_audio_dsp_vrf18_req_mask_b = 0x0,
	.reg_cam_apsrc_req_mask_b = 0x1,
	.reg_cam_ddren_req_mask_b = 0x1,
	.reg_cam_emi_req_mask_b = 0x1,
	.reg_cam_infra_req_mask_b = 0x0,
	.reg_cam_pmic_req_mask_b = 0x0,
	.reg_cam_srcclkena_mask_b = 0x0,
	.reg_cam_vrf18_req_mask_b = 0x0,
	.reg_mdp_emi_req_mask_b = 0x1,

	/* SPM_SRC_MASK_1 */
	.reg_ccif_apsrc_req_mask_b = 0x0,
	.reg_ccif_emi_req_mask_b = 0xfff,

	/* SPM_SRC_MASK_2 */
	.reg_ccif_infra_req_mask_b = 0x0,
	.reg_ccif_pmic_req_mask_b = 0xfff,

	/* SPM_SRC_MASK_3 */
	.reg_ccif_srcclkena_mask_b = 0x0,
	.reg_ccif_vrf18_req_mask_b = 0xfff,
	.reg_ccu_apsrc_req_mask_b = 0x0,
	.reg_ccu_ddren_req_mask_b = 0x0,
	.reg_ccu_emi_req_mask_b = 0x0,
	.reg_ccu_infra_req_mask_b = 0x0,
	.reg_ccu_pmic_req_mask_b = 0x0,
	.reg_ccu_srcclkena_mask_b = 0x0,
	.reg_ccu_vrf18_req_mask_b = 0x0,
	.reg_cg_check_apsrc_req_mask_b = 0x1,

	/* SPM_SRC_MASK_4 */
	.reg_cg_check_ddren_req_mask_b = 0x1,
	.reg_cg_check_emi_req_mask_b = 0x1,
	.reg_cg_check_infra_req_mask_b = 0x0,
	.reg_cg_check_pmic_req_mask_b = 0x0,
	.reg_cg_check_srcclkena_mask_b = 0x0,
	.reg_cg_check_vcore_req_mask_b = 0x1,
	.reg_cg_check_vrf18_req_mask_b = 0x0,
	.reg_conn_apsrc_req_mask_b = 0x1,
	.reg_conn_ddren_req_mask_b = 0x1,
	.reg_conn_emi_req_mask_b = 0x1,
	.reg_conn_infra_req_mask_b = 0x1,
	.reg_conn_pmic_req_mask_b = 0x1,
	.reg_conn_srcclkena_mask_b = 0x1,
	.reg_conn_srcclkenb_mask_b = 0x1,
	.reg_conn_vcore_req_mask_b = 0x1,
	.reg_conn_vrf18_req_mask_b = 0x1,
	.reg_cpueb_apsrc_req_mask_b = 0x1,
	.reg_cpueb_ddren_req_mask_b = 0x1,
	.reg_cpueb_emi_req_mask_b = 0x1,
	.reg_cpueb_infra_req_mask_b = 0x1,
	.reg_cpueb_pmic_req_mask_b = 0x1,
	.reg_cpueb_srcclkena_mask_b = 0x1,
	.reg_cpueb_vrf18_req_mask_b = 0x1,
	.reg_disp0_apsrc_req_mask_b = 0x1,
	.reg_disp0_ddren_req_mask_b = 0x1,
	.reg_disp0_emi_req_mask_b = 0x1,
	.reg_disp0_infra_req_mask_b = 0x1,
	.reg_disp0_pmic_req_mask_b = 0x0,
	.reg_disp0_srcclkena_mask_b = 0x0,
	.reg_disp0_vrf18_req_mask_b = 0x1,
	.reg_disp1_apsrc_req_mask_b = 0x0,
	.reg_disp1_ddren_req_mask_b = 0x0,

	/* SPM_SRC_MASK_5 */
	.reg_disp1_emi_req_mask_b = 0x0,
	.reg_disp1_infra_req_mask_b = 0x0,
	.reg_disp1_pmic_req_mask_b = 0x0,
	.reg_disp1_srcclkena_mask_b = 0x0,
	.reg_disp1_vrf18_req_mask_b = 0x0,
	.reg_dpm_apsrc_req_mask_b = 0xf,
	.reg_dpm_ddren_req_mask_b = 0xf,
	.reg_dpm_emi_req_mask_b = 0xf,
	.reg_dpm_infra_req_mask_b = 0xf,
	.reg_dpm_pmic_req_mask_b = 0xf,
	.reg_dpm_srcclkena_mask_b = 0xf,

	/* SPM_SRC_MASK_6 */
	.reg_dpm_vcore_req_mask_b = 0xf,
	.reg_dpm_vrf18_req_mask_b = 0xf,
	.reg_dpmaif_apsrc_req_mask_b = 0x1,
	.reg_dpmaif_ddren_req_mask_b = 0x1,
	.reg_dpmaif_emi_req_mask_b = 0x1,
	.reg_dpmaif_infra_req_mask_b = 0x1,
	.reg_dpmaif_pmic_req_mask_b = 0x1,
	.reg_dpmaif_srcclkena_mask_b = 0x1,
	.reg_dpmaif_vrf18_req_mask_b = 0x1,
	.reg_dvfsrc_level_req_mask_b = 0x1,
	.reg_emisys_apsrc_req_mask_b = 0x0,
	.reg_emisys_ddren_req_mask_b = 0x1,
	.reg_emisys_emi_req_mask_b = 0x0,
	.reg_gce_d_apsrc_req_mask_b = 0x1,
	.reg_gce_d_ddren_req_mask_b = 0x1,
	.reg_gce_d_emi_req_mask_b = 0x1,
	.reg_gce_d_infra_req_mask_b = 0x0,
	.reg_gce_d_pmic_req_mask_b = 0x0,
	.reg_gce_d_srcclkena_mask_b = 0x0,
	.reg_gce_d_vrf18_req_mask_b = 0x0,
	.reg_gce_m_apsrc_req_mask_b = 0x1,
	.reg_gce_m_ddren_req_mask_b = 0x1,
	.reg_gce_m_emi_req_mask_b = 0x1,
	.reg_gce_m_infra_req_mask_b = 0x0,
	.reg_gce_m_pmic_req_mask_b = 0x0,
	.reg_gce_m_srcclkena_mask_b = 0x0,

	/* SPM_SRC_MASK_7 */
	.reg_gce_m_vrf18_req_mask_b = 0x0,
	.reg_gpueb_apsrc_req_mask_b = 0x0,
	.reg_gpueb_ddren_req_mask_b = 0x0,
	.reg_gpueb_emi_req_mask_b = 0x0,
	.reg_gpueb_infra_req_mask_b = 0x0,
	.reg_gpueb_pmic_req_mask_b = 0x0,
	.reg_gpueb_srcclkena_mask_b = 0x0,
	.reg_gpueb_vrf18_req_mask_b = 0x0,
	.reg_hwccf_apsrc_req_mask_b = 0x1,
	.reg_hwccf_ddren_req_mask_b = 0x1,
	.reg_hwccf_emi_req_mask_b = 0x1,
	.reg_hwccf_infra_req_mask_b = 0x1,
	.reg_hwccf_pmic_req_mask_b = 0x1,
	.reg_hwccf_srcclkena_mask_b = 0x1,
	.reg_hwccf_vcore_req_mask_b = 0x1,
	.reg_hwccf_vrf18_req_mask_b = 0x1,
	.reg_img_apsrc_req_mask_b = 0x1,
	.reg_img_ddren_req_mask_b = 0x1,
	.reg_img_emi_req_mask_b = 0x1,
	.reg_img_infra_req_mask_b = 0x0,
	.reg_img_pmic_req_mask_b = 0x0,
	.reg_img_srcclkena_mask_b = 0x0,
	.reg_img_vrf18_req_mask_b = 0x0,
	.reg_infrasys_apsrc_req_mask_b = 0x1,
	.reg_infrasys_ddren_req_mask_b = 0x1,
	.reg_infrasys_emi_req_mask_b = 0x1,
	.reg_ipic_infra_req_mask_b = 0x1,
	.reg_ipic_vrf18_req_mask_b = 0x1,
	.reg_mcu_apsrc_req_mask_b = 0x0,
	.reg_mcu_ddren_req_mask_b = 0x0,
	.reg_mcu_emi_req_mask_b = 0x0,

	/* SPM_SRC_MASK_8 */
	.reg_mcusys_apsrc_req_mask_b = 0x7,
	.reg_mcusys_ddren_req_mask_b = 0x7,
	.reg_mcusys_emi_req_mask_b = 0x7,
	.reg_mcusys_infra_req_mask_b = 0x0,

	/* SPM_SRC_MASK_9 */
	.reg_mcusys_pmic_req_mask_b = 0x0,
	.reg_mcusys_srcclkena_mask_b = 0x0,
	.reg_mcusys_vrf18_req_mask_b = 0x0,
	.reg_md_apsrc_req_mask_b = 0x0,
	.reg_md_ddren_req_mask_b = 0x0,
	.reg_md_emi_req_mask_b = 0x0,
	.reg_md_infra_req_mask_b = 0x0,
	.reg_md_pmic_req_mask_b = 0x0,
	.reg_md_srcclkena_mask_b = 0x0,
	.reg_md_srcclkena1_mask_b = 0x0,
	.reg_md_vcore_req_mask_b = 0x0,

	/* SPM_SRC_MASK_10 */
	.reg_md_vrf18_req_mask_b = 0x0,
	.reg_mdp_apsrc_req_mask_b = 0x0,
	.reg_mdp_ddren_req_mask_b = 0x0,
	.reg_mm_proc_apsrc_req_mask_b = 0x0,
	.reg_mm_proc_ddren_req_mask_b = 0x0,
	.reg_mm_proc_emi_req_mask_b = 0x0,
	.reg_mm_proc_infra_req_mask_b = 0x0,
	.reg_mm_proc_pmic_req_mask_b = 0x0,
	.reg_mm_proc_srcclkena_mask_b = 0x0,
	.reg_mm_proc_vrf18_req_mask_b = 0x0,
	.reg_mmsys_apsrc_req_mask_b = 0x0,
	.reg_mmsys_ddren_req_mask_b = 0x0,
	.reg_mmsys_vrf18_req_mask_b = 0x0,
	.reg_pcie0_apsrc_req_mask_b = 0x0,
	.reg_pcie0_ddren_req_mask_b = 0x0,
	.reg_pcie0_infra_req_mask_b = 0x0,
	.reg_pcie0_srcclkena_mask_b = 0x0,
	.reg_pcie0_vrf18_req_mask_b = 0x0,
	.reg_pcie1_apsrc_req_mask_b = 0x0,
	.reg_pcie1_ddren_req_mask_b = 0x0,
	.reg_pcie1_infra_req_mask_b = 0x0,
	.reg_pcie1_srcclkena_mask_b = 0x0,
	.reg_pcie1_vrf18_req_mask_b = 0x0,
	.reg_perisys_apsrc_req_mask_b = 0x1,
	.reg_perisys_ddren_req_mask_b = 0x1,
	.reg_perisys_emi_req_mask_b = 0x1,
	.reg_perisys_infra_req_mask_b = 0x1,
	.reg_perisys_pmic_req_mask_b = 0x1,
	.reg_perisys_srcclkena_mask_b = 0x1,
	.reg_perisys_vcore_req_mask_b = 0x1,
	.reg_perisys_vrf18_req_mask_b = 0x1,
	.reg_scp_apsrc_req_mask_b = 0x1,

	/* SPM_SRC_MASK_11 */
	.reg_scp_ddren_req_mask_b = 0x1,
	.reg_scp_emi_req_mask_b = 0x1,
	.reg_scp_infra_req_mask_b = 0x1,
	.reg_scp_pmic_req_mask_b = 0x1,
	.reg_scp_srcclkena_mask_b = 0x1,
	.reg_scp_vcore_req_mask_b = 0x1,
	.reg_scp_vrf18_req_mask_b = 0x1,
	.reg_srcclkeni_infra_req_mask_b = 0x1,
	.reg_srcclkeni_pmic_req_mask_b = 0x1,
	.reg_srcclkeni_srcclkena_mask_b = 0x1,
	.reg_sspm_apsrc_req_mask_b = 0x1,
	.reg_sspm_ddren_req_mask_b = 0x1,
	.reg_sspm_emi_req_mask_b = 0x1,
	.reg_sspm_infra_req_mask_b = 0x1,
	.reg_sspm_pmic_req_mask_b = 0x1,
	.reg_sspm_srcclkena_mask_b = 0x1,
	.reg_sspm_vrf18_req_mask_b = 0x1,
	.reg_ssr_apsrc_req_mask_b = 0x0,
	.reg_ssr_ddren_req_mask_b = 0x0,
	.reg_ssr_emi_req_mask_b = 0x0,
	.reg_ssr_infra_req_mask_b = 0x0,
	.reg_ssr_pmic_req_mask_b = 0x0,
	.reg_ssr_srcclkena_mask_b = 0x0,
	.reg_ssr_vrf18_req_mask_b = 0x0,
	.reg_ufs_apsrc_req_mask_b = 0x1,
	.reg_ufs_ddren_req_mask_b = 0x1,
	.reg_ufs_emi_req_mask_b = 0x1,
	.reg_ufs_infra_req_mask_b = 0x1,
	.reg_ufs_pmic_req_mask_b = 0x1,

	/* SPM_SRC_MASK_12 */
	.reg_ufs_srcclkena_mask_b = 0x1,
	.reg_ufs_vrf18_req_mask_b = 0x1,
	.reg_vdec_apsrc_req_mask_b = 0x1,
	.reg_vdec_ddren_req_mask_b = 0x1,
	.reg_vdec_emi_req_mask_b = 0x1,
	.reg_vdec_infra_req_mask_b = 0x0,
	.reg_vdec_pmic_req_mask_b = 0x0,
	.reg_vdec_srcclkena_mask_b = 0x0,
	.reg_vdec_vrf18_req_mask_b = 0x0,
	.reg_venc_apsrc_req_mask_b = 0x1,
	.reg_venc_ddren_req_mask_b = 0x1,
	.reg_venc_emi_req_mask_b = 0x1,
	.reg_venc_infra_req_mask_b = 0x0,
	.reg_venc_pmic_req_mask_b = 0x0,
	.reg_venc_srcclkena_mask_b = 0x0,
	.reg_venc_vrf18_req_mask_b = 0x0,
	.reg_ipe_apsrc_req_mask_b = 0x1,
	.reg_ipe_ddren_req_mask_b = 0x1,
	.reg_ipe_emi_req_mask_b = 0x1,
	.reg_ipe_infra_req_mask_b = 0x1,
	.reg_ipe_pmic_req_mask_b = 0x1,
	.reg_ipe_srcclkena_mask_b = 0x1,
	.reg_ipe_vrf18_req_mask_b = 0x1,
	.reg_ufs_vcore_req_mask_b = 0x1,

	/* SPM_EVENT_CON_MISC */
	.reg_srcclken_fast_resp = 0,
	.reg_csyspwrup_ack_mask = 1,

	/* Auto-gen End */

	/* SPM_WAKEUP_EVENT_MASK */
	.reg_wakeup_event_mask = 0xC1B33012,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/*sw flag setting */
	.pcm_flags = SPM_SUSPEND_PCM_FLAG,
	.pcm_flags1 = SPM_SUSPEND_PCM_FLAG1,
};

static struct suspend_dbg_ctrl suspend_spm_dbg_ext = {
	.sleep_suspend_cnt = 0,
};

static struct dbg_ctrl suspend_spm_dbg = {
	.count = 0,
	.duration = 0,
	.ext = &suspend_spm_dbg_ext,
};

static struct spm_lp_stat suspend_lp_stat;

struct spm_lp_scen __spm_suspend = {
	.pwrctrl = &suspend_ctrl,
	.dbgctrl = &suspend_spm_dbg,
	.lpstat = &suspend_lp_stat,
};

static uint8_t bak_spm_vcore_req;

int mt_spm_suspend_mode_set(enum mt_spm_suspend_mode mode, void *prv)
{
	if (mode == MT_SPM_SUSPEND_SLEEP) {
		suspend_ctrl.pcm_flags = SPM_SUSPEND_SLEEP_PCM_FLAG;
		suspend_ctrl.pcm_flags1 = SPM_SUSPEND_SLEEP_PCM_FLAG1;
		suspend_ctrl.reg_spm_vcore_req = 1; /* disable AOC */
	} else {
		suspend_ctrl.pcm_flags = SPM_SUSPEND_PCM_FLAG;
		suspend_ctrl.pcm_flags1 = SPM_SUSPEND_PCM_FLAG1;
	}
	return 0;
}

static void mt_spm_suspend_ec_pin(void)
{
	/* GPIO140 LOW */
	gpio_set_value(EC_SUSPEND_PIN, GPIO_LEVEL_LOW);
}

static void mt_spm_resume_ec_pin(void)
{
	/* GPIO140 HIGH */
	gpio_set_value(EC_SUSPEND_PIN, GPIO_LEVEL_HIGH);
}

int mt_spm_suspend_enter(int state_id,
			 uint32_t ext_opand, uint32_t resource_req)
{
	int ret = 0;

	bak_spm_vcore_req = suspend_ctrl.reg_spm_vcore_req;

	/* if FMAudio, ADSP, USB headset is active, change to sleep suspend mode */
	if (ext_opand & MT_SPM_EX_OP_SET_SUSPEND_MODE)
		mt_spm_suspend_mode_set(MT_SPM_SUSPEND_SLEEP, NULL);

	mmio_write_32(SPM2SW_MAILBOX_0, 0x1);

	ext_opand |= MT_SPM_EX_OP_DEVICES_SAVE;

#if defined(CONFIG_MTK_VCOREDVFS_SUPPORT)
	/* Notify vcoredvfs suspend enter */
	spm_vcorefs_plat_suspend();
#endif

	ret = spm_conservation(state_id, ext_opand, &__spm_suspend,
			       resource_req);
	if (ret == 0) {
		struct mt_lp_publish_event event = {
			.id = MT_LPM_PUBEVENTS_SYS_POWER_OFF,
			.val.u32 = 0,
			.level = MT_LP_SYSPOWER_LEVEL_SUSPEND,
		};

		MT_LP_SUSPEND_PUBLISH_EVENT(&event);
	}

	mt_spm_suspend_ec_pin();

	return ret;
}

void mt_spm_suspend_resume(int state_id, uint32_t ext_opand,
			   struct wake_status **status)
{
	struct mt_lp_publish_event event;
	struct wake_status *st = NULL;

	ext_opand |= MT_SPM_EX_OP_DEVICES_SAVE;

	mt_spm_resume_ec_pin();

	spm_conservation_finish(state_id, ext_opand, &__spm_suspend, &st);

	mt_spm_update_lp_stat(&suspend_lp_stat);
#if defined(CONFIG_MTK_VCOREDVFS_SUPPORT)
	/* Notify vcoredvfs suspend enter */
	spm_vcorefs_plat_resume();
	mmio_write_32(SPM2SW_MAILBOX_0, 0x0);
#endif

	/*****************************************
	 * if FMAudio, ADSP, USB headset is active,
	 * change back to suspend mode and counting in resume
	 *****************************************/

	if (ext_opand & MT_SPM_EX_OP_SET_SUSPEND_MODE) {
		mt_spm_suspend_mode_set(MT_SPM_SUSPEND_SYSTEM_PDN, NULL);
		suspend_spm_dbg_ext.sleep_suspend_cnt += 1;
	}

	suspend_ctrl.reg_spm_vcore_req = bak_spm_vcore_req;

	suspend_spm_dbg.count += 1;
	event.id = MT_LPM_PUBEVENTS_SYS_POWER_ON;
	event.val.u32 = 0;
	event.level = MT_LP_SYSPOWER_LEVEL_SUSPEND;

	if (st) {
		if (st->tr.comm.r12 & R12_AP2AP_PEER_WAKEUP_B)
			event.val.u32 = MT_LPM_WAKE_MD_WAKEUP_DPMAIF;
		if (st->tr.comm.r12 & R12_CCIF0_EVENT_B)
			event.val.u32 = MT_LPM_WAKE_MD_WAKEUP_CCIF0;
		if (st->tr.comm.r12 & R12_CCIF1_EVENT_B)
			event.val.u32 = MT_LPM_WAKE_MD_WAKEUP_CCIF1;
	}
	if (status)
		*status = st;
	MT_LP_SUSPEND_PUBLISH_EVENT(&event);
}

int mt_spm_suspend_get_spm_lp(struct spm_lp_scen **lp)
{
	if (!lp)
		return -1;

	*lp = &__spm_suspend;
	return 0;
}
