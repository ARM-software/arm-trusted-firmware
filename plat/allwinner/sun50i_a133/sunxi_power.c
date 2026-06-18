/*
 * Copyright (c) 2020-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <libfdt.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/allwinner/axp.h>
#include <drivers/mentor/mi2cv.h>
#include <lib/mmio.h>

#include <sunxi_cpucfg.h>
#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

static uint16_t pmic_bus_addr;

static enum pmic_type {
	UNKNOWN,
	AXP717,
	AXP803,
} pmic;

int axp_read(uint8_t reg)
{
	uint8_t val = 0;
	int ret;

	ret = i2c_write(pmic_bus_addr, 0, 0, &reg, 1);
	if (ret == 0) {
		ret = i2c_read(pmic_bus_addr, 0, 0, &val, 1);
	}
	if (ret) {
		ERROR("PMIC: Cannot read PMIC register %02x\n", reg);
		return ret;
	}

	return val;
}

int axp_write(uint8_t reg, uint8_t val)
{
	int ret;

	ret = i2c_write(pmic_bus_addr, reg, 1, &val, 1);
	if (ret) {
		ERROR("PMIC: Cannot write PMIC register %02x\n", reg);
	}

	return ret;
}

static int pmic_bus_init(uint16_t socid)
{
	int ret;

	ret = sunxi_init_platform_r_twi(socid, false);
	if (ret) {
		INFO("Could not init platform bus: %d\n", ret);
		pmic = UNKNOWN;
		return ret;
	}

	/* initialise mi2cv driver */
	i2c_init((void *)SUNXI_R_I2C_BASE);

	return 0;
}

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	int node, ret;
	uint32_t reg;

	if (!fdt) {
		INFO("No DTB, skipping PMIC detection and setup\n");
		return -ENOENT;
	}

	node = fdt_node_offset_by_compatible(fdt, 0, "x-powers,axp717");
	if (node >= 0) {
		pmic = AXP717;
	}

	if (pmic == UNKNOWN) {
		node = fdt_node_offset_by_compatible(fdt, 0, "x-powers,axp803");
		if (node >= 0) {
			pmic = AXP803;
		}
	}

	if (pmic == UNKNOWN) {
		INFO("PMIC: No known PMIC in DT, skipping setup.\n");
		return -ENODEV;
	}

	if (fdt_read_uint32(fdt, node, "reg", &reg)) {
		ERROR("PMIC: PMIC DT node does not contain reg property.\n");
		return -EINVAL;
	}

	pmic_bus_addr = reg;
	INFO("Probing for PMIC on %s\n", "I2C");

	ret = pmic_bus_init(socid);
	if (ret) {
		return ret;
	}

	ret = axp_read(0x03);
	switch (ret & 0xcf) {
	case 0xcf: /* version reg not implemented on AXP717 */
		if (pmic == AXP717) {
			INFO("PMIC: found AXP717\n");
			/* no regulators to set up, U-Boot takes care of this */
		} else {
			pmic = UNKNOWN;
		}
		break;
	case 0x41:
		if (pmic == AXP803) {
			INFO("PMIC: found AXP803\n");
		} else {
			pmic = UNKNOWN;
		}
	}

	if (pmic == UNKNOWN) {
		INFO("Incompatible or unknown PMIC found.\n");
		return -ENODEV;
	}

	return 0;
}

void sunxi_power_down(void)
{
	int ret;

	if (pmic == UNKNOWN) {
		return;
	}

	/* Re-initialise after rich OS might have used it. */
	ret = pmic_bus_init(SUNXI_SOC_A133);
	if (ret) {
		return;
	}

	switch (pmic) {
	case AXP717:
		axp_setbits(0x27, BIT(0));
		break;
	case AXP803:
		axp_setbits(0x32, BIT(7));
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
