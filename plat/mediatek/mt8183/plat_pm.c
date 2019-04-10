/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* common headers */
#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <errno.h>

/* mediatek platform specific headers */
#include <platform_def.h>
#include <scu.h>
#include <mtk_plat_common.h>
#include <power_tracer.h>
#include <plat_private.h>

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static uintptr_t secure_entrypoint;

static const plat_psci_ops_t plat_plat_pm_ops = {
	.cpu_standby			= NULL,
	.pwr_domain_on			= NULL,
	.pwr_domain_on_finish		= NULL,
	.pwr_domain_off			= NULL,
	.pwr_domain_suspend		= NULL,
	.pwr_domain_suspend_finish	= NULL,
	.system_off			= NULL,
	.system_reset			= NULL,
	.validate_power_state		= NULL,
	.get_sys_suspend_power_state	= NULL,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_plat_pm_ops;
	secure_entrypoint = sec_entrypoint;
	return 0;
}
