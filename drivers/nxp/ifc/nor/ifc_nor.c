/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>

#include <assert.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <plat_common.h>

int nor_init(void)
{
	/* TBD Add FTIM initializations */
	return 0;
}

int ifc_nor_io_setup(void)
{
	nor_init();

	/* Adding NOR Memory Map in XLAT Table */
	mmap_add_region(NXP_NOR_FLASH_ADDR, NXP_NOR_FLASH_ADDR,
			NXP_NOR_FLASH_SIZE, MT_DEVICE | MT_RW);

	return plat_io_memmap_setup(NXP_NOR_FLASH_ADDR + PLAT_FIP_OFFSET);
}
