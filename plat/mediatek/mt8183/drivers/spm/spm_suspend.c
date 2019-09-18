/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <mt_gic_v3.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <pmic.h>
#include <spm.h>
#include <uart.h>

#define SPM_SYSCLK_SETTLE       99

#define WAKE_SRC_FOR_SUSPEND \
	(WAKE_SRC_R12_PCM_TIMER | \
	 WAKE_SRC_R12_SSPM_WDT_EVENT_B | \
	 WAKE_SRC_R12_KP_IRQ_B | \
	 WAKE_SRC_R12_CONN2AP_SPM_WAKEUP_B | \
	 WAKE_SRC_R12_EINT_EVENT_B | \
	 WAKE_SRC_R12_CONN_WDT_IRQ_B | \
	 WAKE_SRC_R12_CCIF0_EVENT_B | \
	 WAKE_SRC_R12_SSPM_SPM_IRQ_B | \
	 WAKE_SRC_R12_SCP_SPM_IRQ_B | \
	 WAKE_SRC_R12_SCP_WDT_EVENT_B | \
	 WAKE_SRC_R12_USB_CDSC_B | \
	 WAKE_SRC_R12_USB_POWERDWN_B | \
	 WAKE_SRC_R12_SYS_TIMER_EVENT_B | \
	 WAKE_SRC_R12_EINT_EVENT_SECURE_B | \
	 WAKE_SRC_R12_CCIF1_EVENT_B | \
	 WAKE_SRC_R12_MD2AP_PEER_EVENT_B | \
	 WAKE_SRC_R12_MD1_WDT_B | \
	 WAKE_SRC_R12_CLDMA_EVENT_B | \
	 WAKE_SRC_R12_SEJ_WDT_GPT_B)

#define SLP_PCM_FLAGS \
	(SPM_FLAG_DIS_VCORE_DVS	| SPM_FLAG_DIS_VCORE_DFS | \
	 SPM_FLAG_DIS_ATF_ABORT | SPM_FLAG_DISABLE_MMSYS_DVFS | \
	 SPM_FLAG_DIS_INFRA_PDN | SPM_FLAG_SUSPEND_OPTION)

#define SLP_PCM_FLAGS1 \
	(SPM_FLAG1_DISABLE_MCDSR)

static const struct pwr_ctrl suspend_ctrl = {
	.wake_src = WAKE_SRC_FOR_SUSPEND,
	.pcm_flags = SLP_PCM_FLAGS,
	.pcm_flags1 = SLP_PCM_FLAGS1,

	/* SPM_AP_STANDBY_CON */
	.wfi_op = 0x1,
	.mp0_cputop_idle_mask = 0,
	.mp1_cputop_idle_mask = 0,
	.mcusys_idle_mask = 0,
	.mm_mask_b = 0,
	.md_ddr_en_0_dbc_en = 0x1,
	.md_ddr_en_1_dbc_en = 0,
	.md_mask_b = 0x1,
	.sspm_mask_b = 0x1,
	.scp_mask_b = 0x1,
	.srcclkeni_mask_b = 0x1,
	.md_apsrc_1_sel = 0,
	.md_apsrc_0_sel = 0,
	.conn_ddr_en_dbc_en = 0x1,
	.conn_mask_b = 0x1,
	.conn_apsrc_sel = 0,

	/* SPM_SRC_REQ */
	.spm_apsrc_req = 0,
	.spm_f26m_req = 0,
	.spm_infra_req = 0,
	.spm_vrf18_req = 0,
	.spm_ddren_req = 0,
	.spm_rsv_src_req = 0,
	.spm_ddren_2_req = 0,
	.cpu_md_dvfs_sop_force_on = 0,

	/* SPM_SRC_MASK */
	.csyspwreq_mask = 0x1,
	.ccif0_md_event_mask_b = 0x1,
	.ccif0_ap_event_mask_b = 0x1,
	.ccif1_md_event_mask_b = 0x1,
	.ccif1_ap_event_mask_b = 0x1,
	.ccif2_md_event_mask_b = 0x1,
	.ccif2_ap_event_mask_b = 0x1,
	.ccif3_md_event_mask_b = 0x1,
	.ccif3_ap_event_mask_b = 0x1,
	.md_srcclkena_0_infra_mask_b = 0x1,
	.md_srcclkena_1_infra_mask_b = 0,
	.conn_srcclkena_infra_mask_b = 0,
	.ufs_infra_req_mask_b = 0,
	.srcclkeni_infra_mask_b = 0,
	.md_apsrc_req_0_infra_mask_b = 0x1,
	.md_apsrc_req_1_infra_mask_b = 0x1,
	.conn_apsrcreq_infra_mask_b = 0x1,
	.ufs_srcclkena_mask_b = 0,
	.md_vrf18_req_0_mask_b = 0,
	.md_vrf18_req_1_mask_b = 0,
	.ufs_vrf18_req_mask_b = 0,
	.gce_vrf18_req_mask_b = 0,
	.conn_infra_req_mask_b = 0x1,
	.gce_apsrc_req_mask_b = 0,
	.disp0_apsrc_req_mask_b = 0,
	.disp1_apsrc_req_mask_b = 0,
	.mfg_req_mask_b = 0,
	.vdec_req_mask_b = 0,

	/* SPM_SRC2_MASK */
	.md_ddr_en_0_mask_b = 0x1,
	.md_ddr_en_1_mask_b = 0,
	.conn_ddr_en_mask_b = 0x1,
	.ddren_sspm_apsrc_req_mask_b = 0x1,
	.ddren_scp_apsrc_req_mask_b = 0x1,
	.disp0_ddren_mask_b = 0x1,
	.disp1_ddren_mask_b = 0x1,
	.gce_ddren_mask_b = 0x1,
	.ddren_emi_self_refresh_ch0_mask_b = 0,
	.ddren_emi_self_refresh_ch1_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.spm_wakeup_event_mask = 0xF1782218,

	/* SPM_WAKEUP_EVENT_EXT_MASK */
	.spm_wakeup_event_ext_mask = 0xFFFFFFFF,

	/* SPM_SRC3_MASK */
	.md_ddr_en_2_0_mask_b = 0x1,
	.md_ddr_en_2_1_mask_b = 0,
	.conn_ddr_en_2_mask_b = 0x1,
	.ddren2_sspm_apsrc_req_mask_b = 0x1,
	.ddren2_scp_apsrc_req_mask_b = 0x1,
	.disp0_ddren2_mask_b = 0,
	.disp1_ddren2_mask_b = 0,
	.gce_ddren2_mask_b = 0,
	.ddren2_emi_self_refresh_ch0_mask_b = 0,
	.ddren2_emi_self_refresh_ch1_mask_b = 0,

	.mp0_cpu0_wfi_en = 0x1,
	.mp0_cpu1_wfi_en = 0x1,
	.mp0_cpu2_wfi_en = 0x1,
	.mp0_cpu3_wfi_en = 0x1,

	.mp1_cpu0_wfi_en = 0x1,
	.mp1_cpu1_wfi_en = 0x1,
	.mp1_cpu2_wfi_en = 0x1,
	.mp1_cpu3_wfi_en = 0x1
};

static uint32_t spm_set_sysclk_settle(void)
{
	mmio_write_32(SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE);
	return mmio_read_32(SPM_CLK_SETTLE);
}

void go_to_sleep_before_wfi(void)
{
	int cpu = MPIDR_AFFLVL0_VAL(read_mpidr());
	uint32_t settle;

	settle = spm_set_sysclk_settle();
	spm_set_cpu_status(cpu);
	spm_set_power_control(&suspend_ctrl);
	spm_set_wakeup_event(&suspend_ctrl);
	spm_set_pcm_flags(&suspend_ctrl);
	spm_send_cpu_wakeup_event();
	spm_set_pcm_wdt(0);
	spm_disable_pcm_timer();

	if (is_infra_pdn(suspend_ctrl.pcm_flags))
		mt_uart_save();

	if (!mt_console_uart_cg_status())
		console_switch_state(CONSOLE_FLAG_BOOT);

	INFO("cpu%d: \"%s\", wakesrc = 0x%x, pcm_con1 = 0x%x\n",
	     cpu, spm_get_firmware_version(), suspend_ctrl.wake_src,
	     mmio_read_32(PCM_CON1));
	INFO("settle = %u, sec = %u, sw_flag = 0x%x 0x%x, src_req = 0x%x\n",
	     settle, mmio_read_32(PCM_TIMER_VAL) / 32768,
	     suspend_ctrl.pcm_flags, suspend_ctrl.pcm_flags1,
	     mmio_read_32(SPM_SRC_REQ));

	if (!mt_console_uart_cg_status())
		console_switch_state(CONSOLE_FLAG_RUNTIME);
}

static void go_to_sleep_after_wfi(void)
{
	struct wake_status spm_wakesta;

	if (is_infra_pdn(suspend_ctrl.pcm_flags))
		mt_uart_restore();

	spm_set_pcm_wdt(0);
	spm_get_wakeup_status(&spm_wakesta);
	spm_clean_after_wakeup();

	if (!mt_console_uart_cg_status())
		console_switch_state(CONSOLE_FLAG_BOOT);

	spm_output_wake_reason(&spm_wakesta, "suspend");

	if (!mt_console_uart_cg_status())
		console_switch_state(CONSOLE_FLAG_RUNTIME);
}

static void spm_enable_armpll_l(void)
{
	/* power on */
	mmio_setbits_32(ARMPLL_L_PWR_CON0, 0x1);

	/* clear isolation */
	mmio_clrbits_32(ARMPLL_L_PWR_CON0, 0x2);

	/* enable pll */
	mmio_setbits_32(ARMPLL_L_CON0, 0x1);

	/* Add 20us delay for turning on PLL */
	udelay(20);
}

static void spm_disable_armpll_l(void)
{
	/* disable pll */
	mmio_clrbits_32(ARMPLL_L_CON0, 0x1);

	/* isolation */
	mmio_setbits_32(ARMPLL_L_PWR_CON0, 0x2);

	/* power off */
	mmio_clrbits_32(ARMPLL_L_PWR_CON0, 0x1);
}

void spm_system_suspend(void)
{
	spm_disable_armpll_l();
	bcpu_enable(0);
	bcpu_sram_enable(0);
	spm_lock_get();
	go_to_sleep_before_wfi();
	spm_lock_release();
}

void spm_system_suspend_finish(void)
{
	spm_lock_get();
	go_to_sleep_after_wfi();
	spm_lock_release();
	spm_enable_armpll_l();
	bcpu_sram_enable(1);
	bcpu_enable(1);
}
