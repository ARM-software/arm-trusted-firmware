/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <assert.h>
#include <cci.h>
#include <debug.h>
#include <mmio.h>
#include <stdint.h>

static uintptr_t g_cci_base;
static unsigned int g_max_master_id;
static const int *g_cci_slave_if_map;

#if ENABLE_ASSERTIONS
static int validate_cci_map(const int *map)
{
	unsigned int valid_cci_map = 0;
	int slave_if_id;
	int i;

	/* Validate the map */
	for (i = 0; i <= g_max_master_id; i++) {
		slave_if_id = map[i];

		if (slave_if_id < 0)
			continue;

		if (slave_if_id >= CCI_SLAVE_INTERFACE_COUNT) {
			ERROR("Slave interface ID is invalid\n");
			return 0;
		}

		if (valid_cci_map & (1 << slave_if_id)) {
			ERROR("Multiple masters are assigned same slave interface ID\n");
			return 0;
		}
		valid_cci_map |= 1 << slave_if_id;
	}

	if (!valid_cci_map) {
		ERROR("No master is assigned a valid slave interface\n");
		return 0;
	}

	return 1;
}
#endif /* ENABLE_ASSERTIONS */

void cci_init(uintptr_t cci_base,
		const int *map,
		unsigned int num_cci_masters)
{
	assert(map);
	assert(cci_base);

	g_cci_base = cci_base;

	/*
	 * Master Id's are assigned from zero, So in an array of size n
	 * the max master id is (n - 1).
	 */
	g_max_master_id = num_cci_masters - 1;

	assert(validate_cci_map(map));
	g_cci_slave_if_map = map;
}

void cci_enable_snoop_dvm_reqs(unsigned int master_id)
{
	int slave_if_id;

	assert(g_cci_base);
	assert(master_id <= g_max_master_id);

	slave_if_id = g_cci_slave_if_map[master_id];
	assert((slave_if_id < CCI_SLAVE_INTERFACE_COUNT) && (slave_if_id >= 0));

	/*
	 * Enable Snoops and DVM messages, no need for Read/Modify/Write as
	 * rest of bits are write ignore
	 */
	mmio_write_32(g_cci_base +
		      SLAVE_IFACE_OFFSET(slave_if_id) +
		      SNOOP_CTRL_REG, DVM_EN_BIT | SNOOP_EN_BIT);

	/* Wait for the dust to settle down */
	while (mmio_read_32(g_cci_base + STATUS_REG) & CHANGE_PENDING_BIT)
		;
}

void cci_disable_snoop_dvm_reqs(unsigned int master_id)
{
	int slave_if_id;

	assert(g_cci_base);
	assert(master_id <= g_max_master_id);

	slave_if_id = g_cci_slave_if_map[master_id];
	assert((slave_if_id < CCI_SLAVE_INTERFACE_COUNT) && (slave_if_id >= 0));

	/*
	 * Disable Snoops and DVM messages, no need for Read/Modify/Write as
	 * rest of bits are write ignore.
	 */
	mmio_write_32(g_cci_base +
		      SLAVE_IFACE_OFFSET(slave_if_id) +
		      SNOOP_CTRL_REG, ~(DVM_EN_BIT | SNOOP_EN_BIT));

	/* Wait for the dust to settle down */
	while (mmio_read_32(g_cci_base + STATUS_REG) & CHANGE_PENDING_BIT)
		;
}

