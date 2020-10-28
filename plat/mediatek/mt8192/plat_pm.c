/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* common headers */
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/gpio.h>
#include <lib/psci/psci.h>

/* mediatek platform specific headers */
#include <plat_params.h>

/*******************************************************************************
 * MTK handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_mtk_system_reset(void)
{
	struct bl_aux_gpio_info *gpio_reset = plat_get_mtk_gpio_reset();

	INFO("MTK System Reset\n");

	gpio_set_value(gpio_reset->index, gpio_reset->polarity);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static const plat_psci_ops_t plat_plat_pm_ops = {
	.system_reset         = plat_mtk_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &plat_plat_pm_ops;

	return 0;
}
