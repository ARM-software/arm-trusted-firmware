/*
 * Copyright (c) 2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lib/mmio.h>

#include "s10_clock_manager.h"
#include "socfpga_system_manager.h"

void s10_mmc_init(void)
{
	mmio_clrbits_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_EN,
		ALT_CLKMGR_PERPLL_EN_SDMMCCLK);
	mmio_write_32(SOCFPGA_SYSMGR(SDMMC),
		SYSMGR_SDMMC_SMPLSEL(2) | SYSMGR_SDMMC_DRVSEL(3));
	mmio_setbits_32(ALT_CLKMGR_PERPLL + ALT_CLKMGR_PERPLL_EN,
		ALT_CLKMGR_PERPLL_EN_SDMMCCLK);
}
