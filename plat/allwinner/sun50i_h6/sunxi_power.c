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
#include <drivers/allwinner/axp.h>
#include <drivers/delay_timer.h>
#include <drivers/mentor/mi2cv.h>
#include <lib/mmio.h>

#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define AXP805_ADDR	0x36

static enum pmic_type {
	UNKNOWN,
	AXP805,
} pmic;

int axp_read(uint8_t reg)
{
	uint8_t val;
	int ret;

	ret = i2c_write(AXP805_ADDR, 0, 0, &reg, 1);
	if (ret == 0)
		ret = i2c_read(AXP805_ADDR, 0, 0, &val, 1);
	if (ret) {
		ERROR("PMIC: Cannot read AXP805 register %02x\n", reg);
		return ret;
	}

	return val;
}

int axp_write(uint8_t reg, uint8_t val)
{
	int ret;

	ret = i2c_write(AXP805_ADDR, reg, 1, &val, 1);
	if (ret)
		ERROR("PMIC: Cannot write AXP805 register %02x\n", reg);

	return ret;
}

static int axp805_probe(void)
{
	int ret;

	/* Switch the AXP805 to master/single-PMIC mode. */
	ret = axp_write(0xff, 0x0);
	if (ret)
		return ret;

	ret = axp_check_id();
	if (ret)
		return ret;

	return 0;
}

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	int ret;

	INFO("PMIC: Probing AXP805 on I2C\n");

	ret = sunxi_init_platform_r_twi(SUNXI_SOC_H6, false);
	if (ret)
		return ret;

	/* initialise mi2cv driver */
	i2c_init((void *)SUNXI_R_I2C_BASE);

	ret = axp805_probe();
	if (ret)
		return ret;

	pmic = AXP805;
	axp_setup_regulators(fdt);

	return 0;
}

void sunxi_power_down(void)
{
	switch (pmic) {
	case AXP805:
		/* Re-initialise after rich OS might have used it. */
		sunxi_init_platform_r_twi(SUNXI_SOC_H6, false);
		/* initialise mi2cv driver */
		i2c_init((void *)SUNXI_R_I2C_BASE);
		axp_power_off();
		break;
	default:
		break;
	}
}
