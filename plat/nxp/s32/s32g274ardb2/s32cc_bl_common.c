/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>

#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <s32cc-bl-common.h>

int s32cc_bl_mmu_setup(void)
{
	const unsigned long code_start = BL_CODE_BASE;
	const unsigned long rw_start = BL_CODE_END;
	unsigned long code_size;
	unsigned long rw_size;

	if (code_start > BL_CODE_END) {
		return -EINVAL;
	}

	if (rw_start > BL_END) {
		return -EINVAL;
	}

	code_size = BL_CODE_END - code_start;
	rw_size = BL_END - rw_start;

	mmap_add_region(code_start, code_start, code_size,
			MT_RO | MT_MEMORY | MT_SECURE);
	mmap_add_region(rw_start, rw_start, rw_size,
			MT_RW | MT_MEMORY | MT_SECURE);

	init_xlat_tables();
	enable_mmu_el3(0);

	return 0;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}
