/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <stddef.h>
#include <tzc400.h>

/*
 * Implementation defined values used to validate inputs later.
 * Filters : max of 4 ; 0 to 3
 * Regions : max of 9 ; 0 to 8
 * Address width : Values between 32 to 64
 */
typedef struct tzc_instance {
	uintptr_t base;
	uint8_t addr_width;
	uint8_t num_filters;
	uint8_t num_regions;
} tzc_instance_t;

tzc_instance_t tzc;


static inline uint32_t tzc_read_build_config(uintptr_t base)
{
	return mmio_read_32(base + BUILD_CONFIG_OFF);
}

static inline uint32_t tzc_read_gate_keeper(uintptr_t base)
{
	return mmio_read_32(base + GATE_KEEPER_OFF);
}

static inline void tzc_write_gate_keeper(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GATE_KEEPER_OFF, val);
}

static inline void tzc_write_action(uintptr_t base, tzc_action_t action)
{
	mmio_write_32(base + ACTION_OFF, action);
}

static inline void tzc_write_region_base_low(uintptr_t base,
					uint32_t region,
					uint32_t val)
{
	mmio_write_32(base + REGION_BASE_LOW_OFF +
		REGION_NUM_OFF(region), val);
}

static inline void tzc_write_region_base_high(uintptr_t base,
					uint32_t region,
					uint32_t val)
{
	mmio_write_32(base + REGION_BASE_HIGH_OFF +
		REGION_NUM_OFF(region), val);
}

static inline void tzc_write_region_top_low(uintptr_t base,
					uint32_t region,
					uint32_t val)
{
	mmio_write_32(base + REGION_TOP_LOW_OFF +
		REGION_NUM_OFF(region), val);
}

static inline void tzc_write_region_top_high(uintptr_t base,
					uint32_t region,
					uint32_t val)
{
	mmio_write_32(base + REGION_TOP_HIGH_OFF +
		REGION_NUM_OFF(region), val);
}

static inline void tzc_write_region_attributes(uintptr_t base,
					uint32_t region,
					uint32_t val)
{
	mmio_write_32(base + REGION_ATTRIBUTES_OFF +
		REGION_NUM_OFF(region), val);
}

static inline void tzc_write_region_id_access(uintptr_t base,
					uint32_t region,
					uint32_t val)
{
	mmio_write_32(base + REGION_ID_ACCESS_OFF +
		REGION_NUM_OFF(region), val);
}

static unsigned int tzc_read_peripheral_id(uintptr_t base)
{
	unsigned int id;

	id = mmio_read_8(base + PID0_OFF);
	/* Masks jep106_id_3_0 part in PID1 */
	id |= ((mmio_read_8(base + PID1_OFF) & 0xF) << 8);

	return id;
}

static uint32_t tzc_get_gate_keeper(uintptr_t base, uint8_t filter)
{
	uint32_t tmp;

	tmp = (tzc_read_gate_keeper(base) >> GATE_KEEPER_OS_SHIFT) &
		GATE_KEEPER_OS_MASK;

	return (tmp >> filter) & GATE_KEEPER_FILTER_MASK;
}

/* This function is not MP safe. */
static void tzc_set_gate_keeper(uintptr_t base, uint8_t filter, uint32_t val)
{
	uint32_t tmp;

	/* Upper half is current state. Lower half is requested state. */
	tmp = (tzc_read_gate_keeper(base) >> GATE_KEEPER_OS_SHIFT) &
		GATE_KEEPER_OS_MASK;

	if (val)
		tmp |=  (1 << filter);
	else
		tmp &= ~(1 << filter);

	tzc_write_gate_keeper(base, (tmp & GATE_KEEPER_OR_MASK) <<
			      GATE_KEEPER_OR_SHIFT);

	/* Wait here until we see the change reflected in the TZC status. */
	while (((tzc_read_gate_keeper(base) >> GATE_KEEPER_OS_SHIFT) &
		GATE_KEEPER_OS_MASK) != tmp)
	  ;
}


void tzc_init(uintptr_t base)
{
	unsigned int tzc_id;
	unsigned int tzc_build;

	assert(base);

	tzc.base = base;

	/*
	 * We expect to see a tzc400. Check peripheral ID.
	 */
	tzc_id = tzc_read_peripheral_id(tzc.base);
	if (tzc_id != TZC400_PERIPHERAL_ID) {
		ERROR("TZC : Wrong device ID (0x%x).\n", tzc_id);
		panic();
	}

	/* Save values we will use later. */
	tzc_build = tzc_read_build_config(tzc.base);
	tzc.num_filters = ((tzc_build >> BUILD_CONFIG_NF_SHIFT) &
			   BUILD_CONFIG_NF_MASK) + 1;
	tzc.addr_width  = ((tzc_build >> BUILD_CONFIG_AW_SHIFT) &
			   BUILD_CONFIG_AW_MASK) + 1;
	tzc.num_regions = ((tzc_build >> BUILD_CONFIG_NR_SHIFT) &
			   BUILD_CONFIG_NR_MASK) + 1;
}

/*
 * `tzc_configure_region0` is used to program region 0 into the TrustZone
 * controller. Region 0 covers the whole address space that is not mapped
 * to any other region, and is enabled on all filters; this cannot be
 * changed. This function only changes the access permissions.
 */
void tzc_configure_region0(tzc_region_attributes_t sec_attr,
			   uint32_t ns_device_access)
{
	assert(tzc.base);

	VERBOSE("TZC : Configuring region 0 (sec_attr=0x%x, ns_devs=0x%x)\n",
		sec_attr, ns_device_access);

	assert(sec_attr <= TZC_REGION_S_RDWR);

	/* Set secure attributes on region 0 */
	tzc_write_region_attributes(tzc.base, 0,
		sec_attr << REG_ATTR_SEC_SHIFT);

	/*
	 * Specify which non-secure devices have permission to access
	 * region 0.
	 */
	tzc_write_region_id_access(tzc.base, 0, ns_device_access);
}


/*
 * `tzc_configure_region` is used to program regions into the TrustZone
 * controller. A region can be associated with more than one filter. The
 * associated filters are passed in as a bitmap (bit0 = filter0).
 * NOTE:
 * Region 0 is special; it is preferable to use tzc_configure_region0
 * for this region (see comment for that function).
 */
void tzc_configure_region(uint32_t filters,
			  uint8_t  region,
			  uint64_t region_base,
			  uint64_t region_top,
			  tzc_region_attributes_t sec_attr,
			  uint32_t ns_device_access)
{
	assert(tzc.base);

	VERBOSE("TZC : Configuring region (filters=0x%x, region=%d, ...\n",
		filters, region);
	VERBOSE("TZC : ... base=0x%lx, top=0x%lx, ...\n",
		region_base, region_top);
	VERBOSE("TZC : ... sec_attr=0x%x, ns_devs=0x%x)\n",
		sec_attr, ns_device_access);

	/* Do range checks on filters and regions. */
	assert(((filters >> tzc.num_filters) == 0) &&
	       (region < tzc.num_regions));

	/*
	 * Do address range check based on TZC configuration. A 64bit address is
	 * the max and expected case.
	 */
	assert(((region_top <= (UINT64_MAX >> (64 - tzc.addr_width))) &&
		(region_base < region_top)));

	/* region_base and (region_top + 1) must be 4KB aligned */
	assert(((region_base | (region_top + 1)) & (4096 - 1)) == 0);

	assert(sec_attr <= TZC_REGION_S_RDWR);

	/*
	 * Inputs look ok, start programming registers.
	 * All the address registers are 32 bits wide and have a LOW and HIGH
	 * component used to construct a up to a 64bit address.
	 */
	tzc_write_region_base_low(tzc.base, region,
				(uint32_t)(region_base));
	tzc_write_region_base_high(tzc.base, region,
				(uint32_t)(region_base >> 32));

	tzc_write_region_top_low(tzc.base, region,
				(uint32_t)(region_top));
	tzc_write_region_top_high(tzc.base, region,
				(uint32_t)(region_top >> 32));

	/* Assign the region to a filter and set secure attributes */
	tzc_write_region_attributes(tzc.base, region,
		(sec_attr << REG_ATTR_SEC_SHIFT) | filters);

	/*
	 * Specify which non-secure devices have permission to access this
	 * region.
	 */
	tzc_write_region_id_access(tzc.base, region, ns_device_access);
}


void tzc_set_action(tzc_action_t action)
{
	assert(tzc.base);

	/*
	 * - Currently no handler is provided to trap an error via interrupt
	 *   or exception.
	 * - The interrupt action has not been tested.
	 */
	tzc_write_action(tzc.base, action);
}


void tzc_enable_filters(void)
{
	uint32_t state;
	uint32_t filter;

	assert(tzc.base);

	for (filter = 0; filter < tzc.num_filters; filter++) {
		state = tzc_get_gate_keeper(tzc.base, filter);
		if (state) {
			/* The TZC filter is already configured. Changing the
			 * programmer's view in an active system can cause
			 * unpredictable behavior therefore panic for now rather
			 * than try to determine whether this is safe in this
			 * instance. See:
			 * http://infocenter.arm.com/help/index.jsp?\
			 * topic=/com.arm.doc.ddi0504c/CJHHECBF.html */
			ERROR("TZC : Filter %d Gatekeeper already enabled.\n",
				filter);
			panic();
		}
		tzc_set_gate_keeper(tzc.base, filter, 1);
	}
}


void tzc_disable_filters(void)
{
	uint32_t filter;

	assert(tzc.base);

	/*
	 * We don't do the same state check as above as the Gatekeepers are
	 * disabled after reset.
	 */
	for (filter = 0; filter < tzc.num_filters; filter++)
		tzc_set_gate_keeper(tzc.base, filter, 0);
}
