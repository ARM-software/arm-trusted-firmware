/*
 * Copyright (c) 2017-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/st/etzpc.h>
#include <drivers/st/stm32_gpio.h>

#include <platform_def.h>
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

static const char *shres2str_id_tbl[STM32MP1_SHRES_COUNT] __unused = {
	[STM32MP1_SHRES_GPIOZ(0)] = "GPIOZ0",
	[STM32MP1_SHRES_GPIOZ(1)] = "GPIOZ1",
	[STM32MP1_SHRES_GPIOZ(2)] = "GPIOZ2",
	[STM32MP1_SHRES_GPIOZ(3)] = "GPIOZ3",
	[STM32MP1_SHRES_GPIOZ(4)] = "GPIOZ4",
	[STM32MP1_SHRES_GPIOZ(5)] = "GPIOZ5",
	[STM32MP1_SHRES_GPIOZ(6)] = "GPIOZ6",
	[STM32MP1_SHRES_GPIOZ(7)] = "GPIOZ7",
	[STM32MP1_SHRES_IWDG1] = "IWDG1",
	[STM32MP1_SHRES_USART1] = "USART1",
	[STM32MP1_SHRES_SPI6] = "SPI6",
	[STM32MP1_SHRES_I2C4] = "I2C4",
	[STM32MP1_SHRES_RNG1] = "RNG1",
	[STM32MP1_SHRES_HASH1] = "HASH1",
	[STM32MP1_SHRES_CRYP1] = "CRYP1",
	[STM32MP1_SHRES_I2C6] = "I2C6",
	[STM32MP1_SHRES_RTC] = "RTC",
	[STM32MP1_SHRES_MCU] = "MCU",
	[STM32MP1_SHRES_MDMA] = "MDMA",
	[STM32MP1_SHRES_PLL3] = "PLL3",
};

static const char __unused *shres2str_id(enum stm32mp_shres id)
{
	assert(id < ARRAY_SIZE(shres2str_id_tbl));

	return shres2str_id_tbl[id];
}

static const char __unused *shres2str_state_tbl[] = {
	[SHRES_UNREGISTERED] = "unregistered",
	[SHRES_NON_SECURE] = "non-secure",
	[SHRES_SECURE] = "secure",
};

static const char __unused *shres2str_state(unsigned int state)
{
	assert(state < ARRAY_SIZE(shres2str_state_tbl));

	return shres2str_state_tbl[state];
}

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

		assert((count >= 0) && ((unsigned int)count <= (GPIO_PIN_MAX + 1)));

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

static void register_periph(enum stm32mp_shres id, unsigned int state)
{
	assert((id < STM32MP1_SHRES_COUNT) &&
	       ((state == SHRES_SECURE) || (state == SHRES_NON_SECURE)));

	if (registering_locked) {
		if (shres_state[id] == state) {
			return;
		}
		panic();
	}

	if ((shres_state[id] != SHRES_UNREGISTERED) &&
	    (shres_state[id] != state)) {
		VERBOSE("Cannot change %s from %s to %s\n",
			shres2str_id(id),
			shres2str_state(shres_state[id]),
			shres2str_state(state));
		panic();
	}

	if (shres_state[id] == SHRES_UNREGISTERED) {
		VERBOSE("Register %s as %s\n",
			shres2str_id(id), shres2str_state(state));
	}

	if ((id >= STM32MP1_SHRES_GPIOZ(0)) &&
	    (id <= STM32MP1_SHRES_GPIOZ(7)) &&
	    ((unsigned int)(id - STM32MP1_SHRES_GPIOZ(0)) >= get_gpioz_nbpin())) {
		ERROR("Invalid GPIO pin %d, %u pin(s) available\n",
		      (int)(id - STM32MP1_SHRES_GPIOZ(0)), get_gpioz_nbpin());
		panic();
	}

	shres_state[id] = (uint8_t)state;

	/* Explore clock tree to lock dependencies */
	if (state == SHRES_SECURE) {
		enum stm32mp_shres clock_res_id;

		switch (id) {
		case STM32MP1_SHRES_GPIOZ(0):
		case STM32MP1_SHRES_GPIOZ(1):
		case STM32MP1_SHRES_GPIOZ(2):
		case STM32MP1_SHRES_GPIOZ(3):
		case STM32MP1_SHRES_GPIOZ(4):
		case STM32MP1_SHRES_GPIOZ(5):
		case STM32MP1_SHRES_GPIOZ(6):
		case STM32MP1_SHRES_GPIOZ(7):
			clock_res_id = GPIOZ;
			break;
		case STM32MP1_SHRES_IWDG1:
			clock_res_id = IWDG1;
			break;
		case STM32MP1_SHRES_USART1:
			clock_res_id = USART1_K;
			break;
		case STM32MP1_SHRES_SPI6:
			clock_res_id = SPI6_K;
			break;
		case STM32MP1_SHRES_I2C4:
			clock_res_id = I2C4_K;
			break;
		case STM32MP1_SHRES_RNG1:
			clock_res_id = RNG1_K;
			break;
		case STM32MP1_SHRES_HASH1:
			clock_res_id = HASH1;
			break;
		case STM32MP1_SHRES_CRYP1:
			clock_res_id = CRYP1;
			break;
		case STM32MP1_SHRES_I2C6:
			clock_res_id = I2C6_K;
			break;
		case STM32MP1_SHRES_RTC:
			clock_res_id = RTC;
			break;
		default:
			/* No clock resource dependency */
			return;
		}

		stm32mp1_register_clock_parents_secure(clock_res_id);
	}
}

/* Register resource by ID */
void stm32mp_register_secure_periph(enum stm32mp_shres id)
{
	register_periph(id, SHRES_SECURE);
}

void stm32mp_register_non_secure_periph(enum stm32mp_shres id)
{
	register_periph(id, SHRES_NON_SECURE);
}

static void register_periph_iomem(uintptr_t base, unsigned int state)
{
	enum stm32mp_shres id;

	switch (base) {
	case CRYP1_BASE:
		id = STM32MP1_SHRES_CRYP1;
		break;
	case HASH1_BASE:
		id = STM32MP1_SHRES_HASH1;
		break;
	case I2C4_BASE:
		id = STM32MP1_SHRES_I2C4;
		break;
	case I2C6_BASE:
		id = STM32MP1_SHRES_I2C6;
		break;
	case IWDG1_BASE:
		id = STM32MP1_SHRES_IWDG1;
		break;
	case RNG1_BASE:
		id = STM32MP1_SHRES_RNG1;
		break;
	case RTC_BASE:
		id = STM32MP1_SHRES_RTC;
		break;
	case SPI6_BASE:
		id = STM32MP1_SHRES_SPI6;
		break;
	case USART1_BASE:
		id = STM32MP1_SHRES_USART1;
		break;

	case GPIOA_BASE:
	case GPIOB_BASE:
	case GPIOC_BASE:
	case GPIOD_BASE:
	case GPIOE_BASE:
	case GPIOF_BASE:
	case GPIOG_BASE:
	case GPIOH_BASE:
	case GPIOI_BASE:
	case GPIOJ_BASE:
	case GPIOK_BASE:
	case USART2_BASE:
	case USART3_BASE:
	case UART4_BASE:
	case UART5_BASE:
	case USART6_BASE:
	case UART7_BASE:
	case UART8_BASE:
	case IWDG2_BASE:
		/* Allow drivers to register some non-secure resources */
		VERBOSE("IO for non-secure resource 0x%x\n",
			(unsigned int)base);
		if (state != SHRES_NON_SECURE) {
			panic();
		}

		return;

	default:
		panic();
	}

	register_periph(id, state);
}

void stm32mp_register_secure_periph_iomem(uintptr_t base)
{
	register_periph_iomem(base, SHRES_SECURE);
}

void stm32mp_register_non_secure_periph_iomem(uintptr_t base)
{
	register_periph_iomem(base, SHRES_NON_SECURE);
}

void stm32mp_register_secure_gpio(unsigned int bank, unsigned int pin)
{
	switch (bank) {
	case GPIO_BANK_Z:
		register_periph(STM32MP1_SHRES_GPIOZ(pin), SHRES_SECURE);
		break;
	default:
		ERROR("GPIO bank %u cannot be secured\n", bank);
		panic();
	}
}

void stm32mp_register_non_secure_gpio(unsigned int bank, unsigned int pin)
{
	switch (bank) {
	case GPIO_BANK_Z:
		register_periph(STM32MP1_SHRES_GPIOZ(pin), SHRES_NON_SECURE);
		break;
	default:
		break;
	}
}

static bool stm32mp_gpio_bank_is_non_secure(unsigned int bank)
{
	unsigned int non_secure = 0U;
	unsigned int i;

	lock_registering();

	if (bank != GPIO_BANK_Z) {
		return true;
	}

	for (i = 0U; i < get_gpioz_nbpin(); i++) {
		if (periph_is_non_secure(STM32MP1_SHRES_GPIOZ(i))) {
			non_secure++;
		}
	}

	return non_secure == get_gpioz_nbpin();
}

static bool stm32mp_gpio_bank_is_secure(unsigned int bank)
{
	unsigned int secure = 0U;
	unsigned int i;

	lock_registering();

	if (bank != GPIO_BANK_Z) {
		return false;
	}

	for (i = 0U; i < get_gpioz_nbpin(); i++) {
		if (periph_is_secure(STM32MP1_SHRES_GPIOZ(i))) {
			secure++;
		}
	}

	return secure == get_gpioz_nbpin();
}

bool stm32mp_nsec_can_access_clock(unsigned long clock_id)
{
	enum stm32mp_shres shres_id = STM32MP1_SHRES_COUNT;

	switch (clock_id) {
	case CK_CSI:
	case CK_HSE:
	case CK_HSE_DIV2:
	case CK_HSI:
	case CK_LSE:
	case CK_LSI:
	case PLL1_P:
	case PLL1_Q:
	case PLL1_R:
	case PLL2_P:
	case PLL2_Q:
	case PLL2_R:
	case PLL3_P:
	case PLL3_Q:
	case PLL3_R:
	case RTCAPB:
		return true;
	case GPIOZ:
		/* Allow clock access if at least one pin is non-secure */
		return !stm32mp_gpio_bank_is_secure(GPIO_BANK_Z);
	case CRYP1:
		shres_id = STM32MP1_SHRES_CRYP1;
		break;
	case HASH1:
		shres_id = STM32MP1_SHRES_HASH1;
		break;
	case I2C4_K:
		shres_id = STM32MP1_SHRES_I2C4;
		break;
	case I2C6_K:
		shres_id = STM32MP1_SHRES_I2C6;
		break;
	case IWDG1:
		shres_id = STM32MP1_SHRES_IWDG1;
		break;
	case RNG1_K:
		shres_id = STM32MP1_SHRES_RNG1;
		break;
	case RTC:
		shres_id = STM32MP1_SHRES_RTC;
		break;
	case SPI6_K:
		shres_id = STM32MP1_SHRES_SPI6;
		break;
	case USART1_K:
		shres_id = STM32MP1_SHRES_USART1;
		break;
	default:
		return false;
	}

	return periph_is_non_secure(shres_id);
}

bool stm32mp_nsec_can_access_reset(unsigned int reset_id)
{
	enum stm32mp_shres shres_id = STM32MP1_SHRES_COUNT;

	switch (reset_id) {
	case CRYP1_R:
		shres_id = STM32MP1_SHRES_CRYP1;
		break;
	case GPIOZ_R:
		/* GPIOZ reset mandates all pins are non-secure */
		return stm32mp_gpio_bank_is_non_secure(GPIO_BANK_Z);
	case HASH1_R:
		shres_id = STM32MP1_SHRES_HASH1;
		break;
	case I2C4_R:
		shres_id = STM32MP1_SHRES_I2C4;
		break;
	case I2C6_R:
		shres_id = STM32MP1_SHRES_I2C6;
		break;
	case MCU_R:
		shres_id = STM32MP1_SHRES_MCU;
		break;
	case MDMA_R:
		shres_id = STM32MP1_SHRES_MDMA;
		break;
	case RNG1_R:
		shres_id = STM32MP1_SHRES_RNG1;
		break;
	case SPI6_R:
		shres_id = STM32MP1_SHRES_SPI6;
		break;
	case USART1_R:
		shres_id = STM32MP1_SHRES_USART1;
		break;
	default:
		return false;
	}

	return periph_is_non_secure(shres_id);
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
			ERROR("RCC %s MCKPROT %s and %s secure\n",
			      secure ? "secure" : "non-secure",
			      mckprot ? "set" : "not set",
			      shres2str_id(n));
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
			INFO("stm32mp1 %s is secure\n", shres2str_id(id));
			break;
		case SHRES_NON_SECURE:
		case SHRES_UNREGISTERED:
			VERBOSE("stm32mp %s is non-secure\n", shres2str_id(id));
			break;
		default:
			VERBOSE("stm32mp %s is invalid\n", shres2str_id(id));
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
