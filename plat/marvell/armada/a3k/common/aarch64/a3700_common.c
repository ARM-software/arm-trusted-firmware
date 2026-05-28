/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */
#include <plat_marvell.h>

/* MMU entry for internal (register) space access */
#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

/*
 * Table of regions for various BL stages to map using the MMU.
 */
#ifdef IMAGE_BL1
const mmap_region_t plat_marvell_mmap[] = {
	MARVELL_MAP_SHARED_RAM,
	MAP_DEVICE0,
	{0}
};
#endif
#ifdef IMAGE_BL2
const mmap_region_t plat_marvell_mmap[] = {
	MARVELL_MAP_SHARED_RAM,
	MAP_DEVICE0,
	MARVELL_MAP_DRAM,
	{0}
};
#endif
#ifdef IMAGE_BL2U
const mmap_region_t plat_marvell_mmap[] = {
	MAP_DEVICE0,
	{0}
};
#endif
#ifdef IMAGE_BL31
const mmap_region_t plat_marvell_mmap[] = {
	MARVELL_MAP_SHARED_RAM,
	MAP_DEVICE0,
	MARVELL_MAP_DRAM,
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_marvell_mmap[] = {
	MAP_DEVICE0,
	{0}
};
#endif

MARVELL_CASSERT_MMAP;
