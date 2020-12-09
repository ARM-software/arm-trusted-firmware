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

	ret = fspi_init(fspi_base_reg_addr, nxp_flexspi_flash_addr);
	/* Adding NOR Memory Map in XLAT Table */
	mmap_add_region(nxp_flexspi_flash_addr, nxp_flexspi_flash_addr,
			nxp_flexspi_flash_size, MT_MEMORY | MT_RW);

	return ret;
}
