/*
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <common/debug.h>
#include <drivers/arm/tzc400.h>
#include <lib/mmio.h>

#include "tzc_common_private.h"

/*
 * Macros which will be used by common core functions.
 */
#define TZC_400_REGION_BASE_LOW_0_OFFSET	U(0x100)
#define TZC_400_REGION_BASE_HIGH_0_OFFSET	U(0x104)
#define TZC_400_REGION_TOP_LOW_0_OFFSET		U(0x108)
#define TZC_400_REGION_TOP_HIGH_0_OFFSET	U(0x10c)
#define TZC_400_REGION_ATTR_0_OFFSET		U(0x110)
#define TZC_400_REGION_ID_ACCESS_0_OFFSET	U(0x114)

/*
 * Implementation defined values used to validate inputs later.
 * Filters : max of 4 ; 0 to 3
 * Regions : max of 9 ; 0 to 8
 * Address width : Values between 32 to 64
 */
typedef struct tzc400_instance {
	uintptr_t base;
	uint8_t addr_width;
	uint8_t num_filters;
	uint8_t num_regions;
} tzc400_instance_t;

static tzc400_instance_t tzc400;

static inline unsigned int _tzc400_read_build_config(uintptr_t base)
{
	return mmio_read_32(base + BUILD_CONFIG_OFF);
}

static inline unsigned int _tzc400_read_gate_keeper(uintptr_t base)
{
	return mmio_read_32(base + GATE_KEEPER_OFF);
}

static inline void _tzc400_write_gate_keeper(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GATE_KEEPER_OFF, val);
}

/*
 * Get the open status information for all filter units.
 */
#define get_gate_keeper_os(_base)	((_tzc400_read_gate_keeper(_base) >>  \
					GATE_KEEPER_OS_SHIFT) &		\
					GATE_KEEPER_OS_MASK)


/* Define common core functions used across different TZC peripherals. */
DEFINE_TZC_COMMON_WRITE_ACTION(400, 400)
DEFINE_TZC_COMMON_WRITE_REGION_BASE(400, 400)
DEFINE_TZC_COMMON_WRITE_REGION_TOP(400, 400)
DEFINE_TZC_COMMON_WRITE_REGION_ATTRIBUTES(400, 400)
DEFINE_TZC_COMMON_WRITE_REGION_ID_ACCESS(400, 400)
DEFINE_TZC_COMMON_CONFIGURE_REGION0(400)
DEFINE_TZC_COMMON_CONFIGURE_REGION(400)

static unsigned int _tzc400_get_gate_keeper(uintptr_t base,
				unsigned int filter)
{
	unsigned int open_status;

	open_status = get_gate_keeper_os(base);

	return (open_status >> filter) & GATE_KEEPER_FILTER_MASK;
}

/* This function is not MP safe. */
static void _tzc400_set_gate_keeper(uintptr_t base,
				unsigned int filter,
				int val)
{
	unsigned int open_status;

	/* Upper half is current state. Lower half is requested state. */
	open_status = get_gate_keeper_os(base);

	if (val != 0)
		open_status |=  (1UL << filter);
	else
		open_status &= ~(1UL << filter);

	_tzc400_write_gate_keeper(base, (open_status & GATE_KEEPER_OR_MASK) <<
			      GATE_KEEPER_OR_SHIFT);

	/* Wait here until we see the change reflected in the TZC status. */
	while ((get_gate_keeper_os(base)) != open_status)
		;
}

void tzc400_set_action(unsigned int action)
{
	assert(tzc400.base != 0U);
	assert(action <= TZC_ACTION_ERR_INT);

	/*
	 * - Currently no handler is provided to trap an error via interrupt
	 *   or exception.
	 * - The interrupt action has not been tested.
	 */
	_tzc400_write_action(tzc400.base, action);
}

void tzc400_init(uintptr_t base)
{
#if DEBUG
	unsigned int tzc400_id;
#endif
	unsigned int tzc400_build;

	assert(base != 0U);
	tzc400.base = base;

#if DEBUG
	tzc400_id = _tzc_read_peripheral_id(base);
	if (tzc400_id != TZC_400_PERIPHERAL_ID) {
		ERROR("TZC-400 : Wrong device ID (0x%x).\n", tzc400_id);
		panic();
	}
#endif

	/* Save values we will use later. */
	tzc400_build = _tzc400_read_build_config(tzc400.base);
	tzc400.num_filters = (uint8_t)((tzc400_build >> BUILD_CONFIG_NF_SHIFT) &
					BUILD_CONFIG_NF_MASK) + 1U;
	tzc400.addr_width  = (uint8_t)((tzc400_build >> BUILD_CONFIG_AW_SHIFT) &
					BUILD_CONFIG_AW_MASK) + 1U;
	tzc400.num_regions = (uint8_t)((tzc400_build >> BUILD_CONFIG_NR_SHIFT) &
					BUILD_CONFIG_NR_MASK) + 1U;
}

/*
 * `tzc400_configure_region0` is used to program region 0 into the TrustZone
 * controller. Region 0 covers the whole address space that is not mapped
 * to any other region, and is enabled on all filters; this cannot be
 * changed. This function only changes the access permissions.
 */
void tzc400_configure_region0(unsigned int sec_attr,
			   unsigned int ns_device_access)
{
	assert(tzc400.base != 0U);
	assert(sec_attr <= TZC_REGION_S_RDWR);

	_tzc400_configure_region0(tzc400.base, sec_attr, ns_device_access);
}

/*
 * `tzc400_configure_region` is used to program regions into the TrustZone
 * controller. A region can be associated with more than one filter. The
 * associated filters are passed in as a bitmap (bit0 = filter0).
 * NOTE:
 * Region 0 is special; it is preferable to use tzc400_configure_region0
 * for this region (see comment for that function).
 */
void tzc400_configure_region(unsigned int filters,
			  unsigned int region,
			  unsigned long long region_base,
			  unsigned long long region_top,
			  unsigned int sec_attr,
			  unsigned int nsaid_permissions)
{
	assert(tzc400.base != 0U);

	/* Do range checks on filters and regions. */
	assert(((filters >> tzc400.num_filters) == 0U) &&
	       (region < tzc400.num_regions));

	/*
	 * Do address range check based on TZC configuration. A 64bit address is
	 * the max and expected case.
	 */
	assert((region_top <= (UINT64_MAX >> (64U - tzc400.addr_width))) &&
		(region_base < region_top));

	/* region_base and (region_top + 1) must be 4KB aligned */
	assert(((region_base | (region_top + 1U)) & (4096U - 1U)) == 0U);

	assert(sec_attr <= TZC_REGION_S_RDWR);

	_tzc400_configure_region(tzc400.base, filters, region, region_base,
						region_top,
						sec_attr, nsaid_permissions);
}

void tzc400_enable_filters(void)
{
	unsigned int state;
	unsigned int filter;

	assert(tzc400.base != 0U);

	for (filter = 0U; filter < tzc400.num_filters; filter++) {
		state = _tzc400_get_gate_keeper(tzc400.base, filter);
		if (state != 0U) {
			/*
			 * The TZC filter is already configured. Changing the
			 * programmer's view in an active system can cause
			 * unpredictable behavior therefore panic for now rather
			 * than try to determine whether this is safe in this
			 * instance.
			 *
			 * See the 'ARM (R) CoreLink TM TZC-400 TrustZone (R)
			 * Address Space Controller' Technical Reference Manual.
			 */
			ERROR("TZC-400 : Filter %d Gatekeeper already"
				" enabled.\n", filter);
			panic();
		}
		_tzc400_set_gate_keeper(tzc400.base, filter, 1);
	}
}

void tzc400_disable_filters(void)
{
	unsigned int filter;

	assert(tzc400.base != 0U);

	/*
	 * We don't do the same state check as above as the Gatekeepers are
	 * disabled after reset.
	 */
	for (filter = 0; filter < tzc400.num_filters; filter++)
		_tzc400_set_gate_keeper(tzc400.base, filter, 0);
}
