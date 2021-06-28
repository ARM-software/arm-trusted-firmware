/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <mt_spm.h>
#include <mt_spm_conservation.h>
#include <mt_spm_idle.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <plat_pm.h>

#define __WAKE_SRC_FOR_IDLE_COMMON__	\
	(R12_PCM_TIMER |		\
	 R12_KP_IRQ_B |			\
	 R12_APWDT_EVENT_B |		\
	 R12_APXGPT1_EVENT_B |		\
	 R12_CONN2AP_SPM_WAKEUP_B |	\
	 R12_EINT_EVENT_B |		\
	 R12_CONN_WDT_IRQ_B |		\
	 R12_CCIF0_EVENT_B |		\
	 R12_SSPM2SPM_WAKEUP_B |	\
	 R12_SCP2SPM_WAKEUP_B |		\
	 R12_ADSP2SPM_WAKEUP_B |	\
	 R12_USBX_CDSC_B |		\
	 R12_USBX_POWERDWN_B |		\
	 R12_SYS_TIMER_EVENT_B |	\
	 R12_EINT_EVENT_SECURE_B |	\
	 R12_AFE_IRQ_MCU_B |		\
	 R12_SYS_CIRQ_IRQ_B |		\
	 R12_MD2AP_PEER_EVENT_B |	\
	 R12_MD1_WDT_B |		\
	 R12_CLDMA_EVENT_B |		\
	 R12_REG_CPU_WAKEUP |		\
	 R12_APUSYS_WAKE_HOST_B)

#if defined(CFG_MICROTRUST_TEE_SUPPORT)
#define WAKE_SRC_FOR_IDLE (__WAKE_SRC_FOR_IDLE_COMMON__)
#else
#define WAKE_SRC_FOR_IDLE		\
	(__WAKE_SRC_FOR_IDLE_COMMON__ |	\
	  R12_SEJ_EVENT_B)
#endif

static struct pwr_ctrl idle_spm_pwr = {
	.wake_src = WAKE_SRC_FOR_IDLE,

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
	.reg_spm_f26m_req = 1,
	/* [3] */
	.reg_spm_infra_req = 1,
	/* [4] */
	.reg_spm_vrf18_req = 0,
	/* [7] FIXME: default disable HW Auto S1 */
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
	.reg_mcusys_merge_apsrc_req_mask_b = 0x17,
	/* [17:9] */
	.reg_mcusys_merge_ddr_en_mask_b = 0x17,
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
	.reg_wakeup_event_mask = 0xC1282203,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	/* [31:0] */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,
};

struct spm_lp_scen idle_spm_lp = {
	.pwrctrl = &idle_spm_pwr,
};

int mt_spm_idle_generic_enter(int state_id, unsigned int ext_opand,
			      spm_idle_conduct fn)
{
	unsigned int src_req = 0;

	if (fn != NULL) {
		fn(&idle_spm_lp, &src_req);
	}

	return spm_conservation(state_id, ext_opand, &idle_spm_lp, src_req);
}
void mt_spm_idle_generic_resume(int state_id, unsigned int ext_opand,
				struct wake_status **status)
{
	spm_conservation_finish(state_id, ext_opand, &idle_spm_lp, status);
}

void mt_spm_idle_generic_init(void)
{
	spm_conservation_pwrctrl_init(idle_spm_lp.pwrctrl);
}
