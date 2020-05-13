/*
 * Copyright (c) 2017-2020, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/st/stm32_gpio.h>

#include <stm32mp_shared_resources.h>

/* GPIOZ pin count is saved in RAM to prevent parsing FDT several times */
static int8_t gpioz_nbpin = -1;

static unsigned int get_gpio_nbpin(unsigned int bank)
{
	if (bank != GPIO_BANK_Z) {
		int count = fdt_get_gpio_bank_pin_count(bank);

		assert((count >= 0) || (count <= (GPIO_PIN_MAX + 1)));

		return (unsigned int)count;
	}

	if (gpioz_nbpin < 0) {
		int count = fdt_get_gpio_bank_pin_count(GPIO_BANK_Z);

		assert((count == 0) || (count == STM32MP_GPIOZ_PIN_MAX_COUNT));

		gpioz_nbpin = count;
	}

	return (unsigned int)gpioz_nbpin;
}

static unsigned int __unused get_gpioz_nbpin(void)
{
	return get_gpio_nbpin(GPIO_BANK_Z);
}

/* Currently allow full access by non-secure to platform clock services */
bool stm32mp_nsec_can_access_clock(unsigned long clock_id)
{
	return true;
}

/* Currently allow full access by non-secure to platform reset services */
bool stm32mp_nsec_can_access_reset(unsigned int reset_id)
{
	return true;
}
