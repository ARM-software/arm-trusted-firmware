/*
 * Copyright (C) 2018-2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#include <mvebu.h>
#include <mvebu_def.h>
#include <plat_marvell.h>

#include "phy-comphy-3700.h"
#include "phy-comphy-common.h"

/*
 * COMPHY_INDIRECT_REG points to ahci address space but the ahci region used in
 * Linux is up to 0x178 so none will access it from Linux in runtime
 * concurrently.
 */
#define COMPHY_INDIRECT_REG	(MVEBU_REGS_BASE + 0xE0178)

/* The USB3_GBE1_PHY range is above USB3 registers used in dts */
#define USB3_GBE1_PHY		(MVEBU_REGS_BASE + 0x5C000)
#define COMPHY_SD_ADDR		(MVEBU_REGS_BASE + 0x1F000)

struct sgmii_phy_init_data_fix {
	uint16_t addr;
	uint16_t value;
};

/* Changes to 40M1G25 mode data required for running 40M3G125 init mode */
static struct sgmii_phy_init_data_fix sgmii_phy_init_fix[] = {
	{0x005, 0x07CC}, {0x015, 0x0000}, {0x01B, 0x0000}, {0x01D, 0x0000},
	{0x01E, 0x0000}, {0x01F, 0x0000}, {0x020, 0x0000}, {0x021, 0x0030},
	{0x026, 0x0888}, {0x04D, 0x0152}, {0x04F, 0xA020}, {0x050, 0x07CC},
	{0x053, 0xE9CA}, {0x055, 0xBD97}, {0x071, 0x3015}, {0x076, 0x03AA},
	{0x07C, 0x0FDF}, {0x0C2, 0x3030}, {0x0C3, 0x8000}, {0x0E2, 0x5550},
	{0x0E3, 0x12A4}, {0x0E4, 0x7D00}, {0x0E6, 0x0C83}, {0x101, 0xFCC0},
	{0x104, 0x0C10}
};

/* 40M1G25 mode init data */
static uint16_t sgmii_phy_init[512] = {
	/* 0       1       2       3       4       5       6       7 */
	/*-----------------------------------------------------------*/
	/* 8       9       A       B       C       D       E       F */
	0x3110, 0xFD83, 0x6430, 0x412F, 0x82C0, 0x06FA, 0x4500, 0x6D26,	/* 00 */
	0xAFC0, 0x8000, 0xC000, 0x0000, 0x2000, 0x49CC, 0x0BC9, 0x2A52,	/* 08 */
	0x0BD2, 0x0CDE, 0x13D2, 0x0CE8, 0x1149, 0x10E0, 0x0000, 0x0000,	/* 10 */
	0x0000, 0x0000, 0x0000, 0x0001, 0x0000, 0x4134, 0x0D2D, 0xFFFF,	/* 18 */
	0xFFE0, 0x4030, 0x1016, 0x0030, 0x0000, 0x0800, 0x0866, 0x0000,	/* 20 */
	0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,	/* 28 */
	0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* 30 */
	0x0000, 0x0000, 0x000F, 0x6A62, 0x1988, 0x3100, 0x3100, 0x3100,	/* 38 */
	0x3100, 0xA708, 0x2430, 0x0830, 0x1030, 0x4610, 0xFF00, 0xFF00,	/* 40 */
	0x0060, 0x1000, 0x0400, 0x0040, 0x00F0, 0x0155, 0x1100, 0xA02A,	/* 48 */
	0x06FA, 0x0080, 0xB008, 0xE3ED, 0x5002, 0xB592, 0x7A80, 0x0001,	/* 50 */
	0x020A, 0x8820, 0x6014, 0x8054, 0xACAA, 0xFC88, 0x2A02, 0x45CF,	/* 58 */
	0x000F, 0x1817, 0x2860, 0x064F, 0x0000, 0x0204, 0x1800, 0x6000,	/* 60 */
	0x810F, 0x4F23, 0x4000, 0x4498, 0x0850, 0x0000, 0x000E, 0x1002,	/* 68 */
	0x9D3A, 0x3009, 0xD066, 0x0491, 0x0001, 0x6AB0, 0x0399, 0x3780,	/* 70 */
	0x0040, 0x5AC0, 0x4A80, 0x0000, 0x01DF, 0x0000, 0x0007, 0x0000,	/* 78 */
	0x2D54, 0x00A1, 0x4000, 0x0100, 0xA20A, 0x0000, 0x0000, 0x0000,	/* 80 */
	0x0000, 0x0000, 0x0000, 0x7400, 0x0E81, 0x1000, 0x1242, 0x0210,	/* 88 */
	0x80DF, 0x0F1F, 0x2F3F, 0x4F5F, 0x6F7F, 0x0F1F, 0x2F3F, 0x4F5F,	/* 90 */
	0x6F7F, 0x4BAD, 0x0000, 0x0000, 0x0800, 0x0000, 0x2400, 0xB651,	/* 98 */
	0xC9E0, 0x4247, 0x0A24, 0x0000, 0xAF19, 0x1004, 0x0000, 0x0000,	/* A0 */
	0x0000, 0x0013, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* A8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/* B0 */
	0x0000, 0x0000, 0x0000, 0x0060, 0x0000, 0x0000, 0x0000, 0x0000,	/* B8 */
	0x0000, 0x0000, 0x3010, 0xFA00, 0x0000, 0x0000, 0x0000, 0x0003,	/* C0 */
	0x1618, 0x8200, 0x8000, 0x0400, 0x050F, 0x0000, 0x0000, 0x0000,	/* C8 */
	0x4C93, 0x0000, 0x1000, 0x1120, 0x0010, 0x1242, 0x1242, 0x1E00,	/* D0 */
	0x0000, 0x0000, 0x0000, 0x00F8, 0x0000, 0x0041, 0x0800, 0x0000,	/* D8 */
	0x82A0, 0x572E, 0x2490, 0x14A9, 0x4E00, 0x0000, 0x0803, 0x0541,	/* E0 */
	0x0C15, 0x0000, 0x0000, 0x0400, 0x2626, 0x0000, 0x0000, 0x4200,	/* E8 */
	0x0000, 0xAA55, 0x1020, 0x0000, 0x0000, 0x5010, 0x0000, 0x0000,	/* F0 */
	0x0000, 0x0000, 0x5000, 0x0000, 0x0000, 0x0000, 0x02F2, 0x0000,	/* F8 */
	0x101F, 0xFDC0, 0x4000, 0x8010, 0x0110, 0x0006, 0x0000, 0x0000,	/*100 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*108 */
	0x04CF, 0x0000, 0x04CF, 0x0000, 0x04CF, 0x0000, 0x04C6, 0x0000,	/*110 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*118 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*120 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*128 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*130 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*138 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*140 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*148 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*150 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*158 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*160 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*168 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*170 */
	0x0000, 0x0000, 0x0000, 0x00F0, 0x08A2, 0x3112, 0x0A14, 0x0000,	/*178 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*180 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*188 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*190 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*198 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1A0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1A8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1B0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1B8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1C0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1C8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1D0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1D8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1E0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1E8 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,	/*1F0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000	/*1F8 */
};

/* PHY selector configures with corresponding modes */
static int mvebu_a3700_comphy_set_phy_selector(uint8_t comphy_index,
						uint32_t comphy_mode)
{
	uint32_t reg;
	int mode = COMPHY_GET_MODE(comphy_mode);

	reg = mmio_read_32(MVEBU_COMPHY_REG_BASE + COMPHY_SELECTOR_PHY_REG);
	switch (mode) {
	case (COMPHY_SATA_MODE):
		/* SATA must be in Lane2 */
		if (comphy_index == COMPHY_LANE2)
			reg &= ~COMPHY_SELECTOR_USB3_PHY_SEL_BIT;
		else
			goto error;
		break;

	case (COMPHY_SGMII_MODE):
	case (COMPHY_2500BASEX_MODE):
		if (comphy_index == COMPHY_LANE0)
			reg &= ~COMPHY_SELECTOR_USB3_GBE1_SEL_BIT;
		else if (comphy_index == COMPHY_LANE1)
			reg &= ~COMPHY_SELECTOR_PCIE_GBE0_SEL_BIT;
		else
			goto error;
		break;

	case (COMPHY_USB3H_MODE):
	case (COMPHY_USB3D_MODE):
	case (COMPHY_USB3_MODE):
		if (comphy_index == COMPHY_LANE2)
			reg |= COMPHY_SELECTOR_USB3_PHY_SEL_BIT;
		else if (comphy_index == COMPHY_LANE0)
			reg |= COMPHY_SELECTOR_USB3_GBE1_SEL_BIT;
		else
			goto error;
		break;

	case (COMPHY_PCIE_MODE):
		/* PCIE must be in Lane1 */
		if (comphy_index == COMPHY_LANE1)
			reg |= COMPHY_SELECTOR_PCIE_GBE0_SEL_BIT;
		else
			goto error;
		break;

	default:
		goto error;
	}

	mmio_write_32(MVEBU_COMPHY_REG_BASE + COMPHY_SELECTOR_PHY_REG, reg);
	return 0;
error:
	ERROR("COMPHY[%d] mode[%d] is invalid\n", comphy_index, mode);
	return -EINVAL;
}

/*
 * This is something like the inverse of the previous function: for given
 * lane it returns COMPHY_*_MODE.
 *
 * It is useful when powering the phy off.
 *
 * This function returns COMPHY_USB3_MODE even if the phy was configured
 * with COMPHY_USB3D_MODE or COMPHY_USB3H_MODE. (The usb3 phy initialization
 * code does not differentiate between these modes.)
 * Also it returns COMPHY_SGMII_MODE even if the phy was configures with
 * COMPHY_2500BASEX_MODE. (The sgmii phy initialization code does differentiate
 * between these modes, but it is irrelevant when powering the phy off.)
 */
static int mvebu_a3700_comphy_get_mode(uint8_t comphy_index)
{
	uint32_t reg;

	reg = mmio_read_32(MVEBU_COMPHY_REG_BASE + COMPHY_SELECTOR_PHY_REG);
	switch (comphy_index) {
	case COMPHY_LANE0:
		if ((reg & COMPHY_SELECTOR_USB3_GBE1_SEL_BIT) != 0)
			return COMPHY_USB3_MODE;
		else
			return COMPHY_SGMII_MODE;
	case COMPHY_LANE1:
		if ((reg & COMPHY_SELECTOR_PCIE_GBE0_SEL_BIT) != 0)
			return COMPHY_PCIE_MODE;
		else
			return COMPHY_SGMII_MODE;
	case COMPHY_LANE2:
		if ((reg & COMPHY_SELECTOR_USB3_PHY_SEL_BIT) != 0)
			return COMPHY_USB3_MODE;
		else
			return COMPHY_SATA_MODE;
	}

	return COMPHY_UNUSED;
}

/* It is only used for SATA and USB3 on comphy lane2. */
static void comphy_set_indirect(uintptr_t addr, uint32_t offset, uint16_t data,
				uint16_t mask, bool is_sata)
{
	/*
	 * When Lane 2 PHY is for USB3, access the PHY registers
	 * through indirect Address and Data registers:
	 * INDIR_ACC_PHY_ADDR (RD00E0178h [31:0]),
	 * INDIR_ACC_PHY_DATA (RD00E017Ch [31:0]),
	 * within the SATA Host Controller registers, Lane 2 base register
	 * offset is 0x200
	 */
	if (is_sata) {
		mmio_write_32(addr + COMPHY_LANE2_INDIR_ADDR_OFFSET, offset);
	} else {
		mmio_write_32(addr + COMPHY_LANE2_INDIR_ADDR_OFFSET,
			      offset + USB3PHY_LANE2_REG_BASE_OFFSET);
	}

	reg_set(addr + COMPHY_LANE2_INDIR_DATA_OFFSET, data, mask);
}

/* It is only used for SATA on comphy lane2. */
static void comphy_sata_set_indirect(uintptr_t addr, uint32_t reg_offset,
				     uint16_t data, uint16_t mask)
{
	comphy_set_indirect(addr, reg_offset, data, mask, true);
}

/* It is only used for USB3 indirect access on comphy lane2. */
static void comphy_usb3_set_indirect(uintptr_t addr, uint32_t reg_offset,
				     uint16_t data, uint16_t mask)
{
	comphy_set_indirect(addr, reg_offset, data, mask, false);
}

/* It is only used for USB3 direct access not on comphy lane2. */
static void comphy_usb3_set_direct(uintptr_t addr, uint32_t reg_offset,
				   uint16_t data, uint16_t mask)
{
	reg_set16((reg_offset * PHY_SHFT(USB3) + addr), data, mask);
}

static void comphy_sgmii_phy_init(uintptr_t sd_ip_addr, bool is_1gbps)
{
	const int fix_arr_sz = ARRAY_SIZE(sgmii_phy_init_fix);
	int addr, fix_idx;
	uint16_t val;

	fix_idx = 0;
	for (addr = 0; addr < 512; addr++) {
		/*
		 * All PHY register values are defined in full for 3.125Gbps
		 * SERDES speed. The values required for 1.25 Gbps are almost
		 * the same and only few registers should be "fixed" in
		 * comparison to 3.125 Gbps values. These register values are
		 * stored in "sgmii_phy_init_fix" array.
		 */
		if (!is_1gbps && sgmii_phy_init_fix[fix_idx].addr == addr) {
			/* Use new value */
			val = sgmii_phy_init_fix[fix_idx].value;
			if (fix_idx < fix_arr_sz)
				fix_idx++;
		} else {
			val = sgmii_phy_init[addr];
		}

		reg_set16(SGMIIPHY_ADDR(addr, sd_ip_addr), val, 0xFFFF);
	}
}

static int mvebu_a3700_comphy_sata_power_on(uint8_t comphy_index,
					    uint32_t comphy_mode)
{
	int ret;
	uint32_t offset, data = 0, ref_clk;
	uintptr_t comphy_indir_regs = COMPHY_INDIRECT_REG;
	int invert = COMPHY_GET_POLARITY_INVERT(comphy_mode);

	debug_enter();

	/* Configure phy selector for SATA */
	ret = mvebu_a3700_comphy_set_phy_selector(comphy_index, comphy_mode);
	if (ret) {
		return ret;
	}

	/* Clear phy isolation mode to make it work in normal mode */
	offset =  COMPHY_ISOLATION_CTRL + SATAPHY_LANE2_REG_BASE_OFFSET;
	comphy_sata_set_indirect(comphy_indir_regs, offset, 0, PHY_ISOLATE_MODE);

	/* 0. Check the Polarity invert bits */
	if (invert & COMPHY_POLARITY_TXD_INVERT)
		data |= TXD_INVERT_BIT;
	if (invert & COMPHY_POLARITY_RXD_INVERT)
		data |= RXD_INVERT_BIT;

	offset = COMPHY_SYNC_PATTERN + SATAPHY_LANE2_REG_BASE_OFFSET;
	comphy_sata_set_indirect(comphy_indir_regs, offset, data, TXD_INVERT_BIT |
				 RXD_INVERT_BIT);

	/* 1. Select 40-bit data width width */
	offset = COMPHY_DIG_LOOPBACK_EN + SATAPHY_LANE2_REG_BASE_OFFSET;
	comphy_sata_set_indirect(comphy_indir_regs, offset, DATA_WIDTH_40BIT,
				 SEL_DATA_WIDTH_MASK);

	/* 2. Select reference clock(25M) and PHY mode (SATA) */
	offset = COMPHY_POWER_PLL_CTRL + SATAPHY_LANE2_REG_BASE_OFFSET;
	if (get_ref_clk() == 40)
		ref_clk = REF_FREF_SEL_SERDES_40MHZ;
	else
		ref_clk = REF_FREF_SEL_SERDES_25MHZ;

	comphy_sata_set_indirect(comphy_indir_regs, offset, ref_clk | PHY_MODE_SATA,
				 REF_FREF_SEL_MASK | PHY_MODE_MASK);

	/* 3. Use maximum PLL rate (no power save) */
	offset = COMPHY_KVCO_CAL_CTRL + SATAPHY_LANE2_REG_BASE_OFFSET;
	comphy_sata_set_indirect(comphy_indir_regs, offset, USE_MAX_PLL_RATE_BIT,
				 USE_MAX_PLL_RATE_BIT);

	/* 4. Reset reserved bit */
	comphy_sata_set_indirect(comphy_indir_regs, COMPHY_RESERVED_REG, 0,
				 PHYCTRL_FRM_PIN_BIT);

	/* 5. Set vendor-specific configuration (It is done in sata driver) */
	/* XXX: in U-Boot below sequence was executed in this place, in Linux
	 * not.  Now it is done only in U-Boot before this comphy
	 * initialization - tests shows that it works ok, but in case of any
	 * future problem it is left for reference.
	 *   reg_set(MVEBU_REGS_BASE + 0xe00a0, 0, 0xffffffff);
	 *   reg_set(MVEBU_REGS_BASE + 0xe00a4, BIT(6), BIT(6));
	 */

	/* Wait for > 55 us to allow PLL be enabled */
	udelay(PLL_SET_DELAY_US);

	/* Polling status */
	mmio_write_32(comphy_indir_regs + COMPHY_LANE2_INDIR_ADDR_OFFSET,
		      COMPHY_DIG_LOOPBACK_EN + SATAPHY_LANE2_REG_BASE_OFFSET);

	ret = polling_with_timeout(comphy_indir_regs +
				   COMPHY_LANE2_INDIR_DATA_OFFSET,
				   PLL_READY_TX_BIT, PLL_READY_TX_BIT,
				   COMPHY_PLL_TIMEOUT, REG_32BIT);
	if (ret) {
		return -ETIMEDOUT;
	}

	debug_exit();

	return 0;
}

static int mvebu_a3700_comphy_sgmii_power_on(uint8_t comphy_index,
					     uint32_t comphy_mode)
{
	int ret;
	uint32_t mask, data;
	uintptr_t offset;
	uintptr_t sd_ip_addr;
	int mode = COMPHY_GET_MODE(comphy_mode);
	int invert = COMPHY_GET_POLARITY_INVERT(comphy_mode);

	debug_enter();

	/* Set selector */
	ret = mvebu_a3700_comphy_set_phy_selector(comphy_index, comphy_mode);
	if (ret) {
		return ret;
	}

	/* Serdes IP Base address
	 * COMPHY Lane0 -- USB3/GBE1
	 * COMPHY Lane1 -- PCIe/GBE0
	 */
	if (comphy_index == COMPHY_LANE0) {
		/* Get usb3 and gbe */
		sd_ip_addr = USB3_GBE1_PHY;
	} else
		sd_ip_addr = COMPHY_SD_ADDR;

	/*
	 * 1. Reset PHY by setting PHY input port PIN_RESET=1.
	 * 2. Set PHY input port PIN_TX_IDLE=1, PIN_PU_IVREF=1 to keep
	 *    PHY TXP/TXN output to idle state during PHY initialization
	 * 3. Set PHY input port PIN_PU_PLL=0, PIN_PU_RX=0, PIN_PU_TX=0.
	 */
	data = PIN_PU_IVREF_BIT | PIN_TX_IDLE_BIT | PIN_RESET_COMPHY_BIT;
	mask = data | PIN_RESET_CORE_BIT | PIN_PU_PLL_BIT | PIN_PU_RX_BIT |
		PIN_PU_TX_BIT;
	offset = MVEBU_COMPHY_REG_BASE + COMPHY_PHY_CFG1_OFFSET(comphy_index);
	reg_set(offset, data, mask);

	/* 4. Release reset to the PHY by setting PIN_RESET=0. */
	data = 0;
	mask = PIN_RESET_COMPHY_BIT;
	reg_set(offset, data, mask);

	/*
	 * 5. Set PIN_PHY_GEN_TX[3:0] and PIN_PHY_GEN_RX[3:0] to decide COMPHY
	 * bit rate
	 */
	if (mode == COMPHY_SGMII_MODE) {
		/* SGMII 1G, SerDes speed 1.25G */
		data |= SD_SPEED_1_25_G << GEN_RX_SEL_OFFSET;
		data |= SD_SPEED_1_25_G << GEN_TX_SEL_OFFSET;
	} else if (mode == COMPHY_2500BASEX_MODE) {
		/* 2500Base-X, SerDes speed 3.125G */
		data |= SD_SPEED_3_125_G << GEN_RX_SEL_OFFSET;
		data |= SD_SPEED_3_125_G << GEN_TX_SEL_OFFSET;
	} else {
		/* Other rates are not supported */
		ERROR("unsupported SGMII speed on comphy lane%d\n",
			comphy_index);
		return -EINVAL;
	}
	mask = GEN_RX_SEL_MASK | GEN_TX_SEL_MASK;
	reg_set(offset, data, mask);

	/*
	 * 6. Wait 10mS for bandgap and reference clocks to stabilize; then
	 * start SW programming.
	 */
	mdelay(10);

	/* 7. Program COMPHY register PHY_MODE */
	data = PHY_MODE_SGMII;
	mask = PHY_MODE_MASK;
	reg_set16(SGMIIPHY_ADDR(COMPHY_POWER_PLL_CTRL, sd_ip_addr), data, mask);

	/*
	 * 8. Set COMPHY register REFCLK_SEL to select the correct REFCLK
	 * source
	 */
	data = 0;
	mask = PHY_REF_CLK_SEL;
	reg_set16(SGMIIPHY_ADDR(COMPHY_MISC_CTRL0, sd_ip_addr), data, mask);

	/*
	 * 9. Set correct reference clock frequency in COMPHY register
	 * REF_FREF_SEL.
	 */
	if (get_ref_clk() == 40)
		data = REF_FREF_SEL_SERDES_50MHZ;
	else
		data = REF_FREF_SEL_SERDES_25MHZ;

	mask = REF_FREF_SEL_MASK;
	reg_set16(SGMIIPHY_ADDR(COMPHY_POWER_PLL_CTRL, sd_ip_addr), data, mask);

	/* 10. Program COMPHY register PHY_GEN_MAX[1:0]
	 * This step is mentioned in the flow received from verification team.
	 * However the PHY_GEN_MAX value is only meaningful for other interfaces
	 * (not SGMII). For instance, it selects SATA speed 1.5/3/6 Gbps or PCIe
	 * speed 2.5/5 Gbps
	 */

	/*
	 * 11. Program COMPHY register SEL_BITS to set correct parallel data
	 * bus width
	 */
	data = DATA_WIDTH_10BIT;
	mask = SEL_DATA_WIDTH_MASK;
	reg_set16(SGMIIPHY_ADDR(COMPHY_DIG_LOOPBACK_EN, sd_ip_addr),
		  data, mask);

	/*
	 * 12. As long as DFE function needs to be enabled in any mode,
	 * COMPHY register DFE_UPDATE_EN[5:0] shall be programmed to 0x3F
	 * for real chip during COMPHY power on.
	 * The step 14 exists (and empty) in the original initialization flow
	 * obtained from the verification team. According to the functional
	 * specification DFE_UPDATE_EN already has the default value 0x3F
	 */

	/*
	 * 13. Program COMPHY GEN registers.
	 * These registers should be programmed based on the lab testing result
	 * to achieve optimal performance. Please contact the CEA group to get
	 * the related GEN table during real chip bring-up. We only required to
	 * run though the entire registers programming flow defined by
	 * "comphy_sgmii_phy_init" when the REF clock is 40 MHz. For REF clock
	 * 25 MHz the default values stored in PHY registers are OK.
	 */
	debug("Running C-DPI phy init %s mode\n",
	      mode == COMPHY_2500BASEX_MODE ? "2G5" : "1G");
	if (get_ref_clk() == 40)
		comphy_sgmii_phy_init(sd_ip_addr, mode != COMPHY_2500BASEX_MODE);

	/*
	 * 14. [Simulation Only] should not be used for real chip.
	 * By pass power up calibration by programming EXT_FORCE_CAL_DONE
	 * (R02h[9]) to 1 to shorten COMPHY simulation time.
	 */

	/*
	 * 15. [Simulation Only: should not be used for real chip]
	 * Program COMPHY register FAST_DFE_TIMER_EN=1 to shorten RX training
	 * simulation time.
	 */

	/*
	 * 16. Check the PHY Polarity invert bit
	 */
	data = 0x0;
	if (invert & COMPHY_POLARITY_TXD_INVERT)
		data |= TXD_INVERT_BIT;
	if (invert & COMPHY_POLARITY_RXD_INVERT)
		data |= RXD_INVERT_BIT;
	mask = TXD_INVERT_BIT | RXD_INVERT_BIT;
	reg_set16(SGMIIPHY_ADDR(COMPHY_SYNC_PATTERN, sd_ip_addr), data, mask);

	/*
	 * 17. Set PHY input ports PIN_PU_PLL, PIN_PU_TX and PIN_PU_RX to 1 to
	 * start PHY power up sequence. All the PHY register programming should
	 * be done before PIN_PU_PLL=1. There should be no register programming
	 * for normal PHY operation from this point.
	 */
	reg_set(MVEBU_COMPHY_REG_BASE + COMPHY_PHY_CFG1_OFFSET(comphy_index),
		PIN_PU_PLL_BIT | PIN_PU_RX_BIT | PIN_PU_TX_BIT,
		PIN_PU_PLL_BIT | PIN_PU_RX_BIT | PIN_PU_TX_BIT);

	/*
	 * 18. Wait for PHY power up sequence to finish by checking output ports
	 * PIN_PLL_READY_TX=1 and PIN_PLL_READY_RX=1.
	 */
	ret = polling_with_timeout(MVEBU_COMPHY_REG_BASE +
				   COMPHY_PHY_STATUS_OFFSET(comphy_index),
				   PHY_PLL_READY_TX_BIT | PHY_PLL_READY_RX_BIT,
				   PHY_PLL_READY_TX_BIT | PHY_PLL_READY_RX_BIT,
				   COMPHY_PLL_TIMEOUT, REG_32BIT);
	if (ret) {
		ERROR("Failed to lock PLL for SGMII PHY %d\n", comphy_index);
		return -ETIMEDOUT;
	}

	/*
	 * 19. Set COMPHY input port PIN_TX_IDLE=0
	 */
	reg_set(MVEBU_COMPHY_REG_BASE + COMPHY_PHY_CFG1_OFFSET(comphy_index),
		0x0, PIN_TX_IDLE_BIT);

	/*
	 * 20. After valid data appear on PIN_RXDATA bus, set PIN_RX_INIT=1. To
	 * start RX initialization. PIN_RX_INIT_DONE will be cleared to 0 by the
	 * PHY After RX initialization is done, PIN_RX_INIT_DONE will be set to
	 * 1 by COMPHY Set PIN_RX_INIT=0 after PIN_RX_INIT_DONE= 1. Please
	 * refer to RX initialization part for details.
	 */
	reg_set(MVEBU_COMPHY_REG_BASE + COMPHY_PHY_CFG1_OFFSET(comphy_index),
		PHY_RX_INIT_BIT, PHY_RX_INIT_BIT);

	ret = polling_with_timeout(MVEBU_COMPHY_REG_BASE +
				   COMPHY_PHY_STATUS_OFFSET(comphy_index),
				   PHY_PLL_READY_TX_BIT | PHY_PLL_READY_RX_BIT,
				   PHY_PLL_READY_TX_BIT | PHY_PLL_READY_RX_BIT,
				   COMPHY_PLL_TIMEOUT, REG_32BIT);
	if (ret) {
		ERROR("Failed to lock PLL for SGMII PHY %d\n", comphy_index);
		return -ETIMEDOUT;
	}

	ret = polling_with_timeout(MVEBU_COMPHY_REG_BASE +
				   COMPHY_PHY_STATUS_OFFSET(comphy_index),
				   PHY_RX_INIT_DONE_BIT, PHY_RX_INIT_DONE_BIT,
				   COMPHY_PLL_TIMEOUT, REG_32BIT);
	if (ret) {
		ERROR("Failed to init RX of SGMII PHY %d\n", comphy_index);
		return -ETIMEDOUT;
	}

	debug_exit();

	return 0;
}

static int mvebu_a3700_comphy_sgmii_power_off(uint8_t comphy_index)
{
	uintptr_t offset;
	uint32_t mask, data;

	debug_enter();

	data = PIN_RESET_CORE_BIT | PIN_RESET_COMPHY_BIT;
	mask = data;
	offset = MVEBU_COMPHY_REG_BASE + COMPHY_PHY_CFG1_OFFSET(comphy_index);
	reg_set(offset, data, mask);

	debug_exit();

	return 0;
}

static int mvebu_a3700_comphy_usb3_power_on(uint8_t comphy_index,
					    uint32_t comphy_mode)
{
	int ret;
	uintptr_t reg_base = 0;
	uintptr_t addr;
	uint32_t mask, data, cfg, ref_clk;
	void (*usb3_reg_set)(uintptr_t addr, uint32_t reg_offset, uint16_t data,
			     uint16_t mask);
	int invert = COMPHY_GET_POLARITY_INVERT(comphy_mode);

	debug_enter();

	/* Set phy seclector */
	ret = mvebu_a3700_comphy_set_phy_selector(comphy_index, comphy_mode);
	if (ret) {
		return ret;
	}

	/* Set usb3 reg access func, Lane2 is indirect access */
	if (comphy_index == COMPHY_LANE2) {
		usb3_reg_set = &comphy_usb3_set_indirect;
		reg_base = COMPHY_INDIRECT_REG;
	} else {
		/* Get the direct access register resource and map */
		usb3_reg_set = &comphy_usb3_set_direct;
		reg_base = USB3_GBE1_PHY;
	}

	/*
	 * 0. Set PHY OTG Control(0x5d034), bit 4, Power up OTG module The
	 * register belong to UTMI module, so it is set in UTMI phy driver.
	 */

	/*
	 * 1. Set PRD_TXDEEMPH (3.5db de-emph)
	 */
	mask = PRD_TXDEEMPH0_MASK | PRD_TXMARGIN_MASK | PRD_TXSWING_MASK |
		CFG_TX_ALIGN_POS_MASK;
	usb3_reg_set(reg_base, COMPHY_LANE_CFG0, PRD_TXDEEMPH0_MASK, mask);

	/*
	 * 2. Set BIT0: enable transmitter in high impedance mode
	 *    Set BIT[3:4]: delay 2 clock cycles for HiZ off latency
	 *    Set BIT6: Tx detect Rx at HiZ mode
	 *    Unset BIT15: set to 0 to set USB3 De-emphasize level to -3.5db
	 *            together with bit 0 of COMPHY_LANE_CFG0 register
	 */
	mask = PRD_TXDEEMPH1_MASK | TX_DET_RX_MODE | GEN2_TX_DATA_DLY_MASK |
		TX_ELEC_IDLE_MODE_EN;
	data = TX_DET_RX_MODE | GEN2_TX_DATA_DLY_DEFT | TX_ELEC_IDLE_MODE_EN;
	usb3_reg_set(reg_base, COMPHY_LANE_CFG1, data, mask);

	/*
	 * 3. Set Spread Spectrum Clock Enabled
	 */
	usb3_reg_set(reg_base, COMPHY_LANE_CFG4,
		     SPREAD_SPECTRUM_CLK_EN, SPREAD_SPECTRUM_CLK_EN);

	/*
	 * 4. Set Override Margining Controls From the MAC:
	 *    Use margining signals from lane configuration
	 */
	usb3_reg_set(reg_base, COMPHY_TEST_MODE_CTRL,
		     MODE_MARGIN_OVERRIDE, REG_16_BIT_MASK);

	/*
	 * 5. Set Lane-to-Lane Bundle Clock Sampling Period = per PCLK cycles
	 *    set Mode Clock Source = PCLK is generated from REFCLK
	 */
	usb3_reg_set(reg_base, COMPHY_CLK_SRC_LO, 0x0,
		     (MODE_CLK_SRC | BUNDLE_PERIOD_SEL |
		      BUNDLE_PERIOD_SCALE_MASK | BUNDLE_SAMPLE_CTRL |
		      PLL_READY_DLY_MASK));

	/*
	 * 6. Set G2 Spread Spectrum Clock Amplitude at 4K
	 */
	usb3_reg_set(reg_base, COMPHY_GEN2_SET2,
		     GS2_TX_SSC_AMP_VALUE_20, GS2_TX_SSC_AMP_MASK);

	/*
	 * 7. Unset G3 Spread Spectrum Clock Amplitude
	 *    set G3 TX and RX Register Master Current Select
	 */
	mask = GS2_TX_SSC_AMP_MASK | GS2_VREG_RXTX_MAS_ISET_MASK |
		GS2_RSVD_6_0_MASK;
	usb3_reg_set(reg_base, COMPHY_GEN3_SET2,
		     GS2_VREG_RXTX_MAS_ISET_60U, mask);

	/*
	 * 8. Check crystal jumper setting and program the Power and PLL Control
	 * accordingly Change RX wait
	 */
	if (get_ref_clk() == 40) {
		ref_clk = REF_FREF_SEL_PCIE_USB3_40MHZ;
		cfg = CFG_PM_RXDLOZ_WAIT_12_UNIT;

	} else {
		/* 25 MHz */
		ref_clk = REF_FREF_SEL_PCIE_USB3_25MHZ;
		cfg = CFG_PM_RXDLOZ_WAIT_7_UNIT;
	}

	mask = PU_IVREF_BIT | PU_PLL_BIT | PU_RX_BIT | PU_TX_BIT |
		PU_TX_INTP_BIT | PU_DFE_BIT | PLL_LOCK_BIT | PHY_MODE_MASK |
		REF_FREF_SEL_MASK;
	data = PU_IVREF_BIT | PU_PLL_BIT | PU_RX_BIT | PU_TX_BIT |
		PU_TX_INTP_BIT | PU_DFE_BIT | PHY_MODE_USB3 | ref_clk;
	usb3_reg_set(reg_base, COMPHY_POWER_PLL_CTRL, data,  mask);

	mask = CFG_PM_OSCCLK_WAIT_MASK | CFG_PM_RXDEN_WAIT_MASK |
		CFG_PM_RXDLOZ_WAIT_MASK;
	data = CFG_PM_RXDEN_WAIT_1_UNIT  | cfg;
	usb3_reg_set(reg_base, COMPHY_PWR_MGM_TIM1, data, mask);

	/*
	 * 9. Enable idle sync
	 */
	data = IDLE_SYNC_EN_DEFAULT_VALUE | IDLE_SYNC_EN;
	usb3_reg_set(reg_base, COMPHY_IDLE_SYNC_EN, data, REG_16_BIT_MASK);

	/*
	 * 10. Enable the output of 500M clock
	 */
	data = MISC_CTRL0_DEFAULT_VALUE | CLK500M_EN;
	usb3_reg_set(reg_base, COMPHY_MISC_CTRL0, data, REG_16_BIT_MASK);

	/*
	 * 11. Set 20-bit data width
	 */
	usb3_reg_set(reg_base, COMPHY_DIG_LOOPBACK_EN, DATA_WIDTH_20BIT,
		     REG_16_BIT_MASK);

	/*
	 * 12. Override Speed_PLL value and use MAC PLL
	 */
	usb3_reg_set(reg_base, COMPHY_KVCO_CAL_CTRL,
		     (SPEED_PLL_VALUE_16 | USE_MAX_PLL_RATE_BIT),
		     REG_16_BIT_MASK);

	/*
	 * 13. Check the Polarity invert bit
	 */
	data = 0U;
	if (invert & COMPHY_POLARITY_TXD_INVERT) {
		data |= TXD_INVERT_BIT;
	}
	if (invert & COMPHY_POLARITY_RXD_INVERT) {
		data |= RXD_INVERT_BIT;
	}
	mask = TXD_INVERT_BIT | RXD_INVERT_BIT;
	usb3_reg_set(reg_base, COMPHY_SYNC_PATTERN, data, mask);

	/*
	 * 14. Set max speed generation to USB3.0 5Gbps
	 */
	usb3_reg_set(reg_base, COMPHY_SYNC_MASK_GEN, PHY_GEN_MAX_USB3_5G,
		     PHY_GEN_MAX_MASK);

	/*
	 * 15. Set capacitor value for FFE gain peaking to 0xF
	 */
	usb3_reg_set(reg_base, COMPHY_GEN2_SET3,
		     GS3_FFE_CAP_SEL_VALUE, GS3_FFE_CAP_SEL_MASK);

	/*
	 * 16. Release SW reset
	 */
	data = MODE_CORE_CLK_FREQ_SEL | MODE_PIPE_WIDTH_32 | MODE_REFDIV_BY_4;
	usb3_reg_set(reg_base, COMPHY_RST_CLK_CTRL, data, REG_16_BIT_MASK);

	/* Wait for > 55 us to allow PCLK be enabled */
	udelay(PLL_SET_DELAY_US);

	if (comphy_index == COMPHY_LANE2) {
		data = COMPHY_LANE_STAT1 + USB3PHY_LANE2_REG_BASE_OFFSET;
		mmio_write_32(reg_base + COMPHY_LANE2_INDIR_ADDR_OFFSET,
			      data);

		addr = reg_base + COMPHY_LANE2_INDIR_DATA_OFFSET;
		ret = polling_with_timeout(addr, TXDCLK_PCLK_EN, TXDCLK_PCLK_EN,
					   COMPHY_PLL_TIMEOUT, REG_32BIT);
	} else {
		ret = polling_with_timeout(LANE_STAT1_ADDR(USB3) + reg_base,
					   TXDCLK_PCLK_EN, TXDCLK_PCLK_EN,
					   COMPHY_PLL_TIMEOUT, REG_16BIT);
	}
	if (ret) {
		ERROR("Failed to lock USB3 PLL\n");
		return -ETIMEDOUT;
	}

	debug_exit();

	return 0;
}

static int mvebu_a3700_comphy_pcie_power_on(uint8_t comphy_index,
					    uint32_t comphy_mode)
{
	int ret;
	uint32_t ref_clk;
	uint32_t mask, data;
	int invert = COMPHY_GET_POLARITY_INVERT(comphy_mode);

	debug_enter();

	/* Configure phy selector for PCIe */
	ret = mvebu_a3700_comphy_set_phy_selector(comphy_index, comphy_mode);
	if (ret) {
		return ret;
	}

	/* 1. Enable max PLL. */
	reg_set16(LANE_CFG1_ADDR(PCIE) + COMPHY_SD_ADDR,
		  USE_MAX_PLL_RATE_EN, USE_MAX_PLL_RATE_EN);

	/* 2. Select 20 bit SERDES interface. */
	reg_set16(CLK_SRC_LO_ADDR(PCIE) + COMPHY_SD_ADDR,
		  CFG_SEL_20B, CFG_SEL_20B);

	/* 3. Force to use reg setting for PCIe mode */
	reg_set16(MISC_CTRL1_ADDR(PCIE) + COMPHY_SD_ADDR,
		  SEL_BITS_PCIE_FORCE, SEL_BITS_PCIE_FORCE);

	/* 4. Change RX wait */
	reg_set16(PWR_MGM_TIM1_ADDR(PCIE) + COMPHY_SD_ADDR,
		  CFG_PM_RXDEN_WAIT_1_UNIT | CFG_PM_RXDLOZ_WAIT_12_UNIT,
		  (CFG_PM_OSCCLK_WAIT_MASK | CFG_PM_RXDEN_WAIT_MASK |
		   CFG_PM_RXDLOZ_WAIT_MASK));

	/* 5. Enable idle sync */
	reg_set16(IDLE_SYNC_EN_ADDR(PCIE) + COMPHY_SD_ADDR,
		  IDLE_SYNC_EN_DEFAULT_VALUE | IDLE_SYNC_EN, REG_16_BIT_MASK);

	/* 6. Enable the output of 100M/125M/500M clock */
	reg_set16(MISC_CTRL0_ADDR(PCIE) + COMPHY_SD_ADDR,
		  MISC_CTRL0_DEFAULT_VALUE | CLK500M_EN | TXDCLK_2X_SEL | CLK100M_125M_EN,
		  REG_16_BIT_MASK);

	/*
	 * 7. Enable TX, PCIE global register, 0xd0074814, it is done in
	 * PCI-E driver
	 */

	/*
	 * 8. Check crystal jumper setting and program the Power and PLL
	 * Control accordingly
	 */

	if (get_ref_clk() == 40)
		ref_clk = REF_FREF_SEL_PCIE_USB3_40MHZ;
	else
		ref_clk = REF_FREF_SEL_PCIE_USB3_25MHZ;

	reg_set16(PWR_PLL_CTRL_ADDR(PCIE) + COMPHY_SD_ADDR,
		  (PU_IVREF_BIT | PU_PLL_BIT | PU_RX_BIT | PU_TX_BIT |
		   PU_TX_INTP_BIT | PU_DFE_BIT | ref_clk | PHY_MODE_PCIE),
		  REG_16_BIT_MASK);

	/* 9. Override Speed_PLL value and use MAC PLL */
	reg_set16(KVCO_CAL_CTRL_ADDR(PCIE) + COMPHY_SD_ADDR,
		  SPEED_PLL_VALUE_16 | USE_MAX_PLL_RATE_BIT, REG_16_BIT_MASK);

	/* 10. Check the Polarity invert bit */
	data = 0U;
	if (invert & COMPHY_POLARITY_TXD_INVERT) {
		data |= TXD_INVERT_BIT;
	}
	if (invert & COMPHY_POLARITY_RXD_INVERT) {
		data |= RXD_INVERT_BIT;
	}
	mask = TXD_INVERT_BIT | RXD_INVERT_BIT;
	reg_set16(SYNC_PATTERN_ADDR(PCIE) + COMPHY_SD_ADDR, data, mask);

	/* 11. Release SW reset */
	data = MODE_CORE_CLK_FREQ_SEL | MODE_PIPE_WIDTH_32;
	mask = data | SOFT_RESET | MODE_REFDIV_MASK;
	reg_set16(RST_CLK_CTRL_ADDR(PCIE) + COMPHY_SD_ADDR, data, mask);

	/* Wait for > 55 us to allow PCLK be enabled */
	udelay(PLL_SET_DELAY_US);

	ret = polling_with_timeout(LANE_STAT1_ADDR(PCIE) + COMPHY_SD_ADDR,
				   TXDCLK_PCLK_EN, TXDCLK_PCLK_EN,
				   COMPHY_PLL_TIMEOUT, REG_16BIT);
	if (ret) {
		ERROR("Failed to lock PCIE PLL\n");
		return -ETIMEDOUT;
	}

	debug_exit();

	return 0;
}

int mvebu_3700_comphy_power_on(uint8_t comphy_index, uint32_t comphy_mode)
{
	int mode = COMPHY_GET_MODE(comphy_mode);
	int ret = 0;

	debug_enter();

	switch (mode) {
	case(COMPHY_SATA_MODE):
		ret = mvebu_a3700_comphy_sata_power_on(comphy_index,
						       comphy_mode);
		break;
	case(COMPHY_SGMII_MODE):
	case(COMPHY_2500BASEX_MODE):
		ret = mvebu_a3700_comphy_sgmii_power_on(comphy_index,
							comphy_mode);
		break;
	case (COMPHY_USB3_MODE):
	case (COMPHY_USB3H_MODE):
		ret = mvebu_a3700_comphy_usb3_power_on(comphy_index,
						       comphy_mode);
		break;
	case (COMPHY_PCIE_MODE):
		ret = mvebu_a3700_comphy_pcie_power_on(comphy_index,
						       comphy_mode);
		break;
	default:
		ERROR("comphy%d: unsupported comphy mode\n", comphy_index);
		ret = -EINVAL;
		break;
	}

	debug_exit();

	return ret;
}

static int mvebu_a3700_comphy_usb3_power_off(void)
{
	/*
	 * Currently the USB3 MAC will control the USB3 PHY to set it to low
	 * state, thus do not need to power off USB3 PHY again.
	 */
	debug_enter();
	debug_exit();

	return 0;
}

static int mvebu_a3700_comphy_sata_power_off(void)
{
	uintptr_t comphy_indir_regs = COMPHY_INDIRECT_REG;
	uint32_t offset;

	debug_enter();

	/* Set phy isolation mode */
	offset = COMPHY_ISOLATION_CTRL + SATAPHY_LANE2_REG_BASE_OFFSET;
	comphy_sata_set_indirect(comphy_indir_regs, offset, PHY_ISOLATE_MODE,
				 PHY_ISOLATE_MODE);

	/* Power off PLL, Tx, Rx */
	offset = COMPHY_POWER_PLL_CTRL + SATAPHY_LANE2_REG_BASE_OFFSET;
	comphy_sata_set_indirect(comphy_indir_regs, offset, 0,
				 PU_PLL_BIT | PU_RX_BIT | PU_TX_BIT);

	debug_exit();

	return 0;
}

int mvebu_3700_comphy_power_off(uint8_t comphy_index, uint32_t comphy_mode)
{
	int mode = COMPHY_GET_MODE(comphy_mode);
	int err = 0;

	debug_enter();

	if (!mode) {
		/*
		 * The user did not specify which mode should be powered off.
		 * In this case we can identify this by reading the phy selector
		 * register.
		 */
		mode = mvebu_a3700_comphy_get_mode(comphy_index);
	}

	switch (mode) {
	case(COMPHY_SGMII_MODE):
	case(COMPHY_2500BASEX_MODE):
		err = mvebu_a3700_comphy_sgmii_power_off(comphy_index);
		break;
	case (COMPHY_USB3_MODE):
	case (COMPHY_USB3H_MODE):
		err = mvebu_a3700_comphy_usb3_power_off();
		break;
	case (COMPHY_SATA_MODE):
		err = mvebu_a3700_comphy_sata_power_off();
		break;

	default:
		debug("comphy%d: power off is not implemented for mode %d\n",
		      comphy_index, mode);
		break;
	}

	debug_exit();

	return err;
}

static int mvebu_a3700_comphy_sata_is_pll_locked(void)
{
	uint32_t data, addr;
	uintptr_t comphy_indir_regs = COMPHY_INDIRECT_REG;
	int ret = 0;

	debug_enter();

	/* Polling status */
	mmio_write_32(comphy_indir_regs + COMPHY_LANE2_INDIR_ADDR_OFFSET,
	       COMPHY_DIG_LOOPBACK_EN + SATAPHY_LANE2_REG_BASE_OFFSET);
	addr = comphy_indir_regs + COMPHY_LANE2_INDIR_DATA_OFFSET;
	data = polling_with_timeout(addr, PLL_READY_TX_BIT, PLL_READY_TX_BIT,
				    COMPHY_PLL_TIMEOUT, REG_32BIT);

	if (data != 0) {
		ERROR("TX PLL is not locked\n");
		ret = -ETIMEDOUT;
	}

	debug_exit();

	return ret;
}

int mvebu_3700_comphy_is_pll_locked(uint8_t comphy_index, uint32_t comphy_mode)
{
	int mode = COMPHY_GET_MODE(comphy_mode);
	int ret = 0;

	debug_enter();

	switch (mode) {
	case(COMPHY_SATA_MODE):
		ret = mvebu_a3700_comphy_sata_is_pll_locked();
		break;

	default:
		ERROR("comphy[%d] mode[%d] doesn't support PLL lock check\n",
			comphy_index, mode);
		ret = -EINVAL;
		break;
	}

	debug_exit();

	return ret;
}
