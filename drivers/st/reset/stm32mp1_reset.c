/*
 * Copyright (c) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <limits.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/st/stm32mp1_rcc.h>
#include <drivers/st/stm32mp1_reset.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#define RST_CLR_OFFSET	4U

void stm32mp1_reset_assert(uint32_t id)
{
	uint32_t offset = (id / (uint32_t)__LONG_BIT) * sizeof(uintptr_t);
	uint32_t bit = id % (uint32_t)__LONG_BIT;

	mmio_write_32(RCC_BASE + offset, BIT(bit));
	while ((mmio_read_32(RCC_BASE + offset) & BIT(bit)) == 0U) {
		;
	}
}

void stm32mp1_reset_deassert(uint32_t id)
{
	uint32_t offset = ((id / (uint32_t)__LONG_BIT) * sizeof(uintptr_t)) +
			  RST_CLR_OFFSET;
	uint32_t bit = id % (uint32_t)__LONG_BIT;

	mmio_write_32(RCC_BASE + offset, BIT(bit));
	while ((mmio_read_32(RCC_BASE + offset) & BIT(bit)) != 0U) {
		;
	}
}
