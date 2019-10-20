/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/mentor/mi2cv.h>
#include <lib/mmio.h>

#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define AXP805_ADDR	0x36
#define AXP805_ID	0x03

static enum pmic_type {
	UNKNOWN,
	AXP805,
} pmic;

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

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	int ret;

	sunxi_init_platform_r_twi(SUNXI_SOC_H6, false);
	/* initialise mi2cv driver */
	i2c_init((void *)SUNXI_R_I2C_BASE);

	NOTICE("PMIC: Probing AXP805\n");

	ret = axp805_probe();
	if (ret)
		return ret;

	pmic = AXP805;

	return 0;
}

void __dead2 sunxi_power_down(void)
{
	uint8_t val;

	switch (pmic) {
	case AXP805:
		/* Re-initialise after rich OS might have used it. */
		sunxi_init_platform_r_twi(SUNXI_SOC_H6, false);
		/* initialise mi2cv driver */
		i2c_init((void *)SUNXI_R_I2C_BASE);
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
