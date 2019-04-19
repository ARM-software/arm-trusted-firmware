/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/st/stm32mp_clkfunc.h>
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

void stm32mp_save_boot_ctx_address(uintptr_t address)
{
	boot_ctx_address = address;
}

uintptr_t stm32mp_get_boot_ctx_address(void)
{
	return boot_ctx_address;
}

uintptr_t stm32mp_ddrctrl_base(void)
{
	static uintptr_t ddrctrl_base;

	if (ddrctrl_base == 0) {
		ddrctrl_base = dt_get_ddrctrl_base();

		assert(ddrctrl_base == DDRCTRL_BASE);
	}

	return ddrctrl_base;
}

uintptr_t stm32mp_ddrphyc_base(void)
{
	static uintptr_t ddrphyc_base;

	if (ddrphyc_base == 0) {
		ddrphyc_base = dt_get_ddrphyc_base();

		assert(ddrphyc_base == DDRPHYC_BASE);
	}

	return ddrphyc_base;
}

uintptr_t stm32mp_pwr_base(void)
{
	static uintptr_t pwr_base;

	if (pwr_base == 0) {
		pwr_base = dt_get_pwr_base();

		assert(pwr_base == PWR_BASE);
	}

	return pwr_base;
}

uintptr_t stm32mp_rcc_base(void)
{
	static uintptr_t rcc_base;

	if (rcc_base == 0) {
		rcc_base = fdt_rcc_read_addr();

		assert(rcc_base == RCC_BASE);
	}

	return rcc_base;
}

bool stm32mp_lock_available(void)
{
	const uint32_t c_m_bits = SCTLR_M_BIT | SCTLR_C_BIT;

	/* The spinlocks are used only when MMU and data cache are enabled */
	return (read_sctlr() & c_m_bits) == c_m_bits;
}

uintptr_t stm32_get_gpio_bank_base(unsigned int bank)
{
	if (bank == GPIO_BANK_Z) {
		return GPIOZ_BASE;
	}

	assert(GPIO_BANK_A == 0 && bank <= GPIO_BANK_K);

	return GPIOA_BASE + (bank * GPIO_BANK_OFFSET);
}

uint32_t stm32_get_gpio_bank_offset(unsigned int bank)
{
	if (bank == GPIO_BANK_Z) {
		return 0;
	}

	assert(GPIO_BANK_A == 0 && bank <= GPIO_BANK_K);

	return bank * GPIO_BANK_OFFSET;
}

int stm32mp_check_header(boot_api_image_header_t *header, uintptr_t buffer)
{
	uint32_t i;
	uint32_t img_checksum = 0U;

	/*
	 * Check header/payload validity:
	 *	- Header magic
	 *	- Header version
	 *	- Payload checksum
	 */
	if (header->magic != BOOT_API_IMAGE_HEADER_MAGIC_NB) {
		ERROR("Header magic\n");
		return -EINVAL;
	}

	if (header->header_version != BOOT_API_HEADER_VERSION) {
		ERROR("Header version\n");
		return -EINVAL;
	}

	for (i = 0U; i < header->image_length; i++) {
		img_checksum += *(uint8_t *)(buffer + i);
	}

	if (header->payload_checksum != img_checksum) {
		ERROR("Checksum: 0x%x (awaited: 0x%x)\n", img_checksum,
		      header->payload_checksum);
		return -EINVAL;
	}

	return 0;
}
