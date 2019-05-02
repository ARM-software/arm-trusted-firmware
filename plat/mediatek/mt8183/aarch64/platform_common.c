/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <mcsi/mcsi.h>
#include <platform_def.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables.h>

static const int cci_map[] = {
	PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX,
	PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX
};

/* Table of regions to map using the MMU.  */
const mmap_region_t plat_mmap[] = {
	/* for TF text, RO, RW */
	MAP_REGION_FLAT(TZRAM_BASE, TZRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MTK_DEV_RNG0_BASE, MTK_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MTK_DEV_RNG1_BASE, MTK_DEV_RNG1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MTK_DEV_RNG2_BASE, MTK_DEV_RNG2_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{ 0 }
};

/*******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 ******************************************************************************/
void plat_configure_mmu_el3(uintptr_t total_base,
			    uintptr_t total_size,
			    uintptr_t ro_start,
			    uintptr_t ro_limit,
			    uintptr_t coh_start,
			    uintptr_t coh_limit)
{
	mmap_add_region(total_base, total_base, total_size,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_start, ro_limit - ro_start,
			MT_MEMORY | MT_RO | MT_SECURE);
	mmap_add_region(coh_start, coh_start, coh_limit - coh_start,
			MT_DEVICE | MT_RW | MT_SECURE);
	mmap_add(plat_mmap);
	init_xlat_tables();
	enable_mmu_el3(0);
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}

void plat_mtk_cci_init(void)
{
	/* Initialize CCI driver */
	mcsi_init(PLAT_MT_CCI_BASE, ARRAY_SIZE(cci_map));
}

void plat_mtk_cci_enable(void)
{
	/* Enable CCI coherency for this cluster.
	 * No need for locks as no other cpu is active at the moment.
	 */
	cci_enable_cluster_coherency(read_mpidr());
}

void plat_mtk_cci_disable(void)
{
	cci_disable_cluster_coherency(read_mpidr());
}

void plat_mtk_cci_init_sf(void)
{
	/* Init mcsi snoop filter. */
	cci_init_sf();
}
