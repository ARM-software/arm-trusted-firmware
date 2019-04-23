/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>
#include <drivers/arm/sbsa.h>
#include <lib/mmio.h>
#include <stdint_.h>
#include <assert.h>

void sbsa_watchdog_offset_reg_write(uintptr_t base, uint64_t value)
{
	assert((value >> SBSA_WDOG_WOR_WIDTH) == 0);
	mmio_write_32(base + SBSA_WDOG_WOR_LOW_OFFSET,
		 ((uint32_t)value & UINT32_MAX));
	mmio_write_32(base + SBSA_WDOG_WOR_HIGH_OFFSET, (uint32_t)(value >> 32));
}

/*
 * Start the watchdog timer at base address "base" for a
 * period of "ms" milliseconds.The watchdog has to be
 * refreshed within this time period.
 */
void sbsa_wdog_start(uintptr_t base, uint64_t ms)
{
	uint64_t counter_freq;
	uint64_t offset_reg_value;

	counter_freq = (uint64_t)plat_get_syscnt_freq2();
	offset_reg_value = ms * counter_freq / 1000;

	sbsa_watchdog_offset_reg_write(base, offset_reg_value);
	mmio_write_32(base + SBSA_WDOG_WCS_OFFSET, SBSA_WDOG_WCS_EN);
}

/* Stop the watchdog */
void sbsa_wdog_stop(uintptr_t base)
{
	mmio_write_32(base + SBSA_WDOG_WCS_OFFSET, (0x0));
}
