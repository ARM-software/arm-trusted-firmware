/*
 * Copyright 2019-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <gpc.h>
#include <imx8m_psci.h>
#include <plat_imx8.h>

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_off = imx_pwr_domain_off,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.cpu_standby = imx_cpu_standby,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = imx_pwr_domain_pwr_down_wfi,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	/* sec_entrypoint is used for warm reset */
	imx_mailbox_init(sec_entrypoint);

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
