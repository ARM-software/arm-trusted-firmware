/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_INTERNAL_H
#define MT_SPM_INTERNAL_H

#include <mt_spm.h>

/* PCM_WDT_VAL */
#define PCM_WDT_TIMEOUT		(30 * 32768)	/* 30s */
/* PCM_TIMER_VAL */
#define PCM_TIMER_MAX		(0xffffffff - PCM_WDT_TIMEOUT)

/* PCM_PWR_IO_EN */
#define PCM_PWRIO_EN_R0		BIT(0)
#define PCM_PWRIO_EN_R7		BIT(7)
#define PCM_RF_SYNC_R0		BIT(16)
#define PCM_RF_SYNC_R6		BIT(22)
#define PCM_RF_SYNC_R7		BIT(23)

/* SPM_SWINT */
#define PCM_SW_INT0		BIT(0)
#define PCM_SW_INT1		BIT(1)
#define PCM_SW_INT2		BIT(2)
#define PCM_SW_INT3		BIT(3)
#define PCM_SW_INT4		BIT(4)
#define PCM_SW_INT5		BIT(5)
#define PCM_SW_INT6		BIT(6)
#define PCM_SW_INT7		BIT(7)
#define PCM_SW_INT8		BIT(8)
#define PCM_SW_INT9		BIT(9)
#define PCM_SW_INT_ALL		(PCM_SW_INT9 | PCM_SW_INT8 | PCM_SW_INT7 | \
				 PCM_SW_INT6 | PCM_SW_INT5 | PCM_SW_INT4 | \
				 PCM_SW_INT3 | PCM_SW_INT2 | PCM_SW_INT1 | \
				 PCM_SW_INT0)

/* SPM_AP_STANDBY_CON */
#define WFI_OP_AND		(1U)
#define WFI_OP_OR		(0U)

/* SPM_IRQ_MASK */
#define ISRM_TWAM		BIT(2)
#define ISRM_PCM_RETURN		BIT(3)
#define ISRM_RET_IRQ0		BIT(8)
#define ISRM_RET_IRQ1		BIT(9)
#define ISRM_RET_IRQ2		BIT(10)
#define ISRM_RET_IRQ3		BIT(11)
#define ISRM_RET_IRQ4		BIT(12)
#define ISRM_RET_IRQ5		BIT(13)
#define ISRM_RET_IRQ6		BIT(14)
#define ISRM_RET_IRQ7		BIT(15)
#define ISRM_RET_IRQ8		BIT(16)
#define ISRM_RET_IRQ9		BIT(17)
#define ISRM_RET_IRQ_AUX	((ISRM_RET_IRQ9) | (ISRM_RET_IRQ8) | \
				 (ISRM_RET_IRQ7) | (ISRM_RET_IRQ6) | \
				 (ISRM_RET_IRQ5) | (ISRM_RET_IRQ4) | \
				 (ISRM_RET_IRQ3) | (ISRM_RET_IRQ2) | \
				 (ISRM_RET_IRQ1))
#define ISRM_ALL_EXC_TWAM	ISRM_RET_IRQ_AUX
#define ISRM_ALL		(ISRM_ALL_EXC_TWAM | ISRM_TWAM)

/* SPM_IRQ_STA */
#define ISRS_TWAM		BIT(2)
#define ISRS_PCM_RETURN		BIT(3)
#define ISRC_TWAM		ISRS_TWAM
#define ISRC_ALL_EXC_TWAM	ISRS_PCM_RETURN
#define ISRC_ALL		(ISRC_ALL_EXC_TWAM | ISRC_TWAM)

/* SPM_WAKEUP_MISC */
#define WAKE_MISC_GIC_WAKEUP			(0x3FF)
#define WAKE_MISC_DVFSRC_IRQ			DVFSRC_IRQ_LSB
#define WAKE_MISC_REG_CPU_WAKEUP		SPM_WAKEUP_MISC_REG_CPU_WAKEUP_LSB
#define WAKE_MISC_PCM_TIMER_EVENT		PCM_TIMER_EVENT_LSB
#define WAKE_MISC_TWAM_IRQ_B			TWAM_IRQ_B_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET0		PMSR_IRQ_B_SET0_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET1		PMSR_IRQ_B_SET1_LSB
#define WAKE_MISC_PMSR_IRQ_B_SET2		PMSR_IRQ_B_SET2_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_0		SPM_ACK_CHK_WAKEUP_0_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_1		SPM_ACK_CHK_WAKEUP_1_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_2		SPM_ACK_CHK_WAKEUP_2_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_3		SPM_ACK_CHK_WAKEUP_3_LSB
#define WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL	SPM_ACK_CHK_WAKEUP_ALL_LSB
#define WAKE_MISC_PMIC_IRQ_ACK			PMIC_IRQ_ACK_LSB
#define WAKE_MISC_PMIC_SCP_IRQ			PMIC_SCP_IRQ_LSB

/* MD32PCM ADDR for SPM code fetch */
#define MD32PCM_BASE			(SPM_BASE + 0x0A00)
#define MD32PCM_CFGREG_SW_RSTN		(MD32PCM_BASE + 0x0000)
#define MD32PCM_DMA0_SRC		(MD32PCM_BASE + 0x0200)
#define MD32PCM_DMA0_DST		(MD32PCM_BASE + 0x0204)
#define MD32PCM_DMA0_WPPT		(MD32PCM_BASE + 0x0208)
#define MD32PCM_DMA0_WPTO		(MD32PCM_BASE + 0x020C)
#define MD32PCM_DMA0_COUNT		(MD32PCM_BASE + 0x0210)
#define MD32PCM_DMA0_CON		(MD32PCM_BASE + 0x0214)
#define MD32PCM_DMA0_START		(MD32PCM_BASE + 0x0218)
#define MD32PCM_DMA0_RLCT		(MD32PCM_BASE + 0x0224)
#define MD32PCM_INTC_IRQ_RAW_STA	(MD32PCM_BASE + 0x033C)

/* ABORT MASK for DEBUG FOORTPRINT */
#define DEBUG_ABORT_MASK (SPM_DBG_DEBUG_IDX_DRAM_SREF_ABORT_IN_APSRC | \
			  SPM_DBG_DEBUG_IDX_DRAM_SREF_ABORT_IN_DDREN)

#define DEBUG_ABORT_MASK_1 (SPM_DBG1_DEBUG_IDX_VRCXO_SLEEP_ABORT | \
			    SPM_DBG1_DEBUG_IDX_PWRAP_SLEEP_ACK_LOW_ABORT | \
			    SPM_DBG1_DEBUG_IDX_PWRAP_SLEEP_ACK_HIGH_ABORT | \
			    SPM_DBG1_DEBUG_IDX_EMI_SLP_IDLE_ABORT | \
			    SPM_DBG1_DEBUG_IDX_SCP_SLP_ACK_LOW_ABORT | \
			    SPM_DBG1_DEBUG_IDX_SCP_SLP_ACK_HIGH_ABORT | \
			    SPM_DBG1_DEBUG_IDX_SPM_DVFS_CMD_RDY_ABORT)

#define MCUPM_MBOX_WAKEUP_CPU (0x0C55FD10)

struct pcm_desc {
	const char *version;	/* PCM code version */
	uint32_t *base;		/* binary array base */
	uintptr_t base_dma;	/* dma addr of base */
	uint32_t pmem_words;
	uint32_t total_words;
	uint32_t pmem_start;
	uint32_t dmem_start;
};

struct pwr_ctrl {
	/* for SPM */
	uint32_t pcm_flags;
	/* can override pcm_flags */
	uint32_t pcm_flags_cust;
	/* set bit of pcm_flags, after pcm_flags_cust */
	uint32_t pcm_flags_cust_set;
	/* clr bit of pcm_flags, after pcm_flags_cust */
	uint32_t pcm_flags_cust_clr;
	uint32_t pcm_flags1;
	/* can override pcm_flags1 */
	uint32_t pcm_flags1_cust;
	/* set bit of pcm_flags1, after pcm_flags1_cust */
	uint32_t pcm_flags1_cust_set;
	/* clr bit of pcm_flags1, after pcm_flags1_cust */
	uint32_t pcm_flags1_cust_clr;
	/* @ 1T 32K */
	uint32_t timer_val;
	/* @ 1T 32K, can override timer_val */
	uint32_t timer_val_cust;
	/* stress for dpidle */
	uint32_t timer_val_ramp_en;
	/* stress for suspend */
	uint32_t timer_val_ramp_en_sec;
	uint32_t wake_src;
	/* can override wake_src */
	uint32_t wake_src_cust;
	/* disable wdt in suspend */
	uint8_t wdt_disable;

	/* SPM_AP_STANDBY_CON */
	/* [0] */
	uint8_t reg_wfi_op;
	/* [1] */
	uint8_t reg_wfi_type;
	/* [2] */
	uint8_t reg_mp0_cputop_idle_mask;
	/* [3] */
	uint8_t reg_mp1_cputop_idle_mask;
	/* [4] */
	uint8_t reg_mcusys_idle_mask;
	/* [25] */
	uint8_t reg_md_apsrc_1_sel;
	/* [26] */
	uint8_t reg_md_apsrc_0_sel;
	/* [29] */
	uint8_t reg_conn_apsrc_sel;

	/* SPM_SRC_REQ */
	/* [0] */
	uint8_t reg_spm_apsrc_req;
	/* [1] */
	uint8_t reg_spm_f26m_req;
	/* [3] */
	uint8_t reg_spm_infra_req;
	/* [4] */
	uint8_t reg_spm_vrf18_req;
	/* [7] */
	uint8_t reg_spm_ddr_en_req;
	/* [8] */
	uint8_t reg_spm_dvfs_req;
	/* [9] */
	uint8_t reg_spm_sw_mailbox_req;
	/* [10] */
	uint8_t reg_spm_sspm_mailbox_req;
	/* [11] */
	uint8_t reg_spm_adsp_mailbox_req;
	/* [12] */
	uint8_t reg_spm_scp_mailbox_req;

	/* SPM_SRC_MASK */
	/* [0] */
	uint8_t reg_sspm_srcclkena_0_mask_b;
	/* [1] */
	uint8_t reg_sspm_infra_req_0_mask_b;
	/* [2] */
	uint8_t reg_sspm_apsrc_req_0_mask_b;
	/* [3] */
	uint8_t reg_sspm_vrf18_req_0_mask_b;
	/* [4] */
	uint8_t reg_sspm_ddr_en_0_mask_b;
	/* [5] */
	uint8_t reg_scp_srcclkena_mask_b;
	/* [6] */
	uint8_t reg_scp_infra_req_mask_b;
	/* [7] */
	uint8_t reg_scp_apsrc_req_mask_b;
	/* [8] */
	uint8_t reg_scp_vrf18_req_mask_b;
	/* [9] */
	uint8_t reg_scp_ddr_en_mask_b;
	/* [10] */
	uint8_t reg_audio_dsp_srcclkena_mask_b;
	/* [11] */
	uint8_t reg_audio_dsp_infra_req_mask_b;
	/* [12] */
	uint8_t reg_audio_dsp_apsrc_req_mask_b;
	/* [13] */
	uint8_t reg_audio_dsp_vrf18_req_mask_b;
	/* [14] */
	uint8_t reg_audio_dsp_ddr_en_mask_b;
	/* [15] */
	uint8_t reg_apu_srcclkena_mask_b;
	/* [16] */
	uint8_t reg_apu_infra_req_mask_b;
	/* [17] */
	uint8_t reg_apu_apsrc_req_mask_b;
	/* [18] */
	uint8_t reg_apu_vrf18_req_mask_b;
	/* [19] */
	uint8_t reg_apu_ddr_en_mask_b;
	/* [20] */
	uint8_t reg_cpueb_srcclkena_mask_b;
	/* [21] */
	uint8_t reg_cpueb_infra_req_mask_b;
	/* [22] */
	uint8_t reg_cpueb_apsrc_req_mask_b;
	/* [23] */
	uint8_t reg_cpueb_vrf18_req_mask_b;
	/* [24] */
	uint8_t reg_cpueb_ddr_en_mask_b;
	/* [25] */
	uint8_t reg_bak_psri_srcclkena_mask_b;
	/* [26] */
	uint8_t reg_bak_psri_infra_req_mask_b;
	/* [27] */
	uint8_t reg_bak_psri_apsrc_req_mask_b;
	/* [28] */
	uint8_t reg_bak_psri_vrf18_req_mask_b;
	/* [29] */
	uint8_t reg_bak_psri_ddr_en_mask_b;
	/* [30] */
	uint8_t reg_cam_ddren_req_mask_b;
	/* [31] */
	uint8_t reg_img_ddren_req_mask_b;

	/* SPM_SRC2_MASK */
	/* [0] */
	uint8_t reg_msdc0_srcclkena_mask_b;
	/* [1] */
	uint8_t reg_msdc0_infra_req_mask_b;
	/* [2] */
	uint8_t reg_msdc0_apsrc_req_mask_b;
	/* [3] */
	uint8_t reg_msdc0_vrf18_req_mask_b;
	/* [4] */
	uint8_t reg_msdc0_ddr_en_mask_b;
	/* [5] */
	uint8_t reg_msdc1_srcclkena_mask_b;
	/* [6] */
	uint8_t reg_msdc1_infra_req_mask_b;
	/* [7] */
	uint8_t reg_msdc1_apsrc_req_mask_b;
	/* [8] */
	uint8_t reg_msdc1_vrf18_req_mask_b;
	/* [9] */
	uint8_t reg_msdc1_ddr_en_mask_b;
	/* [10] */
	uint8_t reg_msdc2_srcclkena_mask_b;
	/* [11] */
	uint8_t reg_msdc2_infra_req_mask_b;
	/* [12] */
	uint8_t reg_msdc2_apsrc_req_mask_b;
	/* [13] */
	uint8_t reg_msdc2_vrf18_req_mask_b;
	/* [14] */
	uint8_t reg_msdc2_ddr_en_mask_b;
	/* [15] */
	uint8_t reg_ufs_srcclkena_mask_b;
	/* [16] */
	uint8_t reg_ufs_infra_req_mask_b;
	/* [17] */
	uint8_t reg_ufs_apsrc_req_mask_b;
	/* [18] */
	uint8_t reg_ufs_vrf18_req_mask_b;
	/* [19] */
	uint8_t reg_ufs_ddr_en_mask_b;
	/* [20] */
	uint8_t reg_usb_srcclkena_mask_b;
	/* [21] */
	uint8_t reg_usb_infra_req_mask_b;
	/* [22] */
	uint8_t reg_usb_apsrc_req_mask_b;
	/* [23] */
	uint8_t reg_usb_vrf18_req_mask_b;
	/* [24] */
	uint8_t reg_usb_ddr_en_mask_b;
	/* [25] */
	uint8_t reg_pextp_p0_srcclkena_mask_b;
	/* [26] */
	uint8_t reg_pextp_p0_infra_req_mask_b;
	/* [27] */
	uint8_t reg_pextp_p0_apsrc_req_mask_b;
	/* [28] */
	uint8_t reg_pextp_p0_vrf18_req_mask_b;
	/* [29] */
	uint8_t reg_pextp_p0_ddr_en_mask_b;

	/* SPM_SRC3_MASK */
	/* [0] */
	uint8_t reg_pextp_p1_srcclkena_mask_b;
	/* [1] */
	uint8_t reg_pextp_p1_infra_req_mask_b;
	/* [2] */
	uint8_t reg_pextp_p1_apsrc_req_mask_b;
	/* [3] */
	uint8_t reg_pextp_p1_vrf18_req_mask_b;
	/* [4] */
	uint8_t reg_pextp_p1_ddr_en_mask_b;
	/* [5] */
	uint8_t reg_gce0_infra_req_mask_b;
	/* [6] */
	uint8_t reg_gce0_apsrc_req_mask_b;
	/* [7] */
	uint8_t reg_gce0_vrf18_req_mask_b;
	/* [8] */
	uint8_t reg_gce0_ddr_en_mask_b;
	/* [9] */
	uint8_t reg_gce1_infra_req_mask_b;
	/* [10] */
	uint8_t reg_gce1_apsrc_req_mask_b;
	/* [11] */
	uint8_t reg_gce1_vrf18_req_mask_b;
	/* [12] */
	uint8_t reg_gce1_ddr_en_mask_b;
	/* [13] */
	uint8_t reg_spm_srcclkena_reserved_mask_b;
	/* [14] */
	uint8_t reg_spm_infra_req_reserved_mask_b;
	/* [15] */
	uint8_t reg_spm_apsrc_req_reserved_mask_b;
	/* [16] */
	uint8_t reg_spm_vrf18_req_reserved_mask_b;
	/* [17] */
	uint8_t reg_spm_ddr_en_reserved_mask_b;
	/* [18] */
	uint8_t reg_disp0_apsrc_req_mask_b;
	/* [19] */
	uint8_t reg_disp0_ddr_en_mask_b;
	/* [20] */
	uint8_t reg_disp1_apsrc_req_mask_b;
	/* [21] */
	uint8_t reg_disp1_ddr_en_mask_b;
	/* [22] */
	uint8_t reg_disp2_apsrc_req_mask_b;
	/* [23] */
	uint8_t reg_disp2_ddr_en_mask_b;
	/* [24] */
	uint8_t reg_disp3_apsrc_req_mask_b;
	/* [25] */
	uint8_t reg_disp3_ddr_en_mask_b;
	/* [26] */
	uint8_t reg_infrasys_apsrc_req_mask_b;
	/* [27] */
	uint8_t reg_infrasys_ddr_en_mask_b;
	/* [28] */
	uint8_t reg_cg_check_srcclkena_mask_b;
	/* [29] */
	uint8_t reg_cg_check_apsrc_req_mask_b;
	/* [30] */
	uint8_t reg_cg_check_vrf18_req_mask_b;
	/* [31] */
	uint8_t reg_cg_check_ddr_en_mask_b;

	/* SPM_SRC4_MASK */
	/* [8:0] */
	uint32_t reg_mcusys_merge_apsrc_req_mask_b;
	/* [17:9] */
	uint32_t reg_mcusys_merge_ddr_en_mask_b;
	/* [19:18] */
	uint8_t reg_dramc_md32_infra_req_mask_b;
	/* [21:20] */
	uint8_t reg_dramc_md32_vrf18_req_mask_b;
	/* [23:22] */
	uint8_t reg_dramc_md32_ddr_en_mask_b;
	/* [24] */
	uint8_t reg_dvfsrc_event_trigger_mask_b;

	/* SPM_WAKEUP_EVENT_MASK2 */
	/* [3:0] */
	uint8_t reg_sc_sw2spm_wakeup_mask_b;
	/* [4] */
	uint8_t reg_sc_adsp2spm_wakeup_mask_b;
	/* [8:5] */
	uint8_t reg_sc_sspm2spm_wakeup_mask_b;
	/* [9] */
	uint8_t reg_sc_scp2spm_wakeup_mask_b;
	/* [10] */
	uint8_t reg_csyspwrup_ack_mask;
	/* [11] */
	uint8_t reg_csyspwrup_req_mask;

	/* SPM_WAKEUP_EVENT_MASK */
	/* [31:0] */
	uint32_t reg_wakeup_event_mask;

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	/* [31:0] */
	uint32_t reg_ext_wakeup_event_mask;
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
	PW_WDT_DISABLE,

	/* SPM_AP_STANDBY_CON */
	PW_REG_WFI_OP,
	PW_REG_WFI_TYPE,
	PW_REG_MP0_CPUTOP_IDLE_MASK,
	PW_REG_MP1_CPUTOP_IDLE_MASK,
	PW_REG_MCUSYS_IDLE_MASK,
	PW_REG_MD_APSRC_1_SEL,
	PW_REG_MD_APSRC_0_SEL,
	PW_REG_CONN_APSRC_SEL,

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
	PW_REG_SSPM_SRCCLKENA_0_MASK_B,
	PW_REG_SSPM_INFRA_REQ_0_MASK_B,
	PW_REG_SSPM_APSRC_REQ_0_MASK_B,
	PW_REG_SSPM_VRF18_REQ_0_MASK_B,
	PW_REG_SSPM_DDR_EN_0_MASK_B,
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
	PW_REG_APU_SRCCLKENA_MASK_B,
	PW_REG_APU_INFRA_REQ_MASK_B,
	PW_REG_APU_APSRC_REQ_MASK_B,
	PW_REG_APU_VRF18_REQ_MASK_B,
	PW_REG_APU_DDR_EN_MASK_B,
	PW_REG_CPUEB_SRCCLKENA_MASK_B,
	PW_REG_CPUEB_INFRA_REQ_MASK_B,
	PW_REG_CPUEB_APSRC_REQ_MASK_B,
	PW_REG_CPUEB_VRF18_REQ_MASK_B,
	PW_REG_CPUEB_DDR_EN_MASK_B,
	PW_REG_BAK_PSRI_SRCCLKENA_MASK_B,
	PW_REG_BAK_PSRI_INFRA_REQ_MASK_B,
	PW_REG_BAK_PSRI_APSRC_REQ_MASK_B,
	PW_REG_BAK_PSRI_VRF18_REQ_MASK_B,
	PW_REG_BAK_PSRI_DDR_EN_MASK_B,
	PW_REG_CAM_DDREN_REQ_MASK_B,
	PW_REG_IMG_DDREN_REQ_MASK_B,

	/* SPM_SRC2_MASK */
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
	PW_REG_MSDC2_SRCCLKENA_MASK_B,
	PW_REG_MSDC2_INFRA_REQ_MASK_B,
	PW_REG_MSDC2_APSRC_REQ_MASK_B,
	PW_REG_MSDC2_VRF18_REQ_MASK_B,
	PW_REG_MSDC2_DDR_EN_MASK_B,
	PW_REG_UFS_SRCCLKENA_MASK_B,
	PW_REG_UFS_INFRA_REQ_MASK_B,
	PW_REG_UFS_APSRC_REQ_MASK_B,
	PW_REG_UFS_VRF18_REQ_MASK_B,
	PW_REG_UFS_DDR_EN_MASK_B,
	PW_REG_USB_SRCCLKENA_MASK_B,
	PW_REG_USB_INFRA_REQ_MASK_B,
	PW_REG_USB_APSRC_REQ_MASK_B,
	PW_REG_USB_VRF18_REQ_MASK_B,
	PW_REG_USB_DDR_EN_MASK_B,
	PW_REG_PEXTP_P0_SRCCLKENA_MASK_B,
	PW_REG_PEXTP_P0_INFRA_REQ_MASK_B,
	PW_REG_PEXTP_P0_APSRC_REQ_MASK_B,
	PW_REG_PEXTP_P0_VRF18_REQ_MASK_B,
	PW_REG_PEXTP_P0_DDR_EN_MASK_B,

	/* SPM_SRC3_MASK */
	PW_REG_PEXTP_P1_SRCCLKENA_MASK_B,
	PW_REG_PEXTP_P1_INFRA_REQ_MASK_B,
	PW_REG_PEXTP_P1_APSRC_REQ_MASK_B,
	PW_REG_PEXTP_P1_VRF18_REQ_MASK_B,
	PW_REG_PEXTP_P1_DDR_EN_MASK_B,
	PW_REG_GCE0_INFRA_REQ_MASK_B,
	PW_REG_GCE0_APSRC_REQ_MASK_B,
	PW_REG_GCE0_VRF18_REQ_MASK_B,
	PW_REG_GCE0_DDR_EN_MASK_B,
	PW_REG_GCE1_INFRA_REQ_MASK_B,
	PW_REG_GCE1_APSRC_REQ_MASK_B,
	PW_REG_GCE1_VRF18_REQ_MASK_B,
	PW_REG_GCE1_DDR_EN_MASK_B,
	PW_REG_SPM_SRCCLKENA_RESERVED_MASK_B,
	PW_REG_SPM_INFRA_REQ_RESERVED_MASK_B,
	PW_REG_SPM_APSRC_REQ_RESERVED_MASK_B,
	PW_REG_SPM_VRF18_REQ_RESERVED_MASK_B,
	PW_REG_SPM_DDR_EN_RESERVED_MASK_B,
	PW_REG_DISP0_APSRC_REQ_MASK_B,
	PW_REG_DISP0_DDR_EN_MASK_B,
	PW_REG_DISP1_APSRC_REQ_MASK_B,
	PW_REG_DISP1_DDR_EN_MASK_B,
	PW_REG_DISP2_APSRC_REQ_MASK_B,
	PW_REG_DISP2_DDR_EN_MASK_B,
	PW_REG_DISP3_APSRC_REQ_MASK_B,
	PW_REG_DISP3_DDR_EN_MASK_B,
	PW_REG_INFRASYS_APSRC_REQ_MASK_B,
	PW_REG_INFRASYS_DDR_EN_MASK_B,
	PW_REG_CG_CHECK_SRCCLKENA_MASK_B,
	PW_REG_CG_CHECK_APSRC_REQ_MASK_B,
	PW_REG_CG_CHECK_VRF18_REQ_MASK_B,
	PW_REG_CG_CHECK_DDR_EN_MASK_B,

	/* SPM_SRC4_MASK */
	PW_REG_MCUSYS_MERGE_APSRC_REQ_MASK_B,
	PW_REG_MCUSYS_MERGE_DDR_EN_MASK_B,
	PW_REG_DRAMC_MD32_INFRA_REQ_MASK_B,
	PW_REG_DRAMC_MD32_VRF18_REQ_MASK_B,
	PW_REG_DRAMC_MD32_DDR_EN_MASK_B,
	PW_REG_DVFSRC_EVENT_TRIGGER_MASK_B,

	/* SPM_WAKEUP_EVENT_MASK2 */
	PW_REG_SC_SW2SPM_WAKEUP_MASK_B,
	PW_REG_SC_ADSP2SPM_WAKEUP_MASK_B,
	PW_REG_SC_SSPM2SPM_WAKEUP_MASK_B,
	PW_REG_SC_SCP2SPM_WAKEUP_MASK_B,
	PW_REG_CSYSPWRUP_ACK_MASK,
	PW_REG_CSYSPWRUP_REQ_MASK,

	/* SPM_WAKEUP_EVENT_MASK */
	PW_REG_WAKEUP_EVENT_MASK,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	PW_REG_EXT_WAKEUP_EVENT_MASK,
	PW_MAX_COUNT,
};

/* spm_internal.c internal status */
#define SPM_INTERNAL_STATUS_HW_S1	BIT(0)
#define SPM_ACK_CHK_3_CON_HW_MODE_TRIG	(0x800)
/* BIT[0]: SW_EN, BIT[4]: STA_EN, BIT[8]: HW_EN */
#define SPM_ACK_CHK_3_CON_EN		(0x110)
#define SPM_ACK_CHK_3_CON_CLR_ALL	(0x2)
/* BIT[15]: RESULT */
#define SPM_ACK_CHK_3_CON_RESULT	(0x8000)

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
	uint32_t raw_sta;	/* SPM_WAKEUP_STA */
	uint32_t times_h;	/* timestamp high bits */
	uint32_t times_l;	/* timestamp low bits */
	uint32_t resumetime;	/* timestamp low bits */
};

struct wake_status_trace {
	struct wake_status_trace_comm comm;
};

struct wake_status {
	struct wake_status_trace tr;
	uint32_t r12_ext;		/* SPM_WAKEUP_EXT_STA */
	uint32_t raw_ext_sta;		/* SPM_WAKEUP_EXT_STA */
	uint32_t md32pcm_wakeup_sta;	/* MD32PCM_WAKEUP_STA */
	uint32_t md32pcm_event_sta;	/* MD32PCM_EVENT_STA */
	uint32_t wake_misc;		/* SPM_SW_RSV_5 */
	uint32_t idle_sta;		/* SUBSYS_IDLE_STA */
	uint32_t cg_check_sta;		/* SPM_CG_CHECK_STA */
	uint32_t sw_flag0;		/* SPM_SW_FLAG_0 */
	uint32_t sw_flag1;		/* SPM_SW_FLAG_1 */
	uint32_t isr;			/* SPM_IRQ_STA */
	uint32_t clk_settle;		/* SPM_CLK_SETTLE */
	uint32_t src_req;		/* SPM_SRC_REQ */
	uint32_t log_index;
	uint32_t is_abort;
	uint32_t rt_req_sta0;		/* SPM_SW_RSV_2 */
	uint32_t rt_req_sta1;		/* SPM_SW_RSV_3 */
	uint32_t rt_req_sta2;		/* SPM_SW_RSV_4 */
	uint32_t rt_req_sta3;		/* SPM_SW_RSV_5 */
	uint32_t rt_req_sta4;		/* SPM_SW_RSV_6 */
};

struct spm_lp_scen {
	struct pcm_desc *pcmdesc;
	struct pwr_ctrl *pwrctrl;
};

void __spm_set_cpu_status(unsigned int cpu);
void __spm_src_req_update(const struct pwr_ctrl *pwrctrl, unsigned int resource_usage);
void __spm_set_power_control(const struct pwr_ctrl *pwrctrl);
void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);
void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl);
void __spm_send_cpu_wakeup_event(void);
void __spm_get_wakeup_status(struct wake_status *wakesta, unsigned int ext_status);
void __spm_clean_after_wakeup(void);
wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta);
void __spm_set_pcm_wdt(int en);
void __spm_ext_int_wakeup_req_clr(void);
void __spm_hw_s1_state_monitor(int en, unsigned int *status);

static inline void spm_hw_s1_state_monitor_resume(void)
{
	__spm_hw_s1_state_monitor(1, NULL);
}

static inline void spm_hw_s1_state_monitor_pause(unsigned int *status)
{
	__spm_hw_s1_state_monitor(0, status);
}

void __spm_clean_before_wfi(void);

#endif /* MT_SPM_INTERNAL */
