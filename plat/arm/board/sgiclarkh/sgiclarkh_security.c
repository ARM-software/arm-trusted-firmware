/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/arm/tzc_dmc620.h>

uintptr_t sgiclarkh_dmc_base[] = {
	SGICLARKH_DMC620_BASE0,
	SGICLARKH_DMC620_BASE1
};

static const tzc_dmc620_driver_data_t sgiclarkh_plat_driver_data = {
	.dmc_base = sgiclarkh_dmc_base,
	.dmc_count = ARRAY_SIZE(sgiclarkh_dmc_base)
};

static const tzc_dmc620_acc_addr_data_t sgiclarkh_acc_addr_data[] = {
	{
		.region_base = ARM_AP_TZC_DRAM1_BASE,
		.region_top = ARM_AP_TZC_DRAM1_BASE + ARM_TZC_DRAM1_SIZE - 1,
		.sec_attr = TZC_DMC620_REGION_S_RDWR
	}
};

static const tzc_dmc620_config_data_t sgiclarkh_plat_config_data = {
	.plat_drv_data = &sgiclarkh_plat_driver_data,
	.plat_acc_addr_data = sgiclarkh_acc_addr_data,
	.acc_addr_count = ARRAY_SIZE(sgiclarkh_acc_addr_data)
};

/* Initialize the secure environment */
void plat_arm_security_setup(void)
{
	arm_tzc_dmc620_setup(&sgiclarkh_plat_config_data);
}
