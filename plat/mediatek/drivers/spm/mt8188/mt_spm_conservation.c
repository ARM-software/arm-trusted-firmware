/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <lib/pm/mtk_pm.h>
#include <lpm/mt_lp_rqm.h>
#include "mt_spm.h"
#include "mt_spm_conservation.h"
#include "mt_spm_reg.h"
#include <platform_def.h>

#define INFRA_EMI_DCM_CFG0	U(0x10002028)

static struct wake_status spm_wakesta; /* record last wakesta */
static wake_reason_t spm_wake_reason = WR_NONE;
static unsigned int emi_bak;

static int go_to_spm_before_wfi(int state_id, unsigned int ext_opand,
				struct spm_lp_scen *spm_lp,
				unsigned int resource_req)
{
	int ret = 0;
	struct pwr_ctrl *pwrctrl;
	unsigned int cpu = plat_my_core_pos();

	pwrctrl = spm_lp->pwrctrl;

	/* EMI workaround */
	emi_bak = mmio_read_32(INFRA_EMI_DCM_CFG0) & BIT(22);
	mmio_setbits_32(INFRA_EMI_DCM_CFG0, BIT(22));

	__spm_set_cpu_status(cpu);
	__spm_set_power_control(pwrctrl);
	__spm_set_wakeup_event(pwrctrl);

	__spm_set_pcm_flags(pwrctrl);

	__spm_src_req_update(pwrctrl, resource_req);

	if ((ext_opand & MT_SPM_EX_OP_CLR_26M_RECORD) != 0U) {
		__spm_clean_before_wfi();
	}

	if ((ext_opand & MT_SPM_EX_OP_SET_WDT) != 0U) {
		__spm_set_pcm_wdt(1);
	}

	if ((ext_opand & MT_SPM_EX_OP_HW_S1_DETECT) != 0U) {
		spm_hw_s1_state_monitor_resume();
	}

	__spm_send_cpu_wakeup_event();

	INFO("cpu%d: wakesrc = 0x%x, settle = 0x%x, sec = %u\n",
	     cpu, pwrctrl->wake_src, mmio_read_32(SPM_CLK_SETTLE),
	     (mmio_read_32(PCM_TIMER_VAL) / 32768));
	INFO("sw_flag = 0x%x 0x%x, req = 0x%x, pwr = 0x%x 0x%x\n",
	     pwrctrl->pcm_flags, pwrctrl->pcm_flags1,
	     mmio_read_32(SPM_SRC_REQ), mmio_read_32(PWR_STATUS),
	     mmio_read_32(PWR_STATUS_2ND));

	return ret;
}

static void go_to_spm_after_wfi(int state_id, unsigned int ext_opand,
				struct spm_lp_scen *spm_lp,
				struct wake_status **status)
{
	unsigned int ext_status = 0U;

	if ((ext_opand & MT_SPM_EX_OP_SET_WDT) != 0U) {
		__spm_set_pcm_wdt(0);
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
	spm_wake_reason = __spm_output_wake_reason(&spm_wakesta);

	/* EMI workaround */
	if (emi_bak == 0U) {
		mmio_clrbits_32(INFRA_EMI_DCM_CFG0, BIT(22));
	}
}

int spm_conservation(int state_id, unsigned int ext_opand,
		     struct spm_lp_scen *spm_lp,
		     unsigned int resource_req)
{
	unsigned int rc_state = resource_req;

	if (spm_lp == NULL) {
		return -1;
	}

	spin_lock(&spm_lock);
	go_to_spm_before_wfi(state_id, ext_opand, spm_lp, rc_state);
	spin_unlock(&spm_lock);

	return 0;
}

void spm_conservation_finish(int state_id, unsigned int ext_opand, struct spm_lp_scen *spm_lp,
			     struct wake_status **status)
{
	spin_lock(&spm_lock);
	go_to_spm_after_wfi(state_id, ext_opand, spm_lp, status);
	spin_unlock(&spm_lock);
}

int spm_conservation_get_result(struct wake_status **res)
{
	if (res == NULL) {
		return -1;
	}
	*res = &spm_wakesta;
	return 0;
}
