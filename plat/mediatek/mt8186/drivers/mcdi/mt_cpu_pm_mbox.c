/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <mt_cpu_pm_mbox.h>
#include <platform_def.h>
#include <sspm_reg.h>

#define MCUPM_MBOX_3_BASE	(MTK_MCUPM_SRAM_BASE + 0xFCE0)

#define _sspm_mbox_write(id, val) \
		mmio_write_32(SSPM_MBOX_3_BASE + 4 * (id), val)
#define _sspm_mbox_read(id) \
		mmio_read_32(SSPM_MBOX_3_BASE + 4 * (id))

#define _mcupm_mbox_write(id, val) \
		mmio_write_32(MCUPM_MBOX_3_BASE + 4 * (id), val)
#define _mcupm_mbox_read(id) \
		mmio_read_32(MCUPM_MBOX_3_BASE + 4 * (id))


#define MCUPM_MBOX_OFFSET_PDN		(0x0C55FDA8)
#define MCUPM_POWER_DOWN		(0x4D50444E)

void mtk_set_sspm_lp_cmd(void *buf, unsigned int size)
{
	unsigned int *p = (unsigned int *)buf;
	int i;

	for (i = 0; i < size; i++) {
		_sspm_mbox_write(SSPM_MBOX_SPM_CMD + i, p[i]);
	}
}

void mtk_clr_sspm_lp_cmd(unsigned int size)
{
	int i;

	for (i = 0; i < size; i++) {
		_sspm_mbox_write(SSPM_MBOX_SPM_CMD + i, 0);
	}
}

void mtk_set_cpu_pm_pll_mode(unsigned int mode)
{
	if (mode < NF_MCUPM_ARMPLL_MODE) {
		_mcupm_mbox_write(MCUPM_MBOX_ARMPLL_MODE, mode);
	}
}

int mtk_get_cpu_pm_pll_mode(void)
{
	return _mcupm_mbox_read(MCUPM_MBOX_ARMPLL_MODE);
}

void mtk_set_cpu_pm_buck_mode(unsigned int mode)
{
	if (mode < NF_MCUPM_BUCK_MODE) {
		_mcupm_mbox_write(MCUPM_MBOX_BUCK_MODE, mode);
	}
}

int mtk_get_cpu_pm_buck_mode(void)
{
	return _mcupm_mbox_read(MCUPM_MBOX_BUCK_MODE);
}

void mtk_set_cpu_pm_preffered_cpu(unsigned int cpuid)
{
	return _mcupm_mbox_read(MCUPM_MBOX_WAKEUP_CPU);
}

int mtk_set_cpu_pm_mbox_addr(uint64_t phy_addr)
{
	return 0;
}
