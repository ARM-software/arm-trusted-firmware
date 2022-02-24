/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <mt_spm.h>
#include <mt_spm_conservation.h>
#include <mt_spm_internal.h>
#include <mt_spm_rc_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <mt_spm_suspend.h>
#include <plat_pm.h>
#include <uart.h>

#define SPM_SUSPEND_SLEEP_PCM_FLAG		\
	(SPM_FLAG_DISABLE_INFRA_PDN |		\
	 SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS |		\
	 SPM_FLAG_USE_SRCCLKENO2)

#define SPM_SUSPEND_SLEEP_PCM_FLAG1		(0U)

#define SPM_SUSPEND_PCM_FLAG			\
	(SPM_FLAG_DISABLE_VCORE_DVS |		\
	 SPM_FLAG_DISABLE_VCORE_DFS)

#define SPM_SUSPEND_PCM_FLAG1			(0U)

#define __WAKE_SRC_FOR_SUSPEND_COMMON__		\
	(R12_PCM_TIMER |			\
	 R12_KP_IRQ_B |				\
	 R12_APWDT_EVENT_B |			\
	 R12_CONN2AP_SPM_WAKEUP_B |		\
	 R12_EINT_EVENT_B |			\
	 R12_CONN_WDT_IRQ_B |			\
	 R12_SSPM2SPM_WAKEUP_B |		\
	 R12_SCP2SPM_WAKEUP_B |			\
	 R12_ADSP2SPM_WAKEUP_B |		\
	 R12_USBX_CDSC_B |			\
	 R12_USBX_POWERDWN_B |			\
	 R12_SYS_TIMER_EVENT_B |		\
	 R12_EINT_EVENT_SECURE_B |		\
	 R12_SYS_CIRQ_IRQ_B |			\
	 R12_NNA_WAKEUP |			\
	 R12_REG_CPU_WAKEUP)

#if defined(CFG_MICROTRUST_TEE_SUPPORT)
#define WAKE_SRC_FOR_SUSPEND (__WAKE_SRC_FOR_SUSPEND_COMMON__)
#else
#define WAKE_SRC_FOR_SUSPEND			\
	(__WAKE_SRC_FOR_SUSPEND_COMMON__ |	\
	 R12_SEJ_EVENT_B)
#endif

static struct pwr_ctrl suspend_ctrl = {
	.wake_src = WAKE_SRC_FOR_SUSPEND,

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
	.reg_ccif_event_infra_req_mask_b = 0,
	.reg_ccif_event_apsrc_req_mask_b = 0,

	/* SPM_SRC_REQ */
	.reg_spm_apsrc_req = 0,
	.reg_spm_f26m_req = 0,
	.reg_spm_infra_req = 0,
	.reg_spm_vrf18_req = 0,
	.reg_spm_ddren_req = 0,
	.reg_spm_dvfs_req = 0,
	.reg_spm_sw_mailbox_req = 0,
	.reg_spm_sspm_mailbox_req = 0,
	.reg_spm_adsp_mailbox_req = 0,
	.reg_spm_scp_mailbox_req = 0,

	/* SPM_SRC_MASK */
	.reg_md_0_srcclkena_mask_b = 0,
	.reg_md_0_infra_req_mask_b = 0,
	.reg_md_0_apsrc_req_mask_b = 0,
	.reg_md_0_vrf18_req_mask_b = 0,
	.reg_md_0_ddren_req_mask_b = 0,
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
	.reg_mcupm_srcclkena_mask_b = 0,
	.reg_mcupm_infra_req_mask_b = 0,
	.reg_mcupm_apsrc_req_mask_b = 0,
	.reg_mcupm_vrf18_req_mask_b = 0,
	.reg_mcupm_ddren_req_mask_b = 0,
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
	.reg_ccif_event_srcclkena_mask_b = 0,
	.reg_bak_psri_srcclkena_mask_b = 0,
	.reg_bak_psri_infra_req_mask_b = 0,
	.reg_bak_psri_apsrc_req_mask_b = 0,
	.reg_bak_psri_vrf18_req_mask_b = 0,
	.reg_bak_psri_ddren_req_mask_b = 0,
	.reg_dramc_md32_infra_req_mask_b = 0,
	.reg_dramc_md32_vrf18_req_mask_b = 0,
	.reg_conn_srcclkenb2pwrap_mask_b = 0,
	.reg_dramc_md32_apsrc_req_mask_b = 0,

	/* SPM_SRC5_MASK */
	.reg_mcusys_merge_apsrc_req_mask_b = 0x83,
	.reg_mcusys_merge_ddren_req_mask_b = 0x83,
	.reg_afe_srcclkena_mask_b = 1,
	.reg_afe_infra_req_mask_b = 1,
	.reg_afe_apsrc_req_mask_b = 1,
	.reg_afe_vrf18_req_mask_b = 1,
	.reg_afe_ddren_req_mask_b = 1,
	.reg_msdc2_srcclkena_mask_b = 0,
	.reg_msdc2_infra_req_mask_b = 0,
	.reg_msdc2_apsrc_req_mask_b = 0,
	.reg_msdc2_vrf18_req_mask_b = 0,
	.reg_msdc2_ddren_req_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.reg_wakeup_event_mask = 0x1383213,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/* SPM_SRC7_MASK */
	.reg_pcie_srcclkena_mask_b = 0,
	.reg_pcie_infra_req_mask_b = 0,
	.reg_pcie_apsrc_req_mask_b = 0,
	.reg_pcie_vrf18_req_mask_b = 0,
	.reg_pcie_ddren_req_mask_b = 0,
	.reg_dpmaif_srcclkena_mask_b = 1,
	.reg_dpmaif_infra_req_mask_b = 1,
	.reg_dpmaif_apsrc_req_mask_b = 1,
	.reg_dpmaif_vrf18_req_mask_b = 1,
	.reg_dpmaif_ddren_req_mask_b = 1,

	/* Auto-gen End */

	/*sw flag setting */
	.pcm_flags = SPM_SUSPEND_PCM_FLAG,
	.pcm_flags1 = SPM_SUSPEND_PCM_FLAG1,
};

struct spm_lp_scen __spm_suspend = {
	.pwrctrl = &suspend_ctrl,
};

int mt_spm_suspend_mode_set(int mode)
{
	if (mode == MT_SPM_SUSPEND_SLEEP) {
		suspend_ctrl.pcm_flags = SPM_SUSPEND_SLEEP_PCM_FLAG;
		suspend_ctrl.pcm_flags1 = SPM_SUSPEND_SLEEP_PCM_FLAG1;
	} else {
		suspend_ctrl.pcm_flags = SPM_SUSPEND_PCM_FLAG;
		suspend_ctrl.pcm_flags1 = SPM_SUSPEND_PCM_FLAG1;
	}

	return 0;
}

int mt_spm_suspend_enter(int state_id, unsigned int ext_opand,
			 unsigned int resource_req)
{
	/* If FMAudio / ADSP is active, change to sleep suspend mode */
	if ((ext_opand & MT_SPM_EX_OP_SET_SUSPEND_MODE) != 0U) {
		mt_spm_suspend_mode_set(MT_SPM_SUSPEND_SLEEP);
	}

	/* Notify MCUPM that device is going suspend flow */
	mmio_write_32(MCUPM_MBOX_OFFSET_PDN, MCUPM_POWER_DOWN);

	/* Notify UART to sleep */
	mt_uart_save();

	return spm_conservation(state_id, ext_opand,
				&__spm_suspend, resource_req);
}

void mt_spm_suspend_resume(int state_id, unsigned int ext_opand,
			   struct wake_status **status)
{
	spm_conservation_finish(state_id, ext_opand, &__spm_suspend, status);

	/* Notify UART to wakeup */
	mt_uart_restore();

	/* Notify MCUPM that device leave suspend */
	mmio_write_32(MCUPM_MBOX_OFFSET_PDN, 0);

	/* If FMAudio / ADSP is active, change back to suspend mode */
	if ((ext_opand & MT_SPM_EX_OP_SET_SUSPEND_MODE) != 0U) {
		mt_spm_suspend_mode_set(MT_SPM_SUSPEND_SYSTEM_PDN);
	}
}

void mt_spm_suspend_init(void)
{
	spm_conservation_pwrctrl_init(__spm_suspend.pwrctrl);
}
