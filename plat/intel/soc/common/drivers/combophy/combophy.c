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
#include <drivers/cadence/cdns_sdmmc.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>

#include "combophy.h"
#include "sdmmc/sdmmc.h"

/* Temp assigned handoff data, need to remove when SDM up and run. */
void config_nand(handoff *hoff_ptr)
{
	/* This is hardcoded input value for Combo PHY and SD host controller. */
	hoff_ptr->peripheral_pwr_gate_array = 0x40;

}

/* DFI configuration */
int dfi_select(handoff *hoff_ptr)
{
	uint32_t data = 0;

	/* Temp assigned handoff data, need to remove when SDM up and run. */
	handoff reverse_handoff_ptr;

	/* Temp assigned handoff data, need to remove when SDM up and run. */
	config_nand(&reverse_handoff_ptr);

	if (((reverse_handoff_ptr.peripheral_pwr_gate_array) & PERIPHERAL_SDMMC_MASK) == 0U) {
		ERROR("SDMMC/NAND is not set properly\n");
		return -ENXIO;
	}

	mmio_setbits_32(SOCFPGA_SYSMGR(DFI_INTF),
		(((reverse_handoff_ptr.peripheral_pwr_gate_array) &
		PERIPHERAL_SDMMC_MASK) >> PERIPHERAL_SDMMC_OFFSET));
	data = mmio_read_32(SOCFPGA_SYSMGR(DFI_INTF));
	if ((data & DFI_INTF_MASK) != (((reverse_handoff_ptr.peripheral_pwr_gate_array) &
		PERIPHERAL_SDMMC_MASK) >> PERIPHERAL_SDMMC_OFFSET)) {
		ERROR("DFI is not set properly\n");
		return -ENXIO;
	}

	return 0;
}

int combo_phy_init(handoff *hoff_ptr)
{
	/* SDMMC/NAND DFI selection based on system manager DFI register */
	int ret = dfi_select(hoff_ptr);

	if (ret != 0U) {
		ERROR("DFI configuration failed\n");
		return ret;
	}

	return 0;
}
