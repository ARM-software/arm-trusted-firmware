/*
 * Copyright (c) 2017-2025, Arm Limited. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/allwinner/axp.h>
#include <drivers/allwinner/sunxi_rsb.h>
#include <drivers/mentor/mi2cv.h>
#include <lib/mmio.h>
#include <libfdt.h>

#include <sunxi_cpucfg.h>
#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

static uint16_t pmic_bus_addr;
static uint8_t rsb_rt_addr;

static bool is_using_rsb(void)
{
	return rsb_rt_addr != 0;
}

static enum pmic_type {
	UNKNOWN,
	AXP305,
	AXP313,
	AXP717,
} pmic;

static uint8_t get_rsb_rt_address(uint16_t hw_addr)
{
	switch (hw_addr) {
	case 0x3a3: return 0x2d;
	case 0x745: return 0x3a;
	}

	return 0;
}

int axp_read(uint8_t reg)
{
	uint8_t val = 0;
	int ret;

	if (is_using_rsb()) {
		return rsb_read(rsb_rt_addr, reg);
	}

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

	if (is_using_rsb()) {
		return rsb_write(rsb_rt_addr, reg, val);
	}

	ret = i2c_write(pmic_bus_addr, reg, 1, &val, 1);
	if (ret) {
		ERROR("PMIC: Cannot write PMIC register %02x\n", reg);
	}

	return ret;
}

static int rsb_init(int rsb_hw_addr)
{
	int ret;

	ret = rsb_init_controller();
	if (ret) {
		return ret;
	}

	/* Switch to the recommended 3 MHz bus clock. */
	ret = rsb_set_bus_speed(SUNXI_OSC24M_CLK_IN_HZ, 3000000);
	if (ret) {
		return ret;
	}

	/* Initiate an I2C transaction to switch the PMIC to RSB mode. */
	ret = rsb_set_device_mode(AXP20X_MODE_RSB << 16 | AXP20X_MODE_REG << 8);
	if (ret) {
		return ret;
	}

	/* Associate the 8-bit runtime address with the 12-bit bus address. */
	ret = rsb_assign_runtime_address(rsb_hw_addr, rsb_rt_addr);
	if (ret) {
		return ret;
	}

	return 0;
}

static int pmic_bus_init(uint16_t socid, uint16_t rsb_hw_addr)
{
	int ret;

	ret = sunxi_init_platform_r_twi(socid, is_using_rsb());
	if (ret) {
		INFO("Could not init platform bus: %d\n", ret);
		pmic = UNKNOWN;
		return ret;
	}

	if (is_using_rsb()) {
		ret = rsb_init(rsb_hw_addr);
		if (ret) {
			pmic = UNKNOWN;
			return ret;
		}
	} else {
		/* initialise mi2cv driver */
		i2c_init((void *)SUNXI_R_I2C_BASE);
	}

	return 0;
}

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	int node, parent, ret;
	uint32_t reg;

	if (fdt == NULL) {
		INFO("No DTB, skipping PMIC detection and setup\n");
		return -ENOENT;
	}

	node = fdt_node_offset_by_compatible(fdt, 0, "x-powers,axp806");
	if (node >= 0) {
		pmic = AXP305;
	}

	if (pmic == UNKNOWN) {
		node = fdt_node_offset_by_compatible(fdt, 0, "x-powers,axp313a");
		if (node >= 0) {
			pmic = AXP313;
		}
	}

	if (pmic == UNKNOWN) {
		node = fdt_node_offset_by_compatible(fdt, 0, "x-powers,axp717");
		if (node >= 0) {
			pmic = AXP717;
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
	parent = fdt_parent_offset(fdt, node);
	ret = fdt_node_check_compatible(fdt, parent, "allwinner,sun8i-a23-rsb");
	if (ret == 0) {
		rsb_rt_addr = get_rsb_rt_address(pmic_bus_addr);
		if (rsb_rt_addr == 0) {
			ERROR("PMIC: no mapping for RSB address 0x%x\n",
			      pmic_bus_addr);
			return -EINVAL;
		}
	}

	INFO("Probing for PMIC on %s:\n", is_using_rsb() ? "RSB" : "I2C");

	ret = pmic_bus_init(socid, pmic_bus_addr);
	if (ret) {
		return ret;
	}

	ret = axp_read(0x03);
	switch (ret & 0xcf) {
	case 0x40:				/* AXP305 */
		if (pmic == AXP305) {
			INFO("PMIC: found AXP305, setting up regulators\n");
			axp_setup_regulators(fdt);
		} else {
			pmic = UNKNOWN;
		}
		break;
	case 0x48:				/* AXP1530 */
	case 0x4b:				/* AXP313A */
	case 0x4c:				/* AXP313B */
		if (pmic == AXP313) {
			INFO("PMIC: found AXP313\n");
			/* no regulators to set up */
		} else {
			pmic = UNKNOWN;
		}
		break;
	case 0xcf:		/* version reg not implemented on AXP717 */
		if (pmic == AXP717) {
			INFO("PMIC: found AXP717\n");
			/* no regulators to set up, U-Boot takes care of this */
		} else {
			pmic = UNKNOWN;
		}
		break;
	}

	if (is_using_rsb()) {
		/* Switch the PMIC back to I2C mode. */
		return rsb_write(rsb_rt_addr, AXP20X_MODE_REG, AXP20X_MODE_I2C);
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
	ret = pmic_bus_init(SUNXI_SOC_H616, pmic_bus_addr);
	if (ret) {
		return;
	}

	switch (pmic) {
	case AXP305:
		axp_setbits(0x32, BIT(7));
		break;
	case AXP313:
		axp_setbits(0x1a, BIT(7));
		break;
	case AXP717:
		axp_setbits(0x27, BIT(0));
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
