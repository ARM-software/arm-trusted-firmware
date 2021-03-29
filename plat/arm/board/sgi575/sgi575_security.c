/*
 * Copyright (c) 2018-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <sgi_dmc620_tzc_regions.h>

uintptr_t sgi575_dmc_base[] = {
	SGI575_DMC620_BASE0,
	SGI575_DMC620_BASE1
};

static const tzc_dmc620_driver_data_t sgi575_plat_driver_data = {
	.dmc_base = sgi575_dmc_base,
	.dmc_count = ARRAY_SIZE(sgi575_dmc_base)
};

static const tzc_dmc620_acc_addr_data_t sgi575_acc_addr_data[] = {
	CSS_SGI_DMC620_TZC_REGIONS_DEF
};

static const tzc_dmc620_config_data_t sgi575_plat_config_data = {
	.plat_drv_data = &sgi575_plat_driver_data,
	.plat_acc_addr_data = sgi575_acc_addr_data,
	.acc_addr_count = ARRAY_SIZE(sgi575_acc_addr_data)
};

/* Initialize the secure environment */
void plat_arm_security_setup(void)
{
	arm_tzc_dmc620_setup(&sgi575_plat_config_data);
}
