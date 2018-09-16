/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <allwinner/sunxi_rsb.h>
#include <arch_helpers.h>
#include <debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <libfdt.h>
#include <mmio.h>
#include <platform_def.h>
#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_helpers.h>

enum pmic_type {
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
 * disabled. This leaves core 0 running, it's the caller's responsibility
 * to turn that off (or go to WFI).
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

	/** turn off PLLs **/
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

	ret = rsb_set_bus_speed(SUNXI_OSC24M_CLK_IN_HZ, 400000);
	if (ret)
		return ret;

	ret = rsb_set_device_mode(0x7c3e00);
	if (ret)
		return ret;

	ret = rsb_set_bus_speed(SUNXI_OSC24M_CLK_IN_HZ, 3000000);
	if (ret)
		return ret;

	return rsb_assign_runtime_address(AXP803_HW_ADDR,
					  AXP803_RT_ADDR);
}

static int axp_setbits_8(uint8_t reg, uint8_t set_mask)
{
	uint8_t regval;
	int ret;

	ret = rsb_read(AXP803_RT_ADDR, reg);
	if (ret < 0)
		return ret;

	regval = ret | set_mask;

	return rsb_write(AXP803_RT_ADDR, reg, regval);
}

static bool fdt_handle_regulator(const void *fdt, int node)
{
	if (fdt_getprop(fdt, node, "phandle", NULL) != NULL)
		return true;
	if (fdt_getprop(fdt, node, "regulator-always-on", NULL) != NULL)
		return true;
	return false;
}

static void setup_axp803_rails(const void *fdt)
{
	int node;

	/* locate the PMIC DT node, bail out if not found */
	node = fdt_node_offset_by_compatible(fdt, -1, "x-powers,axp803");
	if (node == -FDT_ERR_NOTFOUND) {
		WARN("BL31: PMIC: No AXP803 DT node, skipping initial setup.\n");
		return;
	}

	if (fdt_getprop(fdt, node, "x-powers,drive-vbus-en", NULL))
		axp_setbits_8(0x8f, BIT(4));

	/* descend into the "regulators" subnode */
	node = fdt_first_subnode(fdt, node);

	/* iterate over all regulators to find used ones */
	for (node = fdt_first_subnode(fdt, node);
	     node != -FDT_ERR_NOTFOUND;
	     node = fdt_next_subnode(fdt, node)) {
		const char *name;
		int length;

		name = fdt_get_name(fdt, node, &length);
		if (!strncmp(name, "dc1sw", length)) {
			if (!fdt_handle_regulator(fdt, node))
				continue;

			INFO("PMIC: AXP803: Enabling DC1SW\n");
			axp_setbits_8(0x12, BIT(7));
			continue;
		}
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
		ret = platform_init_r_twi(socid, false);
		if (ret)
			return ret;

		ret = rsb_init();
		if (ret)
			return ret;

		pmic = AXP803_RSB;
		NOTICE("BL31: PMIC: Detected AXP803 on RSB.\n");

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

		/* turn off pin controller now. */
		mmio_write_32(SUNXI_CCU_BASE + 0x68, 0);

		break;
	case AXP803_RSB:
		/* (Re-)init RSB in case the rich OS has disabled it. */
		platform_init_r_twi(SUNXI_SOC_A64, false);
		rsb_init();

		/* Set "power disable control" bit */
		axp_setbits_8(0x32, BIT(7));
		break;
	default:
		break;
	}

	udelay(1000);
	ERROR("PSCI: Cannot turn off system, halting.\n");
	wfi();
	panic();
}
