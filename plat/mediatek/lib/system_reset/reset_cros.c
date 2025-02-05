/*
 * Copyright (c) 2022-2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio.h>
#if CONFIG_MTK_PMIC_SHUTDOWN_CFG
#include <drivers/pmic/pmic_psc.h>
#endif
#include <lib/mtk_init/mtk_init.h>
#include <lib/pm/mtk_pm.h>
#include <plat_params.h>
#if !CONFIG_MTK_PMIC_SHUTDOWN_CFG
#include <pmic.h>
#include <rtc.h>
#endif

static void __dead2 mtk_system_reset_cros(void)
{
	struct bl_aux_gpio_info *gpio_reset = plat_get_mtk_gpio_reset();

	INFO("MTK System Reset\n");

	gpio_set_value(gpio_reset->index, gpio_reset->polarity);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

static void __dead2 mtk_system_off_cros(void)
{
	INFO("MTK System Off\n");

#if CONFIG_MTK_PMIC_SHUTDOWN_CFG
	platform_power_hold(false);
	mdelay(1000);
#else
	rtc_power_off_sequence();
	pmic_power_off();
#endif

	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static struct plat_pm_reset_ctrl lib_reset_ctrl = {
	.system_off = mtk_system_off_cros,
	.system_reset = mtk_system_reset_cros,
	.system_reset2 = NULL,
};

static int lib_reset_ctrl_init(void)
{
	INFO("Reset init\n");

	plat_pm_ops_setup_reset(&lib_reset_ctrl);

	return 0;
}
MTK_ARCH_INIT(lib_reset_ctrl_init);
