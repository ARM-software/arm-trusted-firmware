/*
 * Copyright (c) 2020-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lib/mmio.h>

#include "agilex3_clock_manager.h"
#include "agilex3_system_manager.h"

void agx5_mmc_init(void)
{
// TODO: To update when handoff data is ready

	//mmio_clrbits_32(CLKMGR_PERPLL + CLKMGR_PERPLL_EN,
	//	CLKMGR_PERPLL_EN_SDMMCCLK);
	//mmio_write_32(SOCFPGA_SYSMGR(SDMMC),
	//	SYSMGR_SDMMC_SMPLSEL(0) | SYSMGR_SDMMC_DRVSEL(3));
	//mmio_setbits_32(CLKMGR_PERPLL + CLKMGR_PERPLL_EN,
	//	CLKMGR_PERPLL_EN_SDMMCCLK);

}
