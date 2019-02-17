/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <libfdt.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/allwinner/sunxi_rsb.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

static enum pmic_type {
	GENERIC_H5,
	GENERIC_A64,
	REF_DESIGN_H5,	/* regulators controlled by GPIO pins on port L */
	AXP803_RSB,	/* PMIC connected via RSB on most A64 boards */
} pmic;

#define AXP803_HW_ADDR	0x3a3
#define AXP803_RT_ADDR	0x2d

/*
 * On boards without a proper PMIC we struggle to turn off the system properly.
 * Try to turn off as much off the system as we can, to reduce power
 * consumption. This should be entered with only one core running and SMP
 * disabled.
 * This function only cares about peripherals.
 */
void sunxi_turn_off_soc(uint16_t socid)
{
	int i;

	/** Turn off most peripherals, most importantly DRAM users. **/
	/* Keep DRAM controller running for now. */
	mmio_clrbits_32(SUNXI_CCU_BASE + 0x2c0, ~BIT_32(14));
	mmio_clrbits_32(SUNXI_CCU_BASE + 0x60, ~BIT_32(14));
	/* Contains msgbox (bit 21) and spinlock (bit 22) */
	mmio_write_32(SUNXI_CCU_BASE + 0x2c4, 0);
	mmio_write_32(SUNXI_CCU_BASE + 0x64, 0);
	mmio_write_32(SUNXI_CCU_BASE + 0x2c8, 0);
	/* Keep PIO controller running for now. */
	mmio_clrbits_32(SUNXI_CCU_BASE + 0x68, ~(BIT_32(5)));
	mmio_write_32(SUNXI_CCU_BASE + 0x2d0, 0);
	/* Contains UART0 (bit 16) */
	mmio_write_32(SUNXI_CCU_BASE + 0x2d8, 0);
	mmio_write_32(SUNXI_CCU_BASE + 0x6c, 0);
	mmio_write_32(SUNXI_CCU_BASE + 0x70, 0);

	/** Turn off DRAM controller. **/
	mmio_clrbits_32(SUNXI_CCU_BASE + 0x2c0, BIT_32(14));
	mmio_clrbits_32(SUNXI_CCU_BASE + 0x60, BIT_32(14));

	/** Migrate CPU and bus clocks away from the PLLs. **/
	/* AHB1: use OSC24M/1, APB1 = AHB1 / 2 */
	mmio_write_32(SUNXI_CCU_BASE + 0x54, 0x1000);
	/* APB2: use OSC24M */
	mmio_write_32(SUNXI_CCU_BASE + 0x58, 0x1000000);
	/* AHB2: use AHB1 clock */
	mmio_write_32(SUNXI_CCU_BASE + 0x5c, 0);
	/* CPU: use OSC24M */
	mmio_write_32(SUNXI_CCU_BASE + 0x50, 0x10000);

	/** Turn off PLLs. **/
	for (i = 0; i < 6; i++)
		mmio_clrbits_32(SUNXI_CCU_BASE + i * 8, BIT(31));
	switch (socid) {
	case SUNXI_SOC_H5:
		mmio_clrbits_32(SUNXI_CCU_BASE + 0x44, BIT(31));
		break;
	case SUNXI_SOC_A64:
		mmio_clrbits_32(SUNXI_CCU_BASE + 0x2c, BIT(31));
		mmio_clrbits_32(SUNXI_CCU_BASE + 0x4c, BIT(31));
		break;
	}
}

static int rsb_init(void)
{
	int ret;

	ret = rsb_init_controller();
	if (ret)
		return ret;

	/* Start with 400 KHz to issue the I2C->RSB switch command. */
	ret = rsb_set_bus_speed(SUNXI_OSC24M_CLK_IN_HZ, 400000);
	if (ret)
		return ret;

	/*
	 * Initiate an I2C transaction to write 0x7c into register 0x3e,
	 * switching the PMIC to RSB mode.
	 */
	ret = rsb_set_device_mode(0x7c3e00);
	if (ret)
		return ret;

	/* Now in RSB mode, switch to the recommended 3 MHz. */
	ret = rsb_set_bus_speed(SUNXI_OSC24M_CLK_IN_HZ, 3000000);
	if (ret)
		return ret;

	/* Associate the 8-bit runtime address with the 12-bit bus address. */
	return rsb_assign_runtime_address(AXP803_HW_ADDR,
					  AXP803_RT_ADDR);
}

static int axp_write(uint8_t reg, uint8_t val)
{
	return rsb_write(AXP803_RT_ADDR, reg, val);
}

static int axp_clrsetbits(uint8_t reg, uint8_t clr_mask, uint8_t set_mask)
{
	uint8_t regval;
	int ret;

	ret = rsb_read(AXP803_RT_ADDR, reg);
	if (ret < 0)
		return ret;

	regval = (ret & ~clr_mask) | set_mask;

	return rsb_write(AXP803_RT_ADDR, reg, regval);
}

#define axp_clrbits(reg, clr_mask) axp_clrsetbits(reg, clr_mask, 0)
#define axp_setbits(reg, set_mask) axp_clrsetbits(reg, 0, set_mask)

static bool should_enable_regulator(const void *fdt, int node)
{
	if (fdt_getprop(fdt, node, "phandle", NULL) != NULL)
		return true;
	if (fdt_getprop(fdt, node, "regulator-always-on", NULL) != NULL)
		return true;
	return false;
}

/*
 * Retrieve the voltage from a given regulator DTB node.
 * Both the regulator-{min,max}-microvolt properties must be present and
 * have the same value. Return that value in millivolts.
 */
static int fdt_get_regulator_millivolt(const void *fdt, int node)
{
	const fdt32_t *prop;
	uint32_t min_volt;

	prop = fdt_getprop(fdt, node, "regulator-min-microvolt", NULL);
	if (prop == NULL)
		return -EINVAL;
	min_volt = fdt32_to_cpu(*prop);

	prop = fdt_getprop(fdt, node, "regulator-max-microvolt", NULL);
	if (prop == NULL)
		return -EINVAL;

	if (fdt32_to_cpu(*prop) != min_volt)
		return -EINVAL;

	return min_volt / 1000;
}

#define NO_SPLIT 0xff

static const struct axp_regulator {
	char *dt_name;
	uint16_t min_volt;
	uint16_t max_volt;
	uint16_t step;
	unsigned char split;
	unsigned char volt_reg;
	unsigned char switch_reg;
	unsigned char switch_bit;
} regulators[] = {
	{"dcdc1", 1600, 3400, 100, NO_SPLIT, 0x20, 0x10, 0},
	{"dcdc5",  800, 1840,  10,       32, 0x24, 0x10, 4},
	{"dcdc6",  600, 1520,  10,       50, 0x25, 0x10, 5},
	{"dldo1",  700, 3300, 100, NO_SPLIT, 0x15, 0x12, 3},
	{"dldo2",  700, 4200, 100,       27, 0x16, 0x12, 4},
	{"dldo3",  700, 3300, 100, NO_SPLIT, 0x17, 0x12, 5},
	{"fldo1",  700, 1450,  50, NO_SPLIT, 0x1c, 0x13, 2},
	{}
};

static int setup_regulator(const void *fdt, int node,
			   const struct axp_regulator *reg)
{
	int mvolt;
	uint8_t regval;

	if (!should_enable_regulator(fdt, node))
		return -ENOENT;

	mvolt = fdt_get_regulator_millivolt(fdt, node);
	if (mvolt < reg->min_volt || mvolt > reg->max_volt)
		return -EINVAL;

	regval = (mvolt / reg->step) - (reg->min_volt / reg->step);
	if (regval > reg->split)
		regval = ((regval - reg->split) / 2) + reg->split;

	axp_write(reg->volt_reg, regval);
	if (reg->switch_reg < 0xff)
		axp_setbits(reg->switch_reg, BIT(reg->switch_bit));

	INFO("PMIC: AXP803: %s voltage: %d.%03dV\n", reg->dt_name,
	     mvolt / 1000, mvolt % 1000);

	return 0;
}

static void setup_axp803_rails(const void *fdt)
{
	int node;
	bool dc1sw = false;

	/* locate the PMIC DT node, bail out if not found */
	node = fdt_node_offset_by_compatible(fdt, -1, "x-powers,axp803");
	if (node < 0) {
		WARN("BL31: PMIC: Cannot find AXP803 DT node, skipping initial setup.\n");
		return;
	}

	if (fdt_getprop(fdt, node, "x-powers,drive-vbus-en", NULL)) {
		axp_clrbits(0x8f, BIT(4));
		axp_setbits(0x30, BIT(2));
		INFO("PMIC: AXP803: Enabling DRIVEVBUS\n");
	}

	/* descend into the "regulators" subnode */
	node = fdt_subnode_offset(fdt, node, "regulators");
	if (node < 0) {
		WARN("BL31: PMIC: Cannot find regulators subnode, skipping initial setup.\n");
		return;
	}

	/* iterate over all regulators to find used ones */
	for (node = fdt_first_subnode(fdt, node);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		const struct axp_regulator *reg;
		const char *name;
		int length;

		/* We only care if it's always on or referenced. */
		if (!should_enable_regulator(fdt, node))
			continue;

		name = fdt_get_name(fdt, node, &length);
		for (reg = regulators; reg->dt_name; reg++) {
			if (!strncmp(name, reg->dt_name, length)) {
				setup_regulator(fdt, node, reg);
				break;
			}
		}

		if (!strncmp(name, "dc1sw", length)) {
			/* Delay DC1SW enablement to avoid overheating. */
			dc1sw = true;
			continue;
		}
	}
	/*
	 * If DLDO2 is enabled after DC1SW, the PMIC overheats and shuts
	 * down. So always enable DC1SW as the very last regulator.
	 */
	if (dc1sw) {
		INFO("PMIC: AXP803: Enabling DC1SW\n");
		axp_setbits(0x12, BIT(7));
	}
}

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	int ret;

	switch (socid) {
	case SUNXI_SOC_H5:
		pmic = REF_DESIGN_H5;
		NOTICE("BL31: PMIC: Defaulting to PortL GPIO according to H5 reference design.\n");
		break;
	case SUNXI_SOC_A64:
		pmic = GENERIC_A64;
		ret = sunxi_init_platform_r_twi(socid, true);
		if (ret)
			return ret;

		ret = rsb_init();
		if (ret)
			return ret;

		pmic = AXP803_RSB;
		NOTICE("BL31: PMIC: Detected AXP803 on RSB.\n");

		if (fdt)
			setup_axp803_rails(fdt);

		break;
	default:
		NOTICE("BL31: PMIC: No support for Allwinner %x SoC.\n", socid);
		return -ENODEV;
	}
	return 0;
}

void __dead2 sunxi_power_down(void)
{
	switch (pmic) {
	case GENERIC_H5:
		/* Turn off as many peripherals and clocks as we can. */
		sunxi_turn_off_soc(SUNXI_SOC_H5);
		/* Turn off the pin controller now. */
		mmio_write_32(SUNXI_CCU_BASE + 0x68, 0);
		break;
	case GENERIC_A64:
		/* Turn off as many peripherals and clocks as we can. */
		sunxi_turn_off_soc(SUNXI_SOC_A64);
		/* Turn off the pin controller now. */
		mmio_write_32(SUNXI_CCU_BASE + 0x68, 0);
		break;
	case REF_DESIGN_H5:
		sunxi_turn_off_soc(SUNXI_SOC_H5);

		/*
		 * Switch PL pins to power off the board:
		 * - PL5 (VCC_IO) -> high
		 * - PL8 (PWR-STB = CPU power supply) -> low
		 * - PL9 (PWR-DRAM) ->low
		 * - PL10 (power LED) -> low
		 * Note: Clearing PL8 will reset the board, so keep it up.
		 */
		sunxi_set_gpio_out('L', 5, 1);
		sunxi_set_gpio_out('L', 9, 0);
		sunxi_set_gpio_out('L', 10, 0);

		/* Turn off pin controller now. */
		mmio_write_32(SUNXI_CCU_BASE + 0x68, 0);

		break;
	case AXP803_RSB:
		/* (Re-)init RSB in case the rich OS has disabled it. */
		sunxi_init_platform_r_twi(SUNXI_SOC_A64, true);
		rsb_init();

		/* Set "power disable control" bit */
		axp_setbits(0x32, BIT(7));
		break;
	default:
		break;
	}

	udelay(1000);
	ERROR("PSCI: Cannot turn off system, halting.\n");
	wfi();
	panic();
}
