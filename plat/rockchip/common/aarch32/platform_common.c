/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables.h>

#include <plat_private.h>

void plat_configure_mmu_svc_mon(unsigned long total_base,
				unsigned long total_size,
				unsigned long ro_start,
				unsigned long ro_limit,
				unsigned long coh_start,
				unsigned long coh_limit)
{
	mmap_add_region(total_base, total_base, total_size,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_start, ro_limit - ro_start,
			MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add_region(coh_start, coh_start, coh_limit - coh_start,
			MT_DEVICE | MT_RW | MT_SECURE);
	mmap_add(plat_rk_mmap);
	rockchip_plat_mmu_svc_mon();
	init_xlat_tables();
	enable_mmu_svc_mon(0);
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

/*
 * generic pm code does cci handling, but rockchip arm32 platforms
 * have ever only 1 cluster, so nothing to do.
 */
void plat_cci_init(void)
{
}

void plat_cci_enable(void)
{
}

void plat_cci_disable(void)
{
}
