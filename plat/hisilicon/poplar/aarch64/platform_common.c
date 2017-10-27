/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <platform.h>
#include <xlat_tables.h>
#include "hi3798cv200.h"
#include "platform_def.h"

#define MAP_DDR		MAP_REGION_FLAT(DDR_BASE,			\
					DDR_SIZE,			\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_DEVICE	MAP_REGION_FLAT(DEVICE_BASE,			\
					DEVICE_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_TSP_MEM	MAP_REGION_FLAT(TSP_SEC_MEM_BASE,		\
					TSP_SEC_MEM_SIZE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

static const mmap_region_t poplar_mmap[] = {
	MAP_DDR,
	MAP_DEVICE,
	MAP_TSP_MEM,
	{0}
};

#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void plat_configure_mmu_el##_el(unsigned long total_base,	\
				  unsigned long total_size,		\
				  unsigned long ro_start,		\
				  unsigned long ro_limit,		\
				  unsigned long coh_start,		\
				  unsigned long coh_limit)		\
	{								\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(ro_start, ro_start,			\
				ro_limit - ro_start,			\
				MT_MEMORY | MT_RO | MT_SECURE);		\
		mmap_add_region(coh_start, coh_start,			\
				coh_limit - coh_start,			\
				MT_DEVICE | MT_RW | MT_SECURE);		\
		mmap_add(poplar_mmap);					\
		init_xlat_tables();					\
									\
		enable_mmu_el##_el(0);					\
	}

DEFINE_CONFIGURE_MMU_EL(3)
DEFINE_CONFIGURE_MMU_EL(1)

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_TICKS;
}
