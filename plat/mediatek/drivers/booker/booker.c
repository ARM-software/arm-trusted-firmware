/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/mmio.h>

#include <drivers/booker.h>
#include <mtk_mmap_pool.h>
#include <platform_booker.h>

static const mmap_region_t booker_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(BOOKER_MAP_REGION, BOOKER_MAP_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};
DECLARE_MTK_MMAP_REGIONS(booker_mmap);

static void booker_mtsx_tc_flush(uintptr_t base)
{
	/* make no new lines be allocated in tag cache */
	mmio_setbits_64(base + MTU_AUX_CTL, BIT(TC_NO_FILL_MODE_BIT));
	dsb();

	/* start flush tag cache with Clean Invalid mode */
	mmio_setbits_64(base + MTU_TC_FLUSH_PR, BIT(TC_FLUSH_ENABLE_BIT));
	dsb();

	/* polling until flush completed */
	while (!(mmio_read_64(base + MTU_TC_FLUSH_SR) & BIT(TC_FLUSH_COMPLETE_BIT)))
		;
}

void booker_flush(void)
{
	unsigned int i;

	/* flush tag cache before power down booker */
	for (i = 0; i < ARRAY_SIZE(booker_mtsx_bases); i++)
		booker_mtsx_tc_flush(booker_mtsx_bases[i]);
}
