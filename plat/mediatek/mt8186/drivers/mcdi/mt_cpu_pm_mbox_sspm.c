/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <mt_cpu_pm.h>
#include <mt_cpu_pm_mbox.h>
#include <platform_def.h>
#include <sspm_reg.h>

#ifdef MCDI_TINYSYS_MBOX_SHARE_SRAM
struct cpu_pm_mbox {
	unsigned int ap_ready;
	unsigned int reserved1;
	unsigned int reserved2;
	unsigned int reserved3;
	unsigned int pwr_ctrl_en;
	unsigned int l3_cache_mode;
	unsigned int buck_mode;
	unsigned int armpll_mode;
	unsigned int task_sta;
	unsigned int reserved9;
	unsigned int reserved10;
	unsigned int reserved11;
	unsigned int wakeup_cpu;
};

struct cpu_pm_mbox *_cpu_pm_box = (struct cpu_pm_mbox *)SSPM_MBOX_3_BASE;
#endif

void mtk_set_cpu_pm_pll_mode(unsigned int mode)
{
#ifdef MCDI_TINYSYS_MBOX_SHARE_SRAM
	if (_cpu_pm_box) {
		_cpu_pm_box->armpll_mode = mode;
	}
#endif
}

int mtk_get_cpu_pm_pll_mode(void)
{
#ifdef MCDI_TINYSYS_MBOX_SHARE_SRAM
	if (!_cpu_pm_box) {
		return 0;
	}
	return _cpu_pm_box->armpll_mode;
#endif
}

void mtk_set_cpu_pm_buck_mode(unsigned int mode)
{
#ifdef MCDI_TINYSYS_MBOX_SHARE_SRAM
	if (_cpu_pm_box) {
		_cpu_pm_box->buck_mode = mode;
	}
#endif
}

int mtk_get_cpu_pm_buck_mode(void)
{
#ifdef MCDI_TINYSYS_MBOX_SHARE_SRAM
	if (!_cpu_pm_box) {
		return 0;
	}
	return _cpu_pm_box->buck_mode;
#endif
}

void mtk_set_cpu_pm_preffered_cpu(unsigned int cpuid)
{
#ifdef MCDI_TINYSYS_MBOX_SHARE_SRAM
	if (_cpu_pm_box) {
		_cpu_pm_box->wakeup_cpu = cpuid;
	}
#endif
}

int mtk_set_cpu_pm_mbox_addr(uint64_t phy_addr)
{
#ifdef MCDI_TINYSYS_MBOX_SHARE_SRAM
	if (_cpu_pm_box || (phy_addr == 0)) {
		return -1;
	}

	_cpu_pm_box = (struct cpu_pm_mbox *)(MTK_SSPM_BASE + phy_addr);
#endif
	return 0;
}
