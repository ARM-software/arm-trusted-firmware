/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
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
	RDASPEN_MAP_DEVICE,
	RDASPEN_MAP_EXTERNAL_FLASH,
	RDASPEN_MAP_SMD_REGION,
#if PLAT_ARM_MEM_PROT_ADDR
	RDASPEN_MAP_MEM_PROTECT,
#endif
#if IMAGE_BL2
	RDASPEN_MAP_NS_DRAM1,
	RDASPEN_MAP_NS_DRAM2,
#endif
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

void plat_arm_security_setup(void)
{
}

unsigned int plat_get_syscnt_freq2(void)
{
	/* Returning the Generic Timer Frequency */
	return SYS_COUNTER_FREQ_IN_TICKS;
}
