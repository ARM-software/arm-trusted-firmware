/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <mt_spm.h>
#include <mt_spm_conservation.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <plat_mtk_lpm.h>
#include <plat_pm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

struct wake_status spm_wakesta; /* record last wakesta */

static int go_to_spm_before_wfi(int state_id, unsigned int ext_opand,
				struct spm_lp_scen *spm_lp,
				unsigned int resource_req)
{
	int ret = 0;
	struct pwr_ctrl *pwrctrl;
	uint32_t cpu = plat_my_core_pos();

	pwrctrl = spm_lp->pwrctrl;

	__spm_set_cpu_status(cpu);
	__spm_set_power_control(pwrctrl);
	__spm_set_wakeup_event(pwrctrl);
	__spm_set_pcm_flags(pwrctrl);
	__spm_src_req_update(pwrctrl, resource_req);

	if ((ext_opand & MT_SPM_EX_OP_SET_WDT) != 0U) {
		__spm_set_pcm_wdt(1);
	}

	if ((ext_opand & MT_SPM_EX_OP_SRCLKEN_RC_BBLPM) != 0U) {
		__spm_xo_soc_bblpm(1);
	}

	if ((ext_opand & MT_SPM_EX_OP_HW_S1_DETECT) != 0U) {
		spm_hw_s1_state_monitor_resume();
	}

	/* Disable auto resume by PCM in system suspend stage */
	if (IS_PLAT_SUSPEND_ID(state_id)) {
		__spm_disable_pcm_timer();
		__spm_set_pcm_wdt(0);
	}

	__spm_send_cpu_wakeup_event();

	INFO("cpu%d: wakesrc = 0x%x, settle = 0x%x, sec = %u\n",
	     cpu, pwrctrl->wake_src, mmio_read_32(SPM_CLK_SETTLE),
	     mmio_read_32(PCM_TIMER_VAL) / 32768);
	INFO("sw_flag = 0x%x 0x%x, req = 0x%x, pwr = 0x%x 0x%x\n",
	     pwrctrl->pcm_flags, pwrctrl->pcm_flags1,
	     mmio_read_32(SPM_SRC_REQ), mmio_read_32(PWR_STATUS),
	     mmio_read_32(PWR_STATUS_2ND));
	INFO("cpu_pwr = 0x%x 0x%x\n", mmio_read_32(CPU_PWR_STATUS),
	     mmio_read_32(CPU_PWR_STATUS_2ND));

	return ret;
}

static void go_to_spm_after_wfi(int state_id, unsigned int ext_opand,
				struct spm_lp_scen *spm_lp,
				struct wake_status **status)
{
	unsigned int ext_status = 0U;

	/* system watchdog will be resumed at kernel stage */
	if ((ext_opand & MT_SPM_EX_OP_SET_WDT) != 0U) {
		__spm_set_pcm_wdt(0);
	}

	if ((ext_opand & MT_SPM_EX_OP_SRCLKEN_RC_BBLPM) != 0U) {
		__spm_xo_soc_bblpm(0);
	}

	if ((ext_opand & MT_SPM_EX_OP_HW_S1_DETECT) != 0U) {
		spm_hw_s1_state_monitor_pause(&ext_status);
	}

	__spm_ext_int_wakeup_req_clr();
	__spm_get_wakeup_status(&spm_wakesta, ext_status);

	if (status != NULL) {
		*status = &spm_wakesta;
	}

	__spm_clean_after_wakeup();

	if (IS_PLAT_SUSPEND_ID(state_id)) {
		__spm_output_wake_reason(state_id, &spm_wakesta);
	}
}

int spm_conservation(int state_id, unsigned int ext_opand,
		     struct spm_lp_scen *spm_lp, unsigned int resource_req)
{
	if (spm_lp == NULL) {
		return -1;
	}

	spm_lock_get();
	go_to_spm_before_wfi(state_id, ext_opand, spm_lp, resource_req);
	spm_lock_release();

	return 0;
}

void spm_conservation_finish(int state_id, unsigned int ext_opand,
			     struct spm_lp_scen *spm_lp,
			     struct wake_status **status)
{
	spm_lock_get();
	go_to_spm_after_wfi(state_id, ext_opand, spm_lp, status);
	spm_lock_release();
}

int spm_conservation_get_result(struct wake_status **res)
{
	if (res == NULL) {
		return -1;
	}

	*res = &spm_wakesta;

	return 0;
}

#define GPIO_BANK	(GPIO_BASE + 0x6F0)
#define TRAP_UFS_FIRST	BIT(11) /* bit 11, 0: UFS, 1: eMMC */

void spm_conservation_pwrctrl_init(struct pwr_ctrl *pwrctrl)
{
	if (pwrctrl == NULL) {
		return;
	}

	/* For ufs, emmc storage type */
	if ((mmio_read_32(GPIO_BANK) & TRAP_UFS_FIRST) != 0U) {
		/* If eMMC is used, mask UFS req */
		pwrctrl->reg_ufs_srcclkena_mask_b = 0;
		pwrctrl->reg_ufs_infra_req_mask_b = 0;
		pwrctrl->reg_ufs_apsrc_req_mask_b = 0;
		pwrctrl->reg_ufs_vrf18_req_mask_b = 0;
		pwrctrl->reg_ufs_ddr_en_mask_b = 0;
	}
}
