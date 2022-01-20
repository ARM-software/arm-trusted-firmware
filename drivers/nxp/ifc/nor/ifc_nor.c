/*
 * Copyright 2020-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <stdint.h>
#include <stdlib.h>

#include <lib/xlat_tables/xlat_tables_v2.h>

int ifc_nor_init(uintptr_t flash_addr, size_t flash_size)
{
	/* Adding NOR Memory Map in XLAT Table */
	mmap_add_region(flash_addr, flash_addr, flash_size, MT_MEMORY | MT_RW);

	return 0;
}
