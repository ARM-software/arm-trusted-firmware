/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_INTERNAL_H
#define MT_SPM_INTERNAL_H

#include "mt_spm.h"

/**************************************
 * Config and Parameter
 **************************************/
#define POWER_ON_VAL0_DEF	0x0000F100
#define POWER_ON_VAL1_DEF	0x80015860
#define PCM_WDT_TIMEOUT		(30 * 32768)	/* 30s */
#define PCM_TIMER_MAX		(0xffffffff - PCM_WDT_TIMEOUT)

/**************************************
 * Define and Declare
 **************************************/
/* PCM_PWR_IO_EN */
#define PCM_PWRIO_EN_R0		(1U << 0)
#define PCM_PWRIO_EN_R7		(1U << 7)
#define PCM_RF_SYNC_R0		(1U << 16)
#define PCM_RF_SYNC_R6		(1U << 22)
#define PCM_RF_SYNC_R7		(1U << 23)

/* SPM_SWINT */
#define PCM_SW_INT0		(1U << 0)
#define PCM_SW_INT1		(1U << 1)
#define PCM_SW_INT2		(1U << 2)
#define PCM_SW_INT3		(1U << 3)
#define PCM_SW_INT4		(1U << 4)
#define PCM_SW_INT5		(1U << 5)
#define PCM_SW_INT6		(1U << 6)
#define PCM_SW_INT7		(1U << 7)
#define PCM_SW_INT8		(1U << 8)
#define PCM_SW_INT9		(1U << 9)
#define PCM_SW_INT_ALL		(PCM_SW_INT9 | PCM_SW_INT8 | PCM_SW_INT7 | \
				 PCM_SW_INT6 | PCM_SW_INT5 | PCM_SW_INT4 | \
				 PCM_SW_INT3 | PCM_SW_INT2 | PCM_SW_INT1 | \
				 PCM_SW_INT0)

/* SPM_AP_STANDBY_CON */
#define WFI_OP_AND		1
#define WFI_OP_OR		0

/* SPM_IRQ_MASK */
#define ISRM_TWAM		(1U << 2)
#define ISRM_PCM_RETURN		(1U << 3)
#define ISRM_RET_IRQ0		(1U << 8)
#define ISRM_RET_IRQ1		(1U << 9)
#define ISRM_RET_IRQ2		(1U << 10)
#define ISRM_RET_IRQ3		(1U << 11)
#define ISRM_RET_IRQ4		(1U << 12)
#define ISRM_RET_IRQ5		(1U << 13)
#define ISRM_RET_IRQ6		(1U << 14)
#define ISRM_RET_IRQ7		(1U << 15)
#define ISRM_RET_IRQ8		(1U << 16)
#define ISRM_RET_IRQ9		(1U << 17)
#define ISRM_RET_IRQ_AUX	((ISRM_RET_IRQ9) | (ISRM_RET_IRQ8) | \
				 (ISRM_RET_IRQ7) | (ISRM_RET_IRQ6) | \
				 (ISRM_RET_IRQ5) | (ISRM_RET_IRQ4) | \
				 (ISRM_RET_IRQ3) | (ISRM_RET_IRQ2) | \
				 (ISRM_RET_IRQ1))
#define ISRM_ALL_EXC_TWAM	(ISRM_RET_IRQ_AUX)
#define ISRM_ALL		(ISRM_ALL_EXC_TWAM | ISRM_TWAM)

/* SPM_IRQ_STA */
#define ISRS_TWAM		(1U << 2)
#define ISRS_PCM_RETURN		(1U << 3)
#define ISRC_TWAM		ISRS_TWAM
#define ISRC_ALL_EXC_TWAM	ISRS_PCM_RETURN
#define ISRC_ALL		(ISRC_ALL_EXC_TWAM | ISRC_TWAM)

/* SPM_WAKEUP_MISC */
#define WAKE_MISC_GIC_WAKEUP             0x3FF
#define WAKE_MISC_DVFSRC_IRQ	         DVFSRC_IRQ_LSB
#define WAKE_MISC_REG_CPU_WAKEUP         SPM_WAKEUP_MISC_REG_CPU_WAKEUP_LSB
#define WAKE_MISC_PCM_TIMER_EVENT        PCM_TIMER_EVENT_LSB
#define WAKE_MISC_PMIC_OUT_B		 ((1U << 19) | (1U << 20))
#define WAKE_MISC_TWAM_IRQ_B             TWAM_IRQ_B_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET0        PMSR_IRQ_B_SET0_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET1        PMSR_IRQ_B_SET1_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET2        PMSR_IRQ_B_SET2_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_0   SPM_ACK_CHK_WAKEUP_0_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_1	 SPM_ACK_CHK_WAKEUP_1_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_2	 SPM_ACK_CHK_WAKEUP_2_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_3	 SPM_ACK_CHK_WAKEUP_3_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL SPM_ACK_CHK_WAKEUP_ALL_LSB
#define WAKE_MISC_PMIC_IRQ_ACK           PMIC_IRQ_ACK_LSB
#define WAKE_MISC_PMIC_SCP_IRQ           PMIC_SCP_IRQ_LSB

/* ABORT MASK for DEBUG FOORTPRINT */
#define DEBUG_ABORT_MASK				\
	(SPM_DBG_DEBUG_IDX_DRAM_SREF_ABORT_IN_APSRC |	\
	 SPM_DBG_DEBUG_IDX_DRAM_SREF_ABORT_IN_DDREN)

#define DEBUG_ABORT_MASK_1					\
	(SPM_DBG1_DEBUG_IDX_VRCXO_SLEEP_ABORT |			\
	 SPM_DBG1_DEBUG_IDX_PWRAP_SLEEP_ACK_LOW_ABORT |		\
	 SPM_DBG1_DEBUG_IDX_PWRAP_SLEEP_ACK_HIGH_ABORT |	\
	 SPM_DBG1_DEBUG_IDX_EMI_SLP_IDLE_ABORT |		\
	 SPM_DBG1_DEBUG_IDX_SCP_SLP_ACK_LOW_ABORT |		\
	 SPM_DBG1_DEBUG_IDX_SCP_SLP_ACK_HIGH_ABORT |		\
	 SPM_DBG1_DEBUG_IDX_SPM_DVFS_CMD_RDY_ABORT)

#define MCUPM_MBOX_WAKEUP_CPU		0x0C55FD10

struct pwr_ctrl {
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
	uint8_t reg_srcclken0_ctl;
	uint8_t reg_srcclken1_ctl;
	uint8_t reg_spm_lock_infra_dcm;
	uint8_t reg_srcclken_mask;
	uint8_t reg_md1_c32rm_en;
	uint8_t reg_md2_c32rm_en;
	uint8_t reg_clksq0_sel_ctrl;
	uint8_t reg_clksq1_sel_ctrl;
	uint8_t reg_srcclken0_en;
	uint8_t reg_srcclken1_en;
	uint32_t reg_sysclk0_src_mask_b;
	uint32_t reg_sysclk1_src_mask_b;

	/* SPM_AP_STANDBY_CON */
	uint8_t reg_wfi_op;
	uint8_t reg_wfi_type;
	uint8_t reg_mp0_cputop_idle_mask;
	uint8_t reg_mp1_cputop_idle_mask;
	uint8_t reg_mcusys_idle_mask;
	uint8_t reg_md_apsrc_1_sel;
	uint8_t reg_md_apsrc_0_sel;
	uint8_t reg_conn_apsrc_sel;

	/* SPM_SRC6_MASK */
	uint8_t reg_dpmaif_srcclkena_mask_b;
	uint8_t reg_dpmaif_infra_req_mask_b;
	uint8_t reg_dpmaif_apsrc_req_mask_b;
	uint8_t reg_dpmaif_vrf18_req_mask_b;
	uint8_t reg_dpmaif_ddr_en_mask_b;
	/* SPM_SRC_REQ */
	uint8_t reg_spm_apsrc_req;
	uint8_t reg_spm_f26m_req;
	uint8_t reg_spm_infra_req;
	uint8_t reg_spm_vrf18_req;
	uint8_t reg_spm_ddr_en_req;
	uint8_t reg_spm_dvfs_req;
	uint8_t reg_spm_sw_mailbox_req;
	uint8_t reg_spm_sspm_mailbox_req;
	uint8_t reg_spm_adsp_mailbox_req;
	uint8_t reg_spm_scp_mailbox_req;

	/* SPM_SRC_MASK */
	uint8_t reg_md_srcclkena_0_mask_b;
	uint8_t reg_md_srcclkena2infra_req_0_mask_b;
	uint8_t reg_md_apsrc2infra_req_0_mask_b;
	uint8_t reg_md_apsrc_req_0_mask_b;
	uint8_t reg_md_vrf18_req_0_mask_b;
	uint8_t reg_md_ddr_en_0_mask_b;
	uint8_t reg_md_srcclkena_1_mask_b;
	uint8_t reg_md_srcclkena2infra_req_1_mask_b;
	uint8_t reg_md_apsrc2infra_req_1_mask_b;
	uint8_t reg_md_apsrc_req_1_mask_b;
	uint8_t reg_md_vrf18_req_1_mask_b;
	uint8_t reg_md_ddr_en_1_mask_b;
	uint8_t reg_conn_srcclkena_mask_b;
	uint8_t reg_conn_srcclkenb_mask_b;
	uint8_t reg_conn_infra_req_mask_b;
	uint8_t reg_conn_apsrc_req_mask_b;
	uint8_t reg_conn_vrf18_req_mask_b;
	uint8_t reg_conn_ddr_en_mask_b;
	uint8_t reg_conn_vfe28_mask_b;
	uint8_t reg_srcclkeni0_srcclkena_mask_b;
	uint8_t reg_srcclkeni0_infra_req_mask_b;
	uint8_t reg_srcclkeni1_srcclkena_mask_b;
	uint8_t reg_srcclkeni1_infra_req_mask_b;
	uint8_t reg_srcclkeni2_srcclkena_mask_b;
	uint8_t reg_srcclkeni2_infra_req_mask_b;
	uint8_t reg_infrasys_apsrc_req_mask_b;
	uint8_t reg_infrasys_ddr_en_mask_b;
	uint8_t reg_md32_srcclkena_mask_b;
	uint8_t reg_md32_infra_req_mask_b;
	uint8_t reg_md32_apsrc_req_mask_b;
	uint8_t reg_md32_vrf18_req_mask_b;
	uint8_t reg_md32_ddr_en_mask_b;

	/* SPM_SRC2_MASK */
	uint8_t reg_scp_srcclkena_mask_b;
	uint8_t reg_scp_infra_req_mask_b;
	uint8_t reg_scp_apsrc_req_mask_b;
	uint8_t reg_scp_vrf18_req_mask_b;
	uint8_t reg_scp_ddr_en_mask_b;
	uint8_t reg_audio_dsp_srcclkena_mask_b;
	uint8_t reg_audio_dsp_infra_req_mask_b;
	uint8_t reg_audio_dsp_apsrc_req_mask_b;
	uint8_t reg_audio_dsp_vrf18_req_mask_b;
	uint8_t reg_audio_dsp_ddr_en_mask_b;
	uint8_t reg_ufs_srcclkena_mask_b;
	uint8_t reg_ufs_infra_req_mask_b;
	uint8_t reg_ufs_apsrc_req_mask_b;
	uint8_t reg_ufs_vrf18_req_mask_b;
	uint8_t reg_ufs_ddr_en_mask_b;
	uint8_t reg_disp0_apsrc_req_mask_b;
	uint8_t reg_disp0_ddr_en_mask_b;
	uint8_t reg_disp1_apsrc_req_mask_b;
	uint8_t reg_disp1_ddr_en_mask_b;
	uint8_t reg_gce_infra_req_mask_b;
	uint8_t reg_gce_apsrc_req_mask_b;
	uint8_t reg_gce_vrf18_req_mask_b;
	uint8_t reg_gce_ddr_en_mask_b;
	uint8_t reg_apu_srcclkena_mask_b;
	uint8_t reg_apu_infra_req_mask_b;
	uint8_t reg_apu_apsrc_req_mask_b;
	uint8_t reg_apu_vrf18_req_mask_b;
	uint8_t reg_apu_ddr_en_mask_b;
	uint8_t reg_cg_check_srcclkena_mask_b;
	uint8_t reg_cg_check_apsrc_req_mask_b;
	uint8_t reg_cg_check_vrf18_req_mask_b;
	uint8_t reg_cg_check_ddr_en_mask_b;

	/* SPM_SRC3_MASK */
	uint8_t reg_dvfsrc_event_trigger_mask_b;
	uint8_t reg_sw2spm_int0_mask_b;
	uint8_t reg_sw2spm_int1_mask_b;
	uint8_t reg_sw2spm_int2_mask_b;
	uint8_t reg_sw2spm_int3_mask_b;
	uint8_t reg_sc_adsp2spm_wakeup_mask_b;
	uint8_t reg_sc_sspm2spm_wakeup_mask_b;
	uint8_t reg_sc_scp2spm_wakeup_mask_b;
	uint8_t reg_csyspwrreq_mask;
	uint8_t reg_spm_srcclkena_reserved_mask_b;
	uint8_t reg_spm_infra_req_reserved_mask_b;
	uint8_t reg_spm_apsrc_req_reserved_mask_b;
	uint8_t reg_spm_vrf18_req_reserved_mask_b;
	uint8_t reg_spm_ddr_en_reserved_mask_b;
	uint8_t reg_mcupm_srcclkena_mask_b;
	uint8_t reg_mcupm_infra_req_mask_b;
	uint8_t reg_mcupm_apsrc_req_mask_b;
	uint8_t reg_mcupm_vrf18_req_mask_b;
	uint8_t reg_mcupm_ddr_en_mask_b;
	uint8_t reg_msdc0_srcclkena_mask_b;
	uint8_t reg_msdc0_infra_req_mask_b;
	uint8_t reg_msdc0_apsrc_req_mask_b;
	uint8_t reg_msdc0_vrf18_req_mask_b;
	uint8_t reg_msdc0_ddr_en_mask_b;
	uint8_t reg_msdc1_srcclkena_mask_b;
	uint8_t reg_msdc1_infra_req_mask_b;
	uint8_t reg_msdc1_apsrc_req_mask_b;
	uint8_t reg_msdc1_vrf18_req_mask_b;
	uint8_t reg_msdc1_ddr_en_mask_b;

	/* SPM_SRC4_MASK */
	uint32_t ccif_event_mask_b;
	uint8_t reg_bak_psri_srcclkena_mask_b;
	uint8_t reg_bak_psri_infra_req_mask_b;
	uint8_t reg_bak_psri_apsrc_req_mask_b;
	uint8_t reg_bak_psri_vrf18_req_mask_b;
	uint8_t reg_bak_psri_ddr_en_mask_b;
	uint8_t reg_dramc0_md32_infra_req_mask_b;
	uint8_t reg_dramc0_md32_vrf18_req_mask_b;
	uint8_t reg_dramc1_md32_infra_req_mask_b;
	uint8_t reg_dramc1_md32_vrf18_req_mask_b;
	uint8_t reg_conn_srcclkenb2pwrap_mask_b;
	uint8_t reg_dramc0_md32_wakeup_mask;
	uint8_t reg_dramc1_md32_wakeup_mask;

	/* SPM_SRC5_MASK */
	uint32_t reg_mcusys_merge_apsrc_req_mask_b;
	uint32_t reg_mcusys_merge_ddr_en_mask_b;
	uint8_t reg_msdc2_srcclkena_mask_b;
	uint8_t reg_msdc2_infra_req_mask_b;
	uint8_t reg_msdc2_apsrc_req_mask_b;
	uint8_t reg_msdc2_vrf18_req_mask_b;
	uint8_t reg_msdc2_ddr_en_mask_b;
	uint8_t reg_pcie_srcclkena_mask_b;
	uint8_t reg_pcie_infra_req_mask_b;
	uint8_t reg_pcie_apsrc_req_mask_b;
	uint8_t reg_pcie_vrf18_req_mask_b;
	uint8_t reg_pcie_ddr_en_mask_b;

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

	/* SPM_CLK_CON */
	PW_REG_SRCCLKEN0_CTL,
	PW_REG_SRCCLKEN1_CTL,
	PW_REG_SPM_LOCK_INFRA_DCM,
	PW_REG_SRCCLKEN_MASK,
	PW_REG_MD1_C32RM_EN,
	PW_REG_MD2_C32RM_EN,
	PW_REG_CLKSQ0_SEL_CTRL,
	PW_REG_CLKSQ1_SEL_CTRL,
	PW_REG_SRCCLKEN0_EN,
	PW_REG_SRCCLKEN1_EN,
	PW_REG_SYSCLK0_SRC_MASK_B,
	PW_REG_SYSCLK1_SRC_MASK_B,

	/* SPM_AP_STANDBY_CON */
	PW_REG_WFI_OP,
	PW_REG_WFI_TYPE,
	PW_REG_MP0_CPUTOP_IDLE_MASK,
	PW_REG_MP1_CPUTOP_IDLE_MASK,
	PW_REG_MCUSYS_IDLE_MASK,
	PW_REG_MD_APSRC_1_SEL,
	PW_REG_MD_APSRC_0_SEL,
	PW_REG_CONN_APSRC_SEL,

	/* SPM_SRC6_MASK */
	PW_REG_DPMAIF_SRCCLKENA_MASK_B,
	PW_REG_DPMAIF_INFRA_REQ_MASK_B,
	PW_REG_DPMAIF_APSRC_REQ_MASK_B,
	PW_REG_DPMAIF_VRF18_REQ_MASK_B,
	PW_REG_DPMAIF_DDR_EN_MASK_B,

	/* SPM_SRC_REQ */
	PW_REG_SPM_APSRC_REQ,
	PW_REG_SPM_F26M_REQ,
	PW_REG_SPM_INFRA_REQ,
	PW_REG_SPM_VRF18_REQ,
	PW_REG_SPM_DDR_EN_REQ,
	PW_REG_SPM_DVFS_REQ,
	PW_REG_SPM_SW_MAILBOX_REQ,
	PW_REG_SPM_SSPM_MAILBOX_REQ,
	PW_REG_SPM_ADSP_MAILBOX_REQ,
	PW_REG_SPM_SCP_MAILBOX_REQ,

	/* SPM_SRC_MASK */
	PW_REG_MD_SRCCLKENA_0_MASK_B,
	PW_REG_MD_SRCCLKENA2INFRA_REQ_0_MASK_B,
	PW_REG_MD_APSRC2INFRA_REQ_0_MASK_B,
	PW_REG_MD_APSRC_REQ_0_MASK_B,
	PW_REG_MD_VRF18_REQ_0_MASK_B,
	PW_REG_MD_DDR_EN_0_MASK_B,
	PW_REG_MD_SRCCLKENA_1_MASK_B,
	PW_REG_MD_SRCCLKENA2INFRA_REQ_1_MASK_B,
	PW_REG_MD_APSRC2INFRA_REQ_1_MASK_B,
	PW_REG_MD_APSRC_REQ_1_MASK_B,
	PW_REG_MD_VRF18_REQ_1_MASK_B,
	PW_REG_MD_DDR_EN_1_MASK_B,
	PW_REG_CONN_SRCCLKENA_MASK_B,
	PW_REG_CONN_SRCCLKENB_MASK_B,
	PW_REG_CONN_INFRA_REQ_MASK_B,
	PW_REG_CONN_APSRC_REQ_MASK_B,
	PW_REG_CONN_VRF18_REQ_MASK_B,
	PW_REG_CONN_DDR_EN_MASK_B,
	PW_REG_CONN_VFE28_MASK_B,
	PW_REG_SRCCLKENI0_SRCCLKENA_MASK_B,
	PW_REG_SRCCLKENI0_INFRA_REQ_MASK_B,
	PW_REG_SRCCLKENI1_SRCCLKENA_MASK_B,
	PW_REG_SRCCLKENI1_INFRA_REQ_MASK_B,
	PW_REG_SRCCLKENI2_SRCCLKENA_MASK_B,
	PW_REG_SRCCLKENI2_INFRA_REQ_MASK_B,
	PW_REG_INFRASYS_APSRC_REQ_MASK_B,
	PW_REG_INFRASYS_DDR_EN_MASK_B,
	PW_REG_MD32_SRCCLKENA_MASK_B,
	PW_REG_MD32_INFRA_REQ_MASK_B,
	PW_REG_MD32_APSRC_REQ_MASK_B,
	PW_REG_MD32_VRF18_REQ_MASK_B,
	PW_REG_MD32_DDR_EN_MASK_B,

	/* SPM_SRC2_MASK */
	PW_REG_SCP_SRCCLKENA_MASK_B,
	PW_REG_SCP_INFRA_REQ_MASK_B,
	PW_REG_SCP_APSRC_REQ_MASK_B,
	PW_REG_SCP_VRF18_REQ_MASK_B,
	PW_REG_SCP_DDR_EN_MASK_B,
	PW_REG_AUDIO_DSP_SRCCLKENA_MASK_B,
	PW_REG_AUDIO_DSP_INFRA_REQ_MASK_B,
	PW_REG_AUDIO_DSP_APSRC_REQ_MASK_B,
	PW_REG_AUDIO_DSP_VRF18_REQ_MASK_B,
	PW_REG_AUDIO_DSP_DDR_EN_MASK_B,
	PW_REG_UFS_SRCCLKENA_MASK_B,
	PW_REG_UFS_INFRA_REQ_MASK_B,
	PW_REG_UFS_APSRC_REQ_MASK_B,
	PW_REG_UFS_VRF18_REQ_MASK_B,
	PW_REG_UFS_DDR_EN_MASK_B,
	PW_REG_DISP0_APSRC_REQ_MASK_B,
	PW_REG_DISP0_DDR_EN_MASK_B,
	PW_REG_DISP1_APSRC_REQ_MASK_B,
	PW_REG_DISP1_DDR_EN_MASK_B,
	PW_REG_GCE_INFRA_REQ_MASK_B,
	PW_REG_GCE_APSRC_REQ_MASK_B,
	PW_REG_GCE_VRF18_REQ_MASK_B,
	PW_REG_GCE_DDR_EN_MASK_B,
	PW_REG_APU_SRCCLKENA_MASK_B,
	PW_REG_APU_INFRA_REQ_MASK_B,
	PW_REG_APU_APSRC_REQ_MASK_B,
	PW_REG_APU_VRF18_REQ_MASK_B,
	PW_REG_APU_DDR_EN_MASK_B,
	PW_REG_CG_CHECK_SRCCLKENA_MASK_B,
	PW_REG_CG_CHECK_APSRC_REQ_MASK_B,
	PW_REG_CG_CHECK_VRF18_REQ_MASK_B,
	PW_REG_CG_CHECK_DDR_EN_MASK_B,

	/* SPM_SRC3_MASK */
	PW_REG_DVFSRC_EVENT_TRIGGER_MASK_B,
	PW_REG_SW2SPM_INT0_MASK_B,
	PW_REG_SW2SPM_INT1_MASK_B,
	PW_REG_SW2SPM_INT2_MASK_B,
	PW_REG_SW2SPM_INT3_MASK_B,
	PW_REG_SC_ADSP2SPM_WAKEUP_MASK_B,
	PW_REG_SC_SSPM2SPM_WAKEUP_MASK_B,
	PW_REG_SC_SCP2SPM_WAKEUP_MASK_B,
	PW_REG_CSYSPWRREQ_MASK,
	PW_REG_SPM_SRCCLKENA_RESERVED_MASK_B,
	PW_REG_SPM_INFRA_REQ_RESERVED_MASK_B,
	PW_REG_SPM_APSRC_REQ_RESERVED_MASK_B,
	PW_REG_SPM_VRF18_REQ_RESERVED_MASK_B,
	PW_REG_SPM_DDR_EN_RESERVED_MASK_B,
	PW_REG_MCUPM_SRCCLKENA_MASK_B,
	PW_REG_MCUPM_INFRA_REQ_MASK_B,
	PW_REG_MCUPM_APSRC_REQ_MASK_B,
	PW_REG_MCUPM_VRF18_REQ_MASK_B,
	PW_REG_MCUPM_DDR_EN_MASK_B,
	PW_REG_MSDC0_SRCCLKENA_MASK_B,
	PW_REG_MSDC0_INFRA_REQ_MASK_B,
	PW_REG_MSDC0_APSRC_REQ_MASK_B,
	PW_REG_MSDC0_VRF18_REQ_MASK_B,
	PW_REG_MSDC0_DDR_EN_MASK_B,
	PW_REG_MSDC1_SRCCLKENA_MASK_B,
	PW_REG_MSDC1_INFRA_REQ_MASK_B,
	PW_REG_MSDC1_APSRC_REQ_MASK_B,
	PW_REG_MSDC1_VRF18_REQ_MASK_B,
	PW_REG_MSDC1_DDR_EN_MASK_B,

	/* SPM_SRC4_MASK */
	PW_CCIF_EVENT_MASK_B,
	PW_REG_BAK_PSRI_SRCCLKENA_MASK_B,
	PW_REG_BAK_PSRI_INFRA_REQ_MASK_B,
	PW_REG_BAK_PSRI_APSRC_REQ_MASK_B,
	PW_REG_BAK_PSRI_VRF18_REQ_MASK_B,
	PW_REG_BAK_PSRI_DDR_EN_MASK_B,
	PW_REG_DRAMC0_MD32_INFRA_REQ_MASK_B,
	PW_REG_DRAMC0_MD32_VRF18_REQ_MASK_B,
	PW_REG_DRAMC1_MD32_INFRA_REQ_MASK_B,
	PW_REG_DRAMC1_MD32_VRF18_REQ_MASK_B,
	PW_REG_CONN_SRCCLKENB2PWRAP_MASK_B,
	PW_REG_DRAMC0_MD32_WAKEUP_MASK,
	PW_REG_DRAMC1_MD32_WAKEUP_MASK,

	/* SPM_SRC5_MASK */
	PW_REG_MCUSYS_MERGE_APSRC_REQ_MASK_B,
	PW_REG_MCUSYS_MERGE_DDR_EN_MASK_B,
	PW_REG_MSDC2_SRCCLKENA_MASK_B,
	PW_REG_MSDC2_INFRA_REQ_MASK_B,
	PW_REG_MSDC2_APSRC_REQ_MASK_B,
	PW_REG_MSDC2_VRF18_REQ_MASK_B,
	PW_REG_MSDC2_DDR_EN_MASK_B,
	PW_REG_PCIE_SRCCLKENA_MASK_B,
	PW_REG_PCIE_INFRA_REQ_MASK_B,
	PW_REG_PCIE_APSRC_REQ_MASK_B,
	PW_REG_PCIE_VRF18_REQ_MASK_B,
	PW_REG_PCIE_DDR_EN_MASK_B,

	/* SPM_WAKEUP_EVENT_MASK */
	PW_REG_WAKEUP_EVENT_MASK,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	PW_REG_EXT_WAKEUP_EVENT_MASK,

	PW_MAX_COUNT,
};

#define SPM_INTERNAL_STATUS_HW_S1	(1U << 0)
#define SPM_ACK_CHK_3_SEL_HW_S1		0x00350098
#define SPM_ACK_CHK_3_HW_S1_CNT		1
#define SPM_ACK_CHK_3_CON_HW_MODE_TRIG	0x800
#define SPM_ACK_CHK_3_CON_EN		0x110
#define SPM_ACK_CHK_3_CON_CLR_ALL	0x2
#define SPM_ACK_CHK_3_CON_RESULT	0x8000

struct wake_status_trace_comm {
	uint32_t debug_flag;	/* PCM_WDT_LATCH_SPARE_0 */
	uint32_t debug_flag1;	/* PCM_WDT_LATCH_SPARE_1 */
	uint32_t timer_out;	/* SPM_SW_RSV_6*/
	uint32_t b_sw_flag0;	/* SPM_SW_RSV_7 */
	uint32_t b_sw_flag1;	/* SPM_SW_RSV_7 */
	uint32_t r12;		/* SPM_SW_RSV_0 */
	uint32_t r13;		/* PCM_REG13_DATA */
	uint32_t req_sta0;	/* SRC_REQ_STA_0 */
	uint32_t req_sta1;	/* SRC_REQ_STA_1 */
	uint32_t req_sta2;	/* SRC_REQ_STA_2 */
	uint32_t req_sta3;	/* SRC_REQ_STA_3 */
	uint32_t req_sta4;	/* SRC_REQ_STA_4 */
};

struct wake_status_trace {
	struct wake_status_trace_comm comm;
};

struct wake_status {
	struct wake_status_trace tr;
	uint32_t r12;			/* SPM_BK_WAKE_EVENT */
	uint32_t r12_ext;		/* SPM_WAKEUP_EXT_STA */
	uint32_t raw_sta;		/* SPM_WAKEUP_STA */
	uint32_t raw_ext_sta;		/* SPM_WAKEUP_EXT_STA */
	uint32_t md32pcm_wakeup_sta;	/* MD32CPM_WAKEUP_STA */
	uint32_t md32pcm_event_sta;	/* MD32PCM_EVENT_STA */
	uint32_t wake_misc;		/* SPM_BK_WAKE_MISC */
	uint32_t timer_out;		/* SPM_BK_PCM_TIMER */
	uint32_t r13;			/* PCM_REG13_DATA */
	uint32_t idle_sta;		/* SUBSYS_IDLE_STA */
	uint32_t req_sta0;		/* SRC_REQ_STA_0 */
	uint32_t req_sta1;		/* SRC_REQ_STA_1 */
	uint32_t req_sta2;		/* SRC_REQ_STA_2 */
	uint32_t req_sta3;		/* SRC_REQ_STA_3 */
	uint32_t req_sta4;		/* SRC_REQ_STA_4 */
	uint32_t cg_check_sta;		/* SPM_CG_CHECK_STA */
	uint32_t debug_flag;		/* PCM_WDT_LATCH_SPARE_0 */
	uint32_t debug_flag1;		/* PCM_WDT_LATCH_SPARE_1 */
	uint32_t b_sw_flag0;		/* SPM_SW_RSV_7 */
	uint32_t b_sw_flag1;		/* SPM_SW_RSV_8 */
	uint32_t isr;			/* SPM_IRQ_STA */
	uint32_t sw_flag0;		/* SPM_SW_FLAG_0 */
	uint32_t sw_flag1;		/* SPM_SW_FLAG_1 */
	uint32_t clk_settle;		/* SPM_CLK_SETTLE */
	uint32_t src_req;		/* SPM_SRC_REQ */
	uint32_t log_index;
	uint32_t abort;
	uint32_t rt_req_sta0;		/* SPM_SW_RSV_2 */
	uint32_t rt_req_sta1;		/* SPM_SW_RSV_3 */
	uint32_t rt_req_sta2;		/* SPM_SW_RSV_4 */
	uint32_t rt_req_sta3;		/* SPM_SW_RSV_5 */
	uint32_t rt_req_sta4;		/* SPM_SW_RSV_6 */
	uint32_t mcupm_req_sta;
};

struct spm_lp_scen {
	struct pcm_desc *pcmdesc;
	struct pwr_ctrl *pwrctrl;
};

extern struct spm_lp_scen __spm_vcorefs;
extern void __spm_set_cpu_status(unsigned int cpu);
extern void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc);
extern void __spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc);
extern void __spm_init_pcm_register(void);
extern void __spm_src_req_update(const struct pwr_ctrl *pwrctrl,
				 unsigned int resource_usage);
extern void __spm_set_power_control(const struct pwr_ctrl *pwrctrl);
extern void __spm_disable_pcm_timer(void);
extern void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);
extern void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl);
extern void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl);
extern void __spm_send_cpu_wakeup_event(void);
extern void __spm_get_wakeup_status(struct wake_status *wakesta,
				    unsigned int ext_status);
extern void __spm_clean_after_wakeup(void);
extern wake_reason_t
__spm_output_wake_reason(int state_id, const struct wake_status *wakesta);
extern void
__spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl,
				    const struct pwr_ctrl *src_pwr_ctrl);
extern void __spm_set_pcm_wdt(int en);
extern uint32_t _spm_get_wake_period(int pwake_time, wake_reason_t last_wr);
extern void __spm_set_fw_resume_option(struct pwr_ctrl *pwrctrl);
extern void __spm_ext_int_wakeup_req_clr(void);
extern void __spm_xo_soc_bblpm(int en);

static inline void set_pwrctrl_pcm_flags(struct pwr_ctrl *pwrctrl,
					 uint32_t flags)
{
	if (pwrctrl->pcm_flags_cust == 0U) {
		pwrctrl->pcm_flags = flags;
	} else {
		pwrctrl->pcm_flags = pwrctrl->pcm_flags_cust;
	}
}

static inline void set_pwrctrl_pcm_flags1(struct pwr_ctrl *pwrctrl,
					  uint32_t flags)
{
	if (pwrctrl->pcm_flags1_cust == 0U) {
		pwrctrl->pcm_flags1 = flags;
	} else {
		pwrctrl->pcm_flags1 = pwrctrl->pcm_flags1_cust;
	}
}

extern void __spm_hw_s1_state_monitor(int en, unsigned int *status);

static inline void spm_hw_s1_state_monitor_resume(void)
{
	__spm_hw_s1_state_monitor(1, NULL);
}

static inline void spm_hw_s1_state_monitor_pause(unsigned int *status)
{
	__spm_hw_s1_state_monitor(0, status);
}
#endif /* MT_SPM_INTERNAL_H */
