/*
 * Copyright (c) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <limits.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define RESET_TIMEOUT_US_1MS	U(1000)

static uint32_t id2reg_offset(unsigned int reset_id)
{
	return ((reset_id & GENMASK(31, 5)) >> 5) * sizeof(uint32_t);
}

static uint8_t id2reg_bit_pos(unsigned int reset_id)
{
	return (uint8_t)(reset_id & GENMASK(4, 0));
}

void stm32mp_reset_assert(uint32_t id)
{
	uint32_t offset = id2reg_offset(id);
	uint32_t bitmsk = BIT(id2reg_bit_pos(id));
	uint64_t timeout_ref;
	uintptr_t rcc_base = stm32mp_rcc_base();

	mmio_write_32(rcc_base + offset, bitmsk);

	timeout_ref = timeout_init_us(RESET_TIMEOUT_US_1MS);
	while ((mmio_read_32(rcc_base + offset) & bitmsk) == 0U) {
		if (timeout_elapsed(timeout_ref)) {
			panic();
		}
	}
}

void stm32mp_reset_deassert(uint32_t id)
{
	uint32_t offset = id2reg_offset(id) + RCC_RSTCLRR_OFFSET;
	uint32_t bitmsk = BIT(id2reg_bit_pos(id));
	uint64_t timeout_ref;
	uintptr_t rcc_base = stm32mp_rcc_base();

	mmio_write_32(rcc_base + offset, bitmsk);

	timeout_ref = timeout_init_us(RESET_TIMEOUT_US_1MS);
	while ((mmio_read_32(rcc_base + offset) & bitmsk) != 0U) {
		if (timeout_elapsed(timeout_ref)) {
			panic();
		}
	}
}
