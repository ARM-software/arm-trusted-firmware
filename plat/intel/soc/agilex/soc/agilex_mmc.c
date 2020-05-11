/*
 * Copyright (c) 2020, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lib/mmio.h>

#include "socfpga_system_manager.h"
#include "agilex_clock_manager.h"

void agx_mmc_init(void)
{
	mmio_clrbits_32(CLKMGR_PERPLL + CLKMGR_PERPLL_EN,
		CLKMGR_PERPLL_EN_SDMMCCLK);
	mmio_write_32(SOCFPGA_SYSMGR(SDMMC),
		SYSMGR_SDMMC_SMPLSEL(0) | SYSMGR_SDMMC_DRVSEL(3));
	mmio_setbits_32(CLKMGR_PERPLL + CLKMGR_PERPLL_EN,
		CLKMGR_PERPLL_EN_SDMMCCLK);
}
