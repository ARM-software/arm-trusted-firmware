/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdint.h>
#include <std_svc.h>
#include <platform_def.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <imx_sip_svc.h>
#include <sci/sci.h>

#ifdef PLAT_IMX8QM
const static int ap_cluster_index[PLATFORM_CLUSTER_COUNT] = {
	SC_R_A53, SC_R_A72,
};
#endif

static int imx_srtc_set_time(uint32_t year_mon,
			unsigned long day_hour,
			unsigned long min_sec)
{
	return sc_timer_set_rtc_time(ipc_handle,
		year_mon >> 16, year_mon & 0xffff,
		day_hour >> 16, day_hour & 0xffff,
		min_sec >> 16, min_sec & 0xffff);
}

int imx_srtc_handler(uint32_t smc_fid,
		    void *handle,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	int ret;

	switch (x1) {
	case IMX_SIP_SRTC_SET_TIME:
		ret = imx_srtc_set_time(x2, x3, x4);
		break;
	default:
		ret = SMC_UNK;
	}

	SMC_RET1(handle, ret);
}

static void imx_cpufreq_set_target(uint32_t cluster_id, unsigned long freq)
{
	sc_pm_clock_rate_t rate = (sc_pm_clock_rate_t)freq;

#ifdef PLAT_IMX8QM
	sc_pm_set_clock_rate(ipc_handle, ap_cluster_index[cluster_id], SC_PM_CLK_CPU, &rate);
#endif
#ifdef PLAT_IMX8QX
	sc_pm_set_clock_rate(ipc_handle, SC_R_A35, SC_PM_CLK_CPU, &rate);
#endif
}

int imx_cpufreq_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch (x1) {
	case IMX_SIP_SET_CPUFREQ:
		imx_cpufreq_set_target(x2, x3);
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}
