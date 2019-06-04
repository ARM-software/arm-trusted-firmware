/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <drivers/st/stm32_iwdg.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#define MAP_SRAM	MAP_REGION_FLAT(STM32MP_SYSRAM_BASE, \
					STM32MP_SYSRAM_SIZE, \
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

unsigned long stm32_get_gpio_bank_clock(unsigned int bank)
{
	if (bank == GPIO_BANK_Z) {
		return GPIOZ;
	}

	assert(GPIO_BANK_A == 0 && bank <= GPIO_BANK_K);

	return GPIOA + (bank - GPIO_BANK_A);
}

uint32_t stm32_iwdg_get_instance(uintptr_t base)
{
	switch (base) {
	case IWDG1_BASE:
		return IWDG1_INST;
	case IWDG2_BASE:
		return IWDG2_INST;
	default:
		panic();
	}
}

uint32_t stm32_iwdg_get_otp_config(uint32_t iwdg_inst)
{
	uint32_t iwdg_cfg = 0U;
	uint32_t otp_value;

#if defined(IMAGE_BL2)
	if (bsec_shadow_register(HW2_OTP) != BSEC_OK) {
		panic();
	}
#endif

	if (bsec_read_otp(&otp_value, HW2_OTP) != BSEC_OK) {
		panic();
	}

	if ((otp_value & BIT(iwdg_inst + HW2_OTP_IWDG_HW_POS)) != 0U) {
		iwdg_cfg |= IWDG_HW_ENABLED;
	}

	if ((otp_value & BIT(iwdg_inst + HW2_OTP_IWDG_FZ_STOP_POS)) != 0U) {
		iwdg_cfg |= IWDG_DISABLE_ON_STOP;
	}

	if ((otp_value & BIT(iwdg_inst + HW2_OTP_IWDG_FZ_STANDBY_POS)) != 0U) {
		iwdg_cfg |= IWDG_DISABLE_ON_STANDBY;
	}

	return iwdg_cfg;
}

#if defined(IMAGE_BL2)
uint32_t stm32_iwdg_shadow_update(uint32_t iwdg_inst, uint32_t flags)
{
	uint32_t otp;
	uint32_t result;

	if (bsec_shadow_read_otp(&otp, HW2_OTP) != BSEC_OK) {
		panic();
	}

	if ((flags & IWDG_DISABLE_ON_STOP) != 0U) {
		otp |= BIT(iwdg_inst + HW2_OTP_IWDG_FZ_STOP_POS);
	}

	if ((flags & IWDG_DISABLE_ON_STANDBY) != 0U) {
		otp |= BIT(iwdg_inst + HW2_OTP_IWDG_FZ_STANDBY_POS);
	}

	result = bsec_write_otp(otp, HW2_OTP);
	if (result != BSEC_OK) {
		return result;
	}

	/* Sticky lock OTP_IWDG (read and write) */
	if (!bsec_write_sr_lock(HW2_OTP, 1U) ||
	    !bsec_write_sw_lock(HW2_OTP, 1U)) {
		return BSEC_LOCK_FAIL;
	}

	return BSEC_OK;
}
#endif
