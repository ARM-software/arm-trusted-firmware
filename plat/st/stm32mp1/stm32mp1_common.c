/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <stm32mp1_private.h>

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

#if defined(IMAGE_BL2)
static const mmap_region_t stm32mp1_mmap[] = {
	MAP_SRAM,
	MAP_DEVICE1,
	MAP_DEVICE2,
	{0}
};
#endif
#if defined(IMAGE_BL32)
static const mmap_region_t stm32mp1_mmap[] = {
	MAP_SRAM,
	MAP_DEVICE1,
	MAP_DEVICE2,
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

uintptr_t stm32_get_gpio_bank_base(unsigned int bank)
{
	switch (bank) {
	case GPIO_BANK_A ... GPIO_BANK_K:
		return GPIOA_BASE + (bank * GPIO_BANK_OFFSET);
	case GPIO_BANK_Z:
		return GPIOZ_BASE;
	default:
		panic();
	}
}

/* Return clock ID on success, negative value on error */
unsigned long stm32_get_gpio_bank_clock(unsigned int bank)
{
	switch (bank) {
	case GPIO_BANK_A ... GPIO_BANK_K:
		return GPIOA + (bank - GPIO_BANK_A);
	case GPIO_BANK_Z:
		return GPIOZ;
	default:
		panic();
	}
}

uint32_t stm32_get_gpio_bank_offset(unsigned int bank)
{
	if (bank == GPIO_BANK_Z) {
		return 0;
	} else {
		return bank * GPIO_BANK_OFFSET;
	}
}
