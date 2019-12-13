/*
 * Copyright (c) 2015-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>

#include <cmn_plat_util.h>
#include <platform_def.h>

uint32_t boot_source_get(void)
{
	/* For now return BOOT_SOURCE_QSPI */
	return BOOT_SOURCE_QSPI;
}

void __dead2 plat_soft_reset(uint32_t reset)
{
	if (reset == SOFT_RESET_L3) {
		mmio_setbits_32(CRMU_IHOST_SW_PERSISTENT_REG1, reset);
		mmio_write_32(CRMU_MAIL_BOX0, 0x0);
		mmio_write_32(CRMU_MAIL_BOX1, 0xFFFFFFFF);
	}

	if (reset != SOFT_SYS_RESET_L1)
		reset = SOFT_PWR_UP_RESET_L0;

	if (reset == SOFT_PWR_UP_RESET_L0)
		INFO("L0 RESET...\n");

	if (reset == SOFT_SYS_RESET_L1)
		INFO("L1 RESET...\n");

	console_flush();

	mmio_clrbits_32(CRMU_SOFT_RESET_CTRL, 1 << reset);

	while (1) {
		;
	}
}
