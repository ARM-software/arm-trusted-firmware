/*
 * Copyright (c) 2021 Sipeed
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <sunxi_mmap.h>
#include <sunxi_cpucfg.h>
#include <sunxi_private.h>

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	/* Currently known hardware has no PMIC */

	return 0;
}

void sunxi_power_down(void)
{
}

void sunxi_cpu_power_off_self(void)
{
	/* TODO: It's still unknown whether CPUIDLE exists on R329 */
}
