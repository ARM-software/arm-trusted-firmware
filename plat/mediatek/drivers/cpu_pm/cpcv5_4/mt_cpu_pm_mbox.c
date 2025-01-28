/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include <mcupm_cfg.h>
#include "mt_cpu_pm_mbox.h"

#ifdef __GNUC__
#define mcdi_likely(x)		__builtin_expect(!!(x), 1)
#define mcdi_unlikely(x)	__builtin_expect(!!(x), 0)
#else
#define mcdi_likely(x)		(x)
#define mcdi_unlikely(x)	(x)
#endif /* __GNUC__ */

#define MCUPM_MBOX_3_BASE	(CPU_EB_TCM_BASE + CPU_EB_MBOX3_OFFSET)

#define _mcupm_mbox_write(id, val) \
	mmio_write_32(MCUPM_MBOX_3_BASE + 4 * (id), val)
#define _mcupm_mbox_read(id) \
	mmio_read_32(MCUPM_MBOX_3_BASE + 4 * (id))

void mtk_set_mcupm_pll_mode(unsigned int mode)
{
	if (mode < NF_MCUPM_ARMPLL_MODE)
		_mcupm_mbox_write(MCUPM_MBOX_ARMPLL_MODE, mode);
}

int mtk_get_mcupm_pll_mode(void)
{
	return _mcupm_mbox_read(MCUPM_MBOX_ARMPLL_MODE);
}

void mtk_set_mcupm_buck_mode(unsigned int mode)
{
	if (mode < NF_MCUPM_BUCK_MODE)
		_mcupm_mbox_write(MCUPM_MBOX_BUCK_MODE, mode);
}

int mtk_get_mcupm_buck_mode(void)
{
	return _mcupm_mbox_read(MCUPM_MBOX_BUCK_MODE);
}

void mtk_set_cpu_pm_preffered_cpu(unsigned int cpuid)
{
	return _mcupm_mbox_write(MCUPM_MBOX_WAKEUP_CPU, cpuid);
}

unsigned int mtk_get_cpu_pm_preffered_cpu(void)
{
	return _mcupm_mbox_read(MCUPM_MBOX_WAKEUP_CPU);
}

static int mtk_wait_mbox_init_done(void)
{
	int sta = _mcupm_mbox_read(MCUPM_MBOX_TASK_STA);

	if (sta != MCUPM_TASK_INIT)
		return sta;

	mtk_set_mcupm_pll_mode(MCUPM_ARMPLL_OFF);
	mtk_set_mcupm_buck_mode(MCUPM_BUCK_OFF_MODE);

	_mcupm_mbox_write(MCUPM_MBOX_PWR_CTRL_EN,
			  MCUPM_MCUSYS_CTRL |
			  MCUPM_CM_CTRL |
			  MCUPM_BUCK_CTRL |
			  MCUPM_ARMPLL_CTRL);

	return sta;
}

int mtk_lp_depd_condition(enum cpupm_mbox_depd_type type)
{
	int ret = 0, status = 0;

	if (type == CPUPM_MBOX_WAIT_DEV_INIT) {
		status = mtk_wait_mbox_init_done();
		if (mcdi_unlikely(status != MCUPM_TASK_INIT))
			ret = -ENXIO;
		else
			_mcupm_mbox_write(MCUPM_MBOX_AP_READY, 1);
	} else if (type == CPUPM_MBOX_WAIT_TASK_READY) {
		status = _mcupm_mbox_read(MCUPM_MBOX_TASK_STA);
		if (mcdi_unlikely((status != MCUPM_TASK_WAIT) &&
				  (status != MCUPM_TASK_INIT_FINISH)))
			ret = -ENXIO;
	}
	return ret;
}

void mtk_set_mcupm_group_hint(unsigned int gmask)
{
	_mcupm_mbox_write(MCUPM_MBOX_GROUP, gmask);
}
