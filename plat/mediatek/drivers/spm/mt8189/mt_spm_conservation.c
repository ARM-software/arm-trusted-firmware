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

#include <lib/pm/mtk_pm.h>
#include <lpm_v2/mt_lp_rqm.h>
#include <mt_spm.h>
#include <mt_spm_common.h>
#include <mt_spm_conservation.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <platform_def.h>
#ifndef MTK_PLAT_SPM_UART_UNSUPPORT
#include <uart.h>
#endif

#define MT_RESUMETIME_THRESHOLD_MAX 5 /*ms*/
#define IS_RESUME_OVERTIME(delta) (delta > MT_RESUMETIME_THRESHOLD_MAX)

static struct wake_status spm_wakesta; /* Record last wakesta */
static wake_reason_t spm_wake_reason = WR_NONE;
static struct resource_req_status generic_spm_resource_req = {
	.id = MT_LP_RQ_ID_ALL_USAGE,
	.val = 0,
};

#ifdef MT_SPM_TIMESTAMP_SUPPORT
struct spm_con_info {
	uint32_t wakeup_obs;
	unsigned short entries;
	unsigned short resume;
	uint64_t entry_time;
};
static struct spm_con_info spm_cst_info;
#endif

#if SPM_FW_NO_RESUME
#define do_spm_init(pwrctrl)       \
	({                         \
		int local_ret = 0; \
		local_ret;         \
	})
#define do_spm_run(pwrctrl) __spm_send_cpu_wakeup_event()
#else
#define do_spm_run(pwrctrl) __spm_kick_pcm_to_run(pwrctrl)
#define do_spm_init(pcmdesc) __spm_conservation_fw_init(pcmdesc)
#endif

#ifndef MTK_PLAT_SPM_TRACE_UNSUPPORT
#define SPM_CONSERVATION_BOUND (MT_SPM_TRACE_LP_RINGBUF_MAX - 1)

static const uint32_t spm_conservation_trace_comm_sz =
	sizeof(struct wake_status_trace_comm);

static void spm_conservation_trace_lp(struct wake_status_trace *trace)
{
	MT_SPM_TRACE_LP_RINGBUF(&trace->comm, spm_conservation_trace_comm_sz);
}

static void spm_conservation_trace_suspend(struct wake_status_trace *trace)
{
	if (!trace) {
		INFO("[%s:%d] Enter\n", __func__, __LINE__);
		return;
	}
}

#endif

#ifdef MT_SPM_TIMESTAMP_SUPPORT
int spm_conservation_wakeup_obs(int is_set, int cat, uint32_t wake_src_bits)
{
	spm_lock_get();
	if (is_set)
		spm_cst_info.wakeup_obs |= wake_src_bits;
	else
		spm_cst_info.wakeup_obs &= ~wake_src_bits;
	spm_lock_release();
	return 0;
}
#endif

static int go_to_spm_before_wfi(int state_id, uint32_t ext_opand,
				struct spm_lp_scen *spm_lp,
				uint32_t resource_req)
{
	int ret = 0;
	struct pwr_ctrl *pwrctrl;
	uint32_t cpu = plat_my_core_pos();

	pwrctrl = spm_lp->pwrctrl;

#if SPM_FW_NO_RESUME == 0
	ret = do_spm_init(pwrctrl);

	if (ret)
		return ret;
#endif
	__spm_set_cpu_status(cpu);
	__spm_set_power_control(pwrctrl, resource_req);
	__spm_set_wakeup_event(pwrctrl);
#if defined(CONFIG_MTK_VCOREDVFS_SUPPORT)
	__spm_sync_vcore_dvfs_power_control(pwrctrl, __spm_vcorefs.pwrctrl);
#endif

	__spm_set_pcm_flags(pwrctrl);

	if (ext_opand & MT_SPM_EX_OP_CLR_26M_RECORD)
		__spm_clean_before_wfi();

	if (ext_opand & MT_SPM_EX_OP_HW_S1_DETECT)
		spm_hw_s1_state_monitor_resume();

	do_spm_run(pwrctrl);

	return ret;
}

static void go_to_spm_after_wfi(int state_id, uint32_t ext_opand,
				struct spm_lp_scen *spm_lp,
				struct wake_status **status)
{
#ifdef MT_SPM_TIMESTAMP_SUPPORT
	uint64_t ktime = 0;
#endif
	uint32_t ext_status = 0;

	spm_wakesta.tr.comm.resumetime = 0;
	spm_wakesta.tr.comm.times_h = spm_wakesta.tr.comm.times_l = 0;
#ifdef MT_SPM_TIMESTAMP_SUPPORT
	if (ext_opand & MT_SPM_EX_OP_TRACE_TIMESTAMP_EN) {
		MT_SPM_TIME_GET(ktime);
		spm_wakesta.tr.comm.times_h = (ktime >> 32);
		spm_wakesta.tr.comm.times_l = (ktime & (uint32_t)-1);
	}
#endif

	if (ext_opand & MT_SPM_EX_OP_HW_S1_DETECT)
		spm_hw_s1_state_monitor_pause(&ext_status);

	__spm_ext_int_wakeup_req_clr();

	__spm_get_wakeup_status(&spm_wakesta, ext_status);

#ifdef MT_SPM_TIMESTAMP_SUPPORT

	if (spm_wakesta.tr.comm.r12 & R12_SYS_CIRQ_IRQ_B)
		INFO("[%s:%d] spm receive wakeup r12 = (0x%" PRIx32
		     ") (0x%" PRIx32 "%" PRIx32 ")\n",
		     __func__, __LINE__, spm_wakesta.tr.comm.r12,
		     mmio_read_32(SYS_TIMER_VALUE_H),
		     mmio_read_32(SYS_TIMER_VALUE_L));

	if (ext_opand & MT_SPM_EX_OP_TRACE_TIMESTAMP_EN) {
		if ((ext_opand & MT_SPM_EX_OP_TIME_CHECK) &&
		    (spm_cst_info.entries != spm_cst_info.resume)) {
			uint64_t t_resume =
				(ktime - spm_cst_info.entry_time) / 1000000;
			uint64_t t_spm = spm_wakesta.tr.comm.timer_out >> 5;

			spm_wakesta.tr.comm.resumetime =
				(t_resume > t_spm) ? (t_resume - t_spm) : 0;
			if ((t_resume > t_spm) &&
			    IS_RESUME_OVERTIME(
				    spm_wakesta.tr.comm.resumetime)) {
				INFO("[%s:%d] Overtime use %lu(ms)(%" PRIu64
				     ",%" PRIu64 ")\n",
				     __func__, __LINE__, (t_resume - t_spm),
				     t_resume, t_spm);
			}
			spm_cst_info.resume = spm_cst_info.entries;
		}
	}
#endif

	if (status)
		*status = &spm_wakesta;

#ifndef MTK_PLAT_SPM_TRACE_UNSUPPORT
	if (ext_opand & MT_SPM_EX_OP_TRACE_LP)
		spm_conservation_trace_lp(&spm_wakesta.tr);
	else if (ext_opand & MT_SPM_EX_OP_TRACE_SUSPEND)
		spm_conservation_trace_suspend(&spm_wakesta.tr);
#endif
	__spm_clean_after_wakeup();
	spm_wake_reason = __spm_output_wake_reason(&spm_wakesta);
}

int spm_conservation(int state_id, uint32_t ext_opand,
		     struct spm_lp_scen *spm_lp, uint32_t resource_req)
{
	uint32_t rc_state = resource_req;

	if (!spm_lp)
		return -1;

	spm_lock_get();

	if (ext_opand & MT_SPM_EX_OP_DEVICES_SAVE) {
#ifndef MTK_PLAT_SPM_UART_UNSUPPORT

		/* Notify UART to sleep */
		mt_uart_save();
#else
		INFO("[%s:%d] - uart not support will crash when infra off\n",
		     __func__, __LINE__);
#endif
	}

	if (!(ext_opand & MT_SPM_EX_OP_NON_GENERIC_RESOURCE_REQ)) {
		/* resource request */
		mt_lp_rq_get_status(PLAT_RQ_REQ_USAGE,
				    &generic_spm_resource_req);
		rc_state |= generic_spm_resource_req.val;
	}

	if (ext_opand & MT_SPM_EX_OP_DISABLE_VCORE_LP)
		spm_lp->pwrctrl->pcm_flags1 |= SPM_FLAG1_DISABLE_VCORE_LP;
	else
		spm_lp->pwrctrl->pcm_flags1 &= ~SPM_FLAG1_DISABLE_VCORE_LP;

	go_to_spm_before_wfi(state_id, ext_opand, spm_lp, rc_state);

	spm_lock_release();

	return 0;
}

void spm_conservation_finish(int state_id, uint32_t ext_opand,
			     struct spm_lp_scen *spm_lp,
			     struct wake_status **status)
{
	if (ext_opand & MT_SPM_EX_OP_DEVICES_SAVE) {
#ifndef MTK_PLAT_SPM_UART_UNSUPPORT
		/* Notify UART to wakeup */
		mt_uart_restore();
#else
		INFO("[%s:%d] - uart not support will crash when infra off\n",
		     __func__, __LINE__);
#endif
	}

	spm_lock_get();
	go_to_spm_after_wfi(state_id, ext_opand, spm_lp, status);
	spm_lock_release();
}

int spm_conservation_get_result(struct wake_status **res)
{
	if (!res)
		return -1;
	*res = &spm_wakesta;
	return 0;
}
