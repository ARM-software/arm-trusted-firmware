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

#include <core_off_arisc.h>
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

	/* Switch to the recommended 3 MHz bus clock. */
	ret = rsb_set_bus_speed(SUNXI_OSC24M_CLK_IN_HZ, 3000000);
	if (ret)
		return ret;

	/* Initiate an I2C transaction to switch the PMIC to RSB mode. */
	ret = rsb_set_device_mode(AXP20X_MODE_RSB << 16 | AXP20X_MODE_REG << 8);
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

		/* Switch the PMIC back to I2C mode. */
		ret = axp_write(AXP20X_MODE_REG, AXP20X_MODE_I2C);
		if (ret)
			return ret;

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

/* This lock synchronises access to the arisc management processor. */
static DEFINE_BAKERY_LOCK(arisc_lock);

/*
 * If we are supposed to turn ourself off, tell the arisc SCP to do that
 * work for us. Without any SCPI provider running there, we place some
 * OpenRISC code into SRAM, put the address of that into the reset vector
 * and release the arisc reset line. The SCP will wait for the core to enter
 * WFI, then execute that code and pull the line up again.
 * The code expects the core mask to be patched into the first instruction.
 */
void sunxi_cpu_power_off_self(void)
{
	u_register_t mpidr = read_mpidr();
	unsigned int core  = MPIDR_AFFLVL0_VAL(mpidr);
	uintptr_t arisc_reset_vec = SUNXI_SRAM_A2_BASE + 0x100;
	uint32_t *code = arisc_core_off;

	do {
		bakery_lock_get(&arisc_lock);
		/* Wait until the arisc is in reset state. */
		if (!(mmio_read_32(SUNXI_R_CPUCFG_BASE) & BIT(0)))
			break;

		bakery_lock_release(&arisc_lock);
	} while (1);

	/* Patch up the code to feed in an input parameter. */
	code[0] = (code[0] & ~0xffff) | BIT_32(core);
	clean_dcache_range((uintptr_t)code, sizeof(arisc_core_off));

	/*
	 * The OpenRISC unconditional branch has opcode 0, the branch offset
	 * is in the lower 26 bits, containing the distance to the target,
	 * in instruction granularity (32 bits).
	 */
	mmio_write_32(arisc_reset_vec, ((uintptr_t)code - arisc_reset_vec) / 4);
	clean_dcache_range(arisc_reset_vec, 4);

	/* De-assert the arisc reset line to let it run. */
	mmio_setbits_32(SUNXI_R_CPUCFG_BASE, BIT(0));

	/*
	 * We release the lock here, although the arisc is still busy.
	 * But as long as it runs, the reset line is high, so other users
	 * won't leave the loop above.
	 * Once it has finished, the code is supposed to clear the reset line,
	 * to signal this to other users.
	 */
	bakery_lock_release(&arisc_lock);
}
