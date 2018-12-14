/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/utils_def.h>

#include "hikey_private.h"

#define PORTNUM_MAX		5

#define MDDRC_SECURITY_BASE	0xF7121000

struct int_en_reg {
	unsigned in_en:1;
	unsigned reserved:31;
};

struct rgn_map_reg {
	unsigned rgn_base_addr:24;
	unsigned rgn_size:6;
	unsigned reserved:1;
	unsigned rgn_en:1;
};

struct rgn_attr_reg {
	unsigned sp:4;
	unsigned security_inv:1;
	unsigned reserved_0:3;
	unsigned mid_en:1;
	unsigned mid_inv:1;
	unsigned reserved_1:6;
	unsigned rgn_en:1;
	unsigned subrgn_disable:16;
};

static volatile struct int_en_reg *get_int_en_reg(uint32_t base)
{
	uint64_t addr = base + 0x20;
	return (struct int_en_reg *)addr;
}

static volatile struct rgn_map_reg *get_rgn_map_reg(uint32_t base, int region, int port)
{
	uint64_t addr = base + 0x100 + 0x10 * region + 0x400 * (uint64_t)port;
	return (struct rgn_map_reg *)addr;
}

static volatile struct rgn_attr_reg *get_rgn_attr_reg(uint32_t base, int region,
					     int port)
{
	uint64_t addr = base + 0x104 + 0x10 * region + 0x400 * (uint64_t)port;
	return (struct rgn_attr_reg *)addr;
}

/*
 * Configure secure memory region
 * region_size must be a power of 2 and at least 64KB
 * region_base must be region_size aligned
 */
static void sec_protect(uint32_t region_base, uint32_t region_size,
			int region)
{
	volatile struct int_en_reg *int_en;
	volatile struct rgn_map_reg *rgn_map;
	volatile struct rgn_attr_reg *rgn_attr;
	uint32_t i = 0;

	/* ensure secure region number is between 1-15 */
	assert(region > 0 && region < 16);
	/* ensure secure region size is a power of 2 >= 64KB */
	assert(IS_POWER_OF_TWO(region_size) && region_size >= 0x10000);
	/* ensure secure region address is aligned to region size */
	assert(!(region_base & (region_size - 1)));

	INFO("BL2: TrustZone: protecting %u bytes of memory at 0x%x\n", region_size,
	     region_base);

	int_en = get_int_en_reg(MDDRC_SECURITY_BASE);
	int_en->in_en = 0x1;

	for (i = 0; i < PORTNUM_MAX; i++) {
		rgn_map = get_rgn_map_reg(MDDRC_SECURITY_BASE, region, i);
		rgn_attr = get_rgn_attr_reg(MDDRC_SECURITY_BASE, region, i);
		rgn_map->rgn_base_addr = region_base >> 16;
		rgn_attr->subrgn_disable = 0x0;
		rgn_attr->sp = (i == 3) ? 0xC : 0x0;
		rgn_map->rgn_size = __builtin_ffs(region_size) - 2;
		rgn_map->rgn_en = 0x1;
	}
}

/*******************************************************************************
 * Initialize the secure environment.
 ******************************************************************************/
void hikey_security_setup(void)
{
	sec_protect(DDR_SEC_BASE, DDR_SEC_SIZE, 1);
	sec_protect(DDR_SDP_BASE, DDR_SDP_SIZE, 2);
}
