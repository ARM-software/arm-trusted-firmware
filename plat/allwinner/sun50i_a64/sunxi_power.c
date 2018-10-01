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
#include <platform_def.h>
#include <sunxi_def.h>
#include <sunxi_mmap.h>

enum pmic_type {
	GENERIC_H5,
	GENERIC_A64,
} pmic;

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

int sunxi_pmic_setup(uint16_t socid)
{
	switch (socid) {
	case SUNXI_SOC_H5:
		pmic = GENERIC_H5;
		break;
	case SUNXI_SOC_A64:
		pmic = GENERIC_A64;
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
	default:
		break;
	}

	udelay(1000);
	ERROR("PSCI: Cannot turn off system, halting.\n");
	wfi();
	panic();
}
