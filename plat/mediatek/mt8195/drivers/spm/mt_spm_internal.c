/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>

#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_pmic_wrap.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <platform_def.h>
#include <plat_pm.h>

/**************************************
 * Define and Declare
 **************************************/
#define ROOT_CORE_ADDR_OFFSET			0x20000000
#define SPM_WAKEUP_EVENT_MASK_CLEAN_MASK	0xefffffff
#define	SPM_INIT_DONE_US			20

static unsigned int mt_spm_bblpm_cnt;

const char *wakeup_src_str[32] = {
	[0] = "R12_PCM_TIMER",
	[1] = "R12_RESERVED_DEBUG_B",
	[2] = "R12_KP_IRQ_B",
	[3] = "R12_APWDT_EVENT_B",
	[4] = "R12_APXGPT1_EVENT_B",
	[5] = "R12_MSDC_WAKEUP_B",
	[6] = "R12_EINT_EVENT_B",
	[7] = "R12_IRRX_WAKEUP_B",
	[8] = "R12_SBD_INTR_WAKEUP_B",
	[9] = "R12_RESERVE0",
	[10] = "R12_SC_SSPM2SPM_WAKEUP_B",
	[11] = "R12_SC_SCP2SPM_WAKEUP_B",
	[12] = "R12_SC_ADSP2SPM_WAKEUP_B",
	[13] = "R12_WDT_WAKEUP_B",
	[14] = "R12_USB_U2_B",
	[15] = "R12_USB_TOP_B",
	[16] = "R12_SYS_TIMER_EVENT_B",
	[17] = "R12_EINT_EVENT_SECURE_B",
	[18] = "R12_ECE_INT_HDMI_B",
	[19] = "R12_RESERVE1",
	[20] = "R12_AFE_IRQ_MCU_B",
	[21] = "R12_THERM_CTRL_EVENT_B",
	[22] = "R12_SCP_CIRQ_IRQ_B",
	[23] = "R12_NNA2INFRA_WAKEUP_B",
	[24] = "R12_CSYSPWREQ_B",
	[25] = "R12_RESERVE2",
	[26] = "R12_PCIE_WAKEUPEVENT_B",
	[27] = "R12_SEJ_EVENT_B",
	[28] = "R12_SPM_CPU_WAKEUPEVENT_B",
	[29] = "R12_APUSYS",
	[30] = "R12_RESERVE3",
	[31] = "R12_RESERVE4",
};

/**************************************
 * Function and API
 **************************************/

wake_reason_t __spm_output_wake_reason(int state_id,
				       const struct wake_status *wakesta)
{
	uint32_t i, bk_vtcxo_dur, spm_26m_off_pct = 0U;
	wake_reason_t wr = WR_UNKNOWN;

	if (wakesta == NULL) {
		return WR_UNKNOWN;
	}

	if (wakesta->abort != 0U) {
		ERROR("spmfw flow is aborted: 0x%x, timer_out = %u\n",
		      wakesta->abort, wakesta->timer_out);
	} else {
		for (i = 0U; i < 32U; i++) {
			if ((wakesta->r12 & (1U << i)) != 0U) {
				INFO("wake up by %s, timer_out = %u\n",
				     wakeup_src_str[i], wakesta->timer_out);
				wr = WR_WAKE_SRC;
				break;
			}
		}
	}

	INFO("r12 = 0x%x, r12_ext = 0x%x, r13 = 0x%x, debug_flag = 0x%x 0x%x\n",
	     wakesta->r12, wakesta->r12_ext, wakesta->r13, wakesta->debug_flag,
	     wakesta->debug_flag1);
	INFO("raw_sta = 0x%x 0x%x 0x%x, idle_sta = 0x%x, cg_check_sta = 0x%x\n",
	     wakesta->raw_sta, wakesta->md32pcm_wakeup_sta,
	     wakesta->md32pcm_event_sta, wakesta->idle_sta,
	     wakesta->cg_check_sta);
	INFO("req_sta = 0x%x 0x%x 0x%x 0x%x 0x%x, isr = 0x%x\n",
	     wakesta->req_sta0, wakesta->req_sta1, wakesta->req_sta2,
	     wakesta->req_sta3, wakesta->req_sta4, wakesta->isr);
	INFO("rt_req_sta0 = 0x%x, rt_req_sta1 = 0x%x, rt_req_sta2 = 0x%x\n",
	     wakesta->rt_req_sta0, wakesta->rt_req_sta1, wakesta->rt_req_sta2);
	INFO("rt_req_sta3 = 0x%x, dram_sw_con_3 = 0x%x, raw_ext_sta = 0x%x\n",
	     wakesta->rt_req_sta3, wakesta->rt_req_sta4, wakesta->raw_ext_sta);
	INFO("wake_misc = 0x%x, pcm_flag = 0x%x 0x%x 0x%x 0x%x, req = 0x%x\n",
	     wakesta->wake_misc, wakesta->sw_flag0, wakesta->sw_flag1,
	     wakesta->b_sw_flag0, wakesta->b_sw_flag1, wakesta->src_req);
	INFO("clk_settle = 0x%x, wlk_cntcv_l = 0x%x, wlk_cntcv_h = 0x%x\n",
	     wakesta->clk_settle, mmio_read_32(SYS_TIMER_VALUE_L),
	     mmio_read_32(SYS_TIMER_VALUE_H));

	if (wakesta->timer_out != 0U) {
		bk_vtcxo_dur = mmio_read_32(SPM_BK_VTCXO_DUR);
		spm_26m_off_pct = (100 * bk_vtcxo_dur) / wakesta->timer_out;
		INFO("spm_26m_off_pct = %u\n", spm_26m_off_pct);
	}

	return wr;
}

void __spm_set_cpu_status(unsigned int cpu)
{
	uint32_t root_core_addr;

	if (cpu < 8U) {
		mmio_write_32(ROOT_CPUTOP_ADDR, (1U << cpu));
		root_core_addr = SPM_CPU0_PWR_CON + (cpu * 0x4);
		root_core_addr += ROOT_CORE_ADDR_OFFSET;
		mmio_write_32(ROOT_CORE_ADDR, root_core_addr);
		/* Notify MCUPM that preferred cpu wakeup */
		mmio_write_32(MCUPM_MBOX_WAKEUP_CPU, cpu);
	} else {
		ERROR("%s: error cpu number %d\n", __func__, cpu);
	}
}

void __spm_src_req_update(const struct pwr_ctrl *pwrctrl,
			  unsigned int resource_usage)
{
	uint8_t apsrc_req = ((resource_usage & MT_SPM_DRAM_S0) != 0U) ?
			    1 : pwrctrl->reg_spm_apsrc_req;
	uint8_t ddr_en_req = ((resource_usage & MT_SPM_DRAM_S1) != 0U) ?
			     1 : pwrctrl->reg_spm_ddr_en_req;
	uint8_t vrf18_req = ((resource_usage & MT_SPM_SYSPLL) != 0U) ?
			    1 : pwrctrl->reg_spm_vrf18_req;
	uint8_t infra_req = ((resource_usage & MT_SPM_INFRA) != 0U) ?
			    1 : pwrctrl->reg_spm_infra_req;
	uint8_t f26m_req  = ((resource_usage &
			      (MT_SPM_26M | MT_SPM_XO_FPM)) != 0U) ?
			    1 : pwrctrl->reg_spm_f26m_req;

	mmio_write_32(SPM_SRC_REQ,
		      ((apsrc_req & 0x1) << 0) |
		      ((f26m_req & 0x1) << 1) |
		      ((infra_req & 0x1) << 3) |
		      ((vrf18_req & 0x1) << 4) |
		      ((ddr_en_req & 0x1) << 7) |
		      ((pwrctrl->reg_spm_dvfs_req & 0x1) << 8) |
		      ((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 9) |
		      ((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 10) |
		      ((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 11) |
		      ((pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 12));
}

void __spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	/* Auto-gen Start */

	/* SPM_AP_STANDBY_CON */
	mmio_write_32(SPM_AP_STANDBY_CON,
		((pwrctrl->reg_wfi_op & 0x1) << 0) |
		((pwrctrl->reg_wfi_type & 0x1) << 1) |
		((pwrctrl->reg_mp0_cputop_idle_mask & 0x1) << 2) |
		((pwrctrl->reg_mp1_cputop_idle_mask & 0x1) << 3) |
		((pwrctrl->reg_mcusys_idle_mask & 0x1) << 4) |
		((pwrctrl->reg_md_apsrc_1_sel & 0x1) << 25) |
		((pwrctrl->reg_md_apsrc_0_sel & 0x1) << 26) |
		((pwrctrl->reg_conn_apsrc_sel & 0x1) << 29));

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((pwrctrl->reg_spm_apsrc_req & 0x1) << 0) |
		((pwrctrl->reg_spm_f26m_req & 0x1) << 1) |
		((pwrctrl->reg_spm_infra_req & 0x1) << 3) |
		((pwrctrl->reg_spm_vrf18_req & 0x1) << 4) |
		((pwrctrl->reg_spm_ddr_en_req & 0x1) << 7) |
		((pwrctrl->reg_spm_dvfs_req & 0x1) << 8) |
		((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 9) |
		((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 10) |
		((pwrctrl->reg_spm_adsp_mailbox_req & 0x1) << 11) |
		((pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 12));

	/* SPM_SRC_MASK */
	mmio_write_32(SPM_SRC_MASK,
		((pwrctrl->reg_sspm_srcclkena_0_mask_b & 0x1) << 0) |
		((pwrctrl->reg_sspm_infra_req_0_mask_b & 0x1) << 1) |
		((pwrctrl->reg_sspm_apsrc_req_0_mask_b & 0x1) << 2) |
		((pwrctrl->reg_sspm_vrf18_req_0_mask_b & 0x1) << 3) |
		((pwrctrl->reg_sspm_ddr_en_0_mask_b & 0x1) << 4) |
		((pwrctrl->reg_scp_srcclkena_mask_b & 0x1) << 5) |
		((pwrctrl->reg_scp_infra_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_scp_apsrc_req_mask_b & 0x1) << 7) |
		((pwrctrl->reg_scp_vrf18_req_mask_b & 0x1) << 8) |
		((pwrctrl->reg_scp_ddr_en_mask_b & 0x1) << 9) |
		((pwrctrl->reg_audio_dsp_srcclkena_mask_b & 0x1) << 10) |
		((pwrctrl->reg_audio_dsp_infra_req_mask_b & 0x1) << 11) |
		((pwrctrl->reg_audio_dsp_apsrc_req_mask_b & 0x1) << 12) |
		((pwrctrl->reg_audio_dsp_vrf18_req_mask_b & 0x1) << 13) |
		((pwrctrl->reg_audio_dsp_ddr_en_mask_b & 0x1) << 14) |
		((pwrctrl->reg_apu_srcclkena_mask_b & 0x1) << 15) |
		((pwrctrl->reg_apu_infra_req_mask_b & 0x1) << 16) |
		((pwrctrl->reg_apu_apsrc_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_apu_vrf18_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_apu_ddr_en_mask_b & 0x1) << 19) |
		((pwrctrl->reg_cpueb_srcclkena_mask_b & 0x1) << 20) |
		((pwrctrl->reg_cpueb_infra_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_cpueb_apsrc_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_cpueb_vrf18_req_mask_b & 0x1) << 23) |
		((pwrctrl->reg_cpueb_ddr_en_mask_b & 0x1) << 24) |
		((pwrctrl->reg_bak_psri_srcclkena_mask_b & 0x1) << 25) |
		((pwrctrl->reg_bak_psri_infra_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_bak_psri_apsrc_req_mask_b & 0x1) << 27) |
		((pwrctrl->reg_bak_psri_vrf18_req_mask_b & 0x1) << 28) |
		((pwrctrl->reg_bak_psri_ddr_en_mask_b & 0x1) << 29));

	/* SPM_SRC2_MASK */
	mmio_write_32(SPM_SRC2_MASK,
		((pwrctrl->reg_msdc0_srcclkena_mask_b & 0x1) << 0) |
		((pwrctrl->reg_msdc0_infra_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_msdc0_apsrc_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_msdc0_vrf18_req_mask_b & 0x1) << 3) |
		((pwrctrl->reg_msdc0_ddr_en_mask_b & 0x1) << 4) |
		((pwrctrl->reg_msdc1_srcclkena_mask_b & 0x1) << 5) |
		((pwrctrl->reg_msdc1_infra_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_msdc1_apsrc_req_mask_b & 0x1) << 7) |
		((pwrctrl->reg_msdc1_vrf18_req_mask_b & 0x1) << 8) |
		((pwrctrl->reg_msdc1_ddr_en_mask_b & 0x1) << 9) |
		((pwrctrl->reg_msdc2_srcclkena_mask_b & 0x1) << 10) |
		((pwrctrl->reg_msdc2_infra_req_mask_b & 0x1) << 11) |
		((pwrctrl->reg_msdc2_apsrc_req_mask_b & 0x1) << 12) |
		((pwrctrl->reg_msdc2_vrf18_req_mask_b & 0x1) << 13) |
		((pwrctrl->reg_msdc2_ddr_en_mask_b & 0x1) << 14) |
		((pwrctrl->reg_ufs_srcclkena_mask_b & 0x1) << 15) |
		((pwrctrl->reg_ufs_infra_req_mask_b & 0x1) << 16) |
		((pwrctrl->reg_ufs_apsrc_req_mask_b & 0x1) << 17) |
		((pwrctrl->reg_ufs_vrf18_req_mask_b & 0x1) << 18) |
		((pwrctrl->reg_ufs_ddr_en_mask_b & 0x1) << 19) |
		((pwrctrl->reg_usb_srcclkena_mask_b & 0x1) << 20) |
		((pwrctrl->reg_usb_infra_req_mask_b & 0x1) << 21) |
		((pwrctrl->reg_usb_apsrc_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_usb_vrf18_req_mask_b & 0x1) << 23) |
		((pwrctrl->reg_usb_ddr_en_mask_b & 0x1) << 24) |
		((pwrctrl->reg_pextp_p0_srcclkena_mask_b & 0x1) << 25) |
		((pwrctrl->reg_pextp_p0_infra_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_pextp_p0_apsrc_req_mask_b & 0x1) << 27) |
		((pwrctrl->reg_pextp_p0_vrf18_req_mask_b & 0x1) << 28) |
		((pwrctrl->reg_pextp_p0_ddr_en_mask_b & 0x1) << 29));

	/* SPM_SRC3_MASK */
	mmio_write_32(SPM_SRC3_MASK,
		((pwrctrl->reg_pextp_p1_srcclkena_mask_b & 0x1) << 0) |
		((pwrctrl->reg_pextp_p1_infra_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_pextp_p1_apsrc_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_pextp_p1_vrf18_req_mask_b & 0x1) << 3) |
		((pwrctrl->reg_pextp_p1_ddr_en_mask_b & 0x1) << 4) |
		((pwrctrl->reg_gce0_infra_req_mask_b & 0x1) << 5) |
		((pwrctrl->reg_gce0_apsrc_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_gce0_vrf18_req_mask_b & 0x1) << 7) |
		((pwrctrl->reg_gce0_ddr_en_mask_b & 0x1) << 8) |
		((pwrctrl->reg_gce1_infra_req_mask_b & 0x1) << 9) |
		((pwrctrl->reg_gce1_apsrc_req_mask_b & 0x1) << 10) |
		((pwrctrl->reg_gce1_vrf18_req_mask_b & 0x1) << 11) |
		((pwrctrl->reg_gce1_ddr_en_mask_b & 0x1) << 12) |
		((pwrctrl->reg_spm_srcclkena_reserved_mask_b & 0x1) << 13) |
		((pwrctrl->reg_spm_infra_req_reserved_mask_b & 0x1) << 14) |
		((pwrctrl->reg_spm_apsrc_req_reserved_mask_b & 0x1) << 15) |
		((pwrctrl->reg_spm_vrf18_req_reserved_mask_b & 0x1) << 16) |
		((pwrctrl->reg_spm_ddr_en_reserved_mask_b & 0x1) << 17) |
		((pwrctrl->reg_disp0_ddr_en_mask_b & 0x1) << 18) |
		((pwrctrl->reg_disp0_ddr_en_mask_b & 0x1) << 19) |
		((pwrctrl->reg_disp1_apsrc_req_mask_b & 0x1) << 20) |
		((pwrctrl->reg_disp1_ddr_en_mask_b & 0x1) << 21) |
		((pwrctrl->reg_disp2_apsrc_req_mask_b & 0x1) << 22) |
		((pwrctrl->reg_disp2_ddr_en_mask_b & 0x1) << 23) |
		((pwrctrl->reg_disp3_apsrc_req_mask_b & 0x1) << 24) |
		((pwrctrl->reg_disp3_ddr_en_mask_b & 0x1) << 25) |
		((pwrctrl->reg_infrasys_apsrc_req_mask_b & 0x1) << 26) |
		((pwrctrl->reg_infrasys_ddr_en_mask_b & 0x1) << 27));

	/* Mask MCUSYS request since SOC HW would check it */
	mmio_write_32(SPM_SRC4_MASK, 0x1fc0000);

	/* SPM_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		((pwrctrl->reg_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_EXT_MASK,
		((pwrctrl->reg_ext_wakeup_event_mask & 0xffffffff) << 0));

	/* Auto-gen End */
}

void __spm_disable_pcm_timer(void)
{
	mmio_clrsetbits_32(PCM_CON1, RG_PCM_TIMER_EN_LSB, SPM_REGWR_CFG_KEY);
}

void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	uint32_t val, mask;

	/* toggle event counter clear */
	mmio_setbits_32(PCM_CON1,
			SPM_REGWR_CFG_KEY | SPM_EVENT_COUNTER_CLR_LSB);

	/* toggle for reset SYS TIMER start point */
	mmio_setbits_32(SYS_TIMER_CON, SYS_TIMER_START_EN_LSB);

	if (pwrctrl->timer_val_cust == 0U) {
		val = pwrctrl->timer_val;
	} else {
		val = pwrctrl->timer_val_cust;
	}

	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_setbits_32(PCM_CON1, SPM_REGWR_CFG_KEY | RG_PCM_TIMER_EN_LSB);

	/* unmask AP wakeup source */
	if (pwrctrl->wake_src_cust == 0U) {
		mask = pwrctrl->wake_src;
	} else {
		mask = pwrctrl->wake_src_cust;
	}

	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	/* unmask SPM ISR (keep TWAM setting) */
	mmio_setbits_32(SPM_IRQ_MASK, ISRM_RET_IRQ_AUX);

	/* toggle event counter clear */
	mmio_clrsetbits_32(PCM_CON1, SPM_EVENT_COUNTER_CLR_LSB,
			   SPM_REGWR_CFG_KEY);
	/* toggle for reset SYS TIMER start point */
	mmio_clrbits_32(SYS_TIMER_CON, SYS_TIMER_START_EN_LSB);
}

void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl)
{
	/* set PCM flags and data */
	if (pwrctrl->pcm_flags_cust_clr != 0U) {
		pwrctrl->pcm_flags &= ~pwrctrl->pcm_flags_cust_clr;
	}

	if (pwrctrl->pcm_flags_cust_set != 0U) {
		pwrctrl->pcm_flags |= pwrctrl->pcm_flags_cust_set;
	}

	if (pwrctrl->pcm_flags1_cust_clr != 0U) {
		pwrctrl->pcm_flags1 &= ~pwrctrl->pcm_flags1_cust_clr;
	}

	if (pwrctrl->pcm_flags1_cust_set != 0U) {
		pwrctrl->pcm_flags1 |= pwrctrl->pcm_flags1_cust_set;
	}

	mmio_write_32(SPM_SW_FLAG_0, pwrctrl->pcm_flags);
	mmio_write_32(SPM_SW_FLAG_1, pwrctrl->pcm_flags1);
	mmio_write_32(SPM_SW_RSV_7, pwrctrl->pcm_flags);
	mmio_write_32(SPM_SW_RSV_8, pwrctrl->pcm_flags1);
}

void __spm_get_wakeup_status(struct wake_status *wakesta,
			     unsigned int ext_status)
{
	wakesta->tr.comm.r12 = mmio_read_32(SPM_BK_WAKE_EVENT);
	wakesta->tr.comm.timer_out = mmio_read_32(SPM_BK_PCM_TIMER);
	wakesta->tr.comm.r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->tr.comm.req_sta0 = mmio_read_32(SRC_REQ_STA_0);
	wakesta->tr.comm.req_sta1 = mmio_read_32(SRC_REQ_STA_1);
	wakesta->tr.comm.req_sta2 = mmio_read_32(SRC_REQ_STA_2);
	wakesta->tr.comm.req_sta3 = mmio_read_32(SRC_REQ_STA_3);
	wakesta->tr.comm.req_sta4 = mmio_read_32(SRC_REQ_STA_4);
	wakesta->tr.comm.debug_flag = mmio_read_32(PCM_WDT_LATCH_SPARE_0);
	wakesta->tr.comm.debug_flag1 = mmio_read_32(PCM_WDT_LATCH_SPARE_1);

	if ((ext_status & SPM_INTERNAL_STATUS_HW_S1) != 0U) {
		wakesta->tr.comm.debug_flag |= (SPM_DBG_DEBUG_IDX_DDREN_WAKE |
						SPM_DBG_DEBUG_IDX_DDREN_SLEEP);
		mmio_write_32(PCM_WDT_LATCH_SPARE_0,
			      wakesta->tr.comm.debug_flag);
	}

	wakesta->tr.comm.b_sw_flag0 = mmio_read_32(SPM_SW_RSV_7);
	wakesta->tr.comm.b_sw_flag1 = mmio_read_32(SPM_SW_RSV_8);

	/* record below spm info for debug */
	wakesta->r12 = mmio_read_32(SPM_BK_WAKE_EVENT);
	wakesta->r12_ext = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_sta = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->md32pcm_wakeup_sta = mmio_read_32(MD32PCM_WAKEUP_STA);
	wakesta->md32pcm_event_sta = mmio_read_32(MD32PCM_EVENT_STA);
	wakesta->src_req = mmio_read_32(SPM_SRC_REQ);

	/* backup of SPM_WAKEUP_MISC */
	wakesta->wake_misc = mmio_read_32(SPM_BK_WAKE_MISC);

	/* get sleep time, backup of PCM_TIMER_OUT */
	wakesta->timer_out = mmio_read_32(SPM_BK_PCM_TIMER);

	/* get other SYS and co-clock status */
	wakesta->r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SUBSYS_IDLE_STA);
	wakesta->req_sta0 = mmio_read_32(SRC_REQ_STA_0);
	wakesta->req_sta1 = mmio_read_32(SRC_REQ_STA_1);
	wakesta->req_sta2 = mmio_read_32(SRC_REQ_STA_2);
	wakesta->req_sta3 = mmio_read_32(SRC_REQ_STA_3);
	wakesta->req_sta4 = mmio_read_32(SRC_REQ_STA_4);

	/* get HW CG check status */
	wakesta->cg_check_sta = mmio_read_32(SPM_CG_CHECK_STA);

	/* get debug flag for PCM execution check */
	wakesta->debug_flag = mmio_read_32(PCM_WDT_LATCH_SPARE_0);
	wakesta->debug_flag1 = mmio_read_32(PCM_WDT_LATCH_SPARE_1);

	/* get backup SW flag status */
	wakesta->b_sw_flag0 = mmio_read_32(SPM_SW_RSV_7);
	wakesta->b_sw_flag1 = mmio_read_32(SPM_SW_RSV_8);

	wakesta->rt_req_sta0 = mmio_read_32(SPM_SW_RSV_2);
	wakesta->rt_req_sta1 = mmio_read_32(SPM_SW_RSV_3);
	wakesta->rt_req_sta2 = mmio_read_32(SPM_SW_RSV_4);
	wakesta->rt_req_sta3 = mmio_read_32(SPM_SW_RSV_5);
	wakesta->rt_req_sta4 = mmio_read_32(SPM_SW_RSV_6);

	/* get ISR status */
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);

	/* get SW flag status */
	wakesta->sw_flag0 = mmio_read_32(SPM_SW_FLAG_0);
	wakesta->sw_flag1 = mmio_read_32(SPM_SW_FLAG_1);

	/* get CLK SETTLE */
	wakesta->clk_settle = mmio_read_32(SPM_CLK_SETTLE);

	/* check abort */
	wakesta->abort = (wakesta->debug_flag & DEBUG_ABORT_MASK) |
			 (wakesta->debug_flag1 & DEBUG_ABORT_MASK_1);
}

void __spm_clean_after_wakeup(void)
{
	mmio_write_32(SPM_BK_WAKE_EVENT,
		      mmio_read_32(SPM_WAKEUP_STA) |
		      mmio_read_32(SPM_BK_WAKE_EVENT));
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);

	/*
	 * clean wakeup event raw status (for edge trigger event)
	 * bit[28] for cpu wake up event
	 */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, SPM_WAKEUP_EVENT_MASK_CLEAN_MASK);

	/* clean ISR status (except TWAM) */
	mmio_setbits_32(SPM_IRQ_MASK, ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL_EXC_TWAM);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);
}

void __spm_set_pcm_wdt(int en)
{
	mmio_clrsetbits_32(PCM_CON1, RG_PCM_WDT_EN_LSB,
			   SPM_REGWR_CFG_KEY);

	if (en == 1) {
		mmio_clrsetbits_32(PCM_CON1, RG_PCM_WDT_WAKE_LSB,
				   SPM_REGWR_CFG_KEY);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX) {
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		}

		mmio_write_32(PCM_WDT_VAL,
			      mmio_read_32(PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
		mmio_setbits_32(PCM_CON1,
				SPM_REGWR_CFG_KEY | RG_PCM_WDT_EN_LSB);
	}
}

void __spm_send_cpu_wakeup_event(void)
{
	/* SPM will clear SPM_CPU_WAKEUP_EVENT */
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
}

void __spm_ext_int_wakeup_req_clr(void)
{
	mmio_write_32(EXT_INT_WAKEUP_REQ_CLR, mmio_read_32(ROOT_CPUTOP_ADDR));

	/* Clear spm2mcupm wakeup interrupt status */
	mmio_write_32(SPM2CPUEB_CON, 0);
}

void __spm_xo_soc_bblpm(int en)
{
	if (en == 1) {
		mmio_clrsetbits_32(RC_M00_SRCLKEN_CFG,
				   RC_SW_SRCLKEN_FPM, RC_SW_SRCLKEN_RC);
		assert(mt_spm_bblpm_cnt == 0);
		mt_spm_bblpm_cnt += 1;
	} else {
		mmio_clrsetbits_32(RC_M00_SRCLKEN_CFG,
				   RC_SW_SRCLKEN_RC, RC_SW_SRCLKEN_FPM);
		mt_spm_bblpm_cnt -= 1;
	}
}

void __spm_hw_s1_state_monitor(int en, unsigned int *status)
{
	unsigned int reg;

	reg = mmio_read_32(SPM_ACK_CHK_CON_3);

	if (en == 1) {
		reg &= ~SPM_ACK_CHK_3_CON_CLR_ALL;
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
		reg |= SPM_ACK_CHK_3_CON_EN;
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
	} else {
		if (((reg & SPM_ACK_CHK_3_CON_RESULT) != 0U) &&
		    (status != NULL)) {
			*status |= SPM_INTERNAL_STATUS_HW_S1;
		}

		mmio_clrsetbits_32(SPM_ACK_CHK_CON_3, SPM_ACK_CHK_3_CON_EN,
				   SPM_ACK_CHK_3_CON_HW_MODE_TRIG |
				   SPM_ACK_CHK_3_CON_CLR_ALL);
	}
}
