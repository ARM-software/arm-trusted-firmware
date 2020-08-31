/*
 * Copyright (c) 2020, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <spmi_arb.h>

#define REG_APID_MAP(apid)	(0x0C440900U + 4U * i)
#define NUM_APID		0x80

#define PPID_MASK		(0xfffU << 8)

#define REG_ARB_CMD(apid)	(0x0C600000U + 0x10000U * apid)
/* These are opcodes specific to this SPMI arbitrator, *not* SPMI commands. */
#define OPC_EXT_WRITEL		0
#define OPC_EXT_READL		1

#define REG_ARB_STATUS(apid)	(0x0C600008U + 0x10000U * apid)
#define ARB_STATUS_DONE		BIT(0)
#define ARB_STATUS_FAILURE	BIT(1)
#define ARB_STATUS_DENIED	BIT(2)
#define ARB_STATUS_DROPPED	BIT(3)

/* Fake status to report driver errors. */
#define ARB_FAKE_STATUS_TIMEOUT	BIT(8)

#define REG_ARB_RDATA0(apid)	(0x0C600018U + 0x10000U * apid)
#define REG_ARB_WDATA0(apid)	(0x0C600010U + 0x10000U * apid)

static int addr_to_apid(uint32_t addr)
{
	unsigned int i;

	for (i = 0U; i < NUM_APID; i++) {
		uint32_t reg = mmio_read_32(REG_APID_MAP(i));
		if ((reg != 0U) && ((addr & PPID_MASK) == (reg & PPID_MASK))) {
			return i;
		}
	}

	return -1;
}

static int wait_for_done(uint16_t apid)
{
	unsigned int timeout = 100;

	while (timeout-- != 0U) {
		uint32_t status = mmio_read_32(REG_ARB_STATUS(apid));
		if ((status & ARB_STATUS_DONE) != 0U) {
			if ((status & ARB_STATUS_FAILURE) != 0U ||
			    (status & ARB_STATUS_DENIED) != 0U ||
			    (status & ARB_STATUS_DROPPED) != 0U) {
				return status & 0xff;
			}
			return 0;
		}
		mdelay(1);
	}
	ERROR("SPMI_ARB timeout!\n");
	return ARB_FAKE_STATUS_TIMEOUT;
}

static void arb_command(uint16_t apid, uint8_t opcode, uint32_t addr,
			uint8_t bytes)
{
	mmio_write_32(REG_ARB_CMD(apid), (uint32_t)opcode << 27 |
					 (addr & 0xff) << 4 | (bytes - 1));
}

int spmi_arb_read8(uint32_t addr)
{
	int apid = addr_to_apid(addr);

	if (apid < 0) {
		return apid;
	}

	arb_command(apid, OPC_EXT_READL, addr, 1);

	int ret = wait_for_done(apid);
	if (ret != 0) {
		ERROR("SPMI_ARB read error [0x%x]: 0x%x\n", addr, ret);
		return ret;
	}

	return mmio_read_32(REG_ARB_RDATA0(apid)) & 0xff;
}

int spmi_arb_write8(uint32_t addr, uint8_t data)
{
	int apid = addr_to_apid(addr);

	if (apid < 0) {
		return apid;
	}

	mmio_write_32(REG_ARB_WDATA0(apid), data);
	arb_command(apid, OPC_EXT_WRITEL, addr, 1);

	int ret = wait_for_done(apid);
	if (ret != 0) {
		ERROR("SPMI_ARB write error [0x%x] = 0x%x: 0x%x\n",
		      addr, data, ret);
	}

	return ret;
}
