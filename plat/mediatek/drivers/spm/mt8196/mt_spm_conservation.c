/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lp_rqm.h>
#include <mt_spm.h>
#include <mt_spm_common.h>
#include <mt_spm_conservation.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>

#define MT_RESUMETIME_THRESHOLD_MAX	5
#define IS_RESUME_OVERTIME(delta) \
	(delta > MT_RESUMETIME_THRESHOLD_MAX)

static struct wake_status spm_wakesta; /* Record last wakesta */
static wake_reason_t spm_wake_reason = WR_NONE;
static struct resource_req_status generic_spm_resource_req = {
	.id = MT_LP_RQ_ID_ALL_USAGE,
	.val = 0,
};

#define do_spm_init(pwrctrl)	({ int local_ret = 0; local_ret; })
#define do_spm_run(pwrctrl)	__spm_send_cpu_wakeup_event()

static int go_to_spm_before_wfi(int state_id, uint32_t ext_opand,
				struct spm_lp_scen *spm_lp,
				uint32_t resource_req)
{
	int ret = 0;
	struct pwr_ctrl *pwrctrl;

	pwrctrl = spm_lp->pwrctrl;

#if SPM_FW_NO_RESUME == 0
	ret = do_spm_init(pwrctrl);

	if (ret)
		return ret;
#endif
	__spm_set_power_control(pwrctrl, resource_req);
	__spm_set_wakeup_event(pwrctrl);
#if defined(CONFIG_MTK_VCOREDVFS_SUPPORT)
	__spm_sync_vcore_dvfs_power_control(pwrctrl, __spm_vcorefs.pwrctrl);
#endif

#ifdef MTK_SPM_IVI_SUPPORT
	pwrctrl->pcm_flags |= SPM_FLAG_ENABLE_MT8196_IVI;
#endif
	__spm_set_pcm_flags(pwrctrl);

#ifdef HW_S1_DETECT
	if (ext_opand & MT_SPM_EX_OP_HW_S1_DETECT)
		spm_hw_s1_state_monitor_resume();
#endif
	do_spm_run(pwrctrl);
	return ret;
}

static void go_to_spm_after_wfi(int state_id, uint32_t ext_opand,
				struct spm_lp_scen *spm_lp,
				struct wake_status **status)
{
	uint32_t ext_status = 0;

	spm_wakesta.tr.comm.resumetime = 0;
	spm_wakesta.tr.comm.times_h = spm_wakesta.tr.comm.times_l = 0;

	if (ext_opand & MT_SPM_EX_OP_HW_S1_DETECT)
		spm_hw_s1_state_monitor_pause(&ext_status);

	__spm_ext_int_wakeup_req_clr();

	__spm_get_wakeup_status(&spm_wakesta, ext_status);

	if (status)
		*status = &spm_wakesta;

#ifndef MT_SPM_COMMON_SODI_SUPPORT
	__spm_clean_after_wakeup();
#endif
	spm_wake_reason = __spm_output_wake_reason(&spm_wakesta);
}

int spm_conservation(int state_id, uint32_t ext_opand,
		     struct spm_lp_scen *spm_lp, uint32_t resource_req)
{
	uint32_t rc_state = resource_req;

	if (!spm_lp)
		return -1;

	spm_lock_get();

	/* Uart bk/rs is needed if infra off for legacy project
	 * leave code here for reference.
	 */
	if (ext_opand & MT_SPM_EX_OP_NOTIFY_INFRA_OFF) {
#ifndef MTK_PLAT_SPM_UART_UNSUPPORT
		/* Notify UART to sleep */
		mt_uart_save();
#endif
	}

	if (!(ext_opand & MT_SPM_EX_OP_NON_GENERIC_RESOURCE_REQ)) {
		/* Resource request */
		mt_lp_rq_get_status(PLAT_RQ_REQ_USAGE,
				    &generic_spm_resource_req);
		rc_state |= generic_spm_resource_req.val;
	}
	go_to_spm_before_wfi(state_id, ext_opand, spm_lp, rc_state);

	spm_lock_release();

	return 0;
}

void spm_conservation_finish(int state_id, uint32_t ext_opand,
			     struct spm_lp_scen *spm_lp,
			     struct wake_status **status)
{
	/* Uart bk/rs is needed if infra off for legacy project
	 * leave code here for reference.
	 */
	if (ext_opand & MT_SPM_EX_OP_NOTIFY_INFRA_OFF) {
#ifndef MTK_PLAT_SPM_UART_UNSUPPORT
		/* Notify UART to wakeup */
		mt_uart_restore();
#endif
	}

	spm_lock_get();
	go_to_spm_after_wfi(state_id, ext_opand, spm_lp, status);
#ifdef MT_SPM_COMMON_SODI_SUPPORT
	/* Restore common sodi mask and resource req setting */
	mt_spm_set_common_sodi_pwrctr();
	mt_spm_set_common_sodi_pcm_flags();
#endif
	spm_lock_release();
}

int spm_conservation_get_result(struct wake_status **res)
{
	if (!res)
		return -1;
	*res = &spm_wakesta;
	return 0;
}
