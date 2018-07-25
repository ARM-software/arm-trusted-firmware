/*
 * Copyright (c) 2016-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <mmio.h>
#include <stdbool.h>
#include <stm32_gpio.h>

static bool check_gpio(uint32_t bank, uint32_t pin)
{
	if (pin > GPIO_PIN_MAX) {
		ERROR("%s: wrong pin number (%d)\n", __func__, pin);
		return false;
	}

	if ((bank > GPIO_BANK_K) && (bank != GPIO_BANK_Z)) {
		ERROR("%s: wrong GPIO bank number (%d)\n", __func__, bank);
		return false;
	}

	return true;
}

void set_gpio(uint32_t bank, uint32_t pin, uint32_t mode, uint32_t speed,
	      uint32_t pull, uint32_t alternate)
{
	volatile uint32_t bank_address;

	if (!check_gpio(bank, pin)) {
		return;
	}

	if (bank == GPIO_BANK_Z) {
		bank_address = STM32_GPIOZ_BANK;
	} else {
		bank_address = STM32_GPIOA_BANK +
			(bank * STM32_GPIO_BANK_OFFSET);
	}

	mmio_clrbits_32(bank_address + GPIO_MODE_OFFSET,
			((uint32_t)GPIO_MODE_MASK << (pin << 1)));
	mmio_setbits_32(bank_address + GPIO_MODE_OFFSET,
			(mode & ~GPIO_OPEN_DRAIN) << (pin << 1));

	if ((mode & GPIO_OPEN_DRAIN) != 0U) {
		mmio_setbits_32(bank_address + GPIO_TYPE_OFFSET,
				BIT(pin));
	}

	mmio_clrbits_32(bank_address + GPIO_SPEED_OFFSET,
			((uint32_t)GPIO_SPEED_MASK << (pin << 1)));
	mmio_setbits_32(bank_address + GPIO_SPEED_OFFSET, speed << (pin << 1));

	mmio_clrbits_32(bank_address + GPIO_PUPD_OFFSET,
			((uint32_t)GPIO_PULL_MASK << (pin << 1)));
	mmio_setbits_32(bank_address + GPIO_PUPD_OFFSET, pull << (pin << 1));

	if (pin < GPIO_ALT_LOWER_LIMIT) {
		mmio_clrbits_32(bank_address + GPIO_AFRL_OFFSET,
				((uint32_t)GPIO_ALTERNATE_MASK << (pin << 2)));
		mmio_setbits_32(bank_address + GPIO_AFRL_OFFSET,
				alternate << (pin << 2));
	} else {
		mmio_clrbits_32(bank_address + GPIO_AFRH_OFFSET,
				((uint32_t)GPIO_ALTERNATE_MASK <<
				 ((pin - GPIO_ALT_LOWER_LIMIT) << 2)));
		mmio_setbits_32(bank_address + GPIO_AFRH_OFFSET,
				alternate << ((pin - GPIO_ALT_LOWER_LIMIT) <<
					      2));
	}

	VERBOSE("GPIO %u mode set to 0x%x\n", bank,
		mmio_read_32(bank_address + GPIO_MODE_OFFSET));
	VERBOSE("GPIO %u speed set to 0x%x\n", bank,
		mmio_read_32(bank_address + GPIO_SPEED_OFFSET));
	VERBOSE("GPIO %u mode pull to 0x%x\n", bank,
		mmio_read_32(bank_address + GPIO_PUPD_OFFSET));
	VERBOSE("GPIO %u mode alternate low to 0x%x\n", bank,
		mmio_read_32(bank_address + GPIO_AFRL_OFFSET));
	VERBOSE("GPIO %u mode alternate high to 0x%x\n", bank,
		mmio_read_32(bank_address + GPIO_AFRH_OFFSET));
}
