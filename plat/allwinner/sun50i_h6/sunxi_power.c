/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <mentor/mi2cv.h>
#include <string.h>
#include <sunxi_mmap.h>

#define AXP805_ADDR	0x36
#define AXP805_ID	0x03

enum pmic_type {
	NO_PMIC,
	AXP805,
};

enum pmic_type pmic;

static int sunxi_init_r_i2c(void)
{
	uint32_t reg;

	/* switch pins PL0 and PL1 to I2C */
	reg = mmio_read_32(SUNXI_R_PIO_BASE + 0x00);
	mmio_write_32(SUNXI_R_PIO_BASE + 0x00, (reg & ~0xff) | 0x33);

	/* level 2 drive strength */
	reg = mmio_read_32(SUNXI_R_PIO_BASE + 0x14);
	mmio_write_32(SUNXI_R_PIO_BASE + 0x14, (reg & ~0x0f) | 0xa);

	/* set both ports to pull-up */
	reg = mmio_read_32(SUNXI_R_PIO_BASE + 0x1c);
	mmio_write_32(SUNXI_R_PIO_BASE + 0x1c, (reg & ~0x0f) | 0x5);

	/* assert & de-assert reset of R_I2C */
	reg = mmio_read_32(SUNXI_R_PRCM_BASE + 0x19c);
	mmio_write_32(SUNXI_R_PRCM_BASE + 0x19c, reg & ~BIT(16));
	mmio_write_32(SUNXI_R_PRCM_BASE + 0x19c, reg | BIT(16));

	/* un-gate R_I2C clock */
	mmio_write_32(SUNXI_R_PRCM_BASE + 0x19c, reg | BIT(16) | BIT(0));

	/* call mi2cv driver */
	i2c_init((void *)SUNXI_R_I2C_BASE);

	return 0;
}

int axp_i2c_read(uint8_t chip, uint8_t reg, uint8_t *val)
{
	int ret;

	ret = i2c_write(chip, 0, 0, &reg, 1);
	if (ret)
		return ret;

	return i2c_read(chip, 0, 0, val, 1);
}

int axp_i2c_write(uint8_t chip, uint8_t reg, uint8_t val)
{
	return i2c_write(chip, reg, 1, &val, 1);
}

static int axp805_probe(void)
{
	int ret;
	uint8_t val;

	ret = axp_i2c_write(AXP805_ADDR, 0xff, 0x0);
	if (ret) {
		ERROR("PMIC: Cannot put AXP805 to master mode.\n");
		return -EPERM;
	}

	ret = axp_i2c_read(AXP805_ADDR, AXP805_ID, &val);

	if (!ret && ((val & 0xcf) == 0x40))
		NOTICE("PMIC: AXP805 detected\n");
	else if (ret) {
		ERROR("PMIC: Cannot communicate with AXP805.\n");
		return -EPERM;
	} else {
		ERROR("PMIC: Non-AXP805 chip attached at AXP805's address.\n");
		return -EINVAL;
	}

	return 0;
}

int sunxi_pmic_setup(void)
{
	int ret;

	sunxi_init_r_i2c();

	NOTICE("PMIC: Probing AXP805\n");
	pmic = AXP805;

	ret = axp805_probe();
	if (ret)
		pmic = NO_PMIC;
	else
		pmic = AXP805;

	return 0;
}

void __dead2 sunxi_power_down(void)
{
	uint8_t val;

	switch (pmic) {
	case AXP805:
		sunxi_init_r_i2c();
		axp_i2c_read(AXP805_ADDR, 0x32, &val);
		axp_i2c_write(AXP805_ADDR, 0x32, val | 0x80);
		break;
	default:
		break;
	}

	udelay(1000);
	ERROR("PSCI: Cannot communicate with PMIC, halting\n");
	wfi();
	panic();
}
