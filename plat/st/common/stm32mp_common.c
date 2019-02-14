/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <plat/common/platform.h>

uintptr_t plat_get_ns_image_entrypoint(void)
{
	return BL33_BASE;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return read_cntfrq_el0();
}

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
	if (bank == GPIO_BANK_Z) {
		return GPIOZ_BASE;
	}

	assert(GPIO_BANK_A == 0 && bank <= GPIO_BANK_K);

	return GPIOA_BASE + (bank * GPIO_BANK_OFFSET);
}

unsigned long stm32_get_gpio_bank_clock(unsigned int bank)
{
	if (bank == GPIO_BANK_Z) {
		return GPIOZ;
	}

	assert(GPIO_BANK_A == 0 && bank <= GPIO_BANK_K);

	return GPIOA + (bank - GPIO_BANK_A);
}

uint32_t stm32_get_gpio_bank_offset(unsigned int bank)
{
	if (bank == GPIO_BANK_Z) {
		return 0;
	}

	assert(GPIO_BANK_A == 0 && bank <= GPIO_BANK_K);

	return bank * GPIO_BANK_OFFSET;
}
