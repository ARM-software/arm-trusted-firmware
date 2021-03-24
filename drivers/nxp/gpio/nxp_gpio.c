/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <nxp_gpio.h>

static gpio_init_info_t *gpio_init_info;

void gpio_init(gpio_init_info_t *gpio_init_data)
{
	gpio_init_info = gpio_init_data;
}

/* This function set GPIO pin for raising POVDD. */
int set_gpio_bit(uint32_t *gpio_base_addr,
		 uint32_t bit_num)
{
	uint32_t val = 0U;
	uint32_t *gpdir = NULL;
	uint32_t *gpdat = NULL;

	if (gpio_init_info == NULL) {
		ERROR("GPIO is not initialized.\n");
		return GPIO_FAILURE;
	}

	gpdir = gpio_base_addr + GPDIR_REG_OFFSET;
	gpdat = gpio_base_addr + (GPDAT_REG_OFFSET >> 2);

	/*
	 * Set the corresponding bit in direction register
	 * to configure the GPIO as output.
	 */
	val = gpio_read32(gpdir);
	val = val | bit_num;
	gpio_write32(gpdir, val);

	/* Set the corresponding bit in GPIO data register */
	val = gpio_read32(gpdat);
	val = val | bit_num;
	gpio_write32(gpdat, val);

	val = gpio_read32(gpdat);

	if ((val & bit_num) == 0U) {
		return GPIO_FAILURE;
	}

	return GPIO_SUCCESS;
}

/* This function reset GPIO pin set for raising POVDD. */
int clr_gpio_bit(uint32_t *gpio_base_addr, uint32_t bit_num)
{
	uint32_t val = 0U;
	uint32_t *gpdir = NULL;
	uint32_t *gpdat = NULL;


	if (gpio_init_info == NULL) {
		ERROR("GPIO is not initialized.\n");
		return GPIO_FAILURE;
	}

	gpdir = gpio_base_addr + GPDIR_REG_OFFSET;
	gpdat = gpio_base_addr + GPDAT_REG_OFFSET;

	/*
	 * Reset the corresponding bit in direction and data register
	 * to configure the GPIO as input.
	 */
	val = gpio_read32(gpdat);
	val = val & ~(bit_num);
	gpio_write32(gpdat, val);

	val = gpio_read32(gpdat);

	val = gpio_read32(gpdir);
	val = val & ~(bit_num);
	gpio_write32(gpdir, val);

	val = gpio_read32(gpdat);

	if ((val & bit_num) != 0U) {
		return GPIO_FAILURE;
	}

	return GPIO_SUCCESS;
}

uint32_t *select_gpio_n_bitnum(uint32_t povdd_gpio, uint32_t *bit_num)
{
	uint32_t *ret_gpio;
	uint32_t povdd_gpio_val = 0U;
	uint32_t gpio_num = 0U;

	if (gpio_init_info == NULL) {
		ERROR("GPIO is not initialized.\n");
	}
	/*
	 * Subtract 1 from fuse_hdr povdd_gpio value as
	 * for 0x1 value, bit 0 is to be set
	 * for 0x20 value i.e 32, bit 31 i.e. 0x1f is to be set.
	 * 0x1f - 0x00 : GPIO_1
	 * 0x3f - 0x20 : GPIO_2
	 * 0x5f - 0x40 : GPIO_3
	 * 0x7f - 0x60 : GPIO_4
	 */
	povdd_gpio_val = (povdd_gpio - 1U) & GPIO_SEL_MASK;

	/* Right shift by 5 to divide by 32 */
	gpio_num = povdd_gpio_val >> GPIO_ID_BASE_ADDR_SHIFT;
	*bit_num = 1U << (GPIO_BITS_PER_BASE_REG
			  - (povdd_gpio_val & GPIO_BIT_MASK)
			  - 1U);

	switch (gpio_num) {
	case GPIO_0:
		ret_gpio = (uint32_t *) gpio_init_info->gpio1_base_addr;
		break;
	case GPIO_1:
		ret_gpio = (uint32_t *) gpio_init_info->gpio2_base_addr;
		break;
	case GPIO_2:
		ret_gpio = (uint32_t *) gpio_init_info->gpio3_base_addr;
		break;
	case GPIO_3:
		ret_gpio = (uint32_t *) gpio_init_info->gpio4_base_addr;
		break;
	default:
		ret_gpio = NULL;
	}

	if (ret_gpio == NULL) {
		INFO("GPIO_NUM = %d doesn't exist.\n", gpio_num);
	}

	return ret_gpio;
}
