/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/sbsa.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	RD1AE_MAP_DEVICE,
	RD1AE_EXTERNAL_FLASH,
	SOC_PLATFORM_PERIPH_MAP_DEVICE,
#if IMAGE_BL2
	RD1AE_MAP_NS_DRAM1,
#endif
	RD1AE_MAP_S_DRAM1,
	{0}
};

void plat_arm_secure_wdt_start(void)
{
	sbsa_wdog_start(SBSA_SECURE_WDOG_BASE, SBSA_SECURE_WDOG_TIMEOUT);
}

void plat_arm_secure_wdt_stop(void)
{
	sbsa_wdog_stop(SBSA_SECURE_WDOG_BASE);
}

/*
 * For rd1ae we should not do anything in these interface functions.
 * They are used to override the weak functions in cci drivers
 */
void plat_arm_interconnect_init(void)
{
}

void plat_arm_interconnect_enter_coherency(void)
{
}

void plat_arm_interconnect_exit_coherency(void)
{
}

/*
 * TZC programming is currently not done.
 */
void plat_arm_security_setup(void)
{
}
