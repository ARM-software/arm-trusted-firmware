/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_GPIO_H
#define PLAT_GPIO_H

#include <endian.h>
#include <lib/mmio.h>

/* GPIO Register offset */
#define GPIO_SEL_MASK		0x7F
#define GPIO_BIT_MASK		0x1F
#define GPDIR_REG_OFFSET	0x0
#define GPDAT_REG_OFFSET	0x8

#define GPIO_ID_BASE_ADDR_SHIFT 5U
#define GPIO_BITS_PER_BASE_REG	32U

#define GPIO_0			0
#define GPIO_1			1
#define GPIO_2			2
#define GPIO_3			3

#define GPIO_SUCCESS		0x0
#define GPIO_FAILURE		0x1

#ifdef NXP_GPIO_BE
#define gpio_read32(a)           bswap32(mmio_read_32((uintptr_t)(a)))
#define gpio_write32(a, v)       mmio_write_32((uintptr_t)(a), bswap32(v))
#elif defined(NXP_GPIO_LE)
#define gpio_read32(a)           mmio_read_32((uintptr_t)(a))
#define gpio_write32(a, v)       mmio_write_32((uintptr_t)(a), (v))
#else
#error Please define GPIO register endianness
#endif

typedef struct {
	uintptr_t gpio1_base_addr;
	uintptr_t gpio2_base_addr;
	uintptr_t gpio3_base_addr;
	uintptr_t gpio4_base_addr;
} gpio_init_info_t;

void gpio_init(gpio_init_info_t *gpio_init_data);
uint32_t *select_gpio_n_bitnum(uint32_t povdd_gpio, uint32_t *bit_num);
int clr_gpio_bit(uint32_t *gpio_base_addr, uint32_t bit_num);
int set_gpio_bit(uint32_t *gpio_base_addr, uint32_t bit_num);

#endif /* PLAT_GPIO_H */
