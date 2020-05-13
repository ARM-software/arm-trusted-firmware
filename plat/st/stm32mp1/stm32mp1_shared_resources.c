/*
 * Copyright (c) 2017-2020, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/st/etzpc.h>
#include <drivers/st/stm32_gpio.h>

#include <stm32mp_shared_resources.h>

/*
 * Once one starts to get the resource registering state, one cannot register
 * new resources. This ensures resource state cannot change.
 */
static bool registering_locked;

/*
 * Shared peripherals and resources registration
 *
 * Each resource assignation is stored in a table. The state defaults
 * to PERIPH_UNREGISTERED if the resource is not explicitly assigned.
 *
 * Resource driver that as not embedded (a.k.a their related CFG_xxx build
 * directive is disabled) are assigned to the non-secure world.
 *
 * Each pin of the GPIOZ bank can be secure or non-secure.
 *
 * It is the platform responsibility the ensure resource assignation
 * matches the access permission firewalls configuration.
 */
enum shres_state {
	SHRES_UNREGISTERED = 0,
	SHRES_SECURE,
	SHRES_NON_SECURE,
};

/* Force uint8_t array for array of enum shres_state for size considerations */
static uint8_t shres_state[STM32MP1_SHRES_COUNT];

/* Get resource state: these accesses lock the registering support */
static void lock_registering(void)
{
	registering_locked = true;
}

static bool periph_is_non_secure(enum stm32mp_shres id)
{
	lock_registering();

	return (shres_state[id] == SHRES_NON_SECURE) ||
	       (shres_state[id] == SHRES_UNREGISTERED);
}

static bool periph_is_secure(enum stm32mp_shres id)
{
	return !periph_is_non_secure(id);
}

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

static unsigned int get_gpioz_nbpin(void)
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

static bool mckprot_protects_periph(enum stm32mp_shres id)
{
	switch (id) {
	case STM32MP1_SHRES_MCU:
	case STM32MP1_SHRES_PLL3:
		return true;
	default:
		return false;
	}
}

/* ETZPC configuration at drivers initialization completion */
static enum etzpc_decprot_attributes shres2decprot_attr(enum stm32mp_shres id)
{
	assert((id < STM32MP1_SHRES_GPIOZ(0)) ||
	       (id > STM32MP1_SHRES_GPIOZ(7)));

	if (periph_is_non_secure(id)) {
		return ETZPC_DECPROT_NS_RW;
	}

	return ETZPC_DECPROT_S_RW;
}

static void set_etzpc_secure_configuration(void)
{
	/* Some system peripherals shall be secure */
	etzpc_configure_decprot(STM32MP1_ETZPC_STGENC_ID, ETZPC_DECPROT_S_RW);
	etzpc_configure_decprot(STM32MP1_ETZPC_BKPSRAM_ID, ETZPC_DECPROT_S_RW);
	etzpc_configure_decprot(STM32MP1_ETZPC_DDRCTRL_ID,
				ETZPC_DECPROT_NS_R_S_W);
	etzpc_configure_decprot(STM32MP1_ETZPC_DDRPHYC_ID,
				ETZPC_DECPROT_NS_R_S_W);

	/* Configure ETZPC with peripheral registering */
	etzpc_configure_decprot(STM32MP1_ETZPC_CRYP1_ID,
				shres2decprot_attr(STM32MP1_SHRES_CRYP1));
	etzpc_configure_decprot(STM32MP1_ETZPC_HASH1_ID,
				shres2decprot_attr(STM32MP1_SHRES_HASH1));
	etzpc_configure_decprot(STM32MP1_ETZPC_I2C4_ID,
				shres2decprot_attr(STM32MP1_SHRES_I2C4));
	etzpc_configure_decprot(STM32MP1_ETZPC_I2C6_ID,
				shres2decprot_attr(STM32MP1_SHRES_I2C6));
	etzpc_configure_decprot(STM32MP1_ETZPC_IWDG1_ID,
				shres2decprot_attr(STM32MP1_SHRES_IWDG1));
	etzpc_configure_decprot(STM32MP1_ETZPC_RNG1_ID,
				shres2decprot_attr(STM32MP1_SHRES_RNG1));
	etzpc_configure_decprot(STM32MP1_ETZPC_USART1_ID,
				shres2decprot_attr(STM32MP1_SHRES_USART1));
	etzpc_configure_decprot(STM32MP1_ETZPC_SPI6_ID,
				shres2decprot_attr(STM32MP1_SHRES_SPI6));
}

static void check_rcc_secure_configuration(void)
{
	uint32_t n;
	uint32_t error = 0U;
	bool mckprot = stm32mp1_rcc_is_mckprot();
	bool secure = stm32mp1_rcc_is_secure();

	for (n = 0U; n < ARRAY_SIZE(shres_state); n++) {
		if (shres_state[n] != SHRES_SECURE) {
			continue;
		}

		if (!secure || (mckprot_protects_periph(n) && (!mckprot))) {
			ERROR("RCC %s MCKPROT %s and %u secure\n",
			      secure ? "secure" : "non-secure",
			      mckprot ? "set" : "not set",
			      n);
			error++;
		}
	}

	if (error != 0U) {
		panic();
	}
}

static void set_gpio_secure_configuration(void)
{
	uint32_t pin;

	for (pin = 0U; pin < get_gpioz_nbpin(); pin++) {
		bool secure_state = periph_is_secure(STM32MP1_SHRES_GPIOZ(pin));

		set_gpio_secure_cfg(GPIO_BANK_Z, pin, secure_state);
	}
}

static void print_shared_resources_state(void)
{
	unsigned int id;

	for (id = 0U; id < STM32MP1_SHRES_COUNT; id++) {
		switch (shres_state[id]) {
		case SHRES_SECURE:
			INFO("stm32mp1 %u is secure\n", id);
			break;
		case SHRES_NON_SECURE:
		case SHRES_UNREGISTERED:
			VERBOSE("stm32mp %u is non-secure\n", id);
			break;
		default:
			VERBOSE("stm32mp %u is invalid\n", id);
			panic();
		}
	}
}

void stm32mp_lock_periph_registering(void)
{
	registering_locked = true;

	print_shared_resources_state();

	check_rcc_secure_configuration();
	set_etzpc_secure_configuration();
	set_gpio_secure_configuration();
}
