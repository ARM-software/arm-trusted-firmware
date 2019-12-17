/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/allwinner/axp.h>
#include <drivers/allwinner/sunxi_rsb.h>
#include <lib/mmio.h>

#include <sunxi_def.h>
#include <sunxi_mmap.h>
#include <sunxi_private.h>

static enum pmic_type {
	UNKNOWN,
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
static void sunxi_turn_off_soc(uint16_t socid)
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
	ret = rsb_assign_runtime_address(AXP803_HW_ADDR,
					 AXP803_RT_ADDR);
	if (ret)
		return ret;

	return axp_check_id();
}

int axp_read(uint8_t reg)
{
	return rsb_read(AXP803_RT_ADDR, reg);
}

int axp_write(uint8_t reg, uint8_t val)
{
	return rsb_write(AXP803_RT_ADDR, reg, val);
}

int sunxi_pmic_setup(uint16_t socid, const void *fdt)
{
	int ret;

	switch (socid) {
	case SUNXI_SOC_H5:
		NOTICE("PMIC: Assuming H5 reference regulator design\n");

		pmic = REF_DESIGN_H5;

		break;
	case SUNXI_SOC_A64:
		pmic = GENERIC_A64;

		INFO("PMIC: Probing AXP803 on RSB\n");

		ret = sunxi_init_platform_r_twi(socid, true);
		if (ret)
			return ret;

		ret = rsb_init();
		if (ret)
			return ret;

		pmic = AXP803_RSB;
		axp_setup_regulators(fdt);

		break;
	default:
		return -ENODEV;
	}
	return 0;
}

void sunxi_power_down(void)
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
		axp_power_off();
		break;
	default:
		break;
	}

}
