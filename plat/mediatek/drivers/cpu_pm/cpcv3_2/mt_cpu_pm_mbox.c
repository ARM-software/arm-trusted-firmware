/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <lib/mmio.h>

#include "mt_cpu_pm_mbox.h"
#include <platform_def.h>

#ifdef __GNUC__
#define MCDI_LIKELY(x)		__builtin_expect(!!(x), 1)
#define MCDI_UNLIKELY(x)	__builtin_expect(!!(x), 0)
#else
#define MCDI_LIKELY(x)		(x)
#define MCDI_UNLIKELY(x)	(x)
#endif

#define MCUPM_MBOX_3_BASE		(CPU_EB_TCM_BASE + CPU_EB_MBOX3_OFFSET)
#define MCUPM_MBOX_WRITE(id, val)	mmio_write_32(MCUPM_MBOX_3_BASE + 4 * (id), val)
#define MCUPM_MBOX_READ(id)		mmio_read_32(MCUPM_MBOX_3_BASE + 4 * (id))

void mtk_set_mcupm_pll_mode(unsigned int mode)
{
	if (mode < NF_MCUPM_ARMPLL_MODE) {
		MCUPM_MBOX_WRITE(MCUPM_MBOX_ARMPLL_MODE, mode);
	}
}

int mtk_get_mcupm_pll_mode(void)
{
	return MCUPM_MBOX_READ(MCUPM_MBOX_ARMPLL_MODE);
}

void mtk_set_mcupm_buck_mode(unsigned int mode)
{
	if (mode < NF_MCUPM_BUCK_MODE) {
		MCUPM_MBOX_WRITE(MCUPM_MBOX_BUCK_MODE, mode);
	}
}

int mtk_get_mcupm_buck_mode(void)
{
	return MCUPM_MBOX_READ(MCUPM_MBOX_BUCK_MODE);
}

void mtk_set_cpu_pm_preffered_cpu(unsigned int cpuid)
{
	return MCUPM_MBOX_WRITE(MCUPM_MBOX_WAKEUP_CPU, cpuid);
}

unsigned int mtk_get_cpu_pm_preffered_cpu(void)
{
	return MCUPM_MBOX_READ(MCUPM_MBOX_WAKEUP_CPU);
}

static int mtk_wait_mbox_init_done(void)
{
	int status = MCUPM_MBOX_READ(MCUPM_MBOX_TASK_STA);

	if (status != MCUPM_TASK_INIT) {
		return status;
	}

	mtk_set_mcupm_pll_mode(MCUPM_ARMPLL_OFF);
	mtk_set_mcupm_buck_mode(MCUPM_BUCK_OFF_MODE);

	MCUPM_MBOX_WRITE(MCUPM_MBOX_PWR_CTRL_EN, (MCUPM_MCUSYS_CTRL | MCUPM_CM_CTRL |
						 MCUPM_BUCK_CTRL | MCUPM_ARMPLL_CTRL));

	return status;
}

int mtk_lp_depd_condition(enum cpupm_mbox_depd_type type)
{
	int status;

	if (type == CPUPM_MBOX_WAIT_DEV_INIT) {
		status = mtk_wait_mbox_init_done();
		if (MCDI_UNLIKELY(status != MCUPM_TASK_INIT)) {
			return -ENXIO;
		}
		MCUPM_MBOX_WRITE(MCUPM_MBOX_AP_READY, 1);
	} else if (type == CPUPM_MBOX_WAIT_TASK_READY) {
		status = MCUPM_MBOX_READ(MCUPM_MBOX_TASK_STA);
		if (MCDI_UNLIKELY((status != MCUPM_TASK_WAIT) &&
				  (status != MCUPM_TASK_INIT_FINISH))) {
			return -ENXIO;
		}
	}
	return 0;
}
