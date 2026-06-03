/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <fspi_api.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

int flexspi_nor_io_setup(uintptr_t nxp_flexspi_flash_addr,
			 size_t nxp_flexspi_flash_size, uint32_t fspi_base_reg_addr)
{
	int ret = 0;

#if BL2_IN_XIP_MEM
	/*
	 * XIP BL2 is fetching instructions from the FlexSPI NOR AHB
	 * window. fspi_init()'s SWRST + MDIS=1 to FSPI_MCR0 tears down
	 * the AHB path; the next insn fetch faults synchronously. The
	 * Service Processor already initialised the controller during
	 * its RCW+PBI+BL2 load, so re-init here is unnecessary. Only
	 * the mmap_add_region below is required for subsequent FIP-via-
	 * memmap reads to hit a translated, RW-mapped region.
	 */
	(void)fspi_base_reg_addr;
#else
	ret = fspi_init(fspi_base_reg_addr, nxp_flexspi_flash_addr);
#endif
	/* Adding NOR Memory Map in XLAT Table */
	mmap_add_region(nxp_flexspi_flash_addr, nxp_flexspi_flash_addr,
			nxp_flexspi_flash_size, MT_MEMORY | MT_RW);

	return ret;
}
