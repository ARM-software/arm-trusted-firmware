/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/cadence/cdns_combo_phy.h>
#include <drivers/cadence/cdns_sdmmc.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>

int cdns_sdmmc_write_phy_reg(uint32_t phy_reg_addr, uint32_t phy_reg_addr_value,
			uint32_t phy_reg_data, uint32_t phy_reg_data_value)
{
	uint32_t data = 0U;
	uint32_t value = 0U;

	/* Get PHY register address, write HRS04*/
	value = mmio_read_32(phy_reg_addr);
	value &= ~PHY_REG_ADDR_MASK;
	value |= phy_reg_addr_value;
	mmio_write_32(phy_reg_addr, value);
	data = mmio_read_32(phy_reg_addr);
	if ((data & PHY_REG_ADDR_MASK) != phy_reg_addr_value) {
		ERROR("PHY_REG_ADDR is not set properly\n");
		return -ENXIO;
	}

	/* Get PHY register data, write HRS05 */
	value &= ~PHY_REG_DATA_MASK;
	value |= phy_reg_data_value;
	mmio_write_32(phy_reg_data, value);
	data = mmio_read_32(phy_reg_data);
	if (data != phy_reg_data_value) {
		ERROR("PHY_REG_DATA is not set properly\n");
		return -ENXIO;
	}

	return 0;
}

int cdns_sd_card_detect(void)
{
	uint32_t value = 0;

	/* Card detection */
	do {
		value = mmio_read_32(SDMMC_CDN(SRS09));
	/* Wait for card insertion. SRS09.CI = 1 */
	} while ((value & (1 << SDMMC_CDN_CI)) == 0);

	if ((value & (1 << SDMMC_CDN_CI)) == 0) {
		ERROR("Card does not detect\n");
		return -ENXIO;
	}

	return 0;
}

int cdns_emmc_card_reset(void)
{
	uint32_t _status = 0;

	/* Reset embedded card */
	mmio_write_32(SDMMC_CDN(SRS10), (7 << SDMMC_CDN_BVS) | (1 << SDMMC_CDN_BP) | _status);
	mdelay(68680); /* ~68680us */
	mmio_write_32(SDMMC_CDN(SRS10), (7 << SDMMC_CDN_BVS) | (0 << SDMMC_CDN_BP));
	udelay(340); /* ~340us */

	/* Turn on supply voltage */
	/* BVS = 7, BP = 1, BP2 only in UHS2 mode */
	mmio_write_32(SDMMC_CDN(SRS10), (7 << SDMMC_CDN_BVS) | (1 << SDMMC_CDN_BP) | _status);

	return 0;
}
