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

#define MAKE_CCI_PART_NUMBER(hi, lo)	((hi << 8) | lo)
#define CCI_PART_LO_MASK		0xff
#define CCI_PART_HI_MASK		0xf

/* CCI part number codes read from Peripheral ID registers 0 and 1 */
#define CCI400_PART_NUM		0x420
#define CCI500_PART_NUM		0x422
#define CCI550_PART_NUM		0x423

#define CCI400_SLAVE_PORTS	5
#define CCI500_SLAVE_PORTS	7
#define CCI550_SLAVE_PORTS	7

static uintptr_t cci_base;
static const int *cci_slave_if_map;

#if ENABLE_ASSERTIONS
static unsigned int max_master_id;
static int cci_num_slave_ports;

static int validate_cci_map(const int *map)
{
	unsigned int valid_cci_map = 0;
	int slave_if_id;
	int i;

	/* Validate the map */
	for (i = 0; i <= max_master_id; i++) {
		slave_if_id = map[i];

		if (slave_if_id < 0)
			continue;

		if (slave_if_id >= cci_num_slave_ports) {
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

/*
 * Read CCI part number from Peripheral ID registers
 */
static unsigned int read_cci_part_number(uintptr_t base)
{
	unsigned int part_lo, part_hi;

	part_lo = mmio_read_32(base + PERIPHERAL_ID0) & CCI_PART_LO_MASK;
	part_hi = mmio_read_32(base + PERIPHERAL_ID1) & CCI_PART_HI_MASK;

	return MAKE_CCI_PART_NUMBER(part_hi, part_lo);
}

/*
 * Identify a CCI device, and return the number of slaves. Return -1 for an
 * unidentified device.
 */
static int get_slave_ports(unsigned int part_num)
{
	/* Macro to match CCI products */
#define RET_ON_MATCH(product) \
	case CCI ## product ## _PART_NUM: \
		return CCI ## product ## _SLAVE_PORTS

	switch (part_num) {

	RET_ON_MATCH(400);
	RET_ON_MATCH(500);
	RET_ON_MATCH(550);

	default:
		return -1;
	}

#undef RET_ON_MATCH
}
#endif /* ENABLE_ASSERTIONS */

void cci_init(uintptr_t base, const int *map, unsigned int num_cci_masters)
{
	assert(map);
	assert(base);

	cci_base = base;
	cci_slave_if_map = map;

#if ENABLE_ASSERTIONS
	/*
	 * Master Id's are assigned from zero, So in an array of size n
	 * the max master id is (n - 1).
	 */
	max_master_id = num_cci_masters - 1;
	cci_num_slave_ports = get_slave_ports(read_cci_part_number(base));
#endif
	assert(cci_num_slave_ports >= 0);

	assert(validate_cci_map(map));
}

void cci_enable_snoop_dvm_reqs(unsigned int master_id)
{
	int slave_if_id = cci_slave_if_map[master_id];

	assert(master_id <= max_master_id);
	assert((slave_if_id < cci_num_slave_ports) && (slave_if_id >= 0));
	assert(cci_base);

	/*
	 * Enable Snoops and DVM messages, no need for Read/Modify/Write as
	 * rest of bits are write ignore
	 */
	mmio_write_32(cci_base +
		      SLAVE_IFACE_OFFSET(slave_if_id) + SNOOP_CTRL_REG,
		      DVM_EN_BIT | SNOOP_EN_BIT);

	/* Wait for the dust to settle down */
	while (mmio_read_32(cci_base + STATUS_REG) & CHANGE_PENDING_BIT)
		;
}

void cci_disable_snoop_dvm_reqs(unsigned int master_id)
{
	int slave_if_id = cci_slave_if_map[master_id];

	assert(master_id <= max_master_id);
	assert((slave_if_id < cci_num_slave_ports) && (slave_if_id >= 0));
	assert(cci_base);

	/*
	 * Disable Snoops and DVM messages, no need for Read/Modify/Write as
	 * rest of bits are write ignore.
	 */
	mmio_write_32(cci_base +
		      SLAVE_IFACE_OFFSET(slave_if_id) + SNOOP_CTRL_REG,
		      ~(DVM_EN_BIT | SNOOP_EN_BIT));

	/* Wait for the dust to settle down */
	while (mmio_read_32(cci_base + STATUS_REG) & CHANGE_PENDING_BIT)
		;
}

