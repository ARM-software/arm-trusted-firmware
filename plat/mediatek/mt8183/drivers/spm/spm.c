/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lib/bakery_lock.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <spm.h>
#include <spm_pmic_wrap.h>

DEFINE_BAKERY_LOCK(spm_lock);

const char *wakeup_src_str[32] = {
	[0] = "R12_PCM_TIMER",
	[1] = "R12_SSPM_WDT_EVENT_B",
	[2] = "R12_KP_IRQ_B",
	[3] = "R12_APWDT_EVENT_B",
	[4] = "R12_APXGPT1_EVENT_B",
	[5] = "R12_CONN2AP_SPM_WAKEUP_B",
	[6] = "R12_EINT_EVENT_B",
	[7] = "R12_CONN_WDT_IRQ_B",
	[8] = "R12_CCIF0_EVENT_B",
	[9] = "R12_LOWBATTERY_IRQ_B",
	[10] = "R12_SSPM_SPM_IRQ_B",
	[11] = "R12_SCP_SPM_IRQ_B",
	[12] = "R12_SCP_WDT_EVENT_B",
	[13] = "R12_PCM_WDT_WAKEUP_B",
	[14] = "R12_USB_CDSC_B ",
	[15] = "R12_USB_POWERDWN_B",
	[16] = "R12_SYS_TIMER_EVENT_B",
	[17] = "R12_EINT_EVENT_SECURE_B",
	[18] = "R12_CCIF1_EVENT_B",
	[19] = "R12_UART0_IRQ_B",
	[20] = "R12_AFE_IRQ_MCU_B",
	[21] = "R12_THERM_CTRL_EVENT_B",
	[22] = "R12_SYS_CIRQ_IRQ_B",
	[23] = "R12_MD2AP_PEER_EVENT_B",
	[24] = "R12_CSYSPWREQ_B",
	[25] = "R12_MD1_WDT_B ",
	[26] = "R12_CLDMA_EVENT_B",
	[27] = "R12_SEJ_WDT_GPT_B",
	[28] = "R12_ALL_SSPM_WAKEUP_B",
	[29] = "R12_CPU_IRQ_B",
	[30] = "R12_CPU_WFI_AND_B"
};

const char *spm_get_firmware_version(void)
{
	return "DYNAMIC_SPM_FW_VERSION";
}

void spm_lock_init(void)
{
	bakery_lock_init(&spm_lock);
}

void spm_lock_get(void)
{
	bakery_lock_get(&spm_lock);
}

void spm_lock_release(void)
{
	bakery_lock_release(&spm_lock);
}

void spm_set_bootaddr(unsigned long bootaddr)
{
	/* initialize core4~7 boot entry address */
	mmio_write_32(SW2SPM_MAILBOX_3, bootaddr);
}

void spm_set_cpu_status(int cpu)
{
	if (cpu >= 0 && cpu < 4) {
		mmio_write_32(ROOT_CPUTOP_ADDR, 0x10006204);
		mmio_write_32(ROOT_CORE_ADDR, 0x10006208 + (cpu * 0x4));
	} else if (cpu >= 4 && cpu < 8) {
		mmio_write_32(ROOT_CPUTOP_ADDR, 0x10006218);
		mmio_write_32(ROOT_CORE_ADDR, 0x1000621c + ((cpu - 4) * 0x4));
	} else {
		ERROR("%s: error cpu number %d\n", __func__, cpu);
	}
}

void spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	mmio_write_32(SPM_AP_STANDBY_CON,
		      ((pwrctrl->wfi_op & 0x1) << 0) |
		      ((pwrctrl->mp0_cputop_idle_mask & 0x1) << 1) |
		      ((pwrctrl->mp1_cputop_idle_mask & 0x1) << 2) |
		      ((pwrctrl->mcusys_idle_mask & 0x1) << 4) |
		      ((pwrctrl->mm_mask_b & 0x3) << 16) |
		      ((pwrctrl->md_ddr_en_0_dbc_en & 0x1) << 18) |
		      ((pwrctrl->md_ddr_en_1_dbc_en & 0x1) << 19) |
		      ((pwrctrl->md_mask_b & 0x3) << 20) |
		      ((pwrctrl->sspm_mask_b & 0x1) << 22) |
		      ((pwrctrl->scp_mask_b & 0x1) << 23) |
		      ((pwrctrl->srcclkeni_mask_b & 0x1) << 24) |
		      ((pwrctrl->md_apsrc_1_sel & 0x1) << 25) |
		      ((pwrctrl->md_apsrc_0_sel & 0x1) << 26) |
		      ((pwrctrl->conn_ddr_en_dbc_en & 0x1) << 27) |
		      ((pwrctrl->conn_mask_b & 0x1) << 28) |
		      ((pwrctrl->conn_apsrc_sel & 0x1) << 29));

	mmio_write_32(SPM_SRC_REQ,
		      ((pwrctrl->spm_apsrc_req & 0x1) << 0) |
		      ((pwrctrl->spm_f26m_req & 0x1) << 1) |
		      ((pwrctrl->spm_infra_req & 0x1) << 3) |
		      ((pwrctrl->spm_vrf18_req & 0x1) << 4) |
		      ((pwrctrl->spm_ddren_req & 0x1) << 7) |
		      ((pwrctrl->spm_rsv_src_req & 0x7) << 8) |
		      ((pwrctrl->spm_ddren_2_req & 0x1) << 11) |
		      ((pwrctrl->cpu_md_dvfs_sop_force_on & 0x1) << 16));

	mmio_write_32(SPM_SRC_MASK,
		      ((pwrctrl->csyspwreq_mask & 0x1) << 0) |
		      ((pwrctrl->ccif0_md_event_mask_b & 0x1) << 1) |
		      ((pwrctrl->ccif0_ap_event_mask_b & 0x1) << 2) |
		      ((pwrctrl->ccif1_md_event_mask_b & 0x1) << 3) |
		      ((pwrctrl->ccif1_ap_event_mask_b & 0x1) << 4) |
		      ((pwrctrl->ccif2_md_event_mask_b & 0x1) << 5) |
		      ((pwrctrl->ccif2_ap_event_mask_b & 0x1) << 6) |
		      ((pwrctrl->ccif3_md_event_mask_b & 0x1) << 7) |
		      ((pwrctrl->ccif3_ap_event_mask_b & 0x1) << 8) |
		      ((pwrctrl->md_srcclkena_0_infra_mask_b & 0x1) << 9) |
		      ((pwrctrl->md_srcclkena_1_infra_mask_b & 0x1) << 10) |
		      ((pwrctrl->conn_srcclkena_infra_mask_b & 0x1) << 11) |
		      ((pwrctrl->ufs_infra_req_mask_b & 0x1) << 12) |
		      ((pwrctrl->srcclkeni_infra_mask_b & 0x1) << 13) |
		      ((pwrctrl->md_apsrc_req_0_infra_mask_b & 0x1) << 14) |
		      ((pwrctrl->md_apsrc_req_1_infra_mask_b & 0x1) << 15) |
		      ((pwrctrl->conn_apsrcreq_infra_mask_b & 0x1) << 16) |
		      ((pwrctrl->ufs_srcclkena_mask_b & 0x1) << 17) |
		      ((pwrctrl->md_vrf18_req_0_mask_b & 0x1) << 18) |
		      ((pwrctrl->md_vrf18_req_1_mask_b & 0x1) << 19) |
		      ((pwrctrl->ufs_vrf18_req_mask_b & 0x1) << 20) |
		      ((pwrctrl->gce_vrf18_req_mask_b & 0x1) << 21) |
		      ((pwrctrl->conn_infra_req_mask_b & 0x1) << 22) |
		      ((pwrctrl->gce_apsrc_req_mask_b & 0x1) << 23) |
		      ((pwrctrl->disp0_apsrc_req_mask_b & 0x1) << 24) |
		      ((pwrctrl->disp1_apsrc_req_mask_b & 0x1) << 25) |
		      ((pwrctrl->mfg_req_mask_b & 0x1) << 26) |
		      ((pwrctrl->vdec_req_mask_b & 0x1) << 27));

	mmio_write_32(SPM_SRC2_MASK,
		      ((pwrctrl->md_ddr_en_0_mask_b & 0x1) << 0) |
		      ((pwrctrl->md_ddr_en_1_mask_b & 0x1) << 1) |
		      ((pwrctrl->conn_ddr_en_mask_b & 0x1) << 2) |
		      ((pwrctrl->ddren_sspm_apsrc_req_mask_b & 0x1) << 3) |
		      ((pwrctrl->ddren_scp_apsrc_req_mask_b & 0x1) << 4) |
		      ((pwrctrl->disp0_ddren_mask_b & 0x1) << 5) |
		      ((pwrctrl->disp1_ddren_mask_b & 0x1) << 6) |
		      ((pwrctrl->gce_ddren_mask_b & 0x1) << 7) |
		      ((pwrctrl->ddren_emi_self_refresh_ch0_mask_b & 0x1)
		       << 8) |
		      ((pwrctrl->ddren_emi_self_refresh_ch1_mask_b & 0x1)
		       << 9));

	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		      ((pwrctrl->spm_wakeup_event_mask & 0xffffffff) << 0));

	mmio_write_32(SPM_WAKEUP_EVENT_EXT_MASK,
		      ((pwrctrl->spm_wakeup_event_ext_mask & 0xffffffff)
		       << 0));

	mmio_write_32(SPM_SRC3_MASK,
		      ((pwrctrl->md_ddr_en_2_0_mask_b & 0x1) << 0) |
		      ((pwrctrl->md_ddr_en_2_1_mask_b & 0x1) << 1) |
		      ((pwrctrl->conn_ddr_en_2_mask_b & 0x1) << 2) |
		      ((pwrctrl->ddren2_sspm_apsrc_req_mask_b & 0x1) << 3) |
		      ((pwrctrl->ddren2_scp_apsrc_req_mask_b & 0x1) << 4) |
		      ((pwrctrl->disp0_ddren2_mask_b & 0x1) << 5) |
		      ((pwrctrl->disp1_ddren2_mask_b & 0x1) << 6) |
		      ((pwrctrl->gce_ddren2_mask_b & 0x1) << 7) |
		      ((pwrctrl->ddren2_emi_self_refresh_ch0_mask_b & 0x1)
		       << 8) |
		      ((pwrctrl->ddren2_emi_self_refresh_ch1_mask_b & 0x1)
		       << 9));

	mmio_write_32(MP0_CPU0_WFI_EN,
		      ((pwrctrl->mp0_cpu0_wfi_en & 0x1) << 0));
	mmio_write_32(MP0_CPU1_WFI_EN,
		      ((pwrctrl->mp0_cpu1_wfi_en & 0x1) << 0));
	mmio_write_32(MP0_CPU2_WFI_EN,
		      ((pwrctrl->mp0_cpu2_wfi_en & 0x1) << 0));
	mmio_write_32(MP0_CPU3_WFI_EN,
		      ((pwrctrl->mp0_cpu3_wfi_en & 0x1) << 0));

	mmio_write_32(MP1_CPU0_WFI_EN,
		      ((pwrctrl->mp1_cpu0_wfi_en & 0x1) << 0));
	mmio_write_32(MP1_CPU1_WFI_EN,
		      ((pwrctrl->mp1_cpu1_wfi_en & 0x1) << 0));
	mmio_write_32(MP1_CPU2_WFI_EN,
		      ((pwrctrl->mp1_cpu2_wfi_en & 0x1) << 0));
	mmio_write_32(MP1_CPU3_WFI_EN,
		      ((pwrctrl->mp1_cpu3_wfi_en & 0x1) << 0));
}

void spm_disable_pcm_timer(void)
{
	mmio_clrsetbits_32(PCM_CON1, PCM_TIMER_EN_LSB, SPM_REGWR_CFG_KEY);
}

void spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	uint32_t val, mask, isr;

	val = pwrctrl->timer_val ? pwrctrl->timer_val : PCM_TIMER_MAX;
	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_setbits_32(PCM_CON1, SPM_REGWR_CFG_KEY | PCM_TIMER_EN_LSB);

	mask = pwrctrl->wake_src;

	if (pwrctrl->csyspwreq_mask)
		mask &= ~WAKE_SRC_R12_CSYSPWREQ_B;

	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	isr = mmio_read_32(SPM_IRQ_MASK) & SPM_TWAM_IRQ_MASK_LSB;
	mmio_write_32(SPM_IRQ_MASK, isr | ISRM_RET_IRQ_AUX);
}

void spm_set_pcm_flags(const struct pwr_ctrl *pwrctrl)
{
	mmio_write_32(SPM_SW_FLAG, pwrctrl->pcm_flags);
	mmio_write_32(SPM_SW_RSV_2, pwrctrl->pcm_flags1);
}

void spm_set_pcm_wdt(int en)
{
	if (en) {
		mmio_clrsetbits_32(PCM_CON1, PCM_WDT_WAKE_MODE_LSB,
				   SPM_REGWR_CFG_KEY);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX)
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		mmio_write_32(PCM_WDT_VAL,
			      mmio_read_32(PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
		mmio_setbits_32(PCM_CON1, SPM_REGWR_CFG_KEY | PCM_WDT_EN_LSB);
	} else {
		mmio_clrsetbits_32(PCM_CON1, PCM_WDT_EN_LSB,
				   SPM_REGWR_CFG_KEY);
	}
}

void spm_send_cpu_wakeup_event(void)
{
	mmio_write_32(PCM_REG_DATA_INI, 0);
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
}

void spm_get_wakeup_status(struct wake_status *wakesta)
{
	wakesta->assert_pc = mmio_read_32(PCM_REG_DATA_INI);
	wakesta->r12 = mmio_read_32(SPM_SW_RSV_0);
	wakesta->r12_ext = mmio_read_32(PCM_REG12_EXT_DATA);
	wakesta->raw_sta = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->wake_misc = mmio_read_32(SPM_BSI_D0_SR);
	wakesta->timer_out = mmio_read_32(SPM_BSI_D1_SR);
	wakesta->r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SUBSYS_IDLE_STA);
	wakesta->req_sta = mmio_read_32(SRC_REQ_STA);
	wakesta->sw_flag = mmio_read_32(SPM_SW_FLAG);
	wakesta->sw_flag1 = mmio_read_32(SPM_SW_RSV_2);
	wakesta->r15 = mmio_read_32(PCM_REG15_DATA);
	wakesta->debug_flag = mmio_read_32(SPM_SW_DEBUG);
	wakesta->debug_flag1 = mmio_read_32(WDT_LATCH_SPARE0_FIX);
	wakesta->event_reg = mmio_read_32(SPM_BSI_D2_SR);
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);
}

void spm_clean_after_wakeup(void)
{
	mmio_write_32(SPM_SW_RSV_0,
		      mmio_read_32(SPM_WAKEUP_STA) |
		      mmio_read_32(SPM_SW_RSV_0));
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~0);
	mmio_setbits_32(SPM_IRQ_MASK, ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL_EXC_TWAM);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);
}

void spm_output_wake_reason(struct wake_status *wakesta, const char *scenario)
{
	uint32_t i;

	if (wakesta->assert_pc != 0) {
		INFO("%s: PCM ASSERT AT %u, ULPOSC_CON = 0x%x\n",
		     scenario, wakesta->assert_pc, mmio_read_32(ULPOSC_CON));
		goto spm_debug_flags;
	}

	for (i = 0; i <= 31; i++) {
		if (wakesta->r12 & (1U << i)) {
			INFO("%s: wake up by %s, timer_out = %u\n",
			     scenario, wakeup_src_str[i], wakesta->timer_out);
			break;
		}
	}

spm_debug_flags:
	INFO("r15 = 0x%x, r13 = 0x%x, debug_flag = 0x%x 0x%x\n",
	     wakesta->r15, wakesta->r13, wakesta->debug_flag,
	     wakesta->debug_flag1);
	INFO("sw_flag = 0x%x 0x%x, r12 = 0x%x, r12_ext = 0x%x\n",
	     wakesta->sw_flag, wakesta->sw_flag1, wakesta->r12,
	     wakesta->r12_ext);
	INFO("idle_sta = 0x%x, req_sta =  0x%x, event_reg = 0x%x\n",
	     wakesta->idle_sta, wakesta->req_sta, wakesta->event_reg);
	INFO("isr = 0x%x, raw_sta = 0x%x, raw_ext_sta = 0x%x\n",
	     wakesta->isr, wakesta->raw_sta, wakesta->raw_ext_sta);
	INFO("wake_misc = 0x%x\n", wakesta->wake_misc);
}

void spm_boot_init(void)
{
	NOTICE("%s() start\n", __func__);

	spm_lock_init();
	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);

	NOTICE("%s() end\n", __func__);
}
