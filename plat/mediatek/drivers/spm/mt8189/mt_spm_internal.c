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

#include <drivers/spm/mt_spm_resource_req.h>
#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <platform_def.h>
#include <pmic_wrap/inc/mt_spm_pmic_wrap.h>

/**************************************
 * Define and Declare
 **************************************/
#define SPM_INIT_DONE_US 20 /* Simulation result */

/**************************************
 * Function and API
 **************************************/

wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta)
{
	uint32_t i;
	wake_reason_t wr = WR_UNKNOWN;

	if (!wakesta)
		return WR_UNKNOWN;

	if (wakesta->is_abort) {
		INFO("SPM EARLY WAKE r13 = 0x%x, ", wakesta->tr.comm.r13);
		INFO("debug_flag = 0x%x 0x%x sw_flag = 0x%x 0x%x b_sw_flag = 0x%x 0x%x\n",
		     wakesta->tr.comm.debug_flag, wakesta->tr.comm.debug_flag1,
		     wakesta->sw_flag0, wakesta->sw_flag1,
		     wakesta->tr.comm.b_sw_flag0, wakesta->tr.comm.b_sw_flag1);
#ifndef MTK_PLAT_SPM_PMIC_WRAP_DUMP_UNSUPPORT
		mt_spm_dump_pmic_warp_reg();
#endif
	}

	if (wakesta->tr.comm.r12 & R12_PCM_TIMER_B) {
		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER_EVENT)
			wr = WR_PCM_TIMER;
	}

	if (wakesta->tr.comm.r12 & R12_TWAM_PMSR_DVFSRC) {
		if (wakesta->wake_misc & WAKE_MISC_DVFSRC_IRQ)
			wr = WR_DVFSRC;

		if (wakesta->wake_misc & WAKE_MISC_TWAM_IRQ_B)
			wr = WR_TWAM;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_0)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_1)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_2)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_3)
			wr = WR_SPM_ACK_CHK;

		if (wakesta->wake_misc & WAKE_MISC_SPM_ACK_CHK_WAKEUP_ALL)
			wr = WR_SPM_ACK_CHK;
	}

	for (i = 2; i < 32; i++) {
		if (wakesta->tr.comm.r12 & (1U << i))
			wr = WR_WAKE_SRC;
	}

	return wr;
}

void __spm_set_cpu_status(int cpu)
{
}

static void spm_code_swapping(void)
{
	uint32_t con1;
	/* int retry = 0, timeout = 5000; */

	con1 = mmio_read_32(SPM_WAKEUP_EVENT_MASK);

	mmio_write_32(SPM_WAKEUP_EVENT_MASK, (con1 & ~(0x1)));
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, con1);
}

void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc)
{
	unsigned char first_load_fw = true;

	/* check the SPM FW is run or not */
	if (mmio_read_32(MD32PCM_CFGREG_SW_RSTN) & 0x1)
		first_load_fw = false;

	if (!first_load_fw) {
		/* SPM code swapping */
		spm_code_swapping();

		/* Backup PCM r0 -> SPM_POWER_ON_VAL0 before `reset PCM` */
		mmio_write_32(SPM_POWER_ON_VAL0,
			      mmio_read_32(MD32PCM_SCU_CTRL0));
	}

	/* disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* disable pcm timer after leaving FW */
	mmio_clrsetbits_32(PCM_CON1, SPM_REGWR_CFG_KEY, REG_PCM_TIMER_EN_LSB);

	/* reset PCM */
	mmio_write_32(PCM_CON0,
		      SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);

	/* init PCM_CON1 (disable PCM timer but keep PCM WDT setting) */
	mmio_clrsetbits_32(PCM_CON1, REG_PCM_WDT_WAKE_LSB,
			   SPM_REGWR_CFG_KEY | REG_SPM_APB_INTERNAL_EN_LSB |
				   REG_SSPM_APB_P2P_EN_LSB);
}

void __spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc)
{
	uint32_t pmem_words;
	uint32_t total_words;
	uint32_t pmem_start;
	uint32_t dmem_start;
	uint32_t ptr;

	/* tell IM where is PCM code (use slave mode if code existed) */
	ptr = pcmdesc->base_dma + 0x40000000;
	pmem_words = pcmdesc->pmem_words;
	total_words = pcmdesc->total_words;
	pmem_start = pcmdesc->pmem_start;
	dmem_start = pcmdesc->dmem_start;

	if (mmio_read_32(MD32PCM_DMA0_SRC) != ptr ||
	    mmio_read_32(MD32PCM_DMA0_DST) != pmem_start ||
	    mmio_read_32(MD32PCM_DMA0_WPPT) != pmem_words ||
	    mmio_read_32(MD32PCM_DMA0_WPTO) != dmem_start ||
	    mmio_read_32(MD32PCM_DMA0_COUNT) != total_words ||
	    mmio_read_32(MD32PCM_DMA0_CON) != 0x0003820E) {
		mmio_write_32(MD32PCM_DMA0_SRC, ptr);
		mmio_write_32(MD32PCM_DMA0_DST, pmem_start);
		mmio_write_32(MD32PCM_DMA0_WPPT, pmem_words);
		mmio_write_32(MD32PCM_DMA0_WPTO, dmem_start);
		mmio_write_32(MD32PCM_DMA0_COUNT, total_words);
		mmio_write_32(MD32PCM_DMA0_CON, 0x0003820E);
		mmio_write_32(MD32PCM_DMA0_START, 0x00008000);
	}

	/* kick IM to fetch (only toggle IM_KICK) */
	mmio_setbits_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
}

void __spm_init_pcm_register(void)
{
	/* disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);
}

void __spm_set_power_control(const struct pwr_ctrl *pwrctrl,
			     uint32_t resource_usage)
{
	/* Auto-gen Start */

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
		  !!(resource_usage & (MT_SPM_26M | MT_SPM_XO_FPM))) & 0x1) << 5) |
		(((pwrctrl->reg_spm_infra_req |
		  !!(resource_usage & MT_SPM_INFRA)) & 0x1) << 6) |
		(((pwrctrl->reg_spm_pmic_req |
		  !!(resource_usage & MT_SPM_PMIC)) & 0x1) << 7) |
		(((uint32_t)pwrctrl->reg_spm_scp_mailbox_req & 0x1) << 8) |
		(((uint32_t)pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 9) |
		(((uint32_t)pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 10) |
		((((uint32_t)pwrctrl->reg_spm_vcore_req |
		  !!(resource_usage & MT_SPM_VCORE)) & 0x1) << 11) |
		((((uint32_t)pwrctrl->reg_spm_vrf18_req |
		  !!(resource_usage & MT_SPM_SYSPLL)) & 0x1) << 12) |
		(((uint32_t)pwrctrl->adsp_mailbox_state & 0x1) << 16) |
		(((uint32_t)pwrctrl->apsrc_state & 0x1) << 17) |
		(((uint32_t)pwrctrl->ddren_state & 0x1) << 18) |
		(((uint32_t)pwrctrl->dvfs_state & 0x1) << 19) |
		(((uint32_t)pwrctrl->emi_state & 0x1) << 20) |
		(((uint32_t)pwrctrl->f26m_state & 0x1) << 21) |
		(((uint32_t)pwrctrl->infra_state & 0x1) << 22) |
		(((uint32_t)pwrctrl->pmic_state & 0x1) << 23) |
		(((uint32_t)pwrctrl->scp_mailbox_state & 0x1) << 24) |
		(((uint32_t)pwrctrl->sspm_mailbox_state & 0x1) << 25) |
		(((uint32_t)pwrctrl->sw_mailbox_state & 0x1) << 26) |
		(((uint32_t)pwrctrl->vcore_state & 0x1) << 27) |
		(((uint32_t)pwrctrl->vrf18_state & 0x1) << 28));

	/* SPM_SRC_MASK_0 */
	mmio_write_32(SPM_SRC_MASK_0,
		(((uint32_t)pwrctrl->reg_apu_apsrc_req_mask_b & 0x1) << 0) |
		(((uint32_t)pwrctrl->reg_apu_ddren_req_mask_b & 0x1) << 1) |
		(((uint32_t)pwrctrl->reg_apu_emi_req_mask_b & 0x1) << 2) |
		(((uint32_t)pwrctrl->reg_apu_infra_req_mask_b & 0x1) << 3) |
		(((uint32_t)pwrctrl->reg_apu_pmic_req_mask_b & 0x1) << 4) |
		(((uint32_t)pwrctrl->reg_apu_srcclkena_mask_b & 0x1) << 5) |
		(((uint32_t)pwrctrl->reg_apu_vrf18_req_mask_b & 0x1) << 6) |
		(((uint32_t)pwrctrl->reg_audio_dsp_apsrc_req_mask_b & 0x1) << 7) |
		(((uint32_t)pwrctrl->reg_audio_dsp_ddren_req_mask_b & 0x1) << 8) |
		(((uint32_t)pwrctrl->reg_audio_dsp_emi_req_mask_b & 0x1) << 9) |
		(((uint32_t)pwrctrl->reg_audio_dsp_infra_req_mask_b & 0x1) << 10) |
		(((uint32_t)pwrctrl->reg_audio_dsp_pmic_req_mask_b & 0x1) << 11) |
		(((uint32_t)pwrctrl->reg_audio_dsp_srcclkena_mask_b & 0x1) << 12) |
		(((uint32_t)pwrctrl->reg_audio_dsp_vcore_req_mask_b & 0x1) << 13) |
		(((uint32_t)pwrctrl->reg_audio_dsp_vrf18_req_mask_b & 0x1) << 14) |
		(((uint32_t)pwrctrl->reg_cam_apsrc_req_mask_b & 0x1) << 15) |
		(((uint32_t)pwrctrl->reg_cam_ddren_req_mask_b & 0x1) << 16) |
		(((uint32_t)pwrctrl->reg_cam_emi_req_mask_b & 0x1) << 17) |
		(((uint32_t)pwrctrl->reg_cam_infra_req_mask_b & 0x1) << 18) |
		(((uint32_t)pwrctrl->reg_cam_pmic_req_mask_b & 0x1) << 19) |
		(((uint32_t)pwrctrl->reg_cam_srcclkena_mask_b & 0x1) << 20) |
		(((uint32_t)pwrctrl->reg_cam_vrf18_req_mask_b & 0x1) << 21) |
		(((uint32_t)pwrctrl->reg_mdp_emi_req_mask_b & 0x1) << 22));

	/* SPM_SRC_MASK_1 */
	mmio_write_32(SPM_SRC_MASK_1,
		(((uint32_t)pwrctrl->reg_ccif_apsrc_req_mask_b & 0xfff) << 0) |
		(((uint32_t)pwrctrl->reg_ccif_emi_req_mask_b & 0xfff) << 12));

	/* SPM_SRC_MASK_2 */
	mmio_write_32(SPM_SRC_MASK_2,
		(((uint32_t)pwrctrl->reg_ccif_infra_req_mask_b & 0xfff) << 0) |
		(((uint32_t)pwrctrl->reg_ccif_pmic_req_mask_b & 0xfff) << 12));

	/* SPM_SRC_MASK_3 */
	mmio_write_32(SPM_SRC_MASK_3,
		(((uint32_t)pwrctrl->reg_ccif_srcclkena_mask_b & 0xfff) << 0) |
		(((uint32_t)pwrctrl->reg_ccif_vrf18_req_mask_b & 0xfff) << 12) |
		(((uint32_t)pwrctrl->reg_ccu_apsrc_req_mask_b & 0x1) << 24) |
		(((uint32_t)pwrctrl->reg_ccu_ddren_req_mask_b & 0x1) << 25) |
		(((uint32_t)pwrctrl->reg_ccu_emi_req_mask_b & 0x1) << 26) |
		(((uint32_t)pwrctrl->reg_ccu_infra_req_mask_b & 0x1) << 27) |
		(((uint32_t)pwrctrl->reg_ccu_pmic_req_mask_b & 0x1) << 28) |
		(((uint32_t)pwrctrl->reg_ccu_srcclkena_mask_b & 0x1) << 29) |
		(((uint32_t)pwrctrl->reg_ccu_vrf18_req_mask_b & 0x1) << 30) |
		(((uint32_t)pwrctrl->reg_cg_check_apsrc_req_mask_b & 0x1) << 31));

	/* SPM_SRC_MASK_4 */
	mmio_write_32(SPM_SRC_MASK_4,
		(((uint32_t)pwrctrl->reg_cg_check_ddren_req_mask_b & 0x1) << 0) |
		(((uint32_t)pwrctrl->reg_cg_check_emi_req_mask_b & 0x1) << 1) |
		(((uint32_t)pwrctrl->reg_cg_check_infra_req_mask_b & 0x1) << 2) |
		(((uint32_t)pwrctrl->reg_cg_check_pmic_req_mask_b & 0x1) << 3) |
		(((uint32_t)pwrctrl->reg_cg_check_srcclkena_mask_b & 0x1) << 4) |
		(((uint32_t)pwrctrl->reg_cg_check_vcore_req_mask_b & 0x1) << 5) |
		(((uint32_t)pwrctrl->reg_cg_check_vrf18_req_mask_b & 0x1) << 6) |
		(((uint32_t)pwrctrl->reg_conn_apsrc_req_mask_b & 0x1) << 7) |
		(((uint32_t)pwrctrl->reg_conn_ddren_req_mask_b & 0x1) << 8) |
		(((uint32_t)pwrctrl->reg_conn_emi_req_mask_b & 0x1) << 9) |
		(((uint32_t)pwrctrl->reg_conn_infra_req_mask_b & 0x1) << 10) |
		(((uint32_t)pwrctrl->reg_conn_pmic_req_mask_b & 0x1) << 11) |
		(((uint32_t)pwrctrl->reg_conn_srcclkena_mask_b & 0x1) << 12) |
		(((uint32_t)pwrctrl->reg_conn_srcclkenb_mask_b & 0x1) << 13) |
		(((uint32_t)pwrctrl->reg_conn_vcore_req_mask_b & 0x1) << 14) |
		(((uint32_t)pwrctrl->reg_conn_vrf18_req_mask_b & 0x1) << 15) |
		(((uint32_t)pwrctrl->reg_cpueb_apsrc_req_mask_b & 0x1) << 16) |
		(((uint32_t)pwrctrl->reg_cpueb_ddren_req_mask_b & 0x1) << 17) |
		(((uint32_t)pwrctrl->reg_cpueb_emi_req_mask_b & 0x1) << 18) |
		(((uint32_t)pwrctrl->reg_cpueb_infra_req_mask_b & 0x1) << 19) |
		(((uint32_t)pwrctrl->reg_cpueb_pmic_req_mask_b & 0x1) << 20) |
		(((uint32_t)pwrctrl->reg_cpueb_srcclkena_mask_b & 0x1) << 21) |
		(((uint32_t)pwrctrl->reg_cpueb_vrf18_req_mask_b & 0x1) << 22) |
		(((uint32_t)pwrctrl->reg_disp0_apsrc_req_mask_b & 0x1) << 23) |
		(((uint32_t)pwrctrl->reg_disp0_ddren_req_mask_b & 0x1) << 24) |
		(((uint32_t)pwrctrl->reg_disp0_emi_req_mask_b & 0x1) << 25) |
		(((uint32_t)pwrctrl->reg_disp0_infra_req_mask_b & 0x1) << 26) |
		(((uint32_t)pwrctrl->reg_disp0_pmic_req_mask_b & 0x1) << 27) |
		(((uint32_t)pwrctrl->reg_disp0_srcclkena_mask_b & 0x1) << 28) |
		(((uint32_t)pwrctrl->reg_disp0_vrf18_req_mask_b & 0x1) << 29) |
		(((uint32_t)pwrctrl->reg_disp1_apsrc_req_mask_b & 0x1) << 30) |
		(((uint32_t)pwrctrl->reg_disp1_ddren_req_mask_b & 0x1) << 31));

	/* SPM_SRC_MASK_5 */
	mmio_write_32(SPM_SRC_MASK_5,
		(((uint32_t)pwrctrl->reg_disp1_emi_req_mask_b & 0x1) << 0) |
		(((uint32_t)pwrctrl->reg_disp1_infra_req_mask_b & 0x1) << 1) |
		(((uint32_t)pwrctrl->reg_disp1_pmic_req_mask_b & 0x1) << 2) |
		(((uint32_t)pwrctrl->reg_disp1_srcclkena_mask_b & 0x1) << 3) |
		(((uint32_t)pwrctrl->reg_disp1_vrf18_req_mask_b & 0x1) << 4) |
		(((uint32_t)pwrctrl->reg_dpm_apsrc_req_mask_b & 0xf) << 5) |
		(((uint32_t)pwrctrl->reg_dpm_ddren_req_mask_b & 0xf) << 9) |
		(((uint32_t)pwrctrl->reg_dpm_emi_req_mask_b & 0xf) << 13) |
		(((uint32_t)pwrctrl->reg_dpm_infra_req_mask_b & 0xf) << 17) |
		(((uint32_t)pwrctrl->reg_dpm_pmic_req_mask_b & 0xf) << 21) |
		(((uint32_t)pwrctrl->reg_dpm_srcclkena_mask_b & 0xf) << 25));

	/* SPM_SRC_MASK_6 */
	mmio_write_32(SPM_SRC_MASK_6,
		(((uint32_t)pwrctrl->reg_dpm_vcore_req_mask_b & 0xf) << 0) |
		(((uint32_t)pwrctrl->reg_dpm_vrf18_req_mask_b & 0xf) << 4) |
		(((uint32_t)pwrctrl->reg_dpmaif_apsrc_req_mask_b & 0x1) << 8) |
		(((uint32_t)pwrctrl->reg_dpmaif_ddren_req_mask_b & 0x1) << 9) |
		(((uint32_t)pwrctrl->reg_dpmaif_emi_req_mask_b & 0x1) << 10) |
		(((uint32_t)pwrctrl->reg_dpmaif_infra_req_mask_b & 0x1) << 11) |
		(((uint32_t)pwrctrl->reg_dpmaif_pmic_req_mask_b & 0x1) << 12) |
		(((uint32_t)pwrctrl->reg_dpmaif_srcclkena_mask_b & 0x1) << 13) |
		(((uint32_t)pwrctrl->reg_dpmaif_vrf18_req_mask_b & 0x1) << 14) |
		(((uint32_t)pwrctrl->reg_dvfsrc_level_req_mask_b & 0x1) << 15) |
		(((uint32_t)pwrctrl->reg_emisys_apsrc_req_mask_b & 0x1) << 16) |
		(((uint32_t)pwrctrl->reg_emisys_ddren_req_mask_b & 0x1) << 17) |
		(((uint32_t)pwrctrl->reg_emisys_emi_req_mask_b & 0x1) << 18) |
		(((uint32_t)pwrctrl->reg_gce_d_apsrc_req_mask_b & 0x1) << 19) |
		(((uint32_t)pwrctrl->reg_gce_d_ddren_req_mask_b & 0x1) << 20) |
		(((uint32_t)pwrctrl->reg_gce_d_emi_req_mask_b & 0x1) << 21) |
		(((uint32_t)pwrctrl->reg_gce_d_infra_req_mask_b & 0x1) << 22) |
		(((uint32_t)pwrctrl->reg_gce_d_pmic_req_mask_b & 0x1) << 23) |
		(((uint32_t)pwrctrl->reg_gce_d_srcclkena_mask_b & 0x1) << 24) |
		(((uint32_t)pwrctrl->reg_gce_d_vrf18_req_mask_b & 0x1) << 25) |
		(((uint32_t)pwrctrl->reg_gce_m_apsrc_req_mask_b & 0x1) << 26) |
		(((uint32_t)pwrctrl->reg_gce_m_ddren_req_mask_b & 0x1) << 27) |
		(((uint32_t)pwrctrl->reg_gce_m_emi_req_mask_b & 0x1) << 28) |
		(((uint32_t)pwrctrl->reg_gce_m_infra_req_mask_b & 0x1) << 29) |
		(((uint32_t)pwrctrl->reg_gce_m_pmic_req_mask_b & 0x1) << 30) |
		(((uint32_t)pwrctrl->reg_gce_m_srcclkena_mask_b & 0x1) << 31));

	/* SPM_SRC_MASK_7 */
	mmio_write_32(SPM_SRC_MASK_7,
		(((uint32_t)pwrctrl->reg_gce_m_vrf18_req_mask_b & 0x1) << 0) |
		(((uint32_t)pwrctrl->reg_gpueb_apsrc_req_mask_b & 0x1) << 1) |
		(((uint32_t)pwrctrl->reg_gpueb_ddren_req_mask_b & 0x1) << 2) |
		(((uint32_t)pwrctrl->reg_gpueb_emi_req_mask_b & 0x1) << 3) |
		(((uint32_t)pwrctrl->reg_gpueb_infra_req_mask_b & 0x1) << 4) |
		(((uint32_t)pwrctrl->reg_gpueb_pmic_req_mask_b & 0x1) << 5) |
		(((uint32_t)pwrctrl->reg_gpueb_srcclkena_mask_b & 0x1) << 6) |
		(((uint32_t)pwrctrl->reg_gpueb_vrf18_req_mask_b & 0x1) << 7) |
		(((uint32_t)pwrctrl->reg_hwccf_apsrc_req_mask_b & 0x1) << 8) |
		(((uint32_t)pwrctrl->reg_hwccf_ddren_req_mask_b & 0x1) << 9) |
		(((uint32_t)pwrctrl->reg_hwccf_emi_req_mask_b & 0x1) << 10) |
		(((uint32_t)pwrctrl->reg_hwccf_infra_req_mask_b & 0x1) << 11) |
		(((uint32_t)pwrctrl->reg_hwccf_pmic_req_mask_b & 0x1) << 12) |
		(((uint32_t)pwrctrl->reg_hwccf_srcclkena_mask_b & 0x1) << 13) |
		(((uint32_t)pwrctrl->reg_hwccf_vcore_req_mask_b & 0x1) << 14) |
		(((uint32_t)pwrctrl->reg_hwccf_vrf18_req_mask_b & 0x1) << 15) |
		(((uint32_t)pwrctrl->reg_img_apsrc_req_mask_b & 0x1) << 16) |
		(((uint32_t)pwrctrl->reg_img_ddren_req_mask_b & 0x1) << 17) |
		(((uint32_t)pwrctrl->reg_img_emi_req_mask_b & 0x1) << 18) |
		(((uint32_t)pwrctrl->reg_img_infra_req_mask_b & 0x1) << 19) |
		(((uint32_t)pwrctrl->reg_img_pmic_req_mask_b & 0x1) << 20) |
		(((uint32_t)pwrctrl->reg_img_srcclkena_mask_b & 0x1) << 21) |
		(((uint32_t)pwrctrl->reg_img_vrf18_req_mask_b & 0x1) << 22) |
		(((uint32_t)pwrctrl->reg_infrasys_apsrc_req_mask_b & 0x1) << 23) |
		(((uint32_t)pwrctrl->reg_infrasys_ddren_req_mask_b & 0x1) << 24) |
		(((uint32_t)pwrctrl->reg_infrasys_emi_req_mask_b & 0x1) << 25) |
		(((uint32_t)pwrctrl->reg_ipic_infra_req_mask_b & 0x1) << 26) |
		(((uint32_t)pwrctrl->reg_ipic_vrf18_req_mask_b & 0x1) << 27) |
		(((uint32_t)pwrctrl->reg_mcu_apsrc_req_mask_b & 0x1) << 28) |
		(((uint32_t)pwrctrl->reg_mcu_ddren_req_mask_b & 0x1) << 29) |
		(((uint32_t)pwrctrl->reg_mcu_emi_req_mask_b & 0x1) << 30));

	/* SPM_SRC_MASK_8 */
	mmio_write_32(SPM_SRC_MASK_8,
		(((uint32_t)pwrctrl->reg_mcusys_apsrc_req_mask_b & 0xff) << 0) |
		(((uint32_t)pwrctrl->reg_mcusys_ddren_req_mask_b & 0xff) << 8) |
		(((uint32_t)pwrctrl->reg_mcusys_emi_req_mask_b & 0xff) << 16) |
		(((uint32_t)pwrctrl->reg_mcusys_infra_req_mask_b & 0xff) << 24));

	/* SPM_SRC_MASK_9 */
	mmio_write_32(SPM_SRC_MASK_9,
		(((uint32_t)pwrctrl->reg_mcusys_pmic_req_mask_b & 0xff) << 0) |
		(((uint32_t)pwrctrl->reg_mcusys_srcclkena_mask_b & 0xff) << 8) |
		(((uint32_t)pwrctrl->reg_mcusys_vrf18_req_mask_b & 0xff) << 16) |
		(((uint32_t)pwrctrl->reg_md_apsrc_req_mask_b & 0x1) << 24) |
		(((uint32_t)pwrctrl->reg_md_ddren_req_mask_b & 0x1) << 25) |
		(((uint32_t)pwrctrl->reg_md_emi_req_mask_b & 0x1) << 26) |
		(((uint32_t)pwrctrl->reg_md_infra_req_mask_b & 0x1) << 27) |
		(((uint32_t)pwrctrl->reg_md_pmic_req_mask_b & 0x1) << 28) |
		(((uint32_t)pwrctrl->reg_md_srcclkena_mask_b & 0x1) << 29) |
		(((uint32_t)pwrctrl->reg_md_srcclkena1_mask_b & 0x1) << 30) |
		(((uint32_t)pwrctrl->reg_md_vcore_req_mask_b & 0x1) << 31));

	/* SPM_SRC_MASK_10 */
	mmio_write_32(SPM_SRC_MASK_10,
		(((uint32_t)pwrctrl->reg_md_vrf18_req_mask_b & 0x1) << 0) |
		(((uint32_t)pwrctrl->reg_mdp_apsrc_req_mask_b & 0x1) << 1) |
		(((uint32_t)pwrctrl->reg_mdp_ddren_req_mask_b & 0x1) << 2) |
		(((uint32_t)pwrctrl->reg_mm_proc_apsrc_req_mask_b & 0x1) << 3) |
		(((uint32_t)pwrctrl->reg_mm_proc_ddren_req_mask_b & 0x1) << 4) |
		(((uint32_t)pwrctrl->reg_mm_proc_emi_req_mask_b & 0x1) << 5) |
		(((uint32_t)pwrctrl->reg_mm_proc_infra_req_mask_b & 0x1) << 6) |
		(((uint32_t)pwrctrl->reg_mm_proc_pmic_req_mask_b & 0x1) << 7) |
		(((uint32_t)pwrctrl->reg_mm_proc_srcclkena_mask_b & 0x1) << 8) |
		(((uint32_t)pwrctrl->reg_mm_proc_vrf18_req_mask_b & 0x1) << 9) |
		(((uint32_t)pwrctrl->reg_mmsys_apsrc_req_mask_b & 0x1) << 10) |
		(((uint32_t)pwrctrl->reg_mmsys_ddren_req_mask_b & 0x1) << 11) |
		(((uint32_t)pwrctrl->reg_mmsys_vrf18_req_mask_b & 0x1) << 12) |
		(((uint32_t)pwrctrl->reg_pcie0_apsrc_req_mask_b & 0x1) << 13) |
		(((uint32_t)pwrctrl->reg_pcie0_ddren_req_mask_b & 0x1) << 14) |
		(((uint32_t)pwrctrl->reg_pcie0_infra_req_mask_b & 0x1) << 15) |
		(((uint32_t)pwrctrl->reg_pcie0_srcclkena_mask_b & 0x1) << 16) |
		(((uint32_t)pwrctrl->reg_pcie0_vrf18_req_mask_b & 0x1) << 17) |
		(((uint32_t)pwrctrl->reg_pcie1_apsrc_req_mask_b & 0x1) << 18) |
		(((uint32_t)pwrctrl->reg_pcie1_ddren_req_mask_b & 0x1) << 19) |
		(((uint32_t)pwrctrl->reg_pcie1_infra_req_mask_b & 0x1) << 20) |
		(((uint32_t)pwrctrl->reg_pcie1_srcclkena_mask_b & 0x1) << 21) |
		(((uint32_t)pwrctrl->reg_pcie1_vrf18_req_mask_b & 0x1) << 22) |
		(((uint32_t)pwrctrl->reg_perisys_apsrc_req_mask_b & 0x1) << 23) |
		(((uint32_t)pwrctrl->reg_perisys_ddren_req_mask_b & 0x1) << 24) |
		(((uint32_t)pwrctrl->reg_perisys_emi_req_mask_b & 0x1) << 25) |
		(((uint32_t)pwrctrl->reg_perisys_infra_req_mask_b & 0x1) << 26) |
		(((uint32_t)pwrctrl->reg_perisys_pmic_req_mask_b & 0x1) << 27) |
		(((uint32_t)pwrctrl->reg_perisys_srcclkena_mask_b & 0x1) << 28) |
		(((uint32_t)pwrctrl->reg_perisys_vcore_req_mask_b & 0x1) << 29) |
		(((uint32_t)pwrctrl->reg_perisys_vrf18_req_mask_b & 0x1) << 30) |
		(((uint32_t)pwrctrl->reg_scp_apsrc_req_mask_b & 0x1) << 31));

	/* SPM_SRC_MASK_11 */
	mmio_write_32(SPM_SRC_MASK_11,
		(((uint32_t)pwrctrl->reg_scp_ddren_req_mask_b & 0x1) << 0) |
		(((uint32_t)pwrctrl->reg_scp_emi_req_mask_b & 0x1) << 1) |
		(((uint32_t)pwrctrl->reg_scp_infra_req_mask_b & 0x1) << 2) |
		(((uint32_t)pwrctrl->reg_scp_pmic_req_mask_b & 0x1) << 3) |
		(((uint32_t)pwrctrl->reg_scp_srcclkena_mask_b & 0x1) << 4) |
		(((uint32_t)pwrctrl->reg_scp_vcore_req_mask_b & 0x1) << 5) |
		(((uint32_t)pwrctrl->reg_scp_vrf18_req_mask_b & 0x1) << 6) |
		(((uint32_t)pwrctrl->reg_srcclkeni_infra_req_mask_b & 0x3) << 7) |
		(((uint32_t)pwrctrl->reg_srcclkeni_pmic_req_mask_b & 0x3) << 9) |
		(((uint32_t)pwrctrl->reg_srcclkeni_srcclkena_mask_b & 0x3) << 11) |
		(((uint32_t)pwrctrl->reg_sspm_apsrc_req_mask_b & 0x1) << 13) |
		(((uint32_t)pwrctrl->reg_sspm_ddren_req_mask_b & 0x1) << 14) |
		(((uint32_t)pwrctrl->reg_sspm_emi_req_mask_b & 0x1) << 15) |
		(((uint32_t)pwrctrl->reg_sspm_infra_req_mask_b & 0x1) << 16) |
		(((uint32_t)pwrctrl->reg_sspm_pmic_req_mask_b & 0x1) << 17) |
		(((uint32_t)pwrctrl->reg_sspm_srcclkena_mask_b & 0x1) << 18) |
		(((uint32_t)pwrctrl->reg_sspm_vrf18_req_mask_b & 0x1) << 19) |
		(((uint32_t)pwrctrl->reg_ssr_apsrc_req_mask_b & 0x1) << 20) |
		(((uint32_t)pwrctrl->reg_ssr_ddren_req_mask_b & 0x1) << 21) |
		(((uint32_t)pwrctrl->reg_ssr_emi_req_mask_b & 0x1) << 22) |
		(((uint32_t)pwrctrl->reg_ssr_infra_req_mask_b & 0x1) << 23) |
		(((uint32_t)pwrctrl->reg_ssr_pmic_req_mask_b & 0x1) << 24) |
		(((uint32_t)pwrctrl->reg_ssr_srcclkena_mask_b & 0x1) << 25) |
		(((uint32_t)pwrctrl->reg_ssr_vrf18_req_mask_b & 0x1) << 26) |
		(((uint32_t)pwrctrl->reg_ufs_apsrc_req_mask_b & 0x1) << 27) |
		(((uint32_t)pwrctrl->reg_ufs_ddren_req_mask_b & 0x1) << 28) |
		(((uint32_t)pwrctrl->reg_ufs_emi_req_mask_b & 0x1) << 29) |
		(((uint32_t)pwrctrl->reg_ufs_infra_req_mask_b & 0x1) << 30) |
		(((uint32_t)pwrctrl->reg_ufs_pmic_req_mask_b & 0x1) << 31));

	/* SPM_SRC_MASK_12 */
	mmio_write_32(SPM_SRC_MASK_12,
		(((uint32_t)pwrctrl->reg_ufs_srcclkena_mask_b & 0x1) << 0) |
		(((uint32_t)pwrctrl->reg_ufs_vrf18_req_mask_b & 0x1) << 1) |
		(((uint32_t)pwrctrl->reg_vdec_apsrc_req_mask_b & 0x1) << 2) |
		(((uint32_t)pwrctrl->reg_vdec_ddren_req_mask_b & 0x1) << 3) |
		(((uint32_t)pwrctrl->reg_vdec_emi_req_mask_b & 0x1) << 4) |
		(((uint32_t)pwrctrl->reg_vdec_infra_req_mask_b & 0x1) << 5) |
		(((uint32_t)pwrctrl->reg_vdec_pmic_req_mask_b & 0x1) << 6) |
		(((uint32_t)pwrctrl->reg_vdec_srcclkena_mask_b & 0x1) << 7) |
		(((uint32_t)pwrctrl->reg_vdec_vrf18_req_mask_b & 0x1) << 8) |
		(((uint32_t)pwrctrl->reg_venc_apsrc_req_mask_b & 0x1) << 9) |
		(((uint32_t)pwrctrl->reg_venc_ddren_req_mask_b & 0x1) << 10) |
		(((uint32_t)pwrctrl->reg_venc_emi_req_mask_b & 0x1) << 11) |
		(((uint32_t)pwrctrl->reg_venc_infra_req_mask_b & 0x1) << 12) |
		(((uint32_t)pwrctrl->reg_venc_pmic_req_mask_b & 0x1) << 13) |
		(((uint32_t)pwrctrl->reg_venc_srcclkena_mask_b & 0x1) << 14) |
		(((uint32_t)pwrctrl->reg_venc_vrf18_req_mask_b & 0x1) << 15) |
		(((uint32_t)pwrctrl->reg_ipe_apsrc_req_mask_b & 0x1) << 16) |
		(((uint32_t)pwrctrl->reg_ipe_ddren_req_mask_b & 0x1) << 17) |
		(((uint32_t)pwrctrl->reg_ipe_emi_req_mask_b & 0x1) << 18) |
		(((uint32_t)pwrctrl->reg_ipe_infra_req_mask_b & 0x1) << 19) |
		(((uint32_t)pwrctrl->reg_ipe_pmic_req_mask_b & 0x1) << 20) |
		(((uint32_t)pwrctrl->reg_ipe_srcclkena_mask_b & 0x1) << 21) |
		(((uint32_t)pwrctrl->reg_ipe_vrf18_req_mask_b & 0x1) << 22) |
		(((uint32_t)pwrctrl->reg_ufs_vcore_req_mask_b & 0x1) << 23));

	/* SPM_EVENT_CON_MISC */
	mmio_write_32(SPM_EVENT_CON_MISC,
		(((uint32_t)pwrctrl->reg_srcclken_fast_resp & 0x1) << 0) |
		(((uint32_t)pwrctrl->reg_csyspwrup_ack_mask & 0x1) << 1));

	/* SPM_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		(((uint32_t)pwrctrl->reg_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_EXT_MASK,
		(((uint32_t)pwrctrl->reg_ext_wakeup_event_mask & 0xffffffff) << 0));
	/* Auto-gen End */
}

#define CHECK_ONE (0xffffffff)
#define CHECK_ZERO (0x0)
static int32_t __spm_check_ack(uint32_t reg, uint32_t mask, uint32_t check_en)
{
	uint32_t val;

	val = mmio_read_32(reg);
	if ((val & mask) == (mask & check_en))
		return 0;
	return -1;
}

int32_t __spm_wait_spm_request_ack(uint32_t spm_resource_req,
				   uint32_t timeout_us)
{
	uint32_t spm_ctrl0_mask, spm_ctrl1_mask;
	int32_t ret, retry;

	if (spm_resource_req == 0)
		return 0;

	spm_ctrl0_mask = 0;
	spm_ctrl1_mask = 0;

	if (spm_resource_req & (MT_SPM_XO_FPM | MT_SPM_26M))
		spm_ctrl0_mask |= CTRL0_SC_MD26M_CK_OFF;

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
			ret = __spm_check_ack(MD32PCM_SCU_CTRL0, spm_ctrl0_mask,
					      CHECK_ZERO);
			if (ret)
				continue;
		}
		if (spm_ctrl1_mask != 0) {
			ret = __spm_check_ack(MD32PCM_SCU_CTRL1, spm_ctrl1_mask,
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
	uint32_t val, mask, isr;

	/* toggle event counter clear */
	mmio_write_32(SPM_EVENT_COUNTER_CLEAR, REG_SPM_EVENT_COUNTER_CLR_LSB);
	/* toggle for reset SYS TIMER start point */
	mmio_write_32(SYS_TIMER_CON,
		      mmio_read_32(SYS_TIMER_CON) | SYS_TIMER_START_EN_LSB);

	if (pwrctrl->timer_val_cust == 0)
		val = pwrctrl->timer_val ? pwrctrl->timer_val :
					   PCM_TIMER_SUSPEND;
	else
		val = pwrctrl->timer_val_cust;

	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY |
					REG_PCM_TIMER_EN_LSB);

	/* unmask AP wakeup source */
	if (pwrctrl->wake_src_cust == 0)
		mask = pwrctrl->wake_src;
	else
		mask = pwrctrl->wake_src_cust;

	if (pwrctrl->reg_csyspwrup_ack_mask)
		mask &= ~R12_CSYSPWREQ_B;
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	/* unmask SPM ISR (keep TWAM setting) */
	isr = mmio_read_32(SPM_IRQ_MASK);
	mmio_write_32(SPM_IRQ_MASK, isr | ISRM_RET_IRQ_AUX);

	/* toggle event counter clear */
	mmio_write_32(SPM_EVENT_COUNTER_CLEAR, 0);
	/* toggle for reset SYS TIMER start point */
	mmio_write_32(SYS_TIMER_CON,
		      mmio_read_32(SYS_TIMER_CON) & ~SYS_TIMER_START_EN_LSB);
}

void __spm_set_fw_resume_option(struct pwr_ctrl *pwrctrl)
{
#if SPM_FW_NO_RESUME
	/* do Nothing */
#else
	pwrctrl->pcm_flags1 |= SPM_FLAG1_DISABLE_NO_RESUME;
#endif
}

void __spm_set_pcm_flags(struct pwr_ctrl *pwrctrl)
{
	/* set PCM flags and data */
	uint32_t pcm_flags =
		(pwrctrl->pcm_flags & ~pwrctrl->pcm_flags_cust_clr) |
		pwrctrl->pcm_flags_cust_set;
	uint32_t pcm_flags1 =
		(pwrctrl->pcm_flags1 & ~pwrctrl->pcm_flags1_cust_clr) |
		pwrctrl->pcm_flags1_cust_set;

	mmio_write_32(SPM_SW_FLAG_0, pcm_flags);

	mmio_write_32(SPM_SW_FLAG_1, pcm_flags1);

	mmio_write_32(PCM_WDT_LATCH_SPARE_7, pcm_flags);

	mmio_write_32(PCM_WDT_LATCH_SPARE_5, pcm_flags1);
}

void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	uint32_t con0;

	/* Waiting for loading SPMFW done*/
	while (mmio_read_32(MD32PCM_DMA0_RLCT) != 0x0)
		;

	__spm_set_pcm_flags(pwrctrl);

	/* kick PCM to run (only toggle PCM_KICK) */
	con0 = mmio_read_32(PCM_CON0);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	/* reset md32pcm */
	con0 = mmio_read_32(MD32PCM_CFGREG_SW_RSTN);
	mmio_write_32(MD32PCM_CFGREG_SW_RSTN, con0 | 0x1);

	/* Waiting for SPM init done and entering WFI*/
	udelay(SPM_INIT_DONE_US);
}

void __spm_get_wakeup_status(struct wake_status *wakesta,
			     unsigned int ext_status)
{
	/* get wakeup event */
	wakesta->tr.comm.r12 = mmio_read_32(SPM_BK_WAKE_EVENT); /* backup of PCM_REG12_DATA */
	wakesta->r12_ext = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->tr.comm.raw_sta = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->md32pcm_wakeup_sta = mmio_read_32(MD32PCM_WAKEUP_STA);
	wakesta->md32pcm_event_sta = mmio_read_32(MD32PCM_EVENT_STA);
	wakesta->wake_misc = mmio_read_32(SPM_BK_WAKE_MISC);   /* backup of SPM_WAKEUP_MISC */

	/* get sleep time */
	wakesta->tr.comm.timer_out = mmio_read_32(SPM_BK_PCM_TIMER);   /* backup of PCM_TIMER_OUT */

	/* get other SYS and co-clock status */
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

	/* get debug flag for PCM execution check */
	wakesta->tr.comm.debug_flag = mmio_read_32(PCM_WDT_LATCH_SPARE_0);
	wakesta->tr.comm.debug_flag1 = mmio_read_32(PCM_WDT_LATCH_SPARE_1);

	/* get backup SW flag status */
	wakesta->tr.comm.b_sw_flag0 = mmio_read_32(PCM_WDT_LATCH_SPARE_7);
	wakesta->tr.comm.b_sw_flag1 = mmio_read_32(PCM_WDT_LATCH_SPARE_5);

	/* get ISR status */
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);

	/* get SW flag status */
	wakesta->sw_flag0 = mmio_read_32(SPM_SW_FLAG_0);
	wakesta->sw_flag1 = mmio_read_32(SPM_SW_FLAG_1);

	/* check abort */
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

	/*
	 * [Vcorefs] can not switch back to POWER_ON_VAL0 here,
	 * the FW stays in VCORE DVFS which use r0 to Ctrl MEM
	 */
	/* disable r0 and r7 to control power */
	/* mmio_write_32(PCM_PWR_IO_EN, 0); */

	/* clean CPU wakeup event */
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);

	/*
	 * [Vcorefs] not disable pcm timer here, due to the
	 * following vcore dvfs will use it for latency check
	 */
	/* clean PCM timer event */
	/*
	 * mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY |
	 *			(mmio_read_32(PCM_CON1) & ~PCM_TIMER_EN_LSB));
	 */

	/* clean wakeup event raw status (for edge trigger event) */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		      0xefffffff); /* bit[28] for cpu wake up event */

	/* clean ISR status (except TWAM) */
	mmio_write_32(SPM_IRQ_MASK,
		      mmio_read_32(SPM_IRQ_MASK) | ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL_EXC_TWAM);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);
}

void __spm_set_pcm_wdt(int en)
{
	/* enable PCM WDT (normal mode) to start count if needed */
	if (en) {
		uint32_t con1;

		con1 = mmio_read_32(PCM_CON1) & ~(REG_PCM_WDT_WAKE_LSB);
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | con1);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX)
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		mmio_write_32(PCM_WDT_VAL,
			      mmio_read_32(PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
		mmio_write_32(PCM_CON1,
			      con1 | SPM_REGWR_CFG_KEY | REG_PCM_WDT_EN_LSB);
	} else {
		mmio_write_32(PCM_CON1,
			      SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) &
						   ~REG_PCM_WDT_EN_LSB));
	}
}

uint32_t __spm_get_pcm_timer_val(void)
{
	/* PCM_TIMER_VAL / 32768 = PCM_TIMER_VAL >> 15 (unit : sec) */
	return mmio_read_32(PCM_TIMER_VAL) >> 15;
}

void __spm_send_cpu_wakeup_event(void)
{
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);
	/* SPM will clear SPM_CPU_WAKEUP_EVENT */
}

void __spm_ext_int_wakeup_req_clr(void)
{
	uint32_t cpu = plat_my_core_pos();
	unsigned int reg;

	mmio_write_32(EXT_INT_WAKEUP_REQ_CLR, (1U << cpu));

	/* Clear spm2mcupm wakeup interrupt status */
	reg = mmio_read_32(SPM2MCUPM_CON);
	reg &= ~SPM2MCUPM_SW_INT_LSB;
	mmio_write_32(SPM2MCUPM_CON, reg);
}

void __spm_clean_before_wfi(void)
{
}

void __spm_hw_s1_state_monitor(int en, unsigned int *status)
{
	unsigned int reg;

	if (en) {
		reg = mmio_read_32(SPM_ACK_CHK_CON_3);
		reg &= ~SPM_ACK_CHK_3_CON_CLR_ALL;
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
		reg |= SPM_ACK_CHK_3_CON_EN;
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
	} else {
		reg = mmio_read_32(SPM_ACK_CHK_CON_3);

		if (reg & SPM_ACK_CHK_3_CON_RESULT) {
			if (status)
				*status |= SPM_INTERNAL_STATUS_HW_S1;
		}

		reg |= (SPM_ACK_CHK_3_CON_HW_MODE_TRIG |
			SPM_ACK_CHK_3_CON_CLR_ALL);
		reg &= ~(SPM_ACK_CHK_3_CON_EN);
		mmio_write_32(SPM_ACK_CHK_CON_3, reg);
	}
}
