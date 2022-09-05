/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <plat/common/platform.h>
#include <lib/pm/mtk_pm.h>

#define MTK_PM_ST_SMP_READY	BIT(0)
#define MTK_PM_ST_PWR_READY	BIT(1)
#define MTK_PM_ST_RESET_READY	BIT(2)

static uintptr_t mtk_secure_entrypoint;
static plat_init_func mtk_plat_smp_init;
static plat_psci_ops_t mtk_pm_ops;
static unsigned int mtk_pm_status;

uintptr_t plat_pm_get_warm_entry(void)
{
	return mtk_secure_entrypoint;
}

int plat_pm_ops_setup_pwr(struct plat_pm_pwr_ctrl *ops)
{
	if (!ops) {
		return MTK_CPUPM_E_FAIL;
	}

#if CONFIG_MTK_CPU_SUSPEND_EN
	if (!mtk_pm_ops.pwr_domain_suspend) {
		mtk_pm_ops.pwr_domain_suspend = ops->pwr_domain_suspend;
	}

	if (!mtk_pm_ops.pwr_domain_suspend_finish) {
		mtk_pm_ops.pwr_domain_suspend_finish = ops->pwr_domain_suspend_finish;
	}

	if (!mtk_pm_ops.validate_power_state) {
		mtk_pm_ops.validate_power_state = ops->validate_power_state;
	}

	if (!mtk_pm_ops.get_sys_suspend_power_state) {
		mtk_pm_ops.get_sys_suspend_power_state = ops->get_sys_suspend_power_state;
	}

	mtk_pm_status |= MTK_PM_ST_PWR_READY;
#endif
	return MTK_CPUPM_E_OK;
}

int plat_pm_ops_setup_smp(struct plat_pm_smp_ctrl *ops)
{
	if (!ops) {
		return MTK_CPUPM_E_FAIL;
	}

#if CONFIG_MTK_SMP_EN
	if (!mtk_pm_ops.pwr_domain_on) {
		mtk_pm_ops.pwr_domain_on = ops->pwr_domain_on;
	}

	if (!mtk_pm_ops.pwr_domain_on_finish) {
		mtk_pm_ops.pwr_domain_on_finish = ops->pwr_domain_on_finish;
	}

	if (!mtk_pm_ops.pwr_domain_off) {
		mtk_pm_ops.pwr_domain_off = ops->pwr_domain_off;
	}

	if (!mtk_plat_smp_init) {
		mtk_plat_smp_init = ops->init;
	}

	mtk_pm_status |= MTK_PM_ST_SMP_READY;
#endif
	return MTK_CPUPM_E_OK;
}

int plat_pm_ops_setup_reset(struct plat_pm_reset_ctrl *ops)
{
	if (!ops) {
		return MTK_CPUPM_E_FAIL;
	}

	if (!mtk_pm_ops.system_off) {
		mtk_pm_ops.system_off = ops->system_off;
	}

	if (!mtk_pm_ops.system_reset) {
		mtk_pm_ops.system_reset = ops->system_reset;
	}

	if (!mtk_pm_ops.system_reset2) {
		mtk_pm_ops.system_reset2 = ops->system_reset2;
	}

	mtk_pm_status |= MTK_PM_ST_RESET_READY;

	return MTK_CPUPM_E_OK;
}

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &mtk_pm_ops;
	mtk_secure_entrypoint = sec_entrypoint;

	if (mtk_plat_smp_init) {
		unsigned int cpu_id = plat_my_core_pos();

		mtk_plat_smp_init(cpu_id, mtk_secure_entrypoint);
	}
	INFO("%s, smp:(%d), pwr_ctrl:(%d), system_reset:(%d)\n", __func__,
	     !!(mtk_pm_status & MTK_PM_ST_SMP_READY),
	     !!(mtk_pm_status & MTK_PM_ST_PWR_READY),
	     !!(mtk_pm_status & MTK_PM_ST_RESET_READY));
	return 0;
}
