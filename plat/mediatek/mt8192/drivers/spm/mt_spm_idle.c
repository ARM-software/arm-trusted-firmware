/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
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
	 R12_CCIF1_EVENT_B |		\
	 R12_AFE_IRQ_MCU_B |		\
	 R12_SYS_CIRQ_IRQ_B |		\
	 R12_MD2AP_PEER_EVENT_B |	\
	 R12_MD1_WDT_B |		\
	 R12_CLDMA_EVENT_B |		\
	 R12_REG_CPU_WAKEUP |		\
	 R12_APUSYS_WAKE_HOST_B |	\
	 R12_PCIE_BRIDGE_IRQ |		\
	 R12_PCIE_IRQ)

#if defined(CFG_MICROTRUST_TEE_SUPPORT)
#define WAKE_SRC_FOR_IDLE (__WAKE_SRC_FOR_IDLE_COMMON__)
#else
#define WAKE_SRC_FOR_IDLE		\
	(__WAKE_SRC_FOR_IDLE_COMMON__ |	\
	  R12_SEJ_EVENT_B)
#endif

static struct pwr_ctrl idle_spm_pwr = {
	.timer_val = 0x28000,
	.wake_src = WAKE_SRC_FOR_IDLE,

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
	.reg_dpmaif_srcclkena_mask_b = 1,
	.reg_dpmaif_infra_req_mask_b = 1,
	.reg_dpmaif_apsrc_req_mask_b = 1,
	.reg_dpmaif_vrf18_req_mask_b = 1,
	.reg_dpmaif_ddr_en_mask_b    = 1,

	/* SPM_SRC_REQ */
	.reg_spm_apsrc_req = 1,
	.reg_spm_f26m_req = 1,
	.reg_spm_infra_req = 1,
	.reg_spm_vrf18_req = 1,
	.reg_spm_ddr_en_req = 1,
	.reg_spm_dvfs_req = 0,
	.reg_spm_sw_mailbox_req = 0,
	.reg_spm_sspm_mailbox_req = 0,
	.reg_spm_adsp_mailbox_req = 0,
	.reg_spm_scp_mailbox_req = 0,

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
	.reg_wakeup_event_mask = 0x01282202,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/* Auto-gen End */
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
