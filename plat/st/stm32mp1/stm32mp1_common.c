/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <gicv2.h>
#include <mmio.h>
#include <platform_def.h>
#include <platform.h>
#include <stm32mp1_private.h>
#include <xlat_tables_v2.h>

#define MAP_SRAM	MAP_REGION_FLAT(STM32MP1_SRAM_BASE, \
					STM32MP1_SRAM_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_DEVICE1	MAP_REGION_FLAT(STM32MP1_DEVICE1_BASE, \
					STM32MP1_DEVICE1_SIZE, \
					MT_DEVICE | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_DEVICE2	MAP_REGION_FLAT(STM32MP1_DEVICE2_BASE, \
					STM32MP1_DEVICE2_SIZE, \
					MT_DEVICE | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_DDR		MAP_REGION_FLAT(STM32MP1_DDR_BASE, \
					STM32MP1_DDR_MAX_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_DDR_NS	MAP_REGION_FLAT(STM32MP1_DDR_BASE, \
					STM32MP1_DDR_MAX_SIZE, \
					MT_MEMORY | \
					MT_RW | \
					MT_NS | \
					MT_EXECUTE_NEVER)

#if defined(IMAGE_BL2)
static const mmap_region_t stm32mp1_mmap[] = {
	MAP_SRAM,
	MAP_DEVICE1,
	MAP_DEVICE2,
	MAP_DDR,
	{0}
};
#endif
#if defined(IMAGE_BL32)
static const mmap_region_t stm32mp1_mmap[] = {
	MAP_SRAM,
	MAP_DEVICE1,
	MAP_DEVICE2,
	MAP_DDR_NS,
	{0}
};
#endif

void configure_mmu(void)
{
	mmap_add(stm32mp1_mmap);
	init_xlat_tables();

	enable_mmu_svc_mon(0);
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
	return BL33_BASE;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return read_cntfrq_el0();
}

/* Functions to save and get boot context address given by ROM code */
static uintptr_t boot_ctx_address;

void stm32mp1_save_boot_ctx_address(uintptr_t address)
{
	boot_ctx_address = address;
}

uintptr_t stm32mp1_get_boot_ctx_address(void)
{
	return boot_ctx_address;
}
