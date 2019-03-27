/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <common/debug.h>
#include <drivers/arm/tzc380.h>
#include <lib/mmio.h>

struct tzc380_instance {
	uintptr_t base;
	uint8_t addr_width;
	uint8_t num_regions;
};

struct tzc380_instance tzc380;

static unsigned int tzc380_read_build_config(uintptr_t base)
{
	return mmio_read_32(base + TZC380_CONFIGURATION_OFF);
}

static void tzc380_write_action(uintptr_t base, unsigned int action)
{
	mmio_write_32(base + ACTION_OFF, action);
}

static void tzc380_write_region_base_low(uintptr_t base, unsigned int region,
				      unsigned int val)
{
	mmio_write_32(base + REGION_SETUP_LOW_OFF(region), val);
}

static void tzc380_write_region_base_high(uintptr_t base, unsigned int region,
				       unsigned int val)
{
	mmio_write_32(base + REGION_SETUP_HIGH_OFF(region), val);
}

static void tzc380_write_region_attributes(uintptr_t base, unsigned int region,
					unsigned int val)
{
	mmio_write_32(base + REGION_ATTRIBUTES_OFF(region), val);
}

void tzc380_init(uintptr_t base)
{
	unsigned int tzc_build;

	assert(base != 0U);
	tzc380.base = base;

	/* Save values we will use later. */
	tzc_build = tzc380_read_build_config(tzc380.base);
	tzc380.addr_width  = ((tzc_build >> BUILD_CONFIG_AW_SHIFT) &
			      BUILD_CONFIG_AW_MASK) + 1;
	tzc380.num_regions = ((tzc_build >> BUILD_CONFIG_NR_SHIFT) &
			       BUILD_CONFIG_NR_MASK) + 1;
}

static uint32_t addr_low(uintptr_t addr)
{
	return (uint32_t)addr;
}

static uint32_t addr_high(uintptr_t addr __unused)
{
#if (UINTPTR_MAX == UINT64_MAX)
	return addr >> 32;
#else
	return 0;
#endif
}

/*
 * `tzc380_configure_region` is used to program regions into the TrustZone
 * controller.
 */
void tzc380_configure_region(uint8_t region, uintptr_t region_base, unsigned int attr)
{
	assert(tzc380.base != 0U);

	assert(region < tzc380.num_regions);

	tzc380_write_region_base_low(tzc380.base, region, addr_low(region_base));
	tzc380_write_region_base_high(tzc380.base, region, addr_high(region_base));
	tzc380_write_region_attributes(tzc380.base, region, attr);
}

void tzc380_set_action(unsigned int action)
{
	assert(tzc380.base != 0U);

	/*
	 * - Currently no handler is provided to trap an error via interrupt
	 *   or exception.
	 * - The interrupt action has not been tested.
	 */
	tzc380_write_action(tzc380.base, action);
}
