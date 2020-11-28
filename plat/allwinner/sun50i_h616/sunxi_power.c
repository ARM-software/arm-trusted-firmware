/*
 * Copyright (c) 2017-2020, ARM Limited. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/allwinner/axp.h>
#include <drivers/allwinner/sunxi_rsb.h>
#include <lib/mmio.h>

#include <sunxi_cpucfg.h>
#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

#define AXP305_I2C_ADDR	0x36
#define AXP305_HW_ADDR	0x745
#define AXP305_RT_ADDR	0x3a

static enum pmic_type {
	UNKNOWN,
	AXP305,
} pmic;

int axp_read(uint8_t reg)
{
	return rsb_read(AXP305_RT_ADDR, reg);
}

int axp_write(uint8_t reg, uint8_t val)
{
	return rsb_write(AXP305_RT_ADDR, reg, val);
}

static int rsb_init(void)
{
	int ret;

	ret = rsb_init_controller();
	if (ret)
		return ret;

	/* Switch to the recommended 3 MHz bus clock. */
	ret = rsb_set_bus_speed(SUNXI_OSC24M_CLK_IN_HZ, 3000000);
	if (ret)
		return ret;

	/* Initiate an I2C transaction to switch the PMIC to RSB mode. */
	ret = rsb_set_device_mode(AXP20X_MODE_RSB << 16 | AXP20X_MODE_REG << 8);
	if (ret)
		return ret;

	/* Associate the 8-bit runtime address with the 12-bit bus address. */
	ret = rsb_assign_runtime_address(AXP305_HW_ADDR, AXP305_RT_ADDR);
	if (ret)
		return ret;

	return axp_check_id();
}

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	int ret;

	INFO("PMIC: Probing AXP305 on RSB\n");

	ret = sunxi_init_platform_r_twi(socid, true);
	if (ret) {
		INFO("Could not init platform bus: %d\n", ret);
		return ret;
	}

	ret = rsb_init();
	if (ret) {
		INFO("Could not init RSB: %d\n", ret);
		return ret;
	}

	pmic = AXP305;
	axp_setup_regulators(fdt);

	/* Switch the PMIC back to I2C mode. */
	ret = axp_write(AXP20X_MODE_REG, AXP20X_MODE_I2C);
	if (ret)
		return ret;

	return 0;
}

void sunxi_power_down(void)
{
	switch (pmic) {
	case AXP305:
		/* Re-initialise after rich OS might have used it. */
		sunxi_init_platform_r_twi(SUNXI_SOC_H616, true);
		rsb_init();
		axp_power_off();
		break;
	default:
		break;
	}
}

void sunxi_cpu_power_off_self(void)
{
	u_register_t mpidr = read_mpidr();
	unsigned int core  = MPIDR_AFFLVL0_VAL(mpidr);

	/* Enable the CPUIDLE hardware (only really needs to be done once). */
	mmio_write_32(SUNXI_CPUIDLE_EN_REG, 0x16aa0000);
	mmio_write_32(SUNXI_CPUIDLE_EN_REG, 0xaa160001);

	/* Trigger power off for this core. */
	mmio_write_32(SUNXI_CORE_CLOSE_REG, BIT_32(core));
}
