/*
 * Copyright (c) 2022-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
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
#include <platform_def.h>
#include "sdmmc/sdmmc.h"

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX72
/* DFI configuration from SDM handoff power-gate bits. */
int dfi_select(handoff *hoff_ptr)
{
	uint32_t reg = 0;
	uint32_t active_dfi_intf = DFI_CTRL_SEL_HPNFC;

	INFO("Power gate enable hand-off: 0x%08x\n", hoff_ptr->peripheral_pwr_gate_array);

	if ((hoff_ptr->peripheral_pwr_gate_array & POWER_GATE_EN_SDEMMC) != 0U) {
		INFO("SDEMMC power gate enabled, DFI selected to NAND\n");
		/*
		 * SDEMMC power gate enabled.
		 * This means SDEMMC controller is disabled and active DFI
		 * interface is selected to NAND via System manager.
		 */
		active_dfi_intf = DFI_CTRL_SEL_HPNFC;
	} else if ((hoff_ptr->peripheral_pwr_gate_array & POWER_GATE_EN_NAND) != 0U) {
		INFO("NAND power gate enabled, DFI selected to SDEMMC\n");
		/*
		 * NAND power gate enabled.
		 * This means NAND controller is disabled and active DFI
		 * interface is selected to SDEMMC via System manager.
		 */
		active_dfi_intf = DFI_CTRL_SEL_SDEMMC;
	} else {
		WARN("Neither SDEMMC nor NAND power gate enabled, by def DFI sel to NAND\n");
	}

	/* Configure the DFI interface select via System manager. */
	mmio_setbits_32(SOCFPGA_SYSMGR(DFI_INTF), active_dfi_intf);

	/* Read back and confirm the same. */
	reg = mmio_read_32(SOCFPGA_SYSMGR(DFI_INTF));
	if ((reg & DFI_INTF_MASK) != active_dfi_intf) {
		ERROR("DFI interface select failed, expected: 0x%08x, got: 0x%08x\n",
		      active_dfi_intf, (reg & DFI_INTF_MASK));
		return -ENXIO;
	}

	NOTICE("DFI interface selected successfully to %s\n",
	       (reg & DFI_INTF_MASK) == DFI_CTRL_SEL_HPNFC ?
	       "NAND" : "SDEMMC");

	return 0;
}
#else
/*
 * Agilex3 still uses the upstream hardcoded SDMMC DFI select. The handoff
 * power-gate path is Agilex72-only until Agilex3 SDM handoff is wired.
 */
static void config_nand(handoff *hoff_ptr)
{
	/* Hardcoded Combo PHY / SD host controller select for Agilex3. */
	hoff_ptr->peripheral_pwr_gate_array = 0x40;
}

int dfi_select(handoff *hoff_ptr)
{
	uint32_t data = 0;
	handoff reverse_handoff_ptr;

	(void)hoff_ptr;

	config_nand(&reverse_handoff_ptr);

	if (((reverse_handoff_ptr.peripheral_pwr_gate_array) &
	     PERIPHERAL_SDMMC_MASK) == 0U) {
		ERROR("SDMMC/NAND is not set properly\n");
		return -ENXIO;
	}

	mmio_setbits_32(SOCFPGA_SYSMGR(DFI_INTF),
			(((reverse_handoff_ptr.peripheral_pwr_gate_array) &
			PERIPHERAL_SDMMC_MASK) >> PERIPHERAL_SDMMC_OFFSET));
	data = mmio_read_32(SOCFPGA_SYSMGR(DFI_INTF));
	if ((data & DFI_INTF_MASK) !=
	    (((reverse_handoff_ptr.peripheral_pwr_gate_array) &
	      PERIPHERAL_SDMMC_MASK) >> PERIPHERAL_SDMMC_OFFSET)) {
		ERROR("DFI is not set properly\n");
		return -ENXIO;
	}

	return 0;
}
#endif

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
