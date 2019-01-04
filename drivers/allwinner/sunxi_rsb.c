/*
 * Copyright (c) 2017-2018 ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <sunxi_mmap.h>

#define RSB_CTRL	0x00
#define RSB_CCR		0x04
#define RSB_INTE	0x08
#define RSB_STAT	0x0c
#define RSB_DADDR0	0x10
#define RSB_DLEN	0x18
#define RSB_DATA0	0x1c
#define RSB_LCR		0x24
#define RSB_PMCR	0x28
#define RSB_CMD		0x2c
#define RSB_SADDR	0x30

#define RSBCMD_SRTA	0xE8
#define RSBCMD_RD8	0x8B
#define RSBCMD_RD16	0x9C
#define RSBCMD_RD32	0xA6
#define RSBCMD_WR8	0x4E
#define RSBCMD_WR16	0x59
#define RSBCMD_WR32	0x63

#define MAX_TRIES	100000

static int rsb_wait_bit(const char *desc, unsigned int offset, uint32_t mask)
{
	uint32_t reg, tries = MAX_TRIES;

	do
		reg = mmio_read_32(SUNXI_R_RSB_BASE + offset);
	while ((reg & mask) && --tries);	/* transaction in progress */
	if (reg & mask) {
		ERROR("%s: timed out\n", desc);
		return -ETIMEDOUT;
	}

	return 0;
}

static int rsb_wait_stat(const char *desc)
{
	uint32_t reg;
	int ret = rsb_wait_bit(desc, RSB_CTRL, BIT(7));

	if (ret)
		return ret;

	reg = mmio_read_32(SUNXI_R_RSB_BASE + RSB_STAT);
	if (reg == 0x01)
		return 0;

	ERROR("%s: 0x%x\n", desc, reg);
	return -reg;
}

/* Initialize the RSB controller. */
int rsb_init_controller(void)
{
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_CTRL, 0x01); /* soft reset */

	return rsb_wait_bit("RSB: reset controller", RSB_CTRL, BIT(0));
}

int rsb_read(uint8_t rt_addr, uint8_t reg_addr)
{
	int ret;

	mmio_write_32(SUNXI_R_RSB_BASE + RSB_CMD, RSBCMD_RD8); /* read a byte */
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_SADDR, rt_addr << 16);
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_DADDR0, reg_addr);
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_CTRL, 0x80);/* start transaction */

	ret = rsb_wait_stat("RSB: read command");
	if (ret)
		return ret;

	return mmio_read_32(SUNXI_R_RSB_BASE + RSB_DATA0) & 0xff; /* result */
}

int rsb_write(uint8_t rt_addr, uint8_t reg_addr, uint8_t value)
{
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_CMD, RSBCMD_WR8);	/* byte write */
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_SADDR, rt_addr << 16);
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_DADDR0, reg_addr);
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_DATA0, value);
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_CTRL, 0x80);/* start transaction */

	return rsb_wait_stat("RSB: write command");
}

int rsb_set_device_mode(uint32_t device_mode)
{
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_PMCR,
		      (device_mode & 0x00ffffff) | BIT(31));

	return rsb_wait_bit("RSB: set device to RSB", RSB_PMCR, BIT(31));
}

int rsb_set_bus_speed(uint32_t source_freq, uint32_t bus_freq)
{
	uint32_t reg;

	if (bus_freq == 0)
		return -EINVAL;

	reg = source_freq / bus_freq;
	if (reg < 2)
		return -EINVAL;

	reg = reg / 2 - 1;
	reg |= (1U << 8);		/* one cycle of CD output delay */

	mmio_write_32(SUNXI_R_RSB_BASE + RSB_CCR, reg);

	return 0;
}

/* Initialize the RSB PMIC connection. */
int rsb_assign_runtime_address(uint16_t hw_addr, uint8_t rt_addr)
{
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_SADDR, hw_addr | (rt_addr << 16));
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_CMD, RSBCMD_SRTA);
	mmio_write_32(SUNXI_R_RSB_BASE + RSB_CTRL, 0x80);

	return rsb_wait_stat("RSB: set run-time address");
}
